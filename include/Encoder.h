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
 * Function: Encoder_setZeroPitch
 * @return None.
 * @remark Zeros the pitch encoder at the current angle.
 * @author David Goodman
 * @date 2013.02.10  */
void Encoder_setZeroPitch();

/**
 * Function: Encoder_setZeroYaw
 * @return None.
 * @remark Zeros the yaw encoder at the current angle.
 * @author David Goodman
 * @date 2013.02.10  */
void Encoder_setZeroYaw();

/**
 * Function: Encoder_enableZeroAngle
 * @return 
 * @remark Uses zero angle when calculating encoder angles, and is
 *  enabled by default once each are set.
 * @author David Goodman
 * @date 2013.03.14  */
void Encoder_enableZeroAngle();

/**
 * Function: Encoder_disableZeroAngle
 * @return 
 * @remark Disables zero angle when calculating encoder angles.
 * @author David Goodman
 * @date 2013.03.14  */
void Encoder_disableZeroAngle();

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
