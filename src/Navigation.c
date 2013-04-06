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


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG
//#define USE_DRIVE

#define UPDATE_DELAY        2500 // (ms)

// don't change heading unless calculated is this much away from last
#define HEADING_TOLERANCE   10 // (deg)

// proportionally scale speed (m/s) for a given distance (m)
#define DISTANCE_TO_SPEED(dist)    ((float)dist*0.065f + 0.22f)

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
                    #ifdef DEBUG
                    printf("Finished navigating.\n\n");
                    #endif
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
    printf("My position: N=%.2f, E=%.2f, D=%.2f\n",nedMine.n, nedMine.e, nedMine.d);
#endif

    // Determine needed course
    CourseVector course;
    getCourseVector(&course, &nedMine, &nedDestination);
#ifdef DEBUG
    printf("\tCourse: distance=%.2f, heading=%.2f\n",course.d,course.yaw);
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
    Drive_forwardHeading(DISTANCE_TO_SPEED(course.d), newHeading);
#elif defined(DEBUG)
    printf("\tDriving: speed=%.2f [m/s], heading=%.2f [deg]\n",
        DISTANCE_TO_SPEED(course.d),newHeading);
#endif
    lastHeading = newHeading;
}


#define NAVIGATION_TEST
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
            printf("\tCompass heading: %.1f\n", TiltCompass_getheading());
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
