/**
 * @file    Drive.h
 * @author  David Goodman
 * @author  Darrel Deo
 *
 * @brief
 * Drive system interface for the AtLAs.
 *
 * @details
 * This module uses three PWM lines to control the boat's drive
 * actuators, which are the rudder, and a left and right motor.
 *
 * @date March 27, 2013, 1:00 PM  -- Created
 */

#ifndef Drive_H
#define Drive_H

#include <stdint.h>
#include <stdbool.h>

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/


/***********************************************************************
 * PUBLIC TYPEDEFS
 ***********************************************************************/


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


/**********************************************************************
 * Function: Drive_init
 * @return SUCCESS or FAILURE.
 * @remark Initializes the drive system state machine.
 * @author David Goodman
 * @date 2013.03.27 
 **********************************************************************/
bool Drive_init(); 
  
/**********************************************************************
 * Function: Drive_runSM
 * @return None.
 * @remark Executes a cycle of the drive system state machine.
 * @author David Goodman
 * @date 2013.03.27 
 **********************************************************************/
void Drive_runSM();

/**********************************************************************
 * Function: Drive_forward
 * @return None
 * @param Speed to drive both motors forward in percent, from 0 to 100.
 * @remark Drives both motors at the given speed.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27 
 **********************************************************************/
void Drive_forward(uint8_t speed);

/**********************************************************************
 * Function: Drive_forwardHeading
 * @return None
 * @param Speed to drive at in meters per second.
 * @param Heading to hold in degrees from north, from 0 to 359.
 * @remark Tracks the given speed and heading.
 * @author David Goodman
 * @date 2013.03.30 
  **********************************************************************/
void Drive_forwardHeading(uint8_t speed, uint16_t angle);

/**********************************************************************
 * Function: Drive_stop
 * @return None
 * @remark Stops both motors from driving.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27 
 **********************************************************************/
void Drive_stop();

#endif // Drive_H

