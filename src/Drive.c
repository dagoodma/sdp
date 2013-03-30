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
#include "RCServo.h"
#include "Ports.h"
#include "GPS.h"
#include "Drive.h"
#include "I2C.h"
#include "Magnetometer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG

#define MOTOR_LEFT              RC_PORTW08 // RB2 -- J7-01
#define MOTOR_RIGHT             RC_PORTW07 // RB3 -- J7-02
// #define RUDDER_TRIS          RC_TRISY06 // RB15 -- J7-12
// #define RUDDER                  RC_LATY06 // RB15

#define RC_FULL_STOP            1500
#define RC_FORWARD_RANGE        (MAXPULSE - RC_FULL_STOP)
#define RC_REVERSE_RANGE        (RC_FULL_STOP - MINPULSE)

#define SPEED_TO_RCTIME(s)      (5*s + RC_FULL_STOP) // PWM 0-100 to 1500-2000

#define HEADING_UPDATE_DELAY    250 // (ms)

//PD Controller Parameter Settings
#define KP 1.0f
#define KD 1.0f
#define velocity 5
#define FORWARD_RANGE (11 - 8.18)
#define BACKWARD_RANGE (8.18 - 5.5)
#define FULL_FORWARD 11
#define FULL_BACKWARD 5.5
#define FULL_STOP 8.18
/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
static enum {
    STATE_IDLE  = 0x0,   // Motors are stopped
    STATE_DRIVE = 0x1, // Driving forward.
    STATE_PIVOT = 0x2, // Pivoting in place.
} state;

static enum {
    PIVOT_LEFT  = 0x1,   // Pivot to the left --> Motor Arrangement
    PIVOT_RIGHT = 0x2, // Pivot to the Right --> Motor Arrangement
} pivotState;

uint16_t lastPivotState, lastPivotError;

uint16_t desiredHeading = 0; // (degrees) from North
 
/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
static void updateHeading();
static void setLeftMotor(uint16_t rc_time);
static void setRightMotor(uint16_t rc_time);
static void startPivotState();
static void startIdleState();
static void startDriveState();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

BOOL Drive_init() {


    uint16_t RC_pins = MOTOR_LEFT  | MOTOR_RIGHT;
    RC_init(RC_pins);

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
    startDriveState();
    uint16_t rc_time = SPEED_TO_RCTIME(speed);
    setLeftMotor(rc_time);
    setRightMotor(rc_time);
}

void Drive_stop() {
    startIdleState();
}

void Drive_setHeading(uint16_t angle) {
    startPivotState();
    
    // For now, just stop and pivot in place.
    Drive_stop();

    Timer_new(TIMER_DRIVE, HEADING_UPDATE_DELAY);
    desiredHeading = angle;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

static void startPivotState() {
    state = STATE_PIVOT;

    lastPivotState = 0;
    lastPivotError = 0;
}

static void startIdleState() {
    state = STATE_IDLE;
    setLeftMotor(RC_FULL_STOP);
    setRightMotor(RC_FULL_STOP);
}

static void startDriveState() {
    state = STATE_DRIVE;
}

static void setLeftMotor(uint16_t rc_time) {
    RC_setPulseTime(MOTOR_LEFT, rc_time);
}

static void setRightMotor(uint16_t rc_time) {
    RC_setPulseTime(MOTOR_RIGHT, rc_time);
}

/**
 * Function: updateHeading
 * @return None
 * @remark Determines the heading error using the magnetometer, and
 *  adjusts the motors/rudder accordingly.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27  */
static void updateHeading() {
//Get error and change PWM Signal based on values

    static uint16_t Umax = KP*(180) + KD*(180/HEADING_UPDATE_DELAY);
    //Obtain the current Heading and error, previous heading and error, and derivative term
    uint16_t currHeading = Magnetometer_getDegree();
    uint16_t thetaError = desiredHeading - currHeading;

    //In the event that our current heading exceeds desired resulting in negative number
    
    if ((thetaError > 0) && (thetaError < 180)){            //Desired leads heading and within heading's right hemisphere --> Turn right, theta stays the same
        pivotState = PIVOT_RIGHT;
    }else if((thetaError < 0) && (thetaError > -180)){      //Heading leads desired and within desired's right hemisphere --> Turn left, theta gets inverted
        pivotState = PIVOT_LEFT;
        thetaError = thetaError*-1;
    }else if((thetaError > 0)&&(thetaError >= 180)){        //Desired leads heading and within heading's left hemisphere--> Turn left, theta is complement
        pivotState = PIVOT_LEFT;
        thetaError = 360 - thetaError;
    }else if((thetaError < 0)&&(thetaError <= -180)){       //Heading leads desired and within desired's left hemisphere --> Turn right, theta is inverted and complement
        pivotState = PIVOT_RIGHT;
        thetaError = 360 - (-1*thetaError);
    }
    if (lastPivotState == 0) {
        lastPivotState = pivotState;
        lastPivotError = thetaError;
    }

    if(lastPivotState != pivotState){
        lastPivotError = 0;
    }
    uint16_t thetaErrorDerivative = (thetaError - lastPivotError)/HEADING_UPDATE_DELAY;
    if (thetaErrorDerivative < 0){
        thetaErrorDerivative = -1*thetaErrorDerivative;
    }

    //Calculate Compensator's Ucommand
    uint16_t Ucmd = KP*(thetaError) + KD*(thetaErrorDerivative);
    uint16_t Unormalized = Ucmd/Umax;
    uint16_t forwardScaled = RC_FULL_STOP + Unormalized*RC_FORWARD_RANGE;
    uint16_t backwardScaled = RC_FULL_STOP - Unormalized*RC_REVERSE_RANGE;


    //uint16_t pwmSpeed = Ucmd*velocity;
    //Set PWM's: we have a ratio of 3:1 when Pulsing the motors given a Ucmd
    if(pivotState == PIVOT_RIGHT ){ //Turning Right
        setRightMotor(backwardScaled);//Give negative duty --> Same scaled range for pwm duty
        setLeftMotor(forwardScaled);
    }else if(pivotState ==  PIVOT_LEFT){ //Turning Left
        setRightMotor(forwardScaled);
        setLeftMotor(backwardScaled);
    }

    lastPivotError = thetaError;
    lastPivotState = pivotState;

}


#define DRIVE_TEST
#ifdef DRIVE_TEST

#define TEST_DELAY      2000 // (ms)

int main() {
    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    //Magnetometer_init();
    printf("Boat initialized.\n");

    Drive_forward(10);
    Timer_new(TIMER_TEST,TEST_DELAY);
    while (1) {
        if (Timer_isExpired(TIMER_TEST))
            break;
        Drive_runSM();
    }

    Drive_stop();
    Drive_runSM();

    return SUCCESS;
}

#endif

//#define PIVOT_TEST
#ifdef PIVOT_TEST

#define FINISH_DELAY      10000 // (ms)


I2C_MODULE      I2C_BUS_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)

int main() {
    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);
    Magnetometer_init();
    //Magnetometer_init();
    printf("Boat initialized.\n");

    Drive_stop();
    DELAY(5);
    Drive_setHeading(0);

    Timer_new(TIMER_TEST,FINISH_DELAY);
    while (1) {
        if (Timer_isExpired(TIMER_TEST))
            break;
        Drive_runSM();
        Magnetometer_runSM();
    }

    Drive_stop();

    return SUCCESS;
}

#endif