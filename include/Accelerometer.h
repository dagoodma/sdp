/*
 * File:   Accelerometer.h
 * Author: David Goodman
 *
 * Created on January 23, 2013, 1:24 PM
 */

#ifndef Accelerometer_H
#define Accelerometer_H

#include <stdint.h>

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**
 * Function: Accelerometer_init
 * @return SUCCESS or FAILURE.
 * @remark Initializes the module and makes contact with the accelerometer.
 * @author David Goodman
 * @date 2013.01.23  */
char Accelerometer_init();

/**
 * Function: Accelerometer_runSM
 * @return None.
 * @remark Steps into the module's state machine, which updates the x,y,z
 *      readings every time a timer expires.
 * @author David Goodman
 * @date 2013.01.23  */
void Accelerometer_runSM();

/**
 * Function: Accelerometer_getX
 * @return G-Count on the X-axis.
 * @remark 
 * @author David Goodman
 * @date 2013.01.23  */
int16_t Accelerometer_getX();

/**
 * Function: Accelerometer_getY
 * @return G-Count on the Y-axis.
 * @remark
 * @author David Goodman
 * @date 2013.01.23  */
int16_t Accelerometer_getY();

/**
 * Function: Accelerometer_getZ
 * @return G-Count on the Z-axis.
 * @remark 
 * @author David Goodman
 * @date 2013.01.23  */
int16_t Accelerometer_getZ();


#endif // Accelerometer_H