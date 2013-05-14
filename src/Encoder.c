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
#define SLAVE_PITCH_READ_ADDRESS          0x81
#define SLAVE_PITCH_WRITE_ADDRESS         0x80
#define SLAVE_YAW_READ_ADDRESS        0x87
#define SLAVE_YAW_WRITE_ADDRESS       0x86
#define SLAVE_ANGLE_ADDRESS                  0xFE

#define ENCODER_RESOLUTION          14 // (bits)
#define ENCODER_NUMBER_TO_DEGREE(n) ((float)n*(360.0f /((float)(2<ENCODER_RESOLUTION))))

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

uint16_t accumulatorIndex;
bool accumulatePitch;

// Currently selected encoder variables
float currentZeroAngle;
uint16_t currentReadAddress;
uint16_t currentWriteAddress;

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

uint16_t readSensor(int SLAVE_READ_ADDRESS,int SLAVE_WRITE_ADDRESS);
void choosePitchEncoder();
void chooseYawEncoder();
void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS);
float calculateAngle();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


 void Encoder_runSM() {

     if (accumulatorIndex < ACCUMULATOR_LENGTH) {
        accumulateAngle(currentReadAddress, currentWriteAddress);
        accumulatorIndex++;
     }
     else {
        // calculate and switch encoder choice, resetting index
         calculateAngle();
     }
 }

void Encoder_init() {
    // Do nothing
    choosePitchEncoder();
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

void choosePitchEncoder() {
    currentZeroAngle = zeroPitchAngle;
    currentReadAddress = SLAVE_PITCH_READ_ADDRESS;
    currentWriteAddress = SLAVE_PITCH_WRITE_ADDRESS;
    accumulatorIndex = 0;
    accumulatePitch = TRUE;
}

void chooseYawEncoder() {
    currentZeroAngle = zeroYawAngle;
    currentReadAddress = SLAVE_YAW_READ_ADDRESS;
    currentWriteAddress = SLAVE_YAW_WRITE_ADDRESS;
    accumulatorIndex = 0;
    accumulatePitch = FALSE;
}

void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS) {
   uint16_t rawAngle = readSensor(READ_ADDRESS,WRITE_ADDRESS);
   angleAccumulator += ENCODER_NUMBER_TO_DEGREE(rawAngle);
}


float calculateAngle() {

    float finalAngle = angleAccumulator/ACCUMULATOR_LENGTH;
    // TODO remove magick numbers
    if(useZeroAngle){
        if(finalAngle >= currentZeroAngle)
            finalAngle = finalAngle - currentZeroAngle;
        else
            finalAngle = 360.0f - (currentZeroAngle - finalAngle);
        if(finalAngle > 359.98f)
            finalAngle = 0.0f;
    }

    if (accumulatePitch) {
        pitchAngle = finalAngle;
        chooseYawEncoder(); // switch encoders
    }
    else {
        yawAngle = finalAngle;
        choosePitchEncoder(); // switch encoders
    }
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

#define ENCODER_TEST
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
