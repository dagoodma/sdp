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

// Ellipsoid (olbate) constants for coordinate conversions
#define ECC     0.0818191908426f // eccentricity
#define ECC2    (ECC*ECC)
#define ECCP2   (ECC2 / (1.0 - ECC2)) // square of second eccentricity
#define FLATR   (ECC2 / (1.0 + sqrt(1.0 - ECC2))) // flattening ratio

// Radius of earth's curviture on semi-major and minor axes respectively
#define R_EN    6378137.0f     // (m) prime vertical radius (semi-major axis)
#define R_EM    (R_EN * (1 - FLATR)) // meridian radius (semi-minor axis)



/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
void convertENU2ECEF(Coordinate *var, float east, float north, float up, float lat_ref,
    float lon_ref, float alt_ref);
void convertGeodetic2ECEF(Coordinate *var, float lat, float lon, float alt);
void convertECEF2Geodetic(Coordinate *var, float ecef_x, float ecef_y, float ecef_z);
void convertEuler2NED(Coordinate *var, float yaw, float pitch, float height);

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


//#ifdef IS_COMPAS
BOOL Navigation_getProjectedCoordinate(Coordinate *coord, float yaw, float pitch, float height) {
    if (yaw >= YAW_LIMIT)
        return FALSE;
    else if (pitch > PITCH_LIMIT)
        return FALSE;

    #ifdef USE_GEODETIC
    if ( ! Navigation_isReady())
        return FALSE;

    float alt = GPS_getAltitude();
    #endif
    float alt = 0.0;
    
    // Convert params to NED vector (x=north, y=east, z=down)
    Coordinate ned;
    convertEuler2NED(&ned, yaw, pitch, height + alt);

    #ifdef USE_GEODETIC
    // Get refence geodetic coordinate
    float lat = GPS_getLatitude();
    float lon = GPS_getLongitude();

    // Convert NED to ENU and obtain projected ECEF
    Coordinate ecef = Coordinate_new(ecef, 0, 0 ,0);
    convertENU2ECEF(ecef, ned->y, ned->x, -(ned->z), lat, lon, alt);

    // Convert projected ECEF into projected Geodetic (LLA)
    convertECEF2Geodetic(geo, ecef->x, ecef->y, ecef->z);

    #else
    coord->x = ned.x;
    coord->y = ned.y;
    coord->z = ned.z;
    /*
    printf("Desired coordinate -- N:%.2f, E: %.2f, D: %.2f (m)\n",
        coord->x, coord->y, coord->z);
    printf("Desired coordinate -- N:%.2f, E: %.2f, D: %.2f (m)\n",
        ned.x, ned.y, ned.z);
    */
    #endif

    return TRUE;
}
//#endif

// -------------------------- Functions for Types ----------------------
/*
Coordinate Coordinate_new(Coordinate coord, float x, float y, float z) {
    //if (coord) {
        coord.x = x;
        coord.y = y;
        coord.z = z;
    //}

    return coord;
}
*/
/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

/**
 * Function: convertENU2ECEF
 * @param A pointer to a new ECEF coordinate variable to save result into.
 * @param East component in meters.
 * @param North component in meters.
 * @param Up component in meters.
 * @return None.
 * @remark Converts the given ENU vector into a ECEF coordinate.
 * @author David Goodman
 * @author MATLAB
 * @date 2013.03.10  */
void convertENU2ECEF(Coordinate *var, float east, float north, float up, float lat_ref,
    float lon_ref, float alt_ref) {
    // Convert geodetic lla  reference to ecef
    Coordinate ecef_ref; //= Coordinate_new(ecef_ref, 0, 0, 0);
    convertGeodetic2ECEF(&ecef_ref, lat_ref, lon_ref, alt_ref);

    float coslat = cos(DEGREE_TO_RADIAN*lat_ref);
    float sinlat = sin(DEGREE_TO_RADIAN*lat_ref);
    float coslon = cos(DEGREE_TO_RADIAN*lon_ref);
    float sinlon = sin(DEGREE_TO_RADIAN*lon_ref);

    float t = coslat * up - sinlat * north;
    float dz = sinlat * up + coslat * north;

    float dx = coslon * t - sinlon * east;
    float dy = sinlon * t + coslon * east;

    var->x = ecef_ref.x + dx;
    var->y = ecef_ref.y + dy;
    var->z = ecef_ref.z + dz;
}


/**
 * Function: convertGeodetic2ECEF
 * @param A pointer to a new ECEF coordinate variable to save result into.
 * @param Latitude in degrees.
 * @param Longitude in degrees.
 * @param Altitude in meters.
 * @return None.
 * @remark Converts the given ECEF coordinates into a geodetic coordinate in degrees.
 *  Note that x=lat, y=lon, z=alt.
 * @author David Goodman
 * @author MATLAB
 * @date 2013.03.10  */
