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

struct {
    uint8_t left, right;
} motorPWM; // (% PWM) from 0 to 100%

uint16_t desiredHeading = 0; // (degrees) from North
 
/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
void updateHeading();
void setLeftMotor(uint16_t speed);
void setRightMotor(uint16_t speed);
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

void setLeftMotor(uint16_t speed) {
    //motorPWM.left = speed * 10;
    PWM_setDutyCycle(motorPWM.left,speed*10);
}

void setRightMotor(uint16_t speed) {
    PWM_setDutyCycle(motorPWM.right,speed*10);
}

/**
 * Function: updateHeading
 * @return None
 * @remark Determines the heading error using the magnetometer, and
 *  adjusts the motors/rudder accordingly.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27  */
void updateHeading() {

//State declaration
enum {
    pivot_Left  = 0x0,   // Pivot to the left --> Motor Arrangement
    pivot_Right = 0x1, // Pivot to the Right --> Motor Arrangement
} pivotState;

    //Calculate Umax for normalizing
    static uint16_t Umax = KP*(180) + KD*(180/HEADING_UPDATE_DELAY);

    //Obtain the current Heading and error, previous heading and error, and derivative term
    uint16_t currHeading = Magnetometer_getDegree();
    uint16_t thetaError = desiredHeading - currHeading;
    
    if ((thetaError > 0) && (thetaError < 180)){            //Desired leads heading and within heading's right hemisphere --> Turn right, theta stays the same
        pivotState = pivot_Right;
    }else if((thetaError < 0) && (thetaError > -180)){      //Heading leads desired and within desired's right hemisphere --> Turn left, theta gets inverted
        pivotState = pivot_Left;
        thetaError = thetaError*-1;
    }else if((thetaError > 0)&&(thetaError >= 180)){        //Desired leads heading and within heading's left hemisphere--> Turn left, theta is complement
        pivotState = pivot_Left;
        thetaError = 360 - thetaError;
    }else if((thetaError < 0)&&(thetaError <= -180)){       //Heading leads desired and within desired's left hemisphere --> Turn right, theta is inverted and complement
        pivotState = pivot_Right;
        thetaError = 360 - (-1*thetaError);
    }
    static uint16_t lastPivotState = pivotState;
    static uint16_t thetaErrorLast = thetaError;

    if(lastPivotState != pivotState){
        thetaErrorLast = 0;
    }
    uint16_t thetaErrorDerivative = (thetaError - thetaErrorLast)/HEADING_UPDATE_DELAY;
    if (thetaErrorDerivative < 0){
        thetaErrorDerivative = -1*thetaErrorDerivative;
    }

    //Calculate Compensator's Ucommand
    uint16_t Ucmd = KP*(thetaError) + KD*(thetaErrorDerivative);
    uint16_t Unormalized = Ucmd/Umax;
    uint16_t forwardScaled = FULL_STOP + Unormalized*FORWARD_RANGE;
    uint16_t backwardScaled = FULL_STOP - Unormalized*BACKWARD_RANGE;


    //uint16_t pwmSpeed = Ucmd*velocity;
    //Set PWM's: we have a ratio of 3:1 when Pulsing the motors given a Ucmd
    if(pivotState == pivot_Right ){ //Turning Right
        setRightMotor(backwardScaled);//Give negative duty --> Same scaled range for pwm duty
        setLeftMotor(forwardScaled);
    }else if(pivotState ==  pivot_Left){ //Turning Left
        setRightMotor(forwardScaled);
        setLeftMotor(backwardScaled);
    }

    thetaErrorLast = thetaError;
    lastPivotState = pivotState;

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