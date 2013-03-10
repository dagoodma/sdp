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
#include "Ports.h"
#include "Magnetometer.h"
#include "Xbee.h"
#include "UART.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


I2C_MODULE      I2C_BUS_ID = I2C1;


// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)

//----------------------------- Accelerometer --------------------------

#define USE_MAGNETOMETER
#define USE_ACCELEROMETER

#define LED_DELAY     1 // (ms)

// Leveling constants
#define G_DELTA_VERTICAL         10 // (0.001 G) scaled by 1e-3 == 0.02 G
#define G_DELTA_HORIZONTAL       25 // (0.001 G) scaled by 1e-3 == 0.02 G
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

//------------------------------- XBEE --------------------------------

#define USE_XBEE

#define XBEE_UART_ID UART2_ID


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void initMasterSM();
void runMasterSM();
void updateAccelerometerLEDs();
void updateHeading();

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

float height = 4.5;
float heading = 0;
// Printing debug messages over serial
BOOL useLevel = FALSE;

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/
/*
int main(void) {
    initMasterSM();
    printf("Command Center Ready for Use. \n\n\n\n\n");
    while(1){
        runMasterSM();
    }
    return (SUCCESS);
}
*/

void initMasterSM() {
    Board_init();
    Timer_init();
    Serial_init();
    Encoder_init();

    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);

    #ifdef USE_ACCELEROMETER
    //printf("Initializing accelerometer...\n");
    if (Accelerometer_init() != SUCCESS) {
        printf("Failed to initialize the accelerometer.\n");
        //return;
    }
    //printf("Initialized the accelerometer.\n");

    // Configure ports as outputs
    LED_N_TRIS = OUTPUT;
    LED_S_TRIS = OUTPUT;
    LED_E_TRIS = OUTPUT;
    LED_W_TRIS = OUTPUT;

    Timer_new(TIMER_ACCELEROMETER, LED_DELAY );
    #endif
}

void runMasterSM() {
    //Magnetometer_runSM();
#ifdef USE_XBEE
    Xbee_runSM();
#endif
    // Record these button presses since we don't know
    //  if they will be pressed after runSM
    BOOL lockPressed = Encoder_isLockPressed();
    BOOL zeroPressed = Encoder_isZeroPressed();
    if(lockPressed || zeroPressed){
        Encoder_runSM();
       
        if(lockPressed) {
            float verticalDistance = Encoder_getVerticalDistance(height);
            float horizontalDistance = Encoder_getHorizontalDistance(verticalDistance);
            printf("Vertical Distance: %.2f (ft)\n",verticalDistance);
            printf("Horizontal Distance: %.2f (ft)\n\n",horizontalDistance);
#ifdef USE_XBEE
            Mavlink_send_start_rescue(XBEE_UART_ID, TRUE, 0, verticalDistance, horizontalDistance);
#endif
        }
        else {
            // Zero was pressed
            Encoder_setZeroAngle();
            useLevel = TRUE;
            Magnetometer_runSM();
            heading = Magnetometer_getDegree();
            updateHeading();
            //printf("Zeroing...\n");
        }
    }
    if (!zeroPressed) {
        if (useLevel)
            printf("Done zeroing.\n");
        
        useLevel = FALSE;
    }

    #ifdef USE_ACCELEROMETER
    Accelerometer_runSM();
    updateAccelerometerLEDs();
    #endif
}

#ifdef USE_MAGNETOMETER
void updateHeading(){
    if(heading < 40 || heading > 320)
        printf("Heading: %.1f (degrees)\n", heading);
    if(heading == 0)
        printf("Heading: NORTH\n");
}

#endif

#ifdef USE_ACCELEROMETER
void updateAccelerometerLEDs() {
    if (!useLevel) {
        LED_N = OFF;
        LED_S = OFF;
        LED_E = OFF;
        LED_W = OFF;
        return;
    }
    //printf("Updating LEDS!\n");
    //if (Timer_isExpired(TIMER_ACCELEROMETER)) {
        // X-Axis
        if (Accelerometer_getX() <= (G_X_DESIRED - G_DELTA_HORIZONTAL)) {
            LED_N = ON;
            LED_S = OFF;
        }
        else if (Accelerometer_getX() >= (G_X_DESIRED + G_DELTA_HORIZONTAL)) {
            LED_N = OFF;
            LED_S = ON;
        }
        else {
            LED_N = OFF;
            LED_S = OFF;
        }

        // Y-Axis
        if (Accelerometer_getY() <= (G_Y_DESIRED - G_DELTA_VERTICAL)) {
            LED_E = OFF;
            LED_W = ON;
        }
        else if (Accelerometer_getY() >= (G_Y_DESIRED + G_DELTA_VERTICAL)) {
            LED_E = ON;
            LED_W = OFF;
        }
        else {
            LED_E = OFF;
            LED_W = OFF;
        }

        //Timer_new(TIMER_ACCELEROMETER, LED_DELAY );
    //}
}
#endif