void convertGeodetic2ECEF(Coordinate *var, float lat, float lon, float alt) {
    float sinlat = sin(DEGREE_TO_RADIAN*lat);
    float coslat = cos(DEGREE_TO_RADIAN*lat);

    float rad_ne = R_EN / sqrt(1.0 - (ECC2 * sinlat * sinlat));
    var->x = (rad_ne + alt) * coslat * cos(lon*DEGREE_TO_RADIAN);
    var->y = (rad_ne + alt) * coslat * sin(lon*DEGREE_TO_RADIAN);
    var->z = (rad_ne*(1.0 - ECC2) + alt) * sinlat;
}


/**
 * Function: convertECEF2Geodetic
 * @param A pointer to a new geodetic (LLA) coordinate variable to save result into.
 * @param ECEF X position.
 * @param ECEF Y position.
 * @param ECEF Z position.
 * @return None.
 * @remark Converts the given ECEF coordinates into a geodetic coordinate in degrees.
 *  Note that x=lat, y=lon, z=alt.
 * @author David Goodman
 * @author MATLAB
 * @date 2013.03.10  */
void convertECEF2Geodetic(Coordinate *var, float ecef_x, float ecef_y, float ecef_z) {
    float lat = 0, lon = 0, alt = 0;

    lon = atan2(ecef_y, ecef_x);

    float rho = hypotf(ecef_x,ecef_y); // distance from z-axis
    float beta = atan2(ecef_z, (1 - FLATR) * rho);

    lat = atan2(ecef_z + R_EM * ECCP2 * sin(beta)*sin(beta)*sin(beta),
        rho - R_EN * ECC2 * cos(beta)*cos(beta)*cos(beta));

    float betaNew = atan2((1 - FLATR)*sin(lat), cos(lat));
    int count = 0;
    while (beta != betaNew && count < 5) {
        beta = betaNew;
        var->x = atan2(ecef_z  + R_EM * ECCP2 * sin(beta)*sin(beta)*sin(beta),
            rho - R_EN * ECC2 * cos(beta)*cos(beta)*cos(beta));

        betaNew = atan2((1 - FLATR)*sin(lat), cos(lat));
        count = count + 1;
    }

    float sinlat = sin(lat);
    float rad_ne = R_EN / sqrt(1.0 - (ECC2 * sinlat * sinlat));

    alt = rho * cos(lat) + (ecef_z + ECC2 * rad_ne * sinlat) * sinlat - rad_ne;

    // Convert radian geodetic to degrees
    var->x = RADIAN_TO_DEGREE*lat;
    var->y = RADIAN_TO_DEGREE*lon;
    var->z = alt;
}


/**
 * Function: convertEuler2NED
 * @param A pointer to a new NED coordinate variable to save result into.
 * @param Yaw in degrees from north.
 * @param Pitch in degrees from level.
 * @param Height in meters from target.
 * @return None.
 * @remark Converts the given yaw, pitch, and height into a NED vector. Note
 *  that x,y, and z are North, East, and Down respectively in coordinate variable.
 * @author David Goodman
 * @date 2013.03.10  */
void convertEuler2NED(Coordinate *var, float yaw, float pitch, float height) {
    //printf("At angle: %.3f and pitch: %.3f\n",yaw,pitch);

    float mag = height * tan((90.0-pitch)*DEGREE_TO_RADIAN);
    #ifdef DEBUG
    printf("\tMagnitude: %.3f\n",mag);
    #endif

    //printf("At mag: %.3f\n",mag);
    
    if (yaw <= 90.0) {
        //First quadrant
        var->x = mag * cos(yaw*DEGREE_TO_RADIAN);
        var->y = mag * sin(yaw*DEGREE_TO_RADIAN);
    }
    else if (yaw > 90.0 && yaw <= 180.0) {
        // Second quadrant
        yaw = yaw - 270.0;
        var->x = mag * sin(yaw*DEGREE_TO_RADIAN);
        var->y = -mag * cos(yaw*DEGREE_TO_RADIAN);
    }
    else if (yaw > 180.0 && yaw <= 270.0) {
        // Third quadrant
        yaw = yaw - 180.0;
        var->x = -mag * cos(yaw*DEGREE_TO_RADIAN);
        var->y = -mag * sin(yaw*DEGREE_TO_RADIAN);
    }
    else if (yaw > 270 < 360.0) {
        // Fourth quadrant
        yaw = yaw - 90.0;
        var->x = -mag * sin(yaw*DEGREE_TO_RADIAN);
        var->y = mag * cos(yaw*DEGREE_TO_RADIAN);
    }


    var->z = height;
    //printf("Desired coordinate -- N:%.2f, E: %.2f, D: %.2f (m)\n",
    //    var->x, var->y, var->z);
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