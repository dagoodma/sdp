/* 
 * File:   I2C.h
 * Author: Shehadeh H. Dajani
 * Author: Darrel R. Deo
 *
 * Created on January 21, 2013, 3:42 PM
 */

/**
 * Function: I2C_startTransfer
 * @param I2C_ID, The I2C bus line that will be used
 * @param restart, determines whether you are sending a start or restart bit.
 * @return Success, TRUE or FALSE
 * @remark Sends a start or restart signal over the I2C bus to alert all
 * peripherals of an incoming transmission.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
BOOL I2C_startTransfer(I2C_MODULE I2C_ID, BOOL restart);

/**
 * Function: I2C_stopTransfer
 * @param I2C_ID, The I2C bus line that will be used
 * @return
 * @remark Sends the stop signal to terminate I2C communication.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
void I2C_stopTransfer(I2C_MODULE I2C_ID);

/**
 * Function: I2C_transmitOneByte
 * @param I2C_ID, The I2C bus line that will be used
 * @param data, The one-byte address to be sent over I2C
 * @return Success, TRUE or FALSE
 * @remark Sends a single byte over the I2C bus.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
BOOL I2C_transmitOneByte(I2C_MODULE I2C_ID, UINT8 data);

/**
 * Function: I2C_sendData
 * @param I2C_ID, The I2C bus line that will be used
 * @param data, The one-byte address to be sent over I2C
 * @return Success, TRUE or FALSE
 * @remark Uses I2C_transmitOneByte() to send a single byte of the I2C bus
 * and checks for an acknowledgement from the slave.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
BOOL I2C_sendData(I2C_MODULE I2C_ID, UINT8 data);

/**
 * Function: I2C_getData
 * @param I2C_ID, The I2C bus line that will be used
 * @return data, (short) single byte data from the slave.
 * @remark After sending the prerequisites for a read, this function will check
 * if the desired data is on the I2C bus line, read it, and return it.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
short I2C_getData(I2C_MODULE I2C_ID);

/**
 * Function: I2C_Init
 * @param I2C_ID, The I2C bus line that will be used
 * @param I2C_clockFreq, The desired frequency for the I2C bus
 * @return
 * @remark Turns on the I2C bus line specified and sets the frequency on it.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
void I2C_Init(I2C_MODULE I2C_ID, UINT32 I2C_clockFreq);
