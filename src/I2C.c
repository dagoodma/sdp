/*
 * File:   I2C.c
 * Author: Shehadeh H. Dajani
 * Author: Darrel R. Deo
 *
 *
 * Created on January 18, 2013, 3:42 PM
 */
#include <p32xxxx.h>
#include <stdio.h>
#include <plib.h>

BOOL I2C_startTransfer(I2C_MODULE I2C_ID, BOOL restart){
    I2C_STATUS  status;

// Send the Start (or Restart) signal
    if(restart){
        if(I2CRepeatStart(I2C_ID) != I2C_SUCCESS){
            printf("Error: Bus collision during transfer Start\n");
            return FALSE;
        }
    }
    else{
    // Wait for the bus to be idle, then start the transfer
        while( !I2CBusIsIdle(I2C_ID) );
        if(I2CStart(I2C_ID) != I2C_SUCCESS){
            printf("Error: Bus collision during transfer Start\n");
            return FALSE;
        }
    }
// Wait for the signal to complete
    do{
        status = I2CGetStatus(I2C_ID);
    }
    while (!(status & I2C_START) );

    return TRUE;
}

void I2C_stopTransfer(I2C_MODULE I2C_ID){
    I2C_STATUS  status;

// Send the Stop signal
    I2CStop(I2C_ID);

// Wait for the signal to complete
    do{
        status = I2CGetStatus(I2C_ID);
    }
    while (!(status & I2C_STOP));
}

BOOL I2C_transmitOneByte(I2C_MODULE I2C_ID, UINT8 data){

// Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(I2C_ID));

// Transmit the byte and check for bus collision
    if(I2CSendByte(I2C_ID, data) == I2C_MASTER_BUS_COLLISION){
        printf("Error: I2C Master Bus Collision\n");
        return FALSE;
    }

// Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(I2C_ID));
    
    return TRUE;
}

BOOL I2C_sendData(I2C_MODULE I2C_ID, UINT8 data){
    BOOL Success = TRUE;

// Initiate a single byte transmit over the I2C bus
    if (!I2C_transmitOneByte(I2C_ID,data)){
        Success = FALSE;
    }

// Verify that the byte was acknowledged
    if(!I2CByteWasAcknowledged(I2C_ID)){
        printf("Error: Sent byte was not acknowledged\n");
        Success = FALSE;
    }
    return Success;
}

short I2C_getData(I2C_MODULE I2C_ID){
    BOOL Success = TRUE;

// Enables the module to receive data from the I2C bus
    if(I2CReceiverEnable(I2C_ID, TRUE) == I2C_RECEIVE_OVERFLOW){
        printf("Error: I2C Receive Overflow\n");
        Success = FALSE;
    }
    else{
    //wait until data is available
        while(!I2CReceivedDataIsAvailable(I2C_ID));
    //get a byte of data received from the I2C bus.
        return I2CGetByte(I2C_ID);
    }
}

void I2C_Init(I2C_MODULE I2C_ID, UINT32 I2C_clockFreq){

// Configure Various I2C Options
    I2CConfigure(I2C_ID, I2C_EN);

// Set Desired Operation Frequency
    I2CSetFrequency(I2C_ID, 40000000L, I2C_clockFreq);
}