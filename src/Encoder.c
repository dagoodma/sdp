/* 
 * File:   Encoder_I2C.c
 * Author: Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <math.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"
#include "Ports.h"
#include "Encoder.h"
#include "Timer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Encoder
#define SLAVE_VERTICAL_READ_ADDRESS          0x81
#define SLAVE_VERTICAL_WRITE_ADDRESS         0x80
#define SLAVE_HORIZONTAL_READ_ADDRESS        0x87
#define SLAVE_HORIZONTAL_WRITE_ADDRESS       0x86
#define SLAVE_ANGLE_ADDRESS                  0xFE

#define PI 3.14159265358979323846

#define ACCUMULATOR_LENGTH  150

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      ENCODER_I2C_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  50000 // (Hz)

// Set Desired Operation Frequency
//#define I2C_CLOCK_FREQ  100000 // (Hz)

float angleAccumulator = 0; // (degrees) accumlated angles
float pitchAngle = 0; // (degrees) calculated angle
float yawAngle = 0; // (degrees) calculated angle
float zeroPitchAngle = 0; // (degrees)
float zeroYawAngle = 0; // (degrees)

bool useZeroAngle = FALSE;
bool haveZeroPitch = FALSE;

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS);
float calculateAngle(float zeroAngle,int READ_ADDRESS,int WRITE_ADDRESS);
uint16_t readSensor(int SLAVE_READ_ADDRESS,int SLAVE_WRITE_ADDRESS);

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


 void Encoder_runSM(){

     pitchAngle = calculateAngle(zeroPitchAngle,SLAVE_VERTICAL_READ_ADDRESS,
                                   SLAVE_VERTICAL_WRITE_ADDRESS);
     //yawAngle = calculateAngle(zeroYawAngle,SLAVE_HORIZONTAL_READ_ADDRESS,
     //                                SLAVE_HORIZONTAL_WRITE_ADDRESS);
 }

void Encoder_init() {
    // Do nothing
}

void Encoder_setZeroPitch() {
    zeroPitchAngle = pitchAngle;
    haveZeroPitch = TRUE;
}


void Encoder_setZeroYaw() {
    zeroYawAngle = yawAngle;
    if (haveZeroPitch)
        Encoder_enableZeroAngle();
}
    
float Encoder_getPitch() {
    return pitchAngle;
}

float Encoder_getYaw() {
    // Invert yaw direction to be CW from north
    return 360.0 - yawAngle;
}

void Encoder_enableZeroAngle() {
    useZeroAngle = TRUE;
}

void Encoder_disableZeroAngle() {
    useZeroAngle = FALSE;
}


/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/


void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS) {
   uint16_t rawAngle = readSensor(READ_ADDRESS,WRITE_ADDRESS);
   angleAccumulator += (float)((360/pow(2,14))*rawAngle);
}


float calculateAngle(float zeroAngle,int READ_ADDRESS,int WRITE_ADDRESS){
     int count;
     angleAccumulator = 0;
     for(count = 0; count < ACCUMULATOR_LENGTH; count++){
        accumulateAngle(READ_ADDRESS,WRITE_ADDRESS);
    }

    float finalAngle = angleAccumulator/ACCUMULATOR_LENGTH;
    // TODO remove magick numbers
    if(useZeroAngle){
        if(finalAngle >= zeroAngle)
            finalAngle = finalAngle - zeroAngle;
        else
            finalAngle = 360 - (zeroAngle - finalAngle);
        if(finalAngle > 359.98)
            finalAngle = 0;
    }
    return finalAngle;
 }


uint16_t readSensor(int SLAVE_READ_ADDRESS,int SLAVE_WRITE_ADDRESS) {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(ENCODER_I2C_ID, I2C_WRITE )){
            #ifdef DEBUG
            printf("FAILED initial transfer!\n");
            #endif
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(ENCODER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(ENCODER_I2C_ID,SLAVE_ANGLE_ADDRESS)){
            #ifdef DEBUG
            printf("Error: Sent byte was not acknowledged\n");
            #endif
            break;
        }

        // Send a Repeated Started condition
        if(!I2C_startTransfer(ENCODER_I2C_ID,I2C_READ)){
            #ifdef DEBUG
            printf("FAILED Repeated start!\n");
            #endif
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(ENCODER_I2C_ID, SLAVE_READ_ADDRESS))
            break;
        
        // Read the I2C bus twice
        data = (I2C_getData(ENCODER_I2C_ID) << 6);
        I2C_acknowledgeRead(ENCODER_I2C_ID, TRUE);
        while(!I2C_hasAcknowledged(ENCODER_I2C_ID));
        data |= (I2C_getData(ENCODER_I2C_ID) & 0x3F);

        // Send the stop bit to finish the transfer
        I2C_stopTransfer(ENCODER_I2C_ID);

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

//#define ENCODER_TEST
#ifdef ENCODER_TEST

#define PRINT_DELAY     1000
#include "Timer.h"

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Serial_init();
    Timer_init();
    printf("Starting encoders...\n");
    I2C_init(ENCODER_I2C_ID, I2C_CLOCK_FREQ);
    Encoder_init();

        Encoder_runSM();

        Encoder_runSM();

        Encoder_runSM();
        
    Encoder_setZeroYaw();
    Encoder_setZeroPitch();

    printf("Encoders initialized.\n");
    Timer_new(TIMER_TEST, PRINT_DELAY );

    while(1){
        if (Timer_isExpired(TIMER_TEST)) {
            printf("Encoders: P=%.1f, Y=%.1f\n",Encoder_getPitch(), Encoder_getYaw());
            
            Timer_new(TIMER_TEST, PRINT_DELAY );
        }
        Encoder_runSM();
    }

    return (SUCCESS);
}

#endif
