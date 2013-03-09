/**********************************************************************
 Module
   Compas.c

 Author: John Ash

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
3/08/2013   6:41PM      dagoodma    Copied initial code from Shah's Position module.
2/25/2013   11:10PM     jash        Creation
***********************************************************************/

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"
#include "Encoder.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

I2C_MODULE      I2C_BUS_ID = I2C1;


//----------------------------- Accelerometer --------------------------
//#define USE_ACCELEROMETER

#define LED_DELAY     1 // (ms)

// Leveling constants
#define G_DELTA         20 // (0.001 G) scaled by 1e-3 == 0.02 G
#define G_X_DESIRED     0
#define G_Y_DESIRED     0
#define G_Z_DESIRED     1000 // (0.001 G) scaled by 1e-3 == 1 G

// Leveling LEDs
#define LED_N           PORTZ06_LAT // RD0
#define LED_S           PORTZ04_LAT // RF1
#define LED_E           PORTY12_LAT // RD1
#define LED_W           PORTY10_LAT // RD2

#define LED_N_TRIS      PORTZ06_TRIS // RD0
#define LED_S_TRIS      PORTZ04_TRIS // RF1
#define LED_E_TRIS      PORTY12_TRIS // RD1
#define LED_W_TRIS      PORTY10_TRIS // RD2




/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void initMasterSM();
void runMasterSM();
void updateAccelerometerLEDs();

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)
float height = 20.4;
// Printing debug messages over serial
#define DEBUG


/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

int main(void) {
    initMasterSM();

    while(1){
        runMasterSM();
    }
    return (SUCCESS);
}


void initMasterSM() {
    Board_init();
    Timer_init();
    Serial_init();
    Encoder_init();

    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);


    #ifdef USE_ACCELEROMETER
    if (Accelerometer_init() != SUCCESS) {
        printf("Failed to initialize the accelerometer.\n");
        return FAILURE;
    }
    printf("Initialized the accelerometer.\n");

    // Configure ports as outputs
    LED_N_TRIS = OUTPUT;
    LED_S_TRIS = OUTPUT;
    LED_E_TRIS = OUTPUT;
    LED_W_TRIS = OUTPUT;

    Timer_new(TIMER_TEST, LED_DELAY );
    #endif
}

void runMasterSM() {
    //Magnetometer_runSM();

    // Record these button presses since we don't know
    //  if they will be pressed after runSM
    BOOL lockPressed = Encoder_isLockPressed();
    BOOL zeroPressed = Encoder_isZeroPressed();

    if(lockPressed || zeroPressed){
        Encoder_runSM();
        if(lockPressed) {
            float verticalDistance = Encoder_getVerticalDistance(height);
            float horizontalDistance = Encoder_getHorizontalDistance(verticalDistance);
            printf("Vertical Distance: %.2f\n",verticalDistance);
            printf("Horizontal Distance: %.2f\n\n",horizontalDistance);
        }
        else {
            // Zero was pressed
            Encoder_setZeroAngle();

            #ifdef USE_ACCELEROMETER
            Accelerometer_runSM();
            updateAccelerometerLEDs();
            #endif
        }
    }
}


#ifdef USE_ACCELEROMETER
void updateAccelerometerLEDs() {
    if (Timer_isExpired(TIMER_ACCELEROMETER)) {
        // X-Axis
        if (Accelerometer_getX() <= (G_X_DESIRED - G_DELTA)) {
            LED_N = ON;
            LED_S = OFF;
        }
        else if (Accelerometer_getX() >= (G_X_DESIRED + G_DELTA)) {
            LED_N = OFF;
            LED_S = ON;
        }
        else {
            LED_N = OFF;
            LED_S = OFF;
        }

        // Y-Axis
        if (Accelerometer_getY() <= (G_Y_DESIRED - G_DELTA)) {
            LED_E = OFF;
            LED_W = ON;
        }
        else if (Accelerometer_getY() >= (G_Y_DESIRED + G_DELTA)) {
            LED_E = ON;
            LED_W = OFF;
        }
        else {
            LED_E = OFF;
            LED_W = OFF;
        }

        Timer_new(TIMER_ACCELEROMETER, LED_DELAY );
    }
}
#endif
