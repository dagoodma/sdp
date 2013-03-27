/*
 * File:   Drive.c
 * Author: 
 *	David Goodman
 *	Darrel Deo
 *
 * Created on March 27, 2013, 12:34 PM
 */
#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include <math.h>
#include "Drive.h"
#include "Timer.h"
#include "Board.h"
#include "PWM.h"
#include "Ports.h"
#include "GPS.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG

#define MOTOR_LEFT              PWM_PORTZ06
#define MOTOR_RIGHT             PWM_PORTY12
//#define RUDDER                  PWM_PORTY10

#define HEADING_UPDATE_DELAY    250 // (ms)

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
static enum {
    STATE_IDLE  = 0x0,   // Motors are stopped
    STATE_DRIVE = 0x1, // Driving forward.
    STATE_PIVOT = 0x2, // Pivoting in place.
} state;

struct {
    uint8_t left, right;
} motorPWM; // (% PWM) from 0 to 100%

uint16_t desiredHeading = 0; // (degrees) from North
 
/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void updateHeading();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

BOOL Drive_init() {


    int PWM_pins = MOTOR_LEFT  | MOTOR_RIGHT;
    PWM_init(PWM_pins, PWM_1KHZ);

    state = STATE_IDLE;
}
   
void Drive_runSM() {
    switch (state) {
        case STATE_IDLE:
            // Do nothing
            break;
        case STATE_DRIVE:
            // Just driving, do nothing
            break;
        case STATE_PIVOT:
            if (Timer_isExpired(TIMER_DRIVE)) {
                updateHeading();
                Timer_new(TIMER_DRIVE, HEADING_UPDATE_DELAY);
            }
            break;
    } // switch
}

void Drive_forward(uint8_t speed) {
    state = STATE_DRIVE;
    motorPWM.left = speed * 10;
    motorPWM.right = speed * 10;
}

void Drive_stop() {
    state = STATE_IDLE;
    motorPWM.left = 0;
    motorPWM.right = 0;
}

void Drive_setHeading(uint16_t angle) {
    state = STATE_PIVOT;
    
    // For now, just stop and pivot in place.
    Drive_stop();

    Timer_new(TIMER_DRIVE, HEADING_UPDATE_DELAY);
    desiredHeading = angle;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

/**
 * Function: updateHeading
 * @return None
 * @remark Determines the heading error using the magnetometer, and
 *  adjusts the motors/rudder accordingly.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27  */
void updateHeading() {

}


//#define DRIVE_TEST
#ifdef DRIVE_TEST

int main() {
    Board_init();
    Serial_init();
    Timer_init();

    

    return SUCCESS;
}

#endif