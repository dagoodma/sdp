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
// Printing debug messages over serial
//#define DEBUG

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <stdint.h>
#include <stdbool.h>
#include "Board.h"
#include "Timer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define I2C_TIMEOUT_DELAY   900 // (ms) till fail to start transfer
/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

static bool hasError;

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


void I2C_acknowledgeRead(I2C_MODULE I2C_ID, bool ack) {
    I2CAcknowledgeByte(I2C_ID, ack);
}

bool I2C_hasAcknowledged(I2C_MODULE I2C_ID) {
    I2CAcknowledgeHasCompleted(I2C_ID);
}

bool I2C_waitForAcknowledgement(I2C_MODULE I2C_ID) {
    Timer_new(TIMER_I2C_TIMEOUT, I2C_TIMEOUT_DELAY);
    while (!I2CAcknowledgeHasCompleted(I2C_ID)) {
        if (Timer_isExpired(TIMER_I2C_TIMEOUT)) {
            DBPRINT("I2C: Timed out waiting for acknowledgement.\n");
            hasError = TRUE;
            return FALSE;
        }
    }
    return TRUE;
}


bool I2C_startTransfer(I2C_MODULE I2C_ID, bool restart){
    I2C_STATUS  status;

// Send the Start (or Restart) signal
    if(restart){
        if(I2CRepeatStart(I2C_ID) != I2C_SUCCESS){
            DBPRINT("I2C: Bus collision during transfer start at read.\n");
            hasError = TRUE;
            return FALSE;
        }
    }
    else{
    // Wait for the bus to be idle, then start the transfer
        Timer_new(TIMER_I2C_TIMEOUT, I2C_TIMEOUT_DELAY);
        while( !I2CBusIsIdle(I2C_ID) ) {
            if (Timer_isExpired(TIMER_I2C_TIMEOUT)) {
                DBPRINT("I2C: Timed out waiting for bus to be idle.\n");
                hasError = TRUE;
                return FALSE;
            }
        }
        if(I2CStart(I2C_ID) != I2C_SUCCESS){
            DBPRINT("I2C: Bus collision during transfer start at write.\n");
            hasError = TRUE;
            return FALSE;
        }
    }
    // Wait for the signal to complete
    Timer_new(TIMER_I2C_TIMEOUT, I2C_TIMEOUT_DELAY);
    do{
        status = I2CGetStatus(I2C_ID);
        if (Timer_isExpired(TIMER_I2C_TIMEOUT)) {
            DBPRINT("I2C: Timed out waiting for signal to complete.\n");
            hasError = TRUE;
            return FALSE;
        }
    }while (!(status & I2C_START) );

    return TRUE;
}

void I2C_stopTransfer(I2C_MODULE I2C_ID){
    I2C_STATUS  status;

    // Send the Stop signal
    I2CStop(I2C_ID);

    // Wait for the signal to complete
    Timer_new(TIMER_I2C_TIMEOUT, I2C_TIMEOUT_DELAY);
    do{
        status = I2CGetStatus(I2C_ID);
        if (Timer_isExpired(TIMER_I2C_TIMEOUT)) {
            DBPRINT("I2C: Timed out waiting for stop transfer to complete.\n");
            hasError = TRUE;
            return;
        }
    }while (!(status & I2C_STOP));
}

bool I2C_transmitOneByte(I2C_MODULE I2C_ID, uint8_t data) {

    // Wait for the transmitter to be ready
    Timer_new(TIMER_I2C_TIMEOUT, I2C_TIMEOUT_DELAY);
    while(!I2CTransmitterIsReady(I2C_ID)) {
        if (Timer_isExpired(TIMER_I2C_TIMEOUT)) {
            DBPRINT("I2C: Timed out waiting for transmitter to be ready.\n");
            hasError = TRUE;
            return FALSE;
        }
    }

    // Transmit the byte and check for bus collision
    if(I2CSendByte(I2C_ID, data) == I2C_MASTER_BUS_COLLISION){
        DBPRINT("I2C: Master bus collision occurred.\n");
        hasError = TRUE;
        return FALSE;
    }

    // Wait for the transmission to finish
    Timer_new(TIMER_I2C_TIMEOUT, I2C_TIMEOUT_DELAY);
    while(!I2CTransmissionHasCompleted(I2C_ID)) {
        if (Timer_isExpired(TIMER_I2C_TIMEOUT)) {
            DBPRINT("I2C: Timed out waiting for transmission to finish.\n");
            hasError = TRUE;
            return FALSE;
        }
    }
    
    return TRUE;
}

bool I2C_sendData(I2C_MODULE I2C_ID, uint8_t data){
    // Initiate a single byte transmit over the I2C bus
    if (!I2C_transmitOneByte(I2C_ID,data)){
        return FALSE;
    }

    // Verify that the byte was acknowledged
    if(!I2CByteWasAcknowledged(I2C_ID)){
        DBPRINT("I2C: Sent byte was not acknowledged.\n");
        hasError = TRUE;
        return FALSE;
    }
    return TRUE;
}

int16_t I2C_getData(I2C_MODULE I2C_ID){
    bool Success = TRUE;

        // Enables the module to receive data from the I2C bus
    if(I2CReceiverEnable(I2C_ID, TRUE) == I2C_RECEIVE_OVERFLOW){
        DBPRINT("I2C: Received and overflow.\n");
        hasError = TRUE;
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
    hasError = FALSE;

    // Configure Various I2C Options
    I2CConfigure(I2C_ID, I2C_EN);

    // Set Desired Operation Frequency
    I2CSetFrequency(I2C_ID, Board_GetPBClock(), I2C_clockFreq);
}

bool I2C_hasError() {
    bool result = hasError;
    hasError = FALSE;
    return result;
}