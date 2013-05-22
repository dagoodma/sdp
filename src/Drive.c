/*
 * File:   Drive.c
 * Author: 
 *	David Goodman
 *	Darrel Deo
 *
 * Created on March 27, 2013, 12:34 PM
 */
//#define DEBUG

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

// --------------------- Acutator ports -----------------------------
#define MOTOR_LEFT              RC_PORTY07  //RD10, J5-01            //RC_PORTW08 // RB2 -- J7-01
#define MOTOR_RIGHT             RC_PORTY06  //RE7,  J6-16            //RC_PORTW07 // RB3 -- J7-02
#define RUDDER                  RC_PORTV03 //RB2, J7-01

// ---------------------- RC Time Definitions -----------------------
#define RC_STOP_PULSE            1500

// Motor limits
#define RC_MOTOR_MAX            1700
#define RC_MOTOR_MIN            1300
#define RC_MOTOR_FORWARD_RANGE  (RC_MOTOR_MAX - RC_STOP_PULSE)
#define RC_MOTOR_REVERSE_RANGE  (RC_STOP_PULSE - RC_MOTOR_MIN)

// Rudder limits
#define RUDDER_ANGLE_MAX        45.0f // (degree) max rudder angle in either direction
#define RC_RUDDER_MAX           2000
#define RC_RUDDER_MIN           1000
#define RC_RUDDER_LEFT_MAX      RC_RUDDER_MAX
#define RC_RUDDER_RIGHT_MAX     RC_RUDDER_MIN
#define RC_RUDDER_RANGE         (RC_RUDDER_MAX - RC_STOP_PULSE) // left/right dynamic range

// Convert speed (0 to 100%) to RC time
#define MOTOR_PERCENT_TO_RCPULSE(s)           (((uint16_t)RC_MOTOR_FORWARD_RANGE/100)*s + RC_STOP_PULSE)
#define MOTOR_PERCENT_TO_RCPULSE_REVERSE(s)   (((uint16_t)RC_MOTOR_REVERSE_RANGE/100)*s + RC_STOP_PULSE)

// Convert rudder angle to RC time
#define RUDDER_PERCENT_TO_RCPULSE_LEFT(s)     (((uint16_t)RC_RUDDER_RANGE/100)*s + RC_STOP_PULSE)
#define RUDDER_PERCENT_TO_RCPULSE_RIGHT(s)    (-((uint16_t)RC_RUDDER_RANGE/100)*s + RC_STOP_PULSE)


// Delays for control system updating
#define TRACK_UPDATE_DELAY    100 // (ms)

#define DEBUG_PRINT_DELAY   1000 // (ms)

// ------------------------- Controller 
//PD Controller Param Settings for Rudder
#define KP_RUDDER 7.0f
#define KD_RUDDER 0.0f
#define RUDDER_BANGBANG_SPEED_THRESHOLD 17 // (speed %) motor speed threshold
#define RUDDER_BANGBANG_THETA_DEADBAND_THRESHOLD 10 // (degrees) heading error threshold



/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
static enum {
    STATE_IDLE  = 0x0,      // Motors are stopped
    STATE_DRIVE = 0x1,      // Driving forward.
    STATE_PIVOT = 0x2,      // Pivoting in place.
    STATE_TRACK = 0x3,      // Tracking desired heading at speed
} state;


enum {
    RUDDER_TURN_NONE = 0x0,
    RUDDER_TURN_LEFT,   
    RUDDER_TURN_RIGHT, 
} rudderDirection, lastRudderDirection;


static uint8_t desiredSpeed = 0; // (percent) from 0 to 100%

static uint16_t desiredHeading = 0; // (degrees) from North


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
static void startIdleState();
static void startDriveState();
static void startTrackState();
static void stopMotors();
static void setLeftMotor(uint8_t speed);
static void setRightMotor(uint8_t speed);
static void setRudder(char direction, uint8_t percentAngle);
static void updateRudder();


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**********************************************************************
 * Function: Drive_init
 * @return SUCCESS or FAILURE.
 * @remark Initializes the drive system state machine.
 * @author David Goodman
 * @date 2013.03.27 
 **********************************************************************/
