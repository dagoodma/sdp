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
#include "Encoder_I2C.h"


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
//LockOn Button
#define LockOn_DIR PORTY05_TRIS
#define LockOn PORTY05_BIT

//Zero Button
#define Zero_DIR PORTY06_TRIS
#define ZeroOn PORTY06_BIT
#define PI 3.14159265358979323846

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      ENCODER_I2C_ID = I2C1;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

uint16_t rawAngle;
float finalAngle = 0; // (degrees)
float finalVerticalAngle = 0; // (degrees)
float finalHorizontalAngle = 0; // (degrees)
float zeroVerticalAngle = 0; // (degrees)
float zeroHorizontalAngle = 0; // (degrees)
float verticalDistance = 0;
float horizontalDistance = 0;
BOOL lockFlag = FALSE;
BOOL zeroFlag = FALSE;

// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

uint16_t readSensor();

void updateReadings();

void getVerticalDistance(float height);
void getHorizontalDistance();
float getAngle(float zeroAngle,int READ_ADDRESS,int WRITE_ADDRESS);
void getLatLong();


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

void Encoder_runSM(int READ_ADDRESS,int WRITE_ADDRESS) {
   rawAngle = readSensor(READ_ADDRESS,WRITE_ADDRESS);
   finalAngle = (float)((360/pow(2,14))*rawAngle);
}

void Encoder_init() {
    LockOn_DIR = 1;
    Zero_DIR = 1;
    I2C_init(ENCODER_I2C_ID, I2C_CLOCK_FREQ);
}

void Encoder_setZeroAngle(){
    zeroVerticalAngle = finalVerticalAngle;
    zeroHorizontalAngle = finalHorizontalAngle;
}

 void Encoder_angleAverage(){
     finalVerticalAngle = getAngle(zeroVerticalAngle,SLAVE_VERTICAL_READ_ADDRESS,
                                   SLAVE_VERTICAL_WRITE_ADDRESS);
     finalHorizontalAngle = getAngle(zeroHorizontalAngle,SLAVE_HORIZONTAL_READ_ADDRESS,
                                     SLAVE_HORIZONTAL_WRITE_ADDRESS);

 }

 void Encoder_getCoordinates(float height){
     getVerticalDistance(height);
     getHorizontalDistance();
     getLatLong();
 }

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

 float getAngle(float zeroAngle,int READ_ADDRESS,int WRITE_ADDRESS){
     int count;
     float accumulator = 0;
          for(count = 0; count < 300; count++){
        Encoder_runSM(READ_ADDRESS,WRITE_ADDRESS);
        accumulator += finalAngle;
    }
    finalAngle = accumulator/300;
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

 BOOL Is_lockOnButtonPressed(){
     if(LockOn == 0)
         lockFlag = TRUE;
     else
         lockFlag = FALSE;
     return lockFlag;
 }

 BOOL Is_zeroOnButtonPressed(){
     if(ZeroOn == 0)
         zeroFlag = TRUE;
     else
         zeroFlag = FALSE;
     return zeroFlag;
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

void getVerticalDistance(float height){
    float theta = (90 - finalVerticalAngle);
    verticalDistance = height*tan(theta*PI/180);
    printf("Vertical Distance: %.2f\n",verticalDistance);
}

void getHorizontalDistance(){
    float theta;
    if(finalHorizontalAngle <= 90)
        theta = finalHorizontalAngle;
    else
        theta = 360  - finalHorizontalAngle;
    horizontalDistance = verticalDistance*sin(theta*PI/180);
    printf("Horizontal Distance: %.2f\n\n",horizontalDistance);
}

void getLatLong(){

}

#ifdef ENCODER_TEST

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Serial_init();
    Encoder_init();
    float angle;
    while(1){
        if(Is_lockOnButtonPressed() || Is_zeroOnButtonPressed()){
            Encoder_angleAverage();
            if(lockFlag){
                angle = Encoder_getAverageAngle();
                while(!Serial_isTransmitEmpty());
                printf("Angle: %.2f\n", angle);
            }
            else
                zeroAngle = Encoder_getZeroAngle();
            
            lockFlag = FALSE;
            zeroFlag = FALSE;
        }
        
    }

    return (SUCCESS);
}

#endif
