/*
 * File:   Navigation.c
 * Author: David     Goodman
 *
 * TODO: Consider adding error correction timeout.
 * TODO: Consider adding
 *
 * Created on March 3, 2013, 10:27 AM
 */
#include <xc.h>
#include <stdio.h>
#include <plib.h>
//#define __XC32
#include <math.h>
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "GPS.h"
#include "Navigation.h"
#include "Drive.h"
#include "Logger.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG
#define USE_DRIVE
#define USE_LOGGER

#define UPDATE_DELAY        1500 // (ms)

// don't change heading unless calculated is this much away from last
#define HEADING_TOLERANCE   10 // (deg)

// proportionally scale speed (m/s) for a given distance (m)
#define DISTANCE_TO_SPEED(dist)    ((float)dist*0.015f + 0.1f) // test speeds
//#define DISTANCE_TO_SPEED(dist)    ((float)dist*0.065f + 0.22f)

#if defined(DEBUG)
#ifdef USE_LOGGER
#define DBPRINT(msg) do { Logger_write(msg); } while(0)
#else
#define DBPRINT(msg) do { printf(msg); } while(0)
#endif
#else
#define DBPRINT(msg)    ((int)0)
#endif

 
#ifdef DEBUG
    char debug[255];
#endif

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/


static enum {
    STATE_IDLE  = 0x0,    // Initializing and obtaining instructions
    STATE_NAVIGATE = 0x1, // Maintaining station coordinates
    STATE_ERROR = 0x2,    // Error occured
} state;

LocalCoordinate nedDestination;
float destinationTolerance = 0.0, lastHeading = 0.0;

GeocentricCoordinate ecefError, ecefOrigin;
GeodeticCoordinate llaOrigin;
BOOL isDone = FALSE;
BOOL hasOrigin = FALSE;
BOOL hasErrorCorrection = FALSE;
BOOL useErrorCorrection = FALSE;


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
void startNavigateState();
void startErrorState();
void startIdleState();
void applyGeocentricErrorCorrection(GeocentricCoordinate *ecefPos);
void updateHeading();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
BOOL Navigation_init() {
    startIdleState();
    Timer_new(TIMER_NAVIGATION, UPDATE_DELAY);
    return SUCCESS;
}