bool Drive_init() {


    uint16_t RC_pins = MOTOR_LEFT  | MOTOR_RIGHT | RUDDER;
    RC_init(RC_pins);
    Timer_new(TIMER_DRIVE, 1);

#ifdef DEBUG
    // Timers for debug print statements
    Timer_new(TIMER_TEST2,1);
    Timer_new(TIMER_TEST3,1);
#endif

    state = STATE_IDLE;
}

   
/**********************************************************************
 * Function: Drive_runSM
 * @return None.
 * @remark Executes a cycle of the drive system state machine.
 * @author David Goodman
 * @date 2013.03.27 
 **********************************************************************/
void Drive_runSM() {
    switch (state) {
        case STATE_IDLE:
            // Do nothing
            break;
        case STATE_DRIVE:
            // Just driving, do nothing
            break;
        case STATE_TRACK:
            if (Timer_isExpired(TIMER_DRIVE)) {
                updateRudder();
                Timer_new(TIMER_DRIVE, TRACK_UPDATE_DELAY);
            }
            break;

    } // switch
}

/**********************************************************************
 * Function: Drive_forward
 * @return None
 * @param Speed to drive both motors forward in percent, from 0 to 100.
 * @remark Drives both motors at the given speed.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27 
 **********************************************************************/
void Drive_forward(uint8_t speed) {
    desiredSpeed = speed;

    // Start driving the given speed
    setLeftMotor(speed);
    setRightMotor(speed);

    startDriveState();
}

/**********************************************************************
 * Function: Drive_forwardHeading
 * @return None
 * @param Speed to drive at in meters per second.
 * @param Heading to hold in degrees from north, from 0 to 359.
 * @remark Tracks the given speed and heading.
 * @author David Goodman
 * @date 2013.03.30 
  **********************************************************************/
void Drive_forwardHeading(uint8_t speed, uint16_t angle) {
    desiredSpeed = speed;
    desiredHeading = angle;
    
    // Start driving the given speed
    setLeftMotor(speed);
    setRightMotor(speed);

    // Let rudder controller steer us
    startTrackState();
}

/**********************************************************************
 * Function: Drive_stop
 * @return None
 * @remark Stops both motors from driving.
 * @author David Goodman
 * @author Darrel Deo
 * @date 2013.03.27 
 **********************************************************************/
