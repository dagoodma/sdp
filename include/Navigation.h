/**
 * @file    Navigation.h
 * @author  David Goodman
 *
 * @brief
 * Navigation system interface for COMPAS and ATLAS systems.
 *
 * @details
 * This module utilizes the GPS module to provide functions
 * for the navigation systems of the COMPAS and ATLAS. IS_COMPAS
 * or IS_ATLAS must be defined in order for this module to provide
 * functionality.
 *
 * @date March 10, 2013, 10:03 AM  -- Created
 */

#ifndef Navigation_H
#define Navigation_H

#include <stdint.h>
#include <math.h>

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

#define PI                      3.14159265359f
#define DEGREE_TO_RADIAN        ((float)PI/180.0)
#define RADIAN_TO_DEGREE        ((float)180.0/PI)

#define DEGREE_TO_NEDFRAME(deg) (-deg + 90.0)
/***********************************************************************
 * PUBLIC TYPEDEFS
 ***********************************************************************/

// Geodetic (lat, lon, alt) or NED coordinate for GPS
typedef struct oCoordinate {
    float x, y ,z;
} Coordinate;

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
/**
 * Function: Navigation_init
 * @return SUCCESS or FAILURE.
 * @remark Initializes the navigation system by intializing the GPS module.
 * @author David Goodman
 * @date 2013.03.10  */
BOOL Navigation_init();
   
/**
 * Function: Navigation_runSM
 * @return None.
 * @remark Wrapper around GPS_runSM(), which executes a cycle of the
 *  navigation state machine.
 * @author David Goodman
 * @date 2013.03.10  */
void Navigation_runSM();

/**
 * Function: Navigation_isReady
 * @return TRUE or FALSE if the Navigation system is ready.
 * @remark Returns TRUE if the GPS sub-system has a fix and a current 
 *  geodetic position.
 * @author David Goodman
 * @date 2013.03.10  */
BOOL Navigation_isReady();


// -------------------- Library Functions ---------------------------
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
    float lon_ref, float alt_ref);

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
void convertGeodetic2ECEF(Coordinate *var, float lat, float lon, float alt);

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
void convertECEF2Geodetic(Coordinate *var, float ecef_x, float ecef_y, float ecef_z);

/**
 * Function: convertEuler2NED
 * @param A pointer to a new NED coordinate variable to save result into.
 * @param Yaw in degrees from north.
 * @param Pitch in degrees from level.
 * @param Height in meters from target.
 * @return None.
 * @remark Projects a ray with the given height from the given yaw and
 *  pitch, and returns a NED for the intersection location.
 * @author David Goodman
 * @date 2013.03.10  */
void convertEuler2NED(Coordinate *var, float yaw, float pitch, float height);

#endif // Navigation_H

