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
#include "TiltCompass.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

//#define DEBUG

#define MOTOR_LEFT              RC_PORTY07  //RD10, J5-01            //RC_PORTW08 // RB2 -- J7-01
#define MOTOR_RIGHT             RC_PORTY06  //RE7,  J6-16            //RC_PORTW07 // RB3 -- J7-02
// #define RUDDER_TRIS          RC_TRISY06 // RB15 -- J7-12
#define RUDDER                  RC_PORTV03 //RB2, J7-01

#define RC_STOP_PULSE            1500
#define RC_FORWARD_RANGE        (MAXPULSE - RC_STOP_PULSE)
#define RC_REVERSE_RANGE        (RC_STOP_PULSE - MINPULSE)
#define RC_ONEWAY_RANGE        500

#define RC_RUDDER_LEFT_MAX      MAXPULSE
#define RC_RUDDER_RIGHT_MAX     MINPULSE

#define PERCENT_TO_RCPULSE(s)      (5*s + RC_STOP_PULSE) // PWM 0-100 to 1500-2000
#define PERCENT_TO_RCPULSE_BACKWARD(s)      (RC_STOP_PULSE - 5*s) // PWM 0-100 to 1500-2000

#define HEADING_UPDATE_DELAY    100 // (ms)

//PD Controller Parameter Settings
#define KP 1.0f
#define KD 1.0f
#define FORWARD_RANGE (11 - 8.18)
#define BACKWARD_RANGE (8.18 - 5.5)
#define FULL_FORWARD 11
#define FULL_BACKWARD 5.5
#define FULL_STOP 8.18


//PD Controller Param Settings for Rudder
#define KP_Rudder 1.5f
#define KD_Rudder 0.0f
#define K_VELOCITY 0.5f

//Velocity definitions
#define VMAX 30 //30 MPH
#define VMIN 0  //0 MPH
#define VRANGE (VMAX - VMIN)


//defines for Override feature
#define MICRO_CONTROL            0
#define RECIEVER_CONTROL         1

//Override test variables
static uint16_t OVERRIDE_TRIGGERED = FALSE;
static uint16_t CONTROL_MASTER = MICRO_CONTROL;
/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
static enum {
    STATE_IDLE  = 0x0,      // Motors are stopped
    STATE_DRIVE = 0x1,      // Driving forward.
    STATE_PIVOT = 0x2,      // Pivoting in place.
    STATE_TRACK = 0x3,      // Tracking a desired  velocity and heading
} state;

static enum {
    PIVOT_LEFT  = 0x1,   // Pivot to the left --> Motor Arrangement
    PIVOT_RIGHT = 0x2, // Pivot to the Right --> Motor Arrangement
} pivotState;

uint16_t lastPivotState, lastPivotError;

uint16_t desiredHeading = 0; // (degrees) from North
float desiredVelocity = 0.0f;  // (m/s) desired velocity
uint16_t velocityPulse = RC_STOP_PULSE; // (ms) velocity RC servo pulse time
/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
static void updateHeadingPivot();
static void updateHeadingRudder();
static void updateVelocity();
static void setLeftMotor(uint16_t rc_time);
static void setRightMotor(uint16_t rc_time);
static void setRudder(uint16_t rc_time);
static void startPivotState();
static void startIdleState();
static void startDriveState();
static void startTrackState();
static uint16_t getVelocityPulse();
/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

BOOL Drive_init() {


    uint16_t RC_pins = MOTOR_LEFT  | MOTOR_RIGHT | RUDDER;
    RC_init(RC_pins);
    Timer_new(TIMER_DRIVE, 1);

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
                updateHeadingPivot();
                Timer_new(TIMER_DRIVE, HEADING_UPDATE_DELAY);
            }
            break;
        case STATE_TRACK:
            if (Timer_isExpired(TIMER_DRIVE)) {
                //updateVelocity();
                updateHeadingRudder();
                Timer_new(TIMER_DRIVE, HEADING_UPDATE_DELAY);
            }
            break;

    } // switch
}

void Drive_forward(uint8_t speed) {
    startDriveState();
    uint16_t rc_time = PERCENT_TO_RCPULSE(speed);
    setLeftMotor(rc_time);
    setRightMotor(rc_time);
}

void Drive_forwardHeading(float speed, uint16_t angle) {
    startTrackState();
    desiredVelocity = speed;
    desiredHeading = angle;
}

