/* 
 * File:   TiltCompass.c
 * Author: Shehadeh, Dagoodma
 *
 * Created on April 5, 2013, 9:32 PM
 */

#include <xc.h>
#include <stdio.h>
#include <math.h>
#include <plib.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"
#include "Timer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Compass
#define SLAVE_READ_ADDRESS          0x33
#define SLAVE_WRITE_ADDRESS         0x32
#define SLAVE_DEGREE_ADDRESS        0x50

//#define USE_ACCUMULATOR
#define ACCUMULATOR_LENGTH          1
#define STARTUP_DELAY               500
#define REFRESH_DELAY               200

#define HEADING_1E1_TO_DEGREES(heading) 	((float)heading/10.0)

#define MAGNETIC_NORTH_OFFSET       13.7275f // (deg) offset eastward from true north
/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
static I2C_MODULE      TILT_COMPASS_I2C_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

static uint16_t accumulatorIndex = 0;
static float headingAccumulator = 0;
static float finalHeading = 0; // (degrees)

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

uint16_t readSensor();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
 
/**********************************************************************
 * Function: TiltCompass_init
 * @return SUCCESS or FAILURE.
 * @remark Waits for a startup delay. Note, I2C bus should have already
 *	 been initialized.
 **********************************************************************/
bool TiltCompass_init() {
    Timer_new(TIMER_TILTCOMPASS,STARTUP_DELAY);
	
    while(!Timer_isExpired(TIMER_TILTCOMPASS)) {
        // Do nothing
        asm("nop");
    }

    return SUCCESS;
}
 
/**********************************************************************
 * Function: TiltCompass_getHeading
 * @return Heading from north in degrees from 0 to 360.
 * @remark 
 **********************************************************************/
float TiltCompass_getHeading(){
    return finalHeading;
}


/**********************************************************************
 * Function: TiltCompass_runSM
 * @return None
 * @remark Takes a reading from the magnetometer and accumulates.
 **********************************************************************/
void TiltCompass_runSM() {
    if(Timer_isExpired(TIMER_TILTCOMPASS)) {
#ifdef USE_ACCUMULATOR
        // Taking reading from the magnetometer and accumulate it
        uint16_t heading = readSensor();
        if(heading < 0020)
            heading += 3600;
        headingAccumulator += (float)(readSensor());
        accumulatorIndex++;
        Timer_new(TIMER_TILTCOMPASS,REFRESH_DELAY);
		
        if (accumulatorIndex >= ACCUMULATOR_LENGTH) {
            // Calculate final heading and reset accumulator
            #if ACCUMULATOR_LENGTH > 1
            finalHeading = ((float)headingAccumulator/ACCUMULATOR_LENGTH);
            #else
            finalHeading = (float)headingAccumulator;
            #endif
            finalHeading = HEADING_1E1_TO_DEGREES(finalHeading) - MAGNETIC_NORTH_OFFSET;
            if(finalHeading > 360.0f)
                finalHeading -= 360.0f;
            if (finalHeading < 0.0f)
                finalHeading += 360.0f;
            if (finalHeading < 0.5f || finalHeading > 359.5f)
                finalHeading = 0.0f;

            headingAccumulator = 0.0f;
            accumulatorIndex = 0;
        }
#else
        finalHeading = HEADING_1E1_TO_DEGREES(readSensor());
        finalHeading -= MAGNETIC_NORTH_OFFSET;
        if (finalHeading < 0.0f)
                finalHeading += 360.0f;
#endif
    }
}
    

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

 

/**********************************************************************
 * Function: readSensor
 * @return Heading from north in 1E1 degrees.
 * @remark Reads two bytes from the tilt compensated magnetometer over the I2C bus.
 **********************************************************************/
uint16_t readSensor() {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(TILT_COMPASS_I2C_ID, I2C_WRITE )){
            #ifdef DEBUG
            printf("FAILED initial transfer!\n");
            #endif
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(TILT_COMPASS_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(TILT_COMPASS_I2C_ID,SLAVE_DEGREE_ADDRESS)){
            #ifdef DEBUG
            printf("Error: Sent byte was not acknowledged\n");
            #endif
            break;
        }
        // Send a Repeated Started condition
        if(!I2C_startTransfer(TILT_COMPASS_I2C_ID,I2C_READ)){
            #ifdef DEBUG
            printf("FAILED Repeated start!\n");
            #endif
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(TILT_COMPASS_I2C_ID, SLAVE_READ_ADDRESS))
            break;
        
        // Read the I2C bus twice
        data = (I2C_getData(TILT_COMPASS_I2C_ID) << 8);
        I2C_acknowledgeRead(TILT_COMPASS_I2C_ID, TRUE);
        while(!I2C_hasAcknowledged(TILT_COMPASS_I2C_ID));
        data += (I2C_getData(TILT_COMPASS_I2C_ID));

        // Send the stop bit to finish the transfer
        I2C_stopTransfer(TILT_COMPASS_I2C_ID);

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

//#define TILT_COMPASS_ATLAS_TEST
#ifdef TILT_COMPASS_ATLAS_TEST

#define PRINT_DELAY		REFRESH_DELAY

int main(void) {
// Initialize the UART, Timers, and I2C
    Board_init();
    Serial_init();
    Timer_init();
    TiltCompass_init();
    I2C_init(TILT_COMPASS_I2C_ID, I2C_CLOCK_FREQ);
	
    Timer_new(TIMER_TEST,PRINT_DELAY);
    while(1){
        if(Timer_isExpired(TIMER_TEST)){
            printf("Heading: %.1f\n", TiltCompass_getHeading());
            Timer_new(TIMER_TEST,PRINT_DELAY);
        }
        TiltCompass_runSM();
    }

    return (SUCCESS);
}

#endif
