/*
 * File:   I2C.h
 * Author: ddeo
 *
 * Created on January 18, 2013, 3:42 PM
 */

#include <stdio.h>
#include <plib.h>
#include "I2C.h"



/*********************************************************************
 * FUNCTION:
 *  BOOl I2C_Start_Comm
 *
 * Summary:
 *  Initiates the Start Sequence command
 *
 * Description:
 *  This function sends a start (Hi-Lo on SDA line)
 *
 *
 *
 *
 *
 *
 * *******************************************************************/

BOOL I2C_startComm(BOOL restart)
{
    //I2C explicit status for error checking
    I2C_STATUS status;
    I2C_RESULT result;

    //If you are repeating a start signal in the event of reading
    //from a device.
    if(restart)
    {
        I2CRepeatStart(I2C1);
    }
    //If you are not repeating a start signal (first time initiation)
    else
    {
        //Wait for the I2C bus to be idle
        while(!I2CBusIsIdle(I2C1))
        {
            ;
        }

        //When the SDA line is idle, initiate a start sequence
        result = I2CStart(I2C1);
        
        //If an error arrises, it is because the line is not idle
        if (result != I2C_SUCCESS){
            DBPRINTF("ERROR: The SDA line is not idle\n");
            return FALSE;
        }
    }
    //We wait for a confirmation that the start condition has been
    //detected
    while (!(I2CGetStatus(I2C1) & I2C_START))
    {
        ;
    }

    return TRUE;
}


BOOL I2C_stopComm(void)
{
    I2C_STATUS status;
    I2C_RESULT result;

    //Initiate stop signal
    I2CStop(I2C1);

    //Confirm that stop request has been detected
    while(!(I2CGetStatus(I2C1) & I2C_STOP))
    {
        ;
    }

    return TRUE;


}


BOOL I2C_sendByte(UINT8 data)
{

    I2C_RESULT result;

    //Check if the transmitter is ready to send out data
    while(I2CTransmitterIsReady(I2C1) != TRUE)
    {
        ;
    }

    //When transmitter is ready, send a byte
    result = I2CSendByte(I2C1,data);

    //check for error if I2C wasn't able to send a byte
    if (result != I2C_SUCCESS)
    {
        DBPRINTF("ERROR: Master device was not able to send a byte to slave.\n");
        return FALSE;
    }

    //check to see if an acknowledgement was sent by the slave device









}