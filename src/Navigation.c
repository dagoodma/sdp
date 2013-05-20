/*
 * File:   Navigation.c
 * Author: David     Goodman
 *
 * TODO: Consider adding error correction timeout.
 * TODO: Consider adding
 *
 * Created on March 3, 2013, 10:27 AM
 */
//#define DEBUG
#define USE_SD_LOGGER
#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <stdbool.h>
//#define __XC32
#include <math.h>
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "GPS.h"
#include "Navigation.h"
#include "Drive.h"
#include "Logger.h"
#include "Error.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define USE_DRIVE

#define UPDATE_DELAY        1500 // (ms)
#define TIMEOUT_DELAY       7000 // (ms)

// don't change heading unless calculated is this much away from last
#define HEADING_TOLERANCE   10 // (deg)

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/


static enum {
    STATE_IDLE  = 0x0,   // Initializing and obtaining instructions
    STATE_NAVIGATE,      // Maintaining station coordinates
    STATE_NAVIGATE_WAIT, // Waiting for GPS lock/connection to come back
    STATE_ERROR,         // Error occured (GPS lost lock or disconnected)
} state;

static LocalCoordinate nedDestination;
static float destinationTolerance = 0.0, lastHeading = 0.0;

static GeocentricCoordinate ecefError, ecefOrigin;
static GeodeticCoordinate llaOrigin;
static bool isDone = FALSE;
static bool hasOrigin = FALSE;
static bool hasErrorCorrection = FALSE;
static bool useErrorCorrection = FALSE;

static error_t lastErrorCode = ERROR_NONE;


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
static void startNavigateState();
static void startNavigateWaitState();
static void startErrorState();
static void startIdleState();
static void applyGeocentricErrorCorrection(GeocentricCoordinate *ecefPos);
static void updateHeading();
static uint8_t distanceToSpeed(float dist);
static void getLocalPosition(LocalCoordinate *nedVar);
static void setError(error_t errorCode);
static error_t findNavigationError();


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**********************************************************************
 * Function: Navigation_init
 * @return TRUE or FALSE whether initialization succeeded.
 * @remark Initializes the navigation state machine.
 **********************************************************************/
bool Navigation_init() {
    startIdleState();
    lastErrorCode = ERROR_NONE;
    Timer_new(TIMER_NAVIGATION, UPDATE_DELAY);
    return SUCCESS;
}


/**********************************************************************
 * Function: Navigation_runSM
 * @return None
 * @remark Steps through the navigation state machine by one cycle.
 **********************************************************************/
void Navigation_runSM() {
    switch (state) {
        case STATE_IDLE:
            // Do Nothing
            break;
        case STATE_NAVIGATE:
            if (!Navigation_isReady()) {
                startNavigateWaitState();
                break;
            }
            if (Timer_isExpired(TIMER_NAVIGATION)) {
                updateHeading();

                if (isDone == TRUE) {
                    DBPRINT("Finished navigating.\n\n");
                    startIdleState();
                }

                Timer_new(TIMER_NAVIGATION, UPDATE_DELAY);
            }
            break;
        case STATE_NAVIGATE_WAIT:
            // Lost lock, waiting or timeout to error
            if (Navigation_isReady())
                startNavigateState();

            if (Timer_isExpired(TIMER_NAVIGATION)) {
                // Couldn't recover GPS
                setError(findNavigationError());
            }
            break;
        case STATE_ERROR:
            break;
        default:
            break;
    }
}


/**********************************************************************
 * Function: Navigation_gotoLocalCoordinate
 * @param
 * @return None
 * @remark Starts navigating to the desired location until within the given
 *  tolerance range.
 **********************************************************************/
void Navigation_gotoLocalCoordinate(LocalCoordinate *ned_des, float tolerance) {
    if (!Navigation_isReady()) {
        setError(findNavigationError());
        return;
    }

    nedDestination.north = ned_des->north;
    nedDestination.east = ned_des->east;
    nedDestination.down = ned_des->down;
    destinationTolerance = tolerance;

    startNavigateState();
}


