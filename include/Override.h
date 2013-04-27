/**
 * @file    Override.h
 * @author  David Goodman
 * @author  Darrel Deo
 *
 * @brief
 * Interface for the receiver override board.
 *
 * @details
 * This module provides an interface for the receiver override board. 
 * The override board is used to to pass control of the motors and 
 * rudder between the receiver and the microcontroller. The caller can
 * also determine if the receiver is online.
 * 
 * @date April 25, 2013, 7:13 PM  -- Created
 */

#ifndef Override_H
#define Override_H

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**********************************************************************
 * Function: Override_init
 * @return None
 * @remark Initializes the override board and the interrupt to detect
 *  when the receiver comes online.
 * @author Darrel Deo
 * @date 2013.04.01  
 **********************************************************************/
void Override_init();


/**********************************************************************
 * Function: Override_giveReceiverControl
 * @return None
 * @remark Passes motor control over to the receiver.
 * @author David Goodman
 * @date 2013.04.01 
 **********************************************************************/
void Override_giveReceiverControl();


/**********************************************************************
 * Function: Override_giveMicroControl
 * @return None
 * @remark Passes motor control over to the micro.
 * @author David Goodman
 * @date 2013.04.01  
 **********************************************************************/
void Override_giveMicroControl();


/**********************************************************************
 * Function: Override_isTriggered
 * @return TRUE or FALSE if the receiver has come online.
 * @remark The receiver will only be considered offline (FALSE), if 
 *  it is turned off for longer and 2 seconds.
 * @author David Goodman
 * @date 2013.04.01  
 **********************************************************************/
BOOL Override_isTriggered();


#endif // Override_H


