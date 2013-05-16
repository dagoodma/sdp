/* 
 * File:   Magnetometer.h
 * Author: sdajani
 *
 * Created on January 28, 2013, 8:50 PM
 */

#ifndef MAGNETOMETER_H
#define	MAGNETOMETER_H


/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/

/**
 * Function: Magnetometer_init
 * @return SUCCESS or FAILURE.
 * @remark Initializes the Magnetometer sensor.
 * @author David Goodman
 * @author Shehadeh H. Dajani
 * @date 2013.03.10  */
char Magnetometer_init();


/**
 * Function: Magnetometer_runSM
 * @return None.
 * @remark Accumulates angles for Magnetometer calculates degrees.
 * @author David Goodman
 * @author Shehadeh H. Dajani
 * @date 2013.03.10  */
 void Magnetometer_runSM();

/**
 * Function: Magnetometer_getHeading
 * @return Degree.
 * @remark Returns magnetometer's measured heading in degrees, accurate to
 *  0.5 degrees.
 * @author David Goodman
 * @author Shehadeh H. Dajani
 * @date 2013.03.10  */
float Magnetometer_getHeading();

/**
 * Function: Magnetometer_isNorth
 * @return Returns TRUE or FALSE if the Magnetometer is pointed very near
 *  North.
 * @remark
 * @author David Goodman
 * @date 2013.03.10  */
bool Magnetometer_isNorth();

#endif