void Drive_backward(uint8_t speed){
    startDriveState();
    uint16_t rc_time = PERCENT_TO_RCPULSE_BACKWARD(speed);
    setLeftMotor(rc_time);
    setRightMotor(rc_time);
}


void Drive_stop() {
    startIdleState();
}

void Drive_pivot(uint16_t angle) {
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
    velocityPulse = RC_STOP_PULSE;
    desiredVelocity = 0.0f;
    setLeftMotor(RC_STOP_PULSE);
    setRightMotor(RC_STOP_PULSE);
}

static void startDriveState() {
    state = STATE_DRIVE;
}

static void startTrackState() {
    state = STATE_TRACK;
    setLeftMotor(RC_STOP_PULSE);
    setRightMotor(RC_STOP_PULSE);

    lastPivotState = 0;
    lastPivotError = 0;
}

static void setLeftMotor(uint16_t rc_time) {
    RC_setPulseTime(MOTOR_LEFT, rc_time);
}

static void setRightMotor(uint16_t rc_time) {
    RC_setPulseTime(MOTOR_RIGHT, rc_time);
}

static void setRudder(uint16_t rc_time) {
    RC_setPulseTime(RUDDER, rc_time);
}


static uint16_t getVelocityPulse(){
    return velocityPulse;
}
/**
 * Function: updateHeadingPivot
 * @return None
 * @remark Determines the heading error using the magnetometer, and
 *  adjusts the motors/rudder accordingly.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27  */
static void updateHeadingPivot() {
//Get error and change PWM Signal based on values

    static uint32_t Umax = KP*(180) + KD*(180/HEADING_UPDATE_DELAY);
    //Obtain the current Heading and error, previous heading and error, and derivative term
    int16_t currHeading = (int)TiltCompass_getHeading();
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
    uint16_t forwardScaled = RC_STOP_PULSE + Unormalized*RC_FORWARD_RANGE;
    uint16_t backwardScaled = RC_STOP_PULSE - Unormalized*RC_REVERSE_RANGE;


    //uint16_t pwmSpeed = Ucmd*velocity;
    //Set PWM's: we have a ratio of 3:1 when Pulsing the motors given a Ucmd
    if(pivotState == PIVOT_RIGHT ){ //Turning Right
        setRightMotor(backwardScaled);//Give negative duty --> Same scaled range for pwm duty
        setLeftMotor(forwardScaled);
    }else if(pivotState ==  PIVOT_LEFT){ //Turning Left
        setRightMotor(forwardScaled);
        setLeftMotor(backwardScaled);
    }
    printf("Unorm: %.2f\n\n", Unormalized);
    lastPivotError = thetaError;
    lastPivotState = pivotState;

}


/**
 * Function: updateHeadingRudder
 * @return None
 * @remark Determines the heading error using the magnetometer, and
 *  adjusts the rudder accordingly.
 * @author Darrel Deo
 * @date 2013.03.27  */
static void updateHeadingRudder(){
static uint32_t Umax = KP_Rudder*(180) + KD_Rudder*(180/HEADING_UPDATE_DELAY);
static int16_t ErrorFlag = 0;
//Obtain the current Heading and error, previous heading and error, and derivative term
    uint16_t currHeading = (uint16_t)(TiltCompass_getHeading());
    int16_t thetaError = desiredHeading - currHeading;
    //In the event that our current heading exceeds desired resulting in negative number
    
    uint16_t tempThetaError = thetaError;
    if(tempThetaError <0) {
        tempThetaError = tempThetaError * -1;
    }
    if(tempThetaError < 10){
        ErrorFlag = 0;
        setRudder(RC_STOP_PULSE);
    }
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
        thetaError = 360 - (-thetaError);
    }
    if (lastPivotState == 0) {
        lastPivotState = pivotState;
        lastPivotError = thetaError;
    }

    if(lastPivotState != pivotState){
        if(lastPivotError > 100){
            ErrorFlag = 1;
        }
        lastPivotError = 0;
        //implement a hard lock in that direction until desired point is
    }
    float thetaErrorDerivative = (thetaError - lastPivotError)/HEADING_UPDATE_DELAY;
    if (thetaErrorDerivative < 0){
        thetaErrorDerivative = -1*thetaErrorDerivative;
    }

    //Calculate Compensator's Ucommand
    float Ucmd = KP_Rudder*(thetaError) + KD_Rudder*(thetaErrorDerivative);
    float Unormalized = Ucmd/Umax;
