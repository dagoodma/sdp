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
 * @remark Initializes the Encoder interface and configures interrupts for input capture/compare1. DOES NOT ENABLE INTERRUPT.
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
 * Function: Is_lockOnButtonPressed
 * @return TRUE or FALSE of whether the lock button was pressed or not.
 * @remark Lock button is pressed to acquire a target and send the bot a
 *  goto coordinate message.
 * @author Shehadeh H. Dajani
 * @date 2013.02.10  */
 BOOL Encoder_isLockPressed();

 /**
 * Function: Is_zeroOnButtonPressed
 * @return TRUE or FALSE of whether the button was pressed or not
 * @remark Event Checker Routine that checks state of pushbutton for Lock on
 * @author Shehadeh H. Dajani
 * @date 2013.02.10  */
 BOOL Encoder_isZeroPressed();

  /**
 * Function: Encoder_getVerticalDistance
 * @return Vertical distance in feet to current target.
 * @remark Given height, does a tangent calculation of vertical distance.
 * @author Shehadeh Dajani
 * @date 2013.02.10  */
 float Encoder_getVerticalDistance(float height);

 /**
 * Function: Encoder_getHorizontalDistance
 * @param Vertical distance to target in feet (obtained with
 *  Encoder_getVerticalDistance()).
 * @return Horizontal distance in feet to the target.
 * @remark Calculates the horizontal distance to the target using the
 *  given vertical distance.
 * @author Shehadeh Dajani
 * @author David Goodman
 * @date 2013.02.10  */
float Encoder_getHorizontalDistance(float verticalDistance);

#endif