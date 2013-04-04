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


#endif // Navigation_H

