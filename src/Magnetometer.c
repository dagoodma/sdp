/* 
 * File:   Magnetometer.c
 * Author: David Goodman, Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */
#define DEBUG

#include <xc.h>
#include <stdio.h>
#include <math.h>
#include <plib.h>
#include <stdbool.h>
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
#define ACCUMULATOR_LENGTH          1

#define MAGNETIC_NORTH_OFFSET       13.7275f // (deg) offset eastward from true north
#define DEGREE_1E1_TO_DEGREE(d)    ((float)d/10.0f)

#define MINIMUM_NORTH_ERROR         1.2f // (degrees) minimum error from North

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      MAGNETOMETER_I2C_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

static uint16_t accumulatorIndex;
static uint32_t accumulator;
static float heading; // (degrees)

static bool haveReading;

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

static uint16_t readSensor();
static void calculateHeading();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

void Magnetometer_init() {
    accumulatorIndex = 0;
    accumulator = 0.0f;
    haveReading = FALSE;
}

float Magnetometer_getHeading(){
    return heading;
}

void Magnetometer_runSM(){

    if (accumulatorIndex < ACCUMULATOR_LENGTH) {
        accumulator += readSensor();
        accumulatorIndex++;
    }

    if (accumulatorIndex >= ACCUMULATOR_LENGTH) {
        calculateHeading();
        accumulatorIndex = 0;
        accumulator = 0.0f;
        haveReading = TRUE;
    }
}


bool Magnetometer_isNorth() {
    return haveReading && (heading <= MINIMUM_NORTH_ERROR
            || heading >= (360.0 - MINIMUM_NORTH_ERROR));
}

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

static void calculateHeading() {
    heading = (float)accumulator/ACCUMULATOR_LENGTH;
    heading = DEGREE_1E1_TO_DEGREE(heading);

    // Remove magnetic north offset
    heading -= MAGNETIC_NORTH_OFFSET;

    // Bound heading
    if (heading > 360.0f)
        heading -= 360.0f;
    else if (heading < 0.0f)
        heading += 360.0f;

}
 

static uint16_t readSensor() {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID, I2C_WRITE )){
            DBPRINT("Magnetometer: FAILED initial transfer!\n");
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(MAGNETOMETER_I2C_ID,SLAVE_DEGREE_ADDRESS)){
            DBPRINT("Magnetometer: Error: Sent byte was not acknowledged\n");
            break;
        }
        // Send a Repeated Started condition
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID,I2C_READ)){
            DBPRINT("Magnetometer: FAILED Repeated start!\n");
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
        DBPRINT("Magnetometer: Data transfer unsuccessful.\n");
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
