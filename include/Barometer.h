/* 
 * File:   Barometer.h
 * Author: Shehadeh H. Dajani
 *
 * Created on January 21, 2013, 4:55 PM
 */

/**
 * Function: Barometer_getTemperatureData
 * @param
 * @return Data, (long) 16-bit temperature data
 * @remark Returns the pre-converted temperature data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
long Barometer_getTemperatureData(void);

/**
 * Function: Barometer_getPressureData
 * @param
 * @return Data, (long) 16-bit temperature data
 * @remark Returns the pre-converted temperature data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
long Barometer_getPressureData(void);

/**
 * Function: Barometer_getPressureData
 * @param
 * @return Data, (long) 16-bit temperature data
 * @remark Updates the barometer's temperature.
 * @author David Goodman
 * @date 2013.01.22  */
void Barometer_runSM();