//    uint16_t leftScaled = (uint16_t)(RC_STOP_PULSE + Unormalized*RC_FORWARD_RANGE);
//    uint16_t rightScaled = (uint16_t)(RC_STOP_PULSE - Unormalized*RC_REVERSE_RANGE);

    uint16_t leftScaled = (uint16_t)(Unormalized*RC_FORWARD_RANGE);
    uint16_t rightScaled = (uint16_t)(Unormalized*RC_REVERSE_RANGE);
    printf("UNORM: %d\n\n",leftScaled);
//Added in to scale rudder for less actuation given how fast we are going
    uint16_t currVelocity = getVelocityPulse();
    printf("OUR CURRENT VELOCITY: %d\n\n",currVelocity);
    uint16_t velocityComplement = (MAXPULSE - currVelocity);
    printf("OUR VELOCITY COMPLEMENT: %d\n\n",velocityComplement);\

    float velocityRatio = (float)velocityComplement/RC_ONEWAY_RANGE;
    velocityRatio = K_VELOCITY*velocityRatio;
    if(velocityRatio > 1){
        velocityRatio = 1;
    }

    printf("VELOCITY RATIO: %f\n\n",velocityRatio);

    leftScaled = RC_STOP_PULSE + leftScaled*(velocityRatio);
    rightScaled = RC_STOP_PULSE - rightScaled*(velocityRatio);
    printf("LEFT_SCALED: %d\nRIGHT_SCALED: %d\n\n",leftScaled,rightScaled);

    //In the event that we are pulsing at MAXPULSE for motors we want to turn rudders slightly
    if (currVelocity == (MAXPULSE)){
        leftScaled = RC_STOP_PULSE + 100;
        rightScaled = RC_STOP_PULSE - 100;
    }

if(ErrorFlag == 0){
    //Set PWM's: we have a ratio of 3:1 when Pulsing the motors given a Ucmd
    if(pivotState == PIVOT_RIGHT ){ //Turning Right
        setRudder(rightScaled);
        printf("RC TIME RIGHT: %d\n\n",rightScaled);
    }else if(pivotState ==  PIVOT_LEFT){ //Turning Left
        setRudder(leftScaled);
        printf("RC TIME LEFT: %d\n\n",leftScaled);
    }

}else if(ErrorFlag == 1){
    if(lastPivotState == PIVOT_RIGHT){
        rightScaled = RC_STOP_PULSE - RC_ONEWAY_RANGE/2;
        setRudder(rightScaled);
        printf("YOU PASSED 180, TURNING RIGHT UNTIL DESIRED HIT with Pulse %d\n\n",rightScaled);
    }else if(lastPivotState == PIVOT_LEFT){
        leftScaled = RC_STOP_PULSE + RC_ONEWAY_RANGE/2;
        setRudder(leftScaled);
        printf("YOU PASSED 180, TURNING LEFT UNTIL DESIRED HIT with Pulse  %d\n\n",leftScaled);
    }
    pivotState = lastPivotState;// To keep it turning in this direction.
}

#ifdef DEBUG
    printf("Unorm: %.2f\n\n", Unormalized);
#endif
    lastPivotError = thetaError;
    lastPivotState = pivotState;

}

/**
 * Function: updateVelocity()
 * @return None
 * @remark Velocity controller
 * @author Darrel Deo
 * @date 2013.04.06
 * @note Keep track of global variable Velocity. You must set to 1500 if you wish to stop, or macro STOP */
