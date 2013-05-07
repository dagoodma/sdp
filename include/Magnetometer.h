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
 * @return None.
 * @remark Initializes the Magnetometer interface
 * @author Shehadeh H. Dajani
 * @date 2013.03.10  */
void Magnetometer_init();


/**
 * Function: Magnetometer_runSM
 * @return None.
 * @remark Accumulates angles for Magnetometer calculates degrees.
 * @author Shehadeh H. Dajani
 * @date 2013.03.10  */
 void Magnetometer_runSM();

/**
 * Function: Magnetometer_getDegree
 * @return Degree.
 * @remark Returns magnetometer degrees
 * @author Shehadeh H. Dajani
 * @date 2013.03.10  */
float Magnetometer_getDegree();

/**
 * Function: Magnetometer_getDegree
 * @return Returns TRUE or FALSE if the Magnetometer is pointed very near
 *  North.
 * @remark
 * @author David Goodman
 * @date 2013.03.10  */
bool Magnetometer_isNorth();

#endif