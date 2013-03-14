/* 
 * File:   Encoder.h
 * Author: sdajani
 *
 * Created on January 28, 2013, 8:50 PM
 */

#ifndef ENCODER_H
#define	ENCODER_H


/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/

/**
 * Function: Encoder_init
 * @return None.
 * @remark Initializes the Encoder interface over I2C. Note that the I2C module
 *  should have already been initialized.
 * @author Shehadeh H. Dajani
 * @date 2013.02.10  */
void Encoder_init();


/**
 * Function: Encoder_runSM
 * @return None.
 * @remark Accumulates angles for both encoders and calculates distances.
 *  TODO: BREAK UP INTO STEPS
 *  TODO: Absorb button presses into this function and make true state machine.
 * @author David Goodman
 * @date 2013.02.10  */
void Encoder_runSM();

/**
 * Function: Encoder_setZeroAngle
 * @return None.
 * @remark Zeros both encoders at the current positions.
 * @author David Goodman
 * @date 2013.02.10  */
void Encoder_setZeroAngle();

/**
 * Function: Encoder_isZeroPressed
 * @return TRUE or FALSE of whether the lock button was pressed or not.
 * @remark Lock button is pressed to acquire a target and send the bot a
 *  goto coordinate message.
 * @author Shehadeh H. Dajani
 * @date 2013.02.10  */
BOOL Encoder_isLockPressed();

/**
 * Function: Encoder_isZeroPressed
 * @return TRUE or FALSE of whether the button was pressed or not
 * @remark Event Checker Routine that checks state of pushbutton for Lock on
 * @author Shehadeh H. Dajani
 * @date 2013.02.10  */
BOOL Encoder_isZeroPressed();

/**
 * Function: Encoder_getPitch
 * @return Current angle of pitch encoder in decimal degrees.
 * @remark 
 * @author David Goodman
 * @date 2013.03.10  */
float Encoder_getPitch();

/**
 * Function: Encoder_getYaw
 * @return Current angle of yaw encoder in decimal degrees.
 * @remark 
 * @author David Goodman
 * @date 2013.03.10  */
float Encoder_getYaw();

#endif