static void updateVelocity(){
    //Get velocity, check if it matches the desired
    //If not, use a propotional ratio to the min/max pulse of the RCServo library


    //Obtain current velocity
    float currentVelocity = GPS_getVelocity();
    float errorVelocity = desiredVelocity - currentVelocity;

    //Correct the error value incase it is negative
    if(errorVelocity < 0.0){
        errorVelocity = -errorVelocity;
    }
    float proportionVelocity = errorVelocity/VRANGE;
    float proportionPulse = proportionVelocity * (RC_ONEWAY_RANGE);

    // Here we add the the amount of propotional pulse to the pulse already
    //We need to get the current pulse width for the motors
    
    //If we need to go faster
    if((desiredVelocity - currentVelocity) > 0.0 ){
        velocityPulse = (uint16_t)(velocityPulse + proportionPulse);
    }else if((desiredVelocity - currentVelocity) < 0.0){
        velocityPulse = (uint16_t)(velocityPulse - proportionPulse);
    }else if(errorVelocity == 0.0){
        //velocityPulse = velocityPulse;
    }

    //Now check if we exceed our maximums and minimums
    if(velocityPulse > MAXPULSE){
        velocityPulse = MAXPULSE;
    }else if(velocityPulse < MINPULSE){
        velocityPulse =  MINPULSE;
    }

    if(desiredVelocity == 0.0){
        velocityPulse = RC_STOP_PULSE;
    }

    setRightMotor(velocityPulse);
    setLeftMotor(velocityPulse);
}






/*TEST HARNESSES*/



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

#define PIVOT_TEST_DRIVE
#ifdef PIVOT_TEST_DRIVE
//#define MOTOR_TEST
#define RUDDER_TEST
#define MICRO 0

#define FINISH_DELAY      5000 // (ms)
#define ENABLE_OUT_TRIS  PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

I2C_MODULE      I2C_BUS_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)

int main() {
    ENABLE_OUT_TRIS = OUTPUT;            //Set Enable output pin to be an output, fed to the AND gates
    ENABLE_OUT_LAT = MICRO;             //Initialize control to that of Microcontroller
    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    I2C_init(I2C_BUS_ID, I2C_CLOCK_FREQ);
    TiltCompass_init();
    printf("Boat initialized.\n");

    Drive_stop();
    DELAY(5);
#ifdef MOTOR_TEST
    Drive_pivot(0);
#endif

#ifdef RUDDER_TEST
   Drive_forwardHeading(0.0, desiredHeading);
#endif
   int i = 0;
   int velocity[] = {1600, 1600, 1600, 1600, 1600};
   velocityPulse = velocity[i];
    Timer_new(TIMER_TEST,FINISH_DELAY);
    while (1) {
        if (Timer_isExpired(TIMER_TEST))
        {
            i++;
            if (i == 5){
                i = 0;
            }
            velocityPulse = velocity[i];
           printf("CURRENT VELOCITY: %d\n\n",velocityPulse);
            Timer_new(TIMER_TEST,FINISH_DELAY);

        }
         
        Drive_runSM();
        TiltCompass_runSM();
    }

    Drive_stop();

    return SUCCESS;
}

#endif


//#define RECIEVER_OVERRIDE_TEST
#ifdef RECIEVER_OVERRIDE_TEST

//Define the Enable input pin which should  be interrupt driven in the future.
#define ENABLE_TRIS  PORTX10_TRIS // 0--> Output    1-->Input, J5-08
#define ENABLE_BIT  PORTX10_BIT // 0--> Low 0     1-->HIGH 1 J5-08

#define ENABLE_OUT_TRIS  PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

#define RECIEVER_DELAY      3000 // (ms)
#define MICRO 0
#define RECIEVER 1


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



//#define RECIEVER_OVERRIDE_INTERRUPT_TEST
#ifdef RECIEVER_OVERRIDE_INTERRUPT_TEST


#define ENABLE_OUT_TRIS  PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

#define RECIEVER_DELAY      3000 // (ms)
#define MICRO 0
#define RECIEVER 1

void Override_init();


int main(){
    //Initializations
    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    int spd = 10;


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
    Override_init();


    while(1){

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
        //If we got a pulse, control to reciever.
        if (OVERRIDE_TRIGGERED == TRUE){
            printf("Reciever Control\n\n");
            Timer_new(TIMER_TEST, 1000);    //Set timer that is greater than the pulsewidth of the CH3 signal(54Hz)
            OVERRIDE_TRIGGERED = FALSE;     //Re-init to zero so that we know when our pulse is triggered again.
            test_state = RECIEVER_STATE;    //Set state equal to reciever where we do nothing autonomous
            ENABLE_OUT_LAT = RECIEVER;      //Give control over to Reciever using the enable line
            INTEnable(INT_CN,1);
        }
        if (Timer_isExpired(TIMER_TEST)){   //Reciever gave up control
            printf("Micro Has Control\n\n");
            Timer_clear(TIMER_TEST);        //Clear timer so that it doesn't keep registering an expired signal
            test_state = MICRO_LIMBO;     //Set state equal to forward for regular function
            OVERRIDE_TRIGGERED = FALSE;     //Set Override to false to be sure that we don't trigger falsely
            ENABLE_OUT_LAT = MICRO;         //Give Control back to microcontroller using enable line
            Timer_new(TIMER_TEST2, RECIEVER_DELAY);
        }

    }


}




