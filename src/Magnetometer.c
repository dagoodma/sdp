/* 
 * File:   Encoder_I2C.c
 * Author: Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */

#include <xc.h>
#include <stdio.h>
#include <math.h>
#include <plib.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Encoder
#define SLAVE_READ_ADDRESS          0x43
#define SLAVE_WRITE_ADDRESS         0x42
#define SLAVE_DEGREE_ADDRESS        0x41
#define ACCUMULATOR_LENGTH          50

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      MAGNETOMETER_I2C_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

uint16_t Degree;
float finalDegree; // (degrees)

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

uint16_t Magnetometer_readSensor();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

void Magnetometer_init() {
    // Do Nothing
}

float Magnetometer_getDegree(){
    return finalDegree;
}

void Magnetometer_runSM(){
     int count;
     float accumulator = 0;
     for(count = 0; count < ACCUMULATOR_LENGTH; count++){
        Degree = Magnetometer_readSensor();
        if(Degree < 0020)
            Degree += 3600;
        accumulator += Degree;
    }
    finalDegree = (float)(accumulator/(ACCUMULATOR_LENGTH*10));
    if(finalDegree > 360)
        finalDegree -= 360;
    if(finalDegree < 0.5 || finalDegree > 359.5)
        finalDegree = 0;
 }

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

 

uint16_t Magnetometer_readSensor() {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID, I2C_WRITE )){
            #ifdef DEBUG
            printf("FAILED initial transfer!\n");
            #endif
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(MAGNETOMETER_I2C_ID,SLAVE_DEGREE_ADDRESS)){
            #ifdef DEBUG
            printf("Error: Sent byte was not acknowledged\n");
            #endif
            break;
        }
        // Send a Repeated Started condition
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID,I2C_READ)){
            #ifdef DEBUG
            printf("FAILED Repeated start!\n");
            #endif
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_READ_ADDRESS))
            break;
        
        // Read the I2C bus twice
        data = (I2C_getData(MAGNETOMETER_I2C_ID) << 8);
        I2C_acknowledgeRead(MAGNETOMETER_I2C_ID, TRUE);
        while(!I2C_hasAcknowledged(MAGNETOMETER_I2C_ID));
        data += (I2C_getData(MAGNETOMETER_I2C_ID));

        // Send the stop bit to finish the transfer
        I2C_stopTransfer(MAGNETOMETER_I2C_ID);

        success = TRUE;
    } while(0);
    if (!success) {
        #ifdef DEBUG
        printf("Data transfer unsuccessful.\n");
        #endif
        return FALSE;
    }
    return data;
}

//#define MAGNETOMETER_TEST
#ifdef MAGNETOMETER_TEST

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Serial_init();
    I2C_init(MAGNETOMETER_I2C_ID, I2C_CLOCK_FREQ);
    Magnetometer_init();
    while(1){
        Magnetometer_runSM();
        while(!Serial_isTransmitEmpty());
        printf("Angle: %.1f\n", Magnetometer_getDegree());
    }

    return (SUCCESS);
}

#endif
