/**********************************************************************
 Module
   Compas.c

 Author: 
	John Ash
	David Goodman
	Shehadeh Dajani

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
3/08/2013   6:41PM      dagoodma    Copied initial code from Shah's Position module.
3/08/2013   12:00PM     shehadeh    Wrote initial code.
2/25/2013   11:10PM     jash        Created project.
***********************************************************************/
#define IS_COMPAS
#define DEBUG
//#define DEBUG_VERBOSE

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
#include "Gps.h"
#include "Navigation.h"
#include "Barometer.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define USE_MAIN



I2C_MODULE      I2C_BUS_ID = I2C1;


// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)


/*DEFINE IO BUTTONS*/
//Lock Button
#define LOCK_BUTTON_TRIS    PORTY05_TRIS
#define LOCK_BUTTON         PORTY05_BIT

//Zero Button
#define ZERO_BUTTON_TRIS    PORTY06_TRIS
#define ZERO_BUTTON         PORTY06_BIT

// Hold time before buttons trigger
#define BUTTON_DELAY   600 // (ms)

//----------------------------- Accelerometer --------------------------

#define USE_ACCELEROMETER

#define LED_DELAY     1 // (ms)

// Leveling constants
#define G_DELTA_HORIZONTAL         10 // (0.001 G) scaled by 1e-3 == 0.02 G
#define G_DELTA_VERTICAL           25 // (0.001 G) scaled by 1e-3 == 0.02 G
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

//----------------------------- Other Modules ---------------------------
#define USE_MAGNETOMETER
#define USE_NAVIGATION
#define USE_ENCODERS
#define USE_BAROMETER


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void initMasterSM();
void runMasterSM();
void updateAccelerometerLEDs();
void updateHeading();

BOOL readLockButton();
BOOL readZeroButton();
BOOL isLockPressed();
BOOL isZeroPressed();

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
#ifndef USE_BAROMETER
float height = 5.44; // (m)
#endif
float heading = 0;
// Printing debug messages over serial
BOOL useLevel = FALSE;

BOOL lockPressed = FALSE, lockTimerStarted = FALSE;
BOOL zeroPressed = FALSE, zeroTimerStarted = FALSE;

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/
/*
/**
 * Function: main
 * @return SUCCESS or FAILURE.
 * @remark Entry point for command center (COMPAS).
 * @author David Goodman
 * @date 2013.03.10  */
#ifdef USE_MAIN
int main(void) {
    initMasterSM();
    printf("Command Center Ready for Use. \n\n\n\n\n");
    while(1){
        runMasterSM();
    }
    return (SUCCESS);
}
#endif


/**
 * Function: initMasterSM
 * @return None.
 * @remark Initializes the master state machine for the command canter.
 * @author David Goodman
 * @date 2013.03.10  */
void initMasterSM() {
    Board_init();
    Serial_init();
    Timer_init();

    // CC buttons
    LOCK_BUTTON_TRIS = 1;
    ZERO_BUTTON_TRIS = 1;

    Encoder_init();

    #ifdef USE_XBEE
    Xbee_init();
    #endif

    #ifdef USE_NAVIGATION
    Navigation_init();
    #endif

    #ifdef USE_ENCODERS
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);
    #endif

    #ifdef USE_BAROMETER
    Barometer_init();
    #endif

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

/**
 * Function: runMasterSM
 * @return None.
 * @remark Executes one cycle of the command center's state machine.
 * @author David Goodman
 * @date 2013.03.09  */
void runMasterSM() {
    //Magnetometer_runSM();
    // Record these button presses since we don't know
    //  if they will be pressed after runSM
    lockPressed = isLockPressed();
    zeroPressed = isZeroPressed();
    if(lockPressed || zeroPressed){
        #ifdef USE_ENCODERS
        Encoder_runSM();
        #endif

        if(lockPressed) {
            printf("Lock was pressed.\n");
            #ifdef USE_NAVIGATION
            #ifdef USE_ENCODERS
            Encoder_enableZeroAngle();
            Encoder_runSM();
            Coordinate ned; // = Coordinate_new(ned, 0, 0 ,0);
            #ifdef USE_BAROMETER
            float height = Barometer_getAltitude() - their_barometer.altitude;
            #endif
            if (Navigation_getProjectedCoordinate(&ned, Encoder_getYaw(),
                Encoder_getPitch(), height)) {
                printf("Desired coordinate -- N: %.6f, E: %.6f, D: %.2f (m)\n",
                    ned.x, ned.y, ned.z);


                #ifdef USE_XBEE
                Mavlink_send_start_rescue(XBEE_UART_ID, TRUE, 0,ned.x, ned.y);
                #endif
            }
            else {
                printf("Failed to obtain desired NED coordinate.\n");
            }
            Encoder_disableZeroAngle();
            #else
            printf("Navigation module is disabled.\n");
            #endif
            #endif

        }
        else if (zeroPressed) {
            // Zero was pressed
            #ifdef USE_ENCODERS
            Encoder_setZeroAngle();
            #endif
            useLevel = TRUE;
            #ifdef  USE_MAGNETOMETER
            Magnetometer_runSM();
            heading = Magnetometer_getDegree();
            updateHeading();
            #endif
            //printf("Zeroing...\n");
        }
        else {

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

    #ifdef USE_NAVIGATION
    Navigation_runSM();
    #endif

    #ifdef USE_XBEE
    Xbee_runSM();
    #endif

    #ifdef USE_BAROMETER
    Barometer_runSM();
    #endif

}


/**
 * Function: updateHeading
 * @return None.
 * @remark Prints the heading value if the magnetometer is enabled.
 * @author Shehadeh Dajani
 * @date 2013.03.09  */
#ifdef USE_MAGNETOMETER
void updateHeading(){
    if(heading < 40 || heading > 320)
        printf("Heading: %.1f (degrees)\n", heading);
    if(heading == 0)
        printf("Heading: NORTH\n");
}

#endif


/**
 * Function: updateHeading
 * @return None.
 * @remark Shines the accelerometer level lights if zeroing.
 * @author David Goodman
 * @date 2013.03.09  */
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



BOOL readLockButton() {
    return !LOCK_BUTTON;
}

BOOL readZeroButton() {
    return !ZERO_BUTTON;
}


 BOOL isLockPressed() {
     // Start lock press timer if pressed
     if (!readLockButton()) {
         if (lockTimerStarted)
             lockTimerStarted = FALSE;

         //printf("Lock released.\n");
         lockPressed = FALSE;
     }
     else if (!lockTimerStarted && readLockButton()) {
         Timer_new(TIMER_BUTTONS, BUTTON_DELAY);
         lockTimerStarted = TRUE;
         lockPressed = FALSE;

         //printf("Lock timer started.\n");
     }
     else if (Timer_isExpired(TIMER_BUTTONS)) {
         lockPressed = TRUE;

         //printf("Lock on.\n");
     }

     return lockPressed;
 }

 BOOL isZeroPressed() {
     // Start lock press timer if pressed
     if (!readZeroButton()) {
         if (zeroTimerStarted)
             zeroTimerStarted = FALSE;

         //printf("Zero released.\n");
         zeroPressed = FALSE;
     }
     else if (!zeroTimerStarted && readZeroButton()) {
         Timer_new(TIMER_BUTTONS, BUTTON_DELAY);
         zeroTimerStarted = TRUE;
         zeroPressed = FALSE;

         //printf("Zero timer started.\n");
     }
     else if (Timer_isExpired(TIMER_BUTTONS)) {
         zeroPressed = TRUE;

         //printf("Zero on.\n");
     }

     return zeroPressed;
 }


