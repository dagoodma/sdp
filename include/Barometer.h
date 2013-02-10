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
 * Function: Barometer_init
 * @return None
 * @remark Intializes the Barometer and state machine.
 * @author David Goodman
 * @date 2013.02.01  */
void Barometer_init();

/**
 * Function: Barometer_getTemperature
 * @return Temperature in 1E1 celcius.
 * @remark Returns the temperature measured by the barometer in 10's of
 *      degerees C.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
float Barometer_getTemperatureFahrenheit();

/**
 * Function: Barometer_getTemperatureFahrenheit
 * @return Temperature in fahrenheit.
 * @remark Returns the temperature measured by the barometer in degrees
 *      fahrenheit.
 * @author David Goodman
 * @date 2013.02.01  */
float Barometer_getTemperatureFahrenheit();

/**
 * Function: Barometer_getPressure
 * @return Pressure in pascals.
 * @remark Returns the pre-converted temperature data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
int32_t Barometer_getPressure();

/**
 * Function: Barometer_getAltitude
 * @return Returns the altitude in meters.
 * @remark Converts pressure from altitude above sea level in meters. 
 * @author David Goodman
 * @date 2013.02.01  */
int32_t Barometer_getAltitude();
/**
 * Function: Barometer_getPressureData
 * @return Data, (long) 16-bit temperature data
 * @remark Updates the barometer's temperature.
 * @author David Goodman
 * @date 2013.01.22  */
void Barometer_runSM();

#endif // Barometer_H
