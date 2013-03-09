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


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Encoder
#define SLAVE_VERTICAL_READ_ADDRESS          0x81
#define SLAVE_VERTICAL_WRITE_ADDRESS         0x80
#define SLAVE_HORIZONTAL_READ_ADDRESS        0x87
#define SLAVE_HORIZONTAL_WRITE_ADDRESS       0x86
#define SLAVE_ANGLE_ADDRESS                  0xFE

/*DEFINE IO BUTTONS*/
//Lock Button
#define LOCK_BUTTON_TRIS    PORTY05_TRIS
#define LOCK_BUTTON         PORTY05_BIT

#define BUTTON_DELAY   600 // (ms)

//Zero Button
#define ZERO_BUTTON_TRIS    PORTY06_TRIS
#define ZERO_BUTTON         PORTY06_BIT
#define PI 3.14159265358979323846

#define ACCUMULATOR_LENGTH  300

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      ENCODER_I2C_ID = I2C1;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

float angleAccumulator = 0; // (degrees) accumlated angles
float verticalAngle = 0; // (degrees) calculated angle
float horizontalAngle = 0; // (degrees) calculated angle
float zeroVerticalAngle = 0; // (degrees)
float zeroHorizontalAngle = 0; // (degrees)
BOOL lockFlag = FALSE, lockTimerStarted = FALSE;
BOOL zeroFlag = FALSE, zeroTimerStarted = FALSE;

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS);
float calculateAngle(float zeroAngle,int READ_ADDRESS,int WRITE_ADDRESS);
uint16_t readSensor(int SLAVE_READ_ADDRESS,int SLAVE_WRITE_ADDRESS);
BOOL readLockButton();
BOOL readZeroButton();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


 void Encoder_runSM(){

     verticalAngle = calculateAngle(zeroVerticalAngle,SLAVE_VERTICAL_READ_ADDRESS,
                                   SLAVE_VERTICAL_WRITE_ADDRESS);
     horizontalAngle = calculateAngle(zeroHorizontalAngle,SLAVE_HORIZONTAL_READ_ADDRESS,
                                     SLAVE_HORIZONTAL_WRITE_ADDRESS);
 }

void Encoder_init() {
    LOCK_BUTTON_TRIS = 1;
    ZERO_BUTTON_TRIS = 1;
}

void Encoder_setZeroAngle(){
    zeroVerticalAngle = verticalAngle;
    zeroHorizontalAngle = horizontalAngle;
}


float Encoder_getVerticalDistance(float height) {
    float theta = (90 - verticalAngle);
    return height*tan(theta*PI/180);
    //printf("Vertical Distance: %.2f\n",verticalDistance);
}

float Encoder_getHorizontalDistance(float verticalDistance) {
    float theta;
    if(horizontalAngle <= 90)
        theta = horizontalAngle;
    else
        theta = 360  - horizontalAngle;
    return verticalDistance*sin(theta*PI/180);
    //printf("Horizontal Distance: %.2f\n\n",horizontalDistance);
}

 BOOL Encoder_isLockPressed(){
     // Start lock press timer if pressed
     if (!readLockButton()) {
         if (lockTimerStarted)
             lockTimerStarted = FALSE;

         //printf("Lock released.\n");
         lockFlag = FALSE;
     }
     else if (!lockTimerStarted && readLockButton()) {
         Timer_new(TIMER_ENCODER, BUTTON_DELAY);
         lockTimerStarted = TRUE;
         lockFlag = FALSE;

         //printf("Lock timer started.\n");
     }
     else if (Timer_isExpired(TIMER_ENCODER)) {
         lockFlag = TRUE;

         printf("Lock on.\n");
     }

     return lockFlag;
 }

 BOOL Encoder_isZeroPressed(){
     // Start lock press timer if pressed
     if (!readZeroButton()) {
         if (zeroTimerStarted)
             zeroTimerStarted = FALSE;

         //printf("Zero released.\n");
         zeroFlag = FALSE;
     }
     else if (!zeroTimerStarted && readZeroButton()) {
         Timer_new(TIMER_ENCODER, BUTTON_DELAY);
         zeroTimerStarted = TRUE;
         zeroFlag = FALSE;

         //printf("Zero timer started.\n");
     }
     else if (Timer_isExpired(TIMER_ENCODER)) {
         zeroFlag = TRUE;

         printf("Zero on.\n");
     }

     return zeroFlag;
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
    if(lockFlag){
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

BOOL readLockButton() {
    return !LOCK_BUTTON;
}

BOOL readZeroButton() {
    return !ZERO_BUTTON;
}

//#define ENCODER_TEST
#ifdef ENCODER_TEST

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Serial_init();
    Encoder_init();
    float angle;
    while(1){
        if(Encoder_isLockPressed() || Encoder_isZeroPressed()){
            Encoder_runSM();
//            if(lockFlag){
//                angle = Encoder_getAverageAngle();
//                while(!Serial_isTransmitEmpty());
//                printf("Angle: %.2f\n", angle);
//            }
//            else
//                zeroAngle = Encoder_getZeroAngle();
//
//            lockFlag = FALSE;
//            zeroFlag = FALSE;
        }
        
    }

    return (SUCCESS);
}

#endif
