/*
 * File:   Navigation.c
 * Author: David Goodman
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


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/



GeocentricCoordinate ecefError;
BOOL isUsingError = FALSE;



/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
BOOL Navigation_init() {
    #ifdef USE_GPS
    uint8_t options = 0x0;
    if (GPS_init(options) != SUCCESS || !GPS_isInitialized()) {
        printf("Failed to initialize Navigation system.\n");
        return FAILURE;
    }
    #endif
    return SUCCESS;
}

void Navigation_runSM() {
    #ifdef USE_GPS
    GPS_runSM();
    #endif
}

BOOL Navigation_isReady() {
    #ifdef USE_GPS
    return GPS_isInitialized() && GPS_isConnected() && GPS_hasFix()
        && GPS_hasPosition();
    #else
    return TRUE;
    #endif
}



/**********************************************************************
 * Function: GPS_setLongitudeError
 * @return None
 * @remark Sets the longitudal error for error corrections.
 **********************************************************************/
void Navigation_setGeocentricError(GeocentricCoordinate error) {
    ecefError = error;
}


/**********************************************************************
 * Function: GPS_enableErrorCorrection
 * @return None
 * @remark Enables error correction for retreived coordinates.
 **********************************************************************/
void GPS_enableErrorCorrection() {
    isUsingError = TRUE;
}

/**********************************************************************
 * Function: GPS_disableErrorCorrection
 * @return None
 * @remark Disables error correction for retreived coordinates.
 **********************************************************************/
void GPS_disableErrorCorrection() {
    isUsingError = FALSE;
}


/**
 * Function: getCurrentPosition
 * @param A pointer to a new coordinate variable to save result into.
 * @return None.
 * @remark Converts the current GPS position into NED and saves it into
 *  the given Position variable.
 * @author David Goodman
 * @date 2013.03.10 
void getCurrentPosition(Position pos) {
    if (!pos || !Navigation_isReady())
        return;

    // Note: x=lat, y=lon, z=alt (in geodetic)
    float lat = GPS_getLatitude();
    float lon = GPS_getLongitude();
    float alt = GPS_getAltitude();
    
    // ------- First convert LLA (geo) to ECEF
    float sinlat = sin(lat);
    float coslat = cos(lat);

    // Prime vertical radius of curviture
    float rad_ne = R_EN / sqrt(abs(1.0 - (ECC2 * sinlat * sinlat)));
    float ecefX = (rad_ne - alt) * coslat * cos(lon);
    float ecefY = (rad_ne - alt) * coslat * sin(lon);
    float ecefZ = (rad_ne * (1.0 - ECC2) - alt) * sinlat;

    // Convert ECEF to NED at current lat,lon reference point
    

} */


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