/**********************************************************************
 * Function: Navigation_getLocalDistance
 * @param A pointer to a local coordinate point.
 * @return Distance to the point in meters.
 * @remark Calculates the distance to the given point from the current
 *  position (in the local frame).
 **********************************************************************/
float Navigation_getLocalDistance(LocalCoordinate *nedPoint) {
    if (!Navigation_isReady()) {
        setError(findNavigationError());
        return;
    }

    // Get local position
    LocalCoordinate nedMine;
    getLocalPosition(&nedMine);

    // Determine needed course
    CourseVector course;
    getCourseVector(&course, &nedMine, nedPoint);

    return course.distance;
}


/**********************************************************************
 * Function: Navigation_getLocalPosition
 * @param A pointer to a local coordinate point.
 * @return None
 * @remark Saves the current local (NED) position into the given variable.
 **********************************************************************/
void Navigation_getLocalPosition(LocalCoordinate *nedPosition) {
    if (!Navigation_isReady()) {
        setError(findNavigationError());
        return;
    }
    getLocalPosition(nedPosition);
}


/**********************************************************************
 * Function: Navigation_setOrigin
 * @param A pointer to geocentric coordinate location.
 * @return None
 * @remark Sets the geodetic and ECEF origin point (generally the location
 *  of the command center), by calculating the geodetic from the given
 *  ECEF coordinate.
 **********************************************************************/
void Navigation_setOrigin(GeocentricCoordinate *ecefRef) {
/*, GeodeticCoordinate *llaRef) { */
    ecefOrigin.x = ecefRef->x;
    ecefOrigin.y = ecefRef->y;
    ecefOrigin.z = ecefRef->z;

    // calculate lla origin from ecef
    convertECEF2Geodetic(&llaOrigin, ecefRef);
    

    hasOrigin = TRUE;
}

/**********************************************************************
 * Function: Navigation_setGeocentricError
 * @param Geocentric error to add to measured geocentric position.
 * @return None
 * @remark Sets the geocentric error for error corrections.
 **********************************************************************/
void Navigation_setGeocentricError(GeocentricCoordinate *error) {
    ecefError.x = error->x;
    ecefError.y = error->y;
    ecefError.z = error->z;
    
    hasErrorCorrection = TRUE;
}


/**********************************************************************
 * Function: Navigation_cancel
 * @return None
 * @remark Cancels the current mission if navigating to a location.
 **********************************************************************/
void Navigation_cancel() {
    if (Navigation_isNavigating())
        startIdleState();

    Drive_stop();
}


/**********************************************************************
 * Function: Navigation_enableErrorCorrection
 * @return None
 * @remark Enables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_enableErrorCorrection() {
    if (hasErrorCorrection)
        useErrorCorrection = TRUE;
}

/**********************************************************************
 * Function: Navigation_disableErrorCorrection
 * @return None
 * @remark Disables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_disableErrorCorrection() {
    useErrorCorrection = FALSE;
}


/**********************************************************************
 * Function: Navigation_isReady
 * @return True if we are ready to navigate with GPS and have an origin.
 * @remark
 **********************************************************************/
bool Navigation_isReady() {
    return GPS_isInitialized() && GPS_isConnected() && GPS_hasFix()
        && GPS_hasPosition() && hasOrigin;
}

/**********************************************************************
 * Function: Navigation_hasError
 * @return TRUE or FALSE if an error occuirred while navigating.
 * @remark Errors occur if the GPS has lost a fix or become disconnected.
 *  Error codes are defined in Error.h, and can be obtained with the
 *  Navigation_getError() function.
 **********************************************************************/
bool Navigation_hasError() {
    return state == STATE_ERROR;
}

/**********************************************************************
 * Function: Navigation_getError
 * @return Error code corresponding to the last error experienced
 *  while navigating.
 * @remark Error codes are defined in Error.h. Note that this function
 *  clears the error. Also, using Navigation_gotoLocalCoordinate will
 *  clear any error codes.
 **********************************************************************/
