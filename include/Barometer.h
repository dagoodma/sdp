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

#define FAHRENHEIT_TO_CELCIUS(T)    (((float)T * (9/5)) + 32.0)
#define CELCIUS_TO_FAHRENHEIT(T)    (((float)T/10)*1.8 + 32.0)
#define METERS_TO_FEET(m)           ((float)m * 3.28084f)

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**********************************************************************
 * Function: Barometer_init
 * @return SUCCESS or FAILURE.
 * @remark Intializes the Barometer and state machine.
 * @author David Goodman
 * @date 2013.02.01 
 **********************************************************************/
char Barometer_init();


/**********************************************************************
 * Function: Barometer_runSM
 * @return None
 * @remark Steps into the barometer's state machine, which updates the 
 *  temperature and pressure/altitude data.
 * @author David Goodman
 * @date 2013.01.22 
 **********************************************************************/
void Barometer_runSM();


/**********************************************************************
 * Function: Barometer_getTemperature
 * @return Temperature in Celcius.
 * @remark Returns the temperature measured by the barometer in degerees Celsius.
 * @author Shehadeh H. Dajani
 * @author David Goodman
 * @date 2013.01.21  
 **********************************************************************/
float Barometer_getTemperature();


/**********************************************************************
 * Function: Barometer_getTemperatureFahrenheit
 * @return Temperature in Fahrenheit.
 * @remark Returns the temperature measured by the barometer in degrees Fahrenheit.
 * @author Shehadeh H. Dajani
 * @author David Goodman
 * @date 2013.02.01 
 **********************************************************************/
float Barometer_getTemperatureFahrenheit();


/**********************************************************************
 * Function: Barometer_getPressure
 * @return Pressure in pascals.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21 
 **********************************************************************/
int32_t Barometer_getPressure();


/**********************************************************************
 * Function: Barometer_getAltitude
 * @return Returns the altitude in meters.
 * @remark Converts pressure into altitude above sea level in meters 
 *  using a hard-coded reference pressure, P_0.
 * @author David Goodman
 * @date 2013.02.01 
 **********************************************************************/
float Barometer_getAltitude();

#endif // Barometer_H
