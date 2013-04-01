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

#define MOTOR_LEFT              RC_PORTY06  //RD10, J5-01            //RC_PORTW08 // RB2 -- J7-01
#define MOTOR_RIGHT             RC_PORTY07  //RE7,  J6-16            //RC_PORTW07 // RB3 -- J7-02
// #define RUDDER_TRIS          RC_TRISY06 // RB15 -- J7-12
// #define RUDDER                  RC_LATY06 // RB15

#define RC_FULL_STOP            1500
#define RC_FORWARD_RANGE        (MAXPULSE - RC_FULL_STOP)
#define RC_REVERSE_RANGE        (RC_FULL_STOP - MINPULSE)

#define SPEED_TO_RCTIME(s)      (5*s + RC_FULL_STOP) // PWM 0-100 to 1500-2000
#define SPEED_TO_RCTIME_BACKWARD(s)      (RC_FULL_STOP - 5*s) // PWM 0-100 to 1500-2000

#define HEADING_UPDATE_DELAY    100 // (ms)

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

unsigned int desiredHeading = 0; // (degrees) from North
 
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

void Drive_backward(uint8_t speed){
    startDriveState();
    uint16_t rc_time = SPEED_TO_RCTIME_BACKWARD(speed);
    setLeftMotor(rc_time);
    setRightMotor(rc_time);
}


void Drive_stop() {
    startIdleState();
}

void Drive_setHeading(uint16_t angle) {
    startPivotState();
    
    // For now, just stop and pivot in place.
    //Drive_stop();

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

    static unsigned int Umax = KP*(180) + KD*(180/HEADING_UPDATE_DELAY);
    //Obtain the current Heading and error, previous heading and error, and derivative term
    int16_t currHeading = (int)Magnetometer_getDegree();
    int16_t thetaError = desiredHeading - currHeading;

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
    float thetaErrorDerivative = (thetaError - lastPivotError)/HEADING_UPDATE_DELAY;
    if (thetaErrorDerivative < 0){
        thetaErrorDerivative = -1*thetaErrorDerivative;
    }

    //Calculate Compensator's Ucommand
    float Ucmd = KP*(thetaError) + KD*(thetaErrorDerivative);
    float Unormalized = Ucmd/Umax;
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
    printf("FORWARD SCALED:  %d\nBACKWARD SCALED: %d\n\n",forwardScaled, backwardScaled);
    lastPivotError = thetaError;
    lastPivotState = pivotState;

}


//#define DRIVE_TEST
#ifdef DRIVE_TEST

#define TEST_DELAY      2000 // (ms)

int main() {
    enum{
        forward =  0x01,
        backward = 0x02,
        idle    =  0x03,
    }drive_state;

    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    int spd = 20;
    //Magnetometer_init();
    printf("Boat initialized.\n");
    Drive_forward(spd);
    Timer_new(TIMER_TEST,TEST_DELAY);
    drive_state = forward;
    int flag = 0;
    while (1) {
        switch(drive_state){
            case forward:
                Drive_forward(spd);
                if(Timer_isExpired(TIMER_TEST)){
                    drive_state = idle;
                    Drive_stop();
                    Timer_new(TIMER_TEST,TEST_DELAY);
                    flag = 0;
                }
                break;
            case idle:
                Drive_stop();
                if(Timer_isExpired(TIMER_TEST)){
                    if (flag == 0){
                        drive_state = backward;
                        Drive_backward(spd);
                    }else if (flag == 1){
                        drive_state = forward;
                        Drive_forward(spd);
                    }
                    Timer_new(TIMER_TEST,TEST_DELAY);
                }

                break;
            case backward:
                Drive_backward(spd);
                if(Timer_isExpired(TIMER_TEST)){
                    drive_state = idle;
                    Drive_stop();
                    Timer_new(TIMER_TEST,TEST_DELAY);
                    flag = 1;
                }
        }

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









#define RECIEVER_OVERRIDE_TEST
#ifdef RECIEVER_OVERRIDE_TEST

//Define the Enable input pin which should  be interrupt driven in the future.
#define ENABLE_TRIS  PORTX10_TRIS // 0--> Output    1-->Input, J5-08
#define ENABLE_BIT  PORTX10_BIT // 0--> Low 0     1-->HIGH 1 J5-08

#define ENABLE_OUT_TRIS  PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

#define RECIEVER_DELAY      3000 // (ms)
#define MICRO 0
#define RECIEVER 1
#define OUTPUT 0
#define INPUT 1

int main(){
    //Initializations
    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    int spd = 10;

    ENABLE_TRIS = INPUT;                 //Set the direction of the Enable pin to be an input, a switch will be used for now
    ENABLE_OUT_TRIS = OUTPUT;            //Set Enable output pin to be an output, fed to the AND gates
    ENABLE_OUT_LAT = MICRO;             //Initialize control to that of Microcontroller

    enum{
        MICRO_FORWARD = 0x01,         //State where Microcontroller is driving forward
        MICRO_STOP = 0x02,        //State where Micro is driving backwards
        MICRO_LIMBO = 0x03,
        RECIEVER_STATE = 0x04,        //Reciever has taken over state
    }test_state;

    printf("Boat is Initialized\n");

    //Initialize the state to begin at forward
    test_state = MICRO_FORWARD;
    Drive_stop();
    Timer_new(TIMER_TEST,RECIEVER_DELAY);
    int state_flag = 0;
    while(1){
        if(Timer_isExpired(TIMER_TEST)){
            if ((ENABLE_BIT == 1)){
                test_state = RECIEVER_STATE;
                printf("YOU HAVE TRIGGERED THE RECIEVER FOR CONTROL\n\n");
                ENABLE_OUT_LAT = RECIEVER;                  //Set the Enable_out pin that will be routed to the AND gates
                state_flag = 1;

            }else if (ENABLE_BIT == 0){
                printf("MICRO HAS CONTROL\n\n");
                ENABLE_OUT_LAT = MICRO;                     //setting Enable_out that will be routed to AND gates
                if(state_flag == 1){
                    Timer_new(TIMER_TEST2,RECIEVER_DELAY);
                    test_state = MICRO_FORWARD;
                    state_flag = 0;
                }
            }
            Timer_new(TIMER_TEST,RECIEVER_DELAY);
        }

        switch(test_state){
            case MICRO_FORWARD:
                printf("STATE: MICRO_FORWARD\n\n\n");
                Drive_forward(spd); //The input param is a pwm duty cycle percentage that gets translated to a RC Servo time pulse
                Timer_new(TIMER_TEST2,RECIEVER_DELAY);
                test_state = MICRO_STOP;

                break;
            case MICRO_STOP:
                if(Timer_isExpired(TIMER_TEST2)){
                    printf("STATE: MICRO_STOP\n\n\n:");
                    Drive_stop();
                    Timer_new(TIMER_TEST2,RECIEVER_DELAY);
                    test_state = MICRO_LIMBO;
                }

                break;
            case MICRO_LIMBO:
                if(Timer_isExpired(TIMER_TEST2)){
                    printf("STATE: MICRO_LIMBO\n\n\n");
                    test_state = MICRO_FORWARD;
                }
                Drive_stop();
                break;
            case RECIEVER_STATE:
                ;
                break;

        }

    }


}

#endif