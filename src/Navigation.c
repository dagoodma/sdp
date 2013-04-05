/*
 * File:   Navigation.c
 * Author: David Goodman
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

#define UPDATE_DELAY        1000 // (ms)

// don't change heading unless calculated is this much away from last
#define HEADING_TOLERANCE   10 // (deg)

// proportionally scale speed (m/s) for a given distance (m)
#define DISTANCE_TO_SPEED(dist)    ((float)dist*0.12598)

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

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
BOOL Navigation_init() {

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

                if (isDone)
                    startIdleState();

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
 * @return True if we are ready to navigate with GPS.
 * @remark
 **********************************************************************/
BOOL Navigation_isReady() {
    return GPS_isInitialized() && GPS_isConnected() && GPS_hasFix()
        && GPS_hasPosition() && hasOrigin;
}

BOOL Navigation_hasError() {
    return state = STATE_ERROR;
}

BOOL Navigation_clearError() {
    if (Navigation_hasError())
        startIdleState();
}

BOOL Navigation_isNavigating() {
    return state = STATE_NAVIGATE;
}


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


startNavigateState() {
    state = STATE_NAVIGATE;
    isDone = FALSE;
    Drive_stop();
    Timer_new(TIMER_NAVIGATION, 1); // let expire quickly
}

startErrorState() {
    state = STATE_ERROR;
    isDone = FALSE;
    Drive_stop();
}

startIdleState() {
    state = STATE_IDLE;
    Drive_stop();
}

applyGeocentricErrorCorrection(GeocentricCoordinate *ecefPos) {
    ecefPos->x += ecefError.x;
    ecefPos->y += ecefError.y;
    ecefPos->z += ecefError.z;
}

updateHeading() {

    // Calculate NED position
    GeocentricCoordinate ecefMine;
    GPS_getPosition(ecefMine);
    if (useErrorCorrection)
        applyGeocentricErrorCorrection(&ecefMine);

    LocalCoordinate nedMine;
    convertECEF2NED(&nedMine, &ecefMine, &ecefOrigin, &llaOrigin);

    // Determine needed course
    CourseVector course;
    getCourseVector(&course, &nedMine, &nedDestination);

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
    Drive_forwardHeading(DISTANCE_TO_SPEED(course.d), newHeading);
    lastHeading = newHeading;
}


//#define NAVIGATION_TEST
#ifdef NAVIGATION_TEST

int main() {
    Board_init();
    Serial_init();
    Timer_init();
    if (Navigation_init() != SUCCESS) {
        printf("Navigation system failed to initialize.\n");
        return FAILURE;
    }

    printf("Navigation system initialized.\n");
    /*while (!Navigation_isReady()) {
        Navigation_runSM();
    }
    */
    Navigation_runSM();

    
    printf("Navigation system is ready.\n");

    Coordinate coord;

    float yaw = 150.0; // (deg)
    float pitch = 85.0; // (deg)
    float height = 4.572; // (m)
    if (Navigation_getProjectedCoordinate(&coord, yaw, pitch, height)) {
        #ifdef USE_GEODETIC
        printf("Desired coordinate -- lat:%.6f, lon: %.6f, alt: %.2f (m)\n",
            coord.x, coord.y, coord.z);
        #else
        printf("Desired coordinate -- N:%.2f, E: %.2f, D: %.2f (m)\n",
            coord.x, coord.y, coord.z);
        #endif
    }
    else {
        #ifdef USE_GEODETIC
        char s[10] = "geodetic";
        #else
        char s[10] = "NED";
        #endif
        printf("Failed to obtain desired %s coordinate.\n",s);
    }


    return SUCCESS;
}

#endif