void Drive_stop() {
    startIdleState();
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/


/**********************************************************************
 * Function: startIdleState
 * @return None
 * @remark Switches into the idle state, which stops the boat.
 **********************************************************************/
static void startIdleState() {
    state = STATE_IDLE;
    desiredSpeed = 0;

    stopMotors();
}

/**********************************************************************
 * Function: startDriveState
 * @return None
 * @remark Switches into the drive state for driving straight forward.
 **********************************************************************/
static void startDriveState() {
    state = STATE_DRIVE;
}

/**********************************************************************
 * Function: startTrackState
 * @return None
 * @remark Switches into the track state for maintaining a heading
 *  and driving forward at a given speed.
 **********************************************************************/
static void startTrackState() {
    state = STATE_TRACK;
    stopMotors();

    lastRudderDirection = RUDDER_TURN_NONE;
}

/**********************************************************************
 * Function: stopMotors
 * @return None
 * @remark Stops both motors from driving.
 **********************************************************************/
static void stopMotors() {
    setLeftMotor(0);
    setRightMotor(0);
}

/**********************************************************************
 * Function: setLeftMotor
 * @param Speed in percent from 0 to 100%.
 * @return None
 * @remark Sets the left motor to drive at the given speed in percent.
 **********************************************************************/
static void setLeftMotor(uint8_t speed) {
    uint16_t rc_time = MOTOR_PERCENT_TO_RCPULSE(speed);
    // Limit the rc times
    if (rc_time > RC_MOTOR_MAX)
        rc_time = RC_MOTOR_MAX;
    if (rc_time < RC_MOTOR_MIN)
        rc_time = RC_MOTOR_MIN;

    RC_setPulseTime(MOTOR_LEFT, rc_time);
}

/**********************************************************************
 * Function: setRightMotor
 * @param Speed in percent from 0 to 100%.
 * @return None
 * @remark Sets the right motor to drive at the given speed in percent.
 **********************************************************************/
static void setRightMotor(uint8_t speed) {
    uint16_t rc_time = MOTOR_PERCENT_TO_RCPULSE(speed);
    // Limit the rc times
    if (rc_time > RC_MOTOR_MAX)
        rc_time = RC_MOTOR_MAX;
    if (rc_time < RC_MOTOR_MIN)
        rc_time = RC_MOTOR_MIN;

    RC_setPulseTime(MOTOR_RIGHT, rc_time);
}

/**********************************************************************
 * Function: setRudder
 * @param Direction left or right to set the rudder to (0 = LEFT, 1 = RIGHT).
 * @param Angle in percent of full rudder range to set to (0 to 100%).
 * @return None
 * @remark Sets the right motor to drive at the given speed in percent.
 **********************************************************************/
static void setRudder(char direction, uint8_t percentAngle) {
    uint16_t rc_time = (direction == RUDDER_TURN_LEFT)?
        RUDDER_PERCENT_TO_RCPULSE_LEFT(percentAngle)
        :
        RUDDER_PERCENT_TO_RCPULSE_RIGHT(percentAngle);

    // Limit the rc times
    if (rc_time > RC_RUDDER_MAX)
        rc_time = RC_RUDDER_MAX;
    if (rc_time < RC_RUDDER_MIN)
        rc_time = RC_RUDDER_MIN;

    RC_setPulseTime(RUDDER, rc_time);
}

// ---------------- Update functions for state machines ---------------

/**********************************************************************
 * Function: updateHeading
 * @return None
 * @remark Determines the heading error using the tilt-compensated compass,
 *  and adjusts the rudder accordingly. Also, a bang-bang control has been
 *  implemented to turn the rudder to the maximum value if the boat's motors
 *  are being driven below some percentage defined above.
 * @author Darrel Deo
 * @author David Goodman
 * @date 2013.03.27 
 **********************************************************************/
static void updateRudder() {

    static int16_t lastThetaError; // used for derivative term
    
    rudderDirection = RUDDER_TURN_LEFT;
    
    // Get current heading, determine theta error
    uint16_t currentHeading = (uint16_t)TiltCompass_getHeading();
    int16_t thetaError = desiredHeading - currentHeading;
    
    // Bound theta error and determine turn direction
    if (thetaError > 0) {
        rudderDirection = (thetaError < 180)? RUDDER_TURN_RIGHT : RUDDER_TURN_LEFT;
        thetaError = (thetaError < 180)? thetaError : (360 - thetaError);
    }
    else {
        // theta error is negative
        rudderDirection = (thetaError > -180)? RUDDER_TURN_LEFT : RUDDER_TURN_RIGHT;
        thetaError = (thetaError > -180)? -thetaError : (360 + thetaError);
    }
    
    // Initialize or dump derivative if changed directions
    if (lastRudderDirection == RUDDER_TURN_NONE || rudderDirection != lastRudderDirection)
        lastThetaError = 0;
        
    /*    Controller Terms    */
    // Derivative (scaled by KD_RUDDER)
    float thetaErrorDerivative = (float)abs(thetaError - 
        lastThetaError)/MS_TO_SEC(TRACK_UPDATE_DELAY);
    
    // Proportional (scaled by KP_RUDDER), convert degrees to percent
    float uDegrees = KP_RUDDER*(float)thetaError + KD_RUDDER*thetaErrorDerivative;
    float uPercent = (uDegrees / RUDDER_ANGLE_MAX ) * 100;
    
    // Limit percentage from 0 to 100
    uPercent = (uPercent > 100.0)? 100.0f : uPercent;
    uPercent = (uPercent < 0.0)? 0.0f : uPercent;

    // Bang-bang control to force rudder all the way if speed is low
    if (desiredSpeed < RUDDER_BANGBANG_SPEED_THRESHOLD
            && thetaError > RUDDER_BANGBANG_THETA_DEADBAND_THRESHOLD)
        uPercent = 100.0f;
    
    // Command the rudder and save 
    setRudder(rudderDirection, (uint8_t)uPercent);
    lastThetaError = thetaError;
    lastRudderDirection = rudderDirection;
        
    #ifdef DEBUG_VERBOSE
    DBPRINT("Rudder control: uDegrees=%.2f, uPercent=%d\n\n", uDegrees, (uint8_t)uPercent);
    #endif
}



/***********************************************************************
 * TEST HARNESSES                                                      *
 ***********************************************************************/

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

//#define PIVOT_TEST_DRIVE
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

//defines for Override feature
#define MICRO_CONTROL            0
#define RECIEVER_CONTROL         1


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


#define ACTUATOR_TEST
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
#define MICRO 1
#define RECIEVER 0


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
    setRudder(RUDDER_TURN_LEFT, 0);
    delayMillisecond(ACTUATOR_DELAY);

    printf("Turning rudder left.\n");
    setRudder(RUDDER_TURN_LEFT, 100); //push to one direction
    delayMillisecond(ACTUATOR_DELAY);
    printf("Centering rudder.\n");
    setRudder(RUDDER_TURN_LEFT, 0);
    delayMillisecond(ACTUATOR_DELAY);

    printf("Turning rudder right.\n");
    setRudder(RUDDER_TURN_RIGHT, 100);
    delayMillisecond(ACTUATOR_DELAY);
    
    printf("Centering rudder.\n");
    setRudder(RUDDER_TURN_LEFT, 0);
    

    //Test Motor Left
    printf("Testing left motor.\n");
    setLeftMotor(0);
    delayMillisecond(ACTUATOR_DELAY);
    printf("Driving left motor forward.\n");
    setLeftMotor(100);
    delayMillisecond(ACTUATOR_DELAY);
    setLeftMotor(0);
    delayMillisecond(ACTUATOR_DELAY);
    //printf("Driving left motor reverse.\n");
    //setLeftMotor(MIN_PULSE);
    //delayMillisecond(ACTUATOR_DELAY);
    //setLeftMotor(0);
   

    //Test Motor Right
    printf("Testing right motor.\n");
    setRightMotor(0);
    delayMillisecond(ACTUATOR_DELAY);
    printf("Driving right motor forward.\n");
    setRightMotor(100);
    delayMillisecond(ACTUATOR_DELAY);
    setRightMotor(0);
    delayMillisecond(ACTUATOR_DELAY);
    //printf("Driving right motor reverse.\n");
    //setRightMotor(MIN_PULSE);
    //delayMillisecond(ACTUATOR_DELAY);
    //setRightMotor(STOP_PULSE);
    //delayMillisecond(ACTUATOR_DELAY);

//    // Remove this code
//    setRightMotor(MAX_PULSE);
//    setLeftMotor(MAX_PULSE);
//    while (1)
//        asm("nop");
//
//
//    printf("\nDone with drive test.\n");
    return SUCCESS;
}
#endif