/**
 * Function: Override_init()
 * @return None
 * @remark Initializes interrupt for Override functionality
 * @author Darrel Deo
 * @date 2013.04.01  */
void Override_init(){
    //Enable the interrupt for the override feature

    mPORTBSetPinsDigitalIn(BIT_0); // CN2

    mCNOpen(CN_ON | CN_IDLE_CON , CN2_ENABLE , CN_PULLUP_DISABLE_ALL);
    uint16_t value = mPORTDRead();
    ConfigIntCN(CHANGE_INT_ON | CHANGE_INT_PRI_2);
    //CN2 J5-15
    INTEnableSystemMultiVectoredInt();
    printf("Override Function has been Initialized\n\n");
    //INTEnableInterrupts();
    INTEnable(INT_CN,1);
}



/**
 * Function: Interrupt Service Routine
 * @return None
 * @remark ISR that is called when CH3 pings external interrupt
 * @author Darrel Deo
 * @date 2013.04.01  */
void __ISR(_CHANGE_NOTICE_VECTOR, ipl2) ChangeNotice_Handler(void){
    mPORTDRead();
    Serial_putChar('Y');

    //Change top-level state machine so it is in state Reciever
    CONTROL_MASTER = RECIEVER_CONTROL;
    OVERRIDE_TRIGGERED = TRUE;
    //Clear the interrupt flag that was risen for the external interrupt
    //might want to set a timer in here

    mCNClearIntFlag();
    INTEnable(INT_CN,0);
}


#endif


//#define ACTUATOR_TEST
#ifdef ACTUATOR_TEST

#include "I2C.h"
#include "TiltCompass.h"
#include "Ports.h"
#include "Drive.h"

// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

//Define and enable the Enable pin for override
#define ENABLE_OUT_TRIS  PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

#define ACTUATOR_DELAY 2500 //ms

#define MAX_PULSE 1750
#define MIN_PULSE 1250
#define STOP_PULSE 1500
#define MICRO 0
#define RECIEVER 1


//#define RECIEVE_CONTROL

int main(){
    //Initializations
    Board_init();
    Serial_init();
    Timer_init();
    Drive_init();
    ENABLE_OUT_TRIS = OUTPUT;  
    ENABLE_OUT_LAT = MICRO;

#ifdef RECIEVE_CONTROL
    ENABLE_OUT_LAT = RECIEVER;
    while(1){
        ;
    }
#endif

    printf("Actuator Test Harness Initiated\n\n");

    //Test Rudder
    printf("Centering rudder.\n");
    setRudder(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);

    printf("Turning rudder left.\n");
    setRudder(MAX_PULSE); //push to one direction
    delayMillisecond(ACTUATOR_DELAY);
    printf("Centering rudder.\n");
    setRudder(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);

    printf("Turning rudder right.\n");
    setRudder(MIN_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    
    printf("Centering rudder.\n");
    setRudder(STOP_PULSE);
    

    //Test Motor Left
    printf("Testing left motor.\n");
    setLeftMotor(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    printf("Driving left motor forward.\n");
    setLeftMotor(MAX_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    setLeftMotor(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    printf("Driving left motor reverse.\n");
    setLeftMotor(MIN_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    setLeftMotor(STOP_PULSE);
   

    //Test Motor Right
    printf("Testing right motor.\n");
    setRightMotor(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    printf("Driving right motor forward.\n");
    setRightMotor(MAX_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    setRightMotor(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    printf("Driving right motor reverse.\n");
    setRightMotor(MIN_PULSE);
    delayMillisecond(ACTUATOR_DELAY);
    setRightMotor(STOP_PULSE);
    delayMillisecond(ACTUATOR_DELAY);

//    // Remove this code
//    setRightMotor(MAX_PULSE);
//    setLeftMotor(MAX_PULSE);
//    while (1)
//        asm("nop");
//
//
//    printf("\nDone with drive test.\n");

}



#endif