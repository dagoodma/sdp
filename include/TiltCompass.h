/**
 * @file    TiltCompass.h
 * @author  Shehadeh Dajani
 * @author  David Goodman
 *
 * @brief
 * Sensor interface for tilt compensated compass.
 *
 * @details 
 * Module for receiving compass heading from magnetic north (CW). This
 * module uses I2C to talk to a tilt compensated compass (Honeywell 
 * HMC6343), which consists of a 3-axis magnetometer, a 3-axis acceler-
 * ometer, and an embedded microcontroller.
 *
 * @note
 *  An accumulator is also implemented.
 *	Device can correct for magnet vs. true north offset (not implemented).
 * 
 * @date April 5, 2013, 9:32 PM   -- Created.
 */
#ifndef TiltCompass_H
#define TiltCompass_H

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

/***********************************************************************
 * PUBLIC FUNCTIONS
 ***********************************************************************/

/**********************************************************************
 * Function: TiltCompass_init
 * @return None
 * @remark Waits for a startup delay. Note, I2C bus should have already
 *	 been initialized.
 **********************************************************************/
void TiltCompass_init();
 
/**********************************************************************
 * Function: TiltCompass_runSM
 * @return None
 * @remark Takes a reading from the magnetometer and accumulates.
 **********************************************************************/
void TiltCompass_runSM();

/**********************************************************************
 * Function: TiltCompass_getheading
 * @return Heading from north in degrees from 0 to 360.
 * @remark 
 **********************************************************************/
float TiltCompass_getheading();
 
 #endif