//#define STATE_MACHINE_TEST
#ifdef STATE_MACHINE_TEST

//Define and enable the Enable pin for override
#define ENABLE_OUT_TRIS  PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06, //0--> Microcontroller control, 1--> Reciever Control

#define STATE_DELAY 2000 //ms

#define MICRO 1
#define RECIEVER 0


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
        asm("nop");
    }
#endif

    printf("State machine test harness.\n\n");

    // Test forward state
    printf("Forward state. Driving at 5, 10, 20, 40, 60, 80, 100, 125 \%.\n");
    uint8_t speed[] = {5, 10, 20, 40, 60, 80, 100, 125};
    int i, len = sizeof(speed);
    delayMillisecond(STATE_DELAY);
    for (i = 0; i < len; i++) {
        Drive_forward(speed[i]);
        delayMillisecond(STATE_DELAY);
    }
    Drive_stop();
    delayMillisecond(STATE_DELAY);

    setRudder(STOP_PULSE);
    delayMillisecond(STATE_DELAY);


    // Forward with heading
    printf("Track state. Driving at 5, 10, 25, 50, 100 \% at 0 deg north.\n");
    uint8_t speed2[] = {5, 10, 25, 50, 100};
    len = sizeof(speed2);
    delayMillisecond(STATE_DELAY);
    for (i = 0; i < len; i++) {
        Drive_forwardHeading(speed2[i], 0);
        delayMillisecond(STATE_DELAY);
    }
    Drive_stop();

    printf("Finished state machine test.\n\n");


    return SUCCESS;
}


#endif
