/* 
 * File:   Encoder_I2C.c
 * Author: Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"
#include "Encoder_I2C.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Encoder
#define SLAVE_READ_ADDRESS          0x43
#define SLAVE_WRITE_ADDRESS         0x42
#define SLAVE_DEGREE_ADDRESS        0x41


/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)
float height = 20.4;
// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/



#define POSITION_TEST
#ifdef POSITION_TEST

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Serial_init();
    Encoder_init();
    while(1){
        //Magnetometer_runSM();
        if(Is_lockOnButtonPressed() || Is_zeroOnButtonPressed()){
            Encoder_angleAverage();
            if(lockFlag){
                Encoder_getCoordinates(height);
            }
            else
                Encoder_setZeroAngle();

            lockFlag = FALSE;
            zeroFlag = FALSE;
        }
    }

    return (SUCCESS);
}

#endif
