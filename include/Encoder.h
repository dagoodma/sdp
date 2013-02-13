/* 
 * File:   Encoder.h
 * Author: ddeo
 *
 * Created on January 28, 2013, 8:50 PM
 */

#ifndef ENCODER_H
#define	ENCODER_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ENCODER_H */




/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/
/**
 * Function: Encoder_init
 * @return None.
 * @remark Initializes the Encoder interface and enables interrupts for input capture/compare1.
 * @author Darrel R. Deo
 * @date 2013.02.10  */
void Encoder_init();


/*******************************************************************************
 * Public Functions                                                            *
 ******************************************************************************/
/**
 * Function: calculate_Angle
 * @return floating point of the angle for approximate.
 * @remark Calculates the angle of the encoder magnet.
 * @author Darrel R. Deo
 * @date 2013.02.10  */
float calculate_Angle(uint16_t pwidth);