void Navigation_runSM() {
    switch (state) {
        case STATE_IDLE:
            // Do Nothing
            break;
        case STATE_NAVIGATE:
            if (!Navigation_isReady()) {
                startErrorState();
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
        case STATE_ERROR:
            // TODO: add functions to check for and clear this
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
        startErrorState();
        return;
    }

    nedDestination.n = ned_des->n;
    nedDestination.e = ned_des->e;
    nedDestination.d = ned_des->d;
    destinationTolerance = tolerance;

    startNavigateState();
}



/**********************************************************************
 * Function: Navigation_setOrigin
 * @return None
 * @remark Sets the longitudal error for error corrections.
 **********************************************************************/
void Navigation_setOrigin(GeocentricCoordinate *ecefRef,
    GeodeticCoordinate *llaRef) {
    ecefOrigin.x = ecefRef->x;
    ecefOrigin.y = ecefRef->y;
    ecefOrigin.z = ecefRef->z;
    llaOrigin.lat = llaRef->lat;
    llaOrigin.lon = llaRef->lon;
    llaOrigin.alt = llaRef->alt;

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

void Navigation_cancel() {
    startIdleState();
}


/**********************************************************************
 * Function: Navigation_enableErrorCorrection
 * @return None
 * @remark Enables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_enablePositionErrorCorrection() {
    if (hasErrorCorrection)
        useErrorCorrection = TRUE;
}

/**********************************************************************
 * Function: Navigation_disableErrorCorrection
 * @return None
 * @remark Disables error correction for retreived coordinates.
 **********************************************************************/
void Navigation_disablePositionErrorCorrection() {
    useErrorCorrection = FALSE;
}


/**********************************************************************
 * Function: Navigation_isReady
 * @return True if we are ready to navigate with GPS and have an origin.
 * @remark
 **********************************************************************/
BOOL Navigation_isReady() {
    return GPS_isInitialized() && GPS_isConnected() && GPS_hasFix()
        && GPS_hasPosition() && hasOrigin;
}

BOOL Navigation_hasError() {
    return state == STATE_ERROR;
}

BOOL Navigation_clearError() {
    if (Navigation_hasError())
        startIdleState();
}

BOOL Navigation_isNavigating() {
    return state == STATE_NAVIGATE;
}

BOOL Navigation_isDone() {
    return isDone;
}

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


void startNavigateState() {
    state = STATE_NAVIGATE;
    isDone = FALSE;
#ifdef USE_DRIVE
    Drive_stop();
#endif
    Timer_new(TIMER_NAVIGATION, 1); // let expire quickly
}

void startErrorState() {
    state = STATE_ERROR;
    isDone = FALSE;
#ifdef USE_DRIVE
    Drive_stop();
#endif
}

void startIdleState() {
    state = STATE_IDLE;
#ifdef USE_DRIVE
    Drive_stop();
#endif
}

void applyGeocentricErrorCorrection(GeocentricCoordinate *ecefPos) {
    ecefPos->x += ecefError.x;
    ecefPos->y += ecefError.y;
    ecefPos->z += ecefError.z;
}

void updateHeading() {

    // Calculate NED position
    GeocentricCoordinate ecefMine;
    GPS_getPosition(&ecefMine);
    if (useErrorCorrection)
        applyGeocentricErrorCorrection(&ecefMine);

    LocalCoordinate nedMine;
    convertECEF2NED(&nedMine, &ecefMine, &ecefOrigin, &llaOrigin);
#ifdef DEBUG
    sprintf(debug, "My position: N=%.2f, E=%.2f, D=%.2f\n",nedMine.n, nedMine.e, nedMine.d);
    DBPRINT(debug);
#endif

    // Determine needed course
    CourseVector course;
    getCourseVector(&course, &nedMine, &nedDestination);
#ifdef DEBUG
    sprintf(debug, "\tCourse: distance=%.2f, heading=%.2f\n",course.d,course.yaw);
    DBPRINT(debug);
#endif

    // Check tolerance
    if (course.d < destinationTolerance) {
        isDone = TRUE;
        return;
    }

    /* Drive motors to new heading and speed, but only change heading if
     it varies enough from the previously calcualted one. */
    float newHeading = (course.yaw < (lastHeading - HEADING_TOLERANCE)
        || course.yaw > (lastHeading + HEADING_TOLERANCE))?
            course.yaw : lastHeading;
#ifdef USE_DRIVE
    Drive_forwardHeading(DISTANCE_TO_SPEED(course.d), (uint16_t)newHeading);
#elif defined(DEBUG)
    sprintf(debug, "\tDriving: speed=%.2f [m/s], heading=%.2f [deg]\n",
        DISTANCE_TO_SPEED(course.d),newHeading);
    DBPRINT(debug);
#endif
    lastHeading = newHeading;
}


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


//#define NAVIGATION_OVERRIDE_TEST
#ifdef NAVIGATION_OVERRIDE_TEST

#include "I2C.h"
#include "TiltCompass.h"
#include "Ports.h"

// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

// Location is BE1 parkinglot bench
#define ECEF_X_ORIGIN -2707512.0f
#define ECEF_Y_ORIGIN -4322157.0f
#define ECEF_Z_ORIGIN 3817571.0f
#define GEO_LAT_ORIGIN 37.000357054124315f
#define GEO_LON_ORIGIN -122.06410154700279f
#define GEO_ALT_ORIGIN 242.607f

#define DESTINATION_TOLERANCE 2.2f // (m)

#define HEADING_DELAY   UPDATE_DELAY // delay for compass

#define STARTUP_DELAY   3000 // time for gps to get stable fix

// Override defines
#define ENABLE_OUT_TRIS PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

#define MICRO       0
#define RECIEVER    1


// Prototypes
void initializeOverride();
void giveReceiverControl();

// Global variables
static BOOL overrideTriggered;

int main() {

    ENABLE_OUT_TRIS = OUTPUT;  // Set pin to be an output (fed to the AND gates)
    ENABLE_OUT_LAT = MICRO;    // Initialize control for Microcontroller

    //Initializations
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

    I2C_init(I2C1, I2C_CLOCK_FREQ);
    TiltCompass_init();
    Navigation_init();
    initializeOverride(); 

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
    DBPRINT("Navigation system initialized.\n");

    // Wait for GPS fix
    while (!Navigation_isReady()) {
        Navigation_runSM();
        GPS_runSM();
    }
    Timer_new(TIMER_TEST,STARTUP_DELAY); // let gps get good fix
    while (!Timer_isExpired(TIMER_TEST)) {
        asm("nop");
    }

    Timer_new(TIMER_TEST,HEADING_DELAY); // Start compass heading print timer
    DBPRINT("Navigation and GPS are ready.\n\n");


    // Make desired point the origin
    LocalCoordinate nedDesired;
    nedDesired.n = 0.0f;
    nedDesired.e = 0.0f;
    nedDesired.d = 0.0f;


    // Navigate to origin
    Navigation_gotoLocalCoordinate(&nedDesired, DESTINATION_TOLERANCE);
    while(Navigation_isNavigating()) {
        GPS_runSM();
        Navigation_runSM();
        TiltCompass_runSM();
#ifdef USE_DRIVE
        Drive_runSM();
#endif
#ifdef DEBUG
        if (Timer_isExpired(TIMER_TEST)) {
            sprintf(debug, "\tCompass heading: %.1f\n", TiltCompass_getHeading());
            DBPRINT(debug);
            Timer_new(TIMER_TEST,HEADING_DELAY);
        }
#endif
        if (overrideTriggered == TRUE) {
            // Receiver came online
            Navigation_cancel();
            Drive_stop();
            break;
        }

    }

    // Run SM's one last time
    Navigation_runSM();
#ifdef USE_DRIVE
    Drive_runSM();
#endif
    
    // Did we reach the desired point, or did an error occur ?
    // TODO: add a GPS position check here, and add error printing for lost fix
    if (Navigation_isDone())
        DBPRINT("At desired point.\n");
    else
        DBPRINT("Failed to reach desired point.\n");

    // Disable everything and let receiver have control
    giveReceiverControl();
    while (1) {
        // Do nothing
        asm("nop");
    }

    return SUCCESS;
}



/**
 * Function: initializeOverride()
 * @return None
 * @remark Initializes interrupt for override functionality
 * @author Darrel Deo
 * @date 2013.04.01  */
void initializeOverride(){
    overrideTriggered = FALSE;

    //Enable the interrupt for the override feature
    mPORTBSetPinsDigitalIn(BIT_0); // CN2

    mCNOpen(CN_ON | CN_IDLE_CON , CN2_ENABLE , CN_PULLUP_DISABLE_ALL);
    uint16_t value = mPORTDRead();
    ConfigIntCN(CHANGE_INT_ON | CHANGE_INT_PRI_2);
    //CN2 J5-15
    INTEnableSystemMultiVectoredInt();
    DBPRINT("Override Function has been Initialized\n");
    //INTEnableInterrupts(); // handled in Board.c
    INTEnable(INT_CN,1);
}

/**
 * Function: Interrupt Service Routine
 * @return None
 * @remark ISR that is called when CH3 pings external interrupt
 * @author Darrel Deo
 * @date 2013.04.01  */
void __ISR(_CHANGE_NOTICE_VECTOR, ipl2) ChangeNotice_Handler(void){
    mPORTDRead();

    overrideTriggered = TRUE;

    //Clear the interrupt flag that was risen for the external interrupt
    //might want to set a timer in here

    mCNClearIntFlag();
    INTEnable(INT_CN,0);
}


/**
 * Function: giveReceiverControl
 * @return None
 * @remark Passes motor control over to the receiver.
 * @author David Goodman
 * @date 2013.04.01  */
void giveReceiverControl() {
    DBPRINT("Reciever Control\n\n");
#ifdef USE_DRIVE
    Drive_stop();
#endif
    ENABLE_OUT_LAT = RECIEVER;      //Give control over to Reciever using the enable line
    INTEnable(INT_CN,1); // may not need this for now
}


#endif

