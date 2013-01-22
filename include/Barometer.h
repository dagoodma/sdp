/* 
 * File:   Barometer.h
 * Author: Shehadeh H. Dajani
 *
 * Created on January 21, 2013, 4:55 PM
 */

/**
 * Function: readData
 * @param Address, The desired address to to ping for a read.
 * @return Data, (short) returns a 16-bit variable containing the desired data.
 * @remark Does the entire read process by first sending the start bit.
 * Then the slave's address and the read address is sent. Finally, the
 * Master sends a restart bit and then reads the incoming data to return.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
short readData( unsigned char address);

/**
 * Function: readSensorValue
 * @param DataAddress, The I2C bus line that will be used
 * @return Data, (long) send back the temperature or pressure raw value
 * @remark Notifies the barometer to send back either temperature or pressure
 * data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
long readSensorValue(unsigned char dataAddress);

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
 * Function: convert
 * @param Temperature, a pointer to the temperature variable
 * @param Pressure, a pointer to the pressure variable
 * @return
 * @remark Gets the raw temperature and pressure readings and then converts
 * them into actual readings. The final readings are stored into the
 * temperature and pressure variables for future access.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
void convert(long* temperature, long* pressure);