error_t Navigation_getError() {
    error_t result = ERROR_NONE;
    if (Navigation_hasError()) {
        result = lastErrorCode;
        lastErrorCode = ERROR_NONE;
    }
    /*
    if (Navigation_hasError()) {
        startIdleState();
        result = lastErrorCode;
    }
    */
    return result;
}


/**********************************************************************
 * Function: Navigation_isNavigating
 * @return TRUE or FALSE whether we are navigating to a location.
 * @remark 
 **********************************************************************/
bool Navigation_isNavigating() {
    return state == STATE_NAVIGATE || state == STATE_NAVIGATE_WAIT;
}

/**********************************************************************
 * Function: Navigation_isDone
 * @return TRUE or FALSE whether we successfully navigated to a
 *  desired location.
 * @remark 
 **********************************************************************/
bool Navigation_isDone() {
    bool result = isDone;
    isDone = FALSE;
    return result;
}

/**********************************************************************
 * Function: Navigation_isUsingErrorCorrection
 * @return TRUE or FALSE whether error correction is enabled.
 * @remark 
 **********************************************************************/
bool Navigation_isUsingErrorCorrection() {
    return useErrorCorrection;
}

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

/**********************************************************************
 * Function: distanceToSpeed
 * @param Distance in meters.
 * @return Speed to drive in percent from 0 to 100.
 * @remark Converts the given distance into a speed in percent for
 *  commanding the motors.
 **********************************************************************/
static uint8_t distanceToSpeed(float dist) {
    //int speed = (int)dist + 5;
    uint32_t speed = ((uint32_t)dist + 3);
    speed = (speed > 100)? 100 : speed;
    return (uint8_t)speed;
}

/**********************************************************************
 * Function: startNavigateState
 * @return None
 * @remark Begins the navigation state, for navigating to a coordinate.
 **********************************************************************/
static void startNavigateState() {
    state = STATE_NAVIGATE;
    isDone = FALSE;
#ifdef USE_DRIVE
    Drive_stop();
#endif
    // Clear error
    (void)Navigation_getError();

    Timer_new(TIMER_NAVIGATION, 1); // let expire quickly
}

/**********************************************************************
 * Function: startNavigateWaitState
 * @return None
 * @remark Begins the navigation  waitstate, for waiting for a GPS
 *  lock to be recovered during navigation.
 **********************************************************************/
static void startNavigateWaitState() {
    state = STATE_NAVIGATE_WAIT;

#ifdef USE_DRIVE
    Drive_stop();
#endif

    Timer_new(TIMER_NAVIGATION, TIMEOUT_DELAY);
}



/**********************************************************************
 * Function: startErrorState
 * @return None
 * @remark Begins the error state if navigation failed, such as when
 *  GPS lost the fix, an obstruction was reached, or a person man have
 *  grabbed on.
 **********************************************************************/
static void startErrorState() {
    state = STATE_ERROR;
    isDone = FALSE;
#ifdef USE_DRIVE
    Drive_stop();
#endif
    if (lastErrorCode == ERROR_NONE)
        lastErrorCode = ERROR_UNKNOWN;
}


/**********************************************************************
 * Function: startIdleState
 * @return None
 * @remark Begins the idle state, which stops the motors.
 **********************************************************************/
static void startIdleState() {
    state = STATE_IDLE;
#ifdef USE_DRIVE
    Drive_stop();
#endif
}

/**********************************************************************
 * Function: applyGeocentricErrorCorrection
 * @return None
 * @remark Applies the error corrections to the given position (geocentric).
 **********************************************************************/
static void applyGeocentricErrorCorrection(GeocentricCoordinate *ecefPos) {
    ecefPos->x += ecefError.x;
    ecefPos->y += ecefError.y;
    ecefPos->z += ecefError.z;
}


/**********************************************************************
 * Function: getLocalPosition
 * @param Local coordinate variable to save position into.
 * @return None
 * @remark Calculates current local position relative to the origin
 *  using the GPS module.
 **********************************************************************/
static void getLocalPosition(LocalCoordinate *nedVar) {
    GeocentricCoordinate ecefMine;
    GPS_getPosition(&ecefMine);
    if (useErrorCorrection)
        applyGeocentricErrorCorrection(&ecefMine);

    convertECEF2NED(nedVar, &ecefMine, &ecefOrigin, &llaOrigin);
}

/**********************************************************************
 * Function: updateHeading
 * @return None
 * @remark Drives the motors by calculating the needed heading and speed
 *  to reach the desired location when navigating.
 **********************************************************************/
static void updateHeading() {

    // Get local position
    LocalCoordinate nedMine;
    getLocalPosition(&nedMine);

    DBPRINT("My position: N=%.2f, E=%.2f, D=%.2f\n",nedMine.north, nedMine.east, nedMine.down);

    // Determine needed course
    CourseVector course;
    getCourseVector(&course, &nedMine, &nedDestination);

    DBPRINT("\tCourse: distance=%.2f, heading=%.2f\n",course.distance, course.heading);

    // Check tolerance
    if (course.distance < destinationTolerance) {
        isDone = TRUE;
        return;
    }

    /* Heading hysteresis: Drive motors to new heading and speed, but only change
     *  heading if it varies enough from the previously calculated one. */
    float newHeading = (course.heading < (lastHeading - HEADING_TOLERANCE)
        || course.heading > (lastHeading + HEADING_TOLERANCE))?
            course.heading : lastHeading;

    uint8_t speed = distanceToSpeed(course.distance);
#ifdef USE_DRIVE
    Drive_forwardHeading(speed, (uint16_t)newHeading);
#endif

    DBPRINT("\tDriving: distance=%.2f [m], speed=%d [\%], heading=%.2f [deg]\n",
        course.distance, speed, newHeading);

    lastHeading = newHeading;
}


/**********************************************************************
 * Function: setError
 * @param Error code to trigger.
 * @return None
 * @remark Sets the navigation module into the error state and sets the 
 *  error code to the given code.
 **********************************************************************/
static void setError(error_t errorCode) {
    lastErrorCode = errorCode;

    startErrorState();
}


/**********************************************************************
 * Function: findNavigationError
 * @return The error code that navigation module experienced.
 * @remark
 **********************************************************************/
static error_t findNavigationError() {
    if (!GPS_isConnected)
        return ERROR_GPS_DISCONNECTED;
    if (!GPS_hasFix() || !GPS_hasPosition())
        return ERROR_GPS_NOFIX;
    if (!hasOrigin)
        return ERROR_NO_ORIGIN;

    return ERROR_NONE;
}

/*********************************************************************
 *                           Test Harnesses                          *
 *********************************************************************/

//#define NAVIGATION_TEST
#ifdef NAVIGATION_TEST

#include "I2C.h"
#include "TiltCompass.h"

// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

// Location is BE1 parkinglot bench
#define ECEF_X_ORIGIN  -2707571.0f
#define ECEF_Y_ORIGIN -4322145.0f
#define ECEF_Z_ORIGIN 3817542.0f
#define GEO_LAT_ORIGIN 37.000042165168395f
#define GEO_LON_ORIGIN -122.06473588943481f
#define GEO_ALT_ORIGIN 241.933f

#define DESTINATION_TOLERANCE 2.2f // (m)

#define HEADING_DELAY   UPDATE_DELAY // delay for compass

int main() {
    Board_init();
    Serial_init();
    Timer_init();
    GPS_init();
#ifdef USE_DRIVE
    Drive_init();
#endif
    I2C_init(I2C1, I2C_CLOCK_FREQ);
    TiltCompass_init();
    Timer_new(TIMER_TEST,HEADING_DELAY);
    Navigation_init();

    // Set command center reference point coordinates
    GeodeticCoordinate geoOrigin;
    geoOrigin.lat = GEO_LAT_ORIGIN;
    geoOrigin.lon = GEO_LON_ORIGIN;
    geoOrigin.alt = GEO_ALT_ORIGIN;
    GeocentricCoordinate ecefOrigin;
    ecefOrigin.x = ECEF_X_ORIGIN;
    ecefOrigin.y = ECEF_Y_ORIGIN;
    ecefOrigin.z = ECEF_Z_ORIGIN;

    Navigation_setOrigin(&ecefOrigin, &geoOrigin);

    printf("Navigation system initialized.\n");
    while (!Navigation_isReady()) {
        Navigation_runSM();
        GPS_runSM();
    }

    printf("Navigation system is ready.\n");


    LocalCoordinate nedDesired;
    nedDesired.n = 0.0f;
    nedDesired.e = 0.0f;
    nedDesired.d = 0.0f;
    Navigation_gotoLocalCoordinate(&nedDesired, DESTINATION_TOLERANCE);

    while(Navigation_isNavigating()) {
        GPS_runSM();
        Navigation_runSM();
        TiltCompass_runSM();
#ifdef USE_DRIVE
        Drive_runSM();
#endif
        if (Timer_isExpired(TIMER_TEST)) {
            printf("\tCompass heading: %.1f\n", TiltCompass_getHeading());
            Timer_new(TIMER_TEST,HEADING_DELAY);
        }
    }

    Navigation_runSM();
#ifdef USE_DRIVE
    Drive_runSM();
#endif
    
    if (Navigation_isDone())
        printf("At desired point.\n");
    else
        printf("Failed to reach desired point.\n");

    // Make sure we don't try and drive any more
    Navigation_runSM();
    Navigation_runSM();
    return SUCCESS;
}

#endif

// ************************************************************************
// ---------------------------- Override Test ----------------------------
// ************************************************************************

//#define NAVIGATION_OVERRIDE_TEST
#ifdef NAVIGATION_OVERRIDE_TEST

#define USE_COMPASS
#define USE_OVERRIDE

#ifdef USE_COMPASS
#include "I2C.h"
#include "TiltCompass.h"
#endif
#include "Override.h"


// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

// --------------- Center of west lake -------------
#define ECEF_X_ORIGIN -2706922.0f
#define ECEF_Y_ORIGIN -4324246.0f
#define ECEF_Z_ORIGIN 3815364.0f

// ------- In front of GSH parking lot entrance ------
/*
#define ECEF_X_ORIGIN -2707534.0f
#define ECEF_Y_ORIGIN -4322167.0f
#define ECEF_Z_ORIGIN  3817539.0f
 * */
///

#define DESTINATION_TOLERANCE 3.2f // (m)

#define HEADING_DELAY   UPDATE_DELAY // delay for compass

#define STARTUP_DELAY   3000 // time for gps to get stable fix

// Others
#define MAX_ERRORS 3 // max nav. errors before receiver has control forever


// ----------------------------- Prototypes -------------------------------

void initializeDestination();
void handleError();
void startInitialize();
void startNavigate();
void startOverride();
bool nearDesiredPoint();

void initializeOverride();
void giveMicroControl();
void giveReceiverControl();
// -------------------------- Global variables ----------------------------
static uint8_t errorsSeen = 0;
static bool startDelayExpired = FALSE;

static enum {
    INITIALIZE = 0x1, // Waiting for GPS lock
    NAVIGATE  = 0x2, // GPS guided navigation with drive controller
    OVERRIDE = 0x3, // reciever has control
} testState;


static LocalCoordinate nedDesired;

// ---------------------------- Entry point ------------------------------
int main() {
    // --------------------------- Initialization ------------------------
    Board_init();

    #ifdef USE_LOGGER
    if (Logger_init() != SUCCESS)
        return FAILURE;
    #else
    Serial_init();
    #endif

    Timer_init();
    GPS_init();
    #ifdef USE_DRIVE
    Drive_init();
    #endif

    #ifdef USE_COMPASS
    I2C_init(I2C1, I2C_CLOCK_FREQ);
    TiltCompass_init();
    #endif
    Navigation_init();
    #ifdef USE_OVERRIDE
    Override_init();
    #endif
    initializeDestination();

    // Start out in intiialize state
    startInitialize();

    DBPRINT("System initialized... waiting for lock.\n");

    // ----------------------------- State machine --------------------------
    while (1) {
        GPS_runSM();
        Navigation_runSM();

        switch (testState) {
            // Initialize state for waiting for GPS lock
            case INITIALIZE:
                if (Override_isTriggered())
                    startOverride();
                if (Navigation_isReady())
                    startNavigate();
                break;
            // Navigate state for micro navigating to a position
            case NAVIGATE:
                #ifdef USE_COMPASS
                TiltCompass_runSM();
                #endif
                #ifdef USE_DRIVE
                Drive_runSM();
                #endif
                if (Override_isTriggered())
                    startOverride();

                // Start up delay before navigating for stable fix
                if (Timer_isExpired(TIMER_TEST) && !startDelayExpired) {
                    startDelayExpired = TRUE;
                    // Send navigate command
                    DBPRINT("Navigating to desired point to within %.2f m.\n",
                        DESTINATION_TOLERANCE);;
                    Navigation_gotoLocalCoordinate(&nedDesired, DESTINATION_TOLERANCE);
                }

                // Compass heading printing
                #ifdef USE_COMPASS
                if (Timer_isExpired(TIMER_TEST)) {
                    DBPRINT("\tCompass heading: %.1f\n", TiltCompass_getHeading());
                    Timer_new(TIMER_TEST,HEADING_DELAY);
                }
                #endif

                // Did we arrive?
                if (Navigation_isDone()) {
                    DBPRINT("At desired point. ");
                    #ifdef USE_OVERRIDE
                    DBPRINT("Giving receiver control.\n");
                    startOverride();
                    #else
                    DBPRINT("Ending test.\n");
                    goto EXIT;
                    #endif
                }

                // Check for navigation error and handle
                if (Navigation_hasError())
                    handleError();

                break;
            case OVERRIDE:
                /* Did receiver turn off, timeout occured, and we moved away
                   from the desired point? */
                if (!Override_isTriggered() && !nearDesiredPoint()) {
                    DBPRINT("Receiver timed out. Restarting test.\n");
                    startInitialize(); // gives micro control
                }

                break;
        } // switch
    } // while
    EXIT:
    return SUCCESS;
}

// ---------------- Override test helper functions --------------------------

bool nearDesiredPoint() {
    // Check tolerance
    if (Navigation_getLocalDistance(&nedDestination) < destinationTolerance) {
        return TRUE;
    }
    return FALSE;
}

void initializeDestination() {
    // Set command center reference point coordinates
    /*
    GeodeticCoordinate geoOrigin;
    geoOrigin.lat = GEO_LAT_ORIGIN;
    geoOrigin.lon = GEO_LON_ORIGIN;
    geoOrigin.alt = GEO_ALT_ORIGIN;
     **/
    GeocentricCoordinate ecefOrigin;
    ecefOrigin.x = ECEF_X_ORIGIN;
    ecefOrigin.y = ECEF_Y_ORIGIN;
    ecefOrigin.z = ECEF_Z_ORIGIN;

    // Make desired point the origin
    nedDesired.north = 0.0f;
    nedDesired.east = 0.0f;
    nedDesired.down = 0.0f;

    Navigation_setOrigin(&ecefOrigin);
}

void handleError() {
    // An error occured, try and reinit to regain lock
    errorsSeen++;
    uint8_t errorCode = Navigation_getError();
    DBPRINT("An error occured navigating: %s.\n",GET_ERROR_MESSAGE(errorCode));
    if (errorsSeen > MAX_ERRORS) {
        // Go into override state and do nothing
        startOverride();
        DBPRINT("\tGoing into override.\n");
    }
    else {
        startInitialize();
        DBPRINT("\tTrying again...\n");
    }
}


// -------------------------- Start test states ---------------------------
void startInitialize() {
    testState = INITIALIZE;
    Override_giveMicroControl();
}

void startNavigate() {
    testState = NAVIGATE;
    DBPRINT("Navigation and GPS are ready.\n");
    Timer_new(TIMER_TEST,STARTUP_DELAY); // let gps get good fix
    startDelayExpired = FALSE;
}

void startOverride() {
    testState = OVERRIDE;
    Navigation_cancel();
    Navigation_runSM();
    Drive_runSM();
    Override_giveReceiverControl();
}



#endif

