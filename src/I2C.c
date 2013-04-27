/*
 * File:   I2C.c
 * Author: Shehadeh H. Dajani
 * Author: Darrel R. Deo
 * @author David Goodman
 *
 *
 * Created on January 18, 2013, 3:42 PM
 */
//include <p32xxxx.h>
#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <stdint.h>


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


void I2C_acknowledgeRead(I2C_MODULE I2C_ID, bool ack) {
    I2CAcknowledgeByte(I2C_ID, ack);
}

bool I2C_hasAcknowledged(I2C_MODULE I2C_ID) {
    I2CAcknowledgeHasCompleted(I2C_ID);
}

bool I2C_startTransfer(I2C_MODULE I2C_ID, bool restart){
    I2C_STATUS  status;

// Send the Start (or Restart) signal
    if(restart){
        if(I2CRepeatStart(I2C_ID) != I2C_SUCCESS){
            #ifdef DEBUG
            printf("Error: Bus collision during transfer Start at Read\n");
            #endif
            return FALSE;
        }
    }
    else{
    // Wait for the bus to be idle, then start the transfer
        while( !I2CBusIsIdle(I2C_ID) );
        if(I2CStart(I2C_ID) != I2C_SUCCESS){
            #ifdef DEBUG
            printf("Error: Bus collision during transfer Start at Write\n");
            #endif
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

bool I2C_transmitOneByte(I2C_MODULE I2C_ID, uint8_t data) {

    // Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(I2C_ID));

    // Transmit the byte and check for bus collision
    if(I2CSendByte(I2C_ID, data) == I2C_MASTER_BUS_COLLISION){
        #ifdef DEBUG
        printf("Error: I2C Master Bus Collision\n");
        #endif
        return FALSE;
    }

// Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(I2C_ID));
    
    return TRUE;
}

bool I2C_sendData(I2C_MODULE I2C_ID, uint8_t data){
    // Initiate a single byte transmit over the I2C bus
    if (!I2C_transmitOneByte(I2C_ID,data)){
        return FALSE;
    }

    // Verify that the byte was acknowledged
    if(!I2CByteWasAcknowledged(I2C_ID)){
        #ifdef DEBUG
        printf("Error: Sent byte was not acknowledged\n");
        #endif
        return FALSE;
    }
    return TRUE;
}

int16_t I2C_getData(I2C_MODULE I2C_ID){
    bool Success = TRUE;

        // Enables the module to receive data from the I2C bus
    if(I2CReceiverEnable(I2C_ID, TRUE) == I2C_RECEIVE_OVERFLOW){
        #ifdef DEBUG
        printf("Error: I2C Receive Overflow\n");
        #endif
        Success = FALSE;
    }
    else{
    //wait until data is available
        while(!I2CReceivedDataIsAvailable(I2C_ID));
    //get a byte of data received from the I2C bus.
        return I2CGetByte(I2C_ID);
    }
    return Success;
}

void I2C_init(I2C_MODULE I2C_ID, uint32_t I2C_clockFreq) {

    // Configure Various I2C Options
    I2CConfigure(I2C_ID, I2C_EN);

    // Set Desired Operation Frequency
    I2CSetFrequency(I2C_ID, Board_GetPBClock(), I2C_clockFreq);
}