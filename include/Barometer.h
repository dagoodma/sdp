/**
 * @file    Barometer.h
 * @author  Shehadeh H. Dajani
 * @author  David Goodman
 *
 * @brief
 * Sensor interface for the barometer.
 *
 * @details
 * Module that wraps the barometer sensor in a state machine
 * that ocassionally takes readings over an I2C bus.

 * @date January 21, 2013, 4:55 PM -- Created
 */


#ifndef Barometer_H
#define Barometer_H


/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**
 * Function: Barometer_getTemperatureData
 * @return Data, (long) 16-bit temperature data
 * @remark Returns the pre-converted temperature data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
long Barometer_getTemperatureData(void);

/**
 * Function: Barometer_getPressureData
 * @return Data, (long) 16-bit temperature data
 * @remark Returns the pre-converted temperature data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
long Barometer_getPressureData(void);

/**
 * Function: Barometer_getPressureData
 * @return Data, (long) 16-bit temperature data
 * @remark Updates the barometer's temperature.
 * @author David Goodman
 * @date 2013.01.22  */
void Barometer_runSM();

#endif // Barometer_H
