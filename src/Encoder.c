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
#include "Encoder.h"
#include "Timer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Encoder
#define SLAVE_PITCH_READ_ADDRESS          0x81
#define SLAVE_PITCH_WRITE_ADDRESS         0x80
#define SLAVE_YAW_READ_ADDRESS        0x87
#define SLAVE_YAW_WRITE_ADDRESS       0x86
#define SLAVE_ANGLE_ADDRESS                  0xFE

#define ENCODER_RESOLUTION          14 // (bits)
#define MAX_ENCODER_NUMBER          (1<<ENCODER_RESOLUTION) // (encoder counts)
#define DEGREE_PER_NUMBER           ((float)360.0f / MAX_ENCODER_NUMBER)
//#define DEGREE_PER_NUMBER           (0.02197265625f)

// Raw angle hysteresis for low/high edge selection
#define DELTA_DEGREE      2.5f // (degree) width of hysteresis for choosing sides
#define MIN_DEGREE    0.0f // (degree) minimum measured
#define MAX_DEGREE    360.0f // (degree) max measured


#define ACCUMULATOR_LENGTH  150

#define DEBUG

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      ENCODER_I2C_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  50000 // (Hz)

// Set Desired Operation Frequency
//#define I2C_CLOCK_FREQ  100000 // (Hz)

// Measured encoder angles
float pitchAngle = 0.0; // (degrees) calculated angle
float yawAngle = 0.0; // (degrees) calculated angle

// Calibration related
float zeroPitchAngle = 0.0; // (degrees)
float zeroYawAngle = 0.0; // (degrees)
bool useZeroPitchAngle, useZeroYawAngle;

// Accumulator related
bool accumulateOnLowEdge;
bool accumulatePitch;
uint16_t accumulatorIndex;
float angleAccumulator = 0.0; // (degrees) accumlated angles

// Currently selected encoder variables
float currentZeroAngle;
uint16_t currentReadAddress;
uint16_t currentWriteAddress;

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

static uint16_t readSensor(int SLAVE_READ_ADDRESS,int SLAVE_WRITE_ADDRESS);
static void choosePitchEncoder();
static void chooseYawEncoder();
static void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS);
static float calculateAngle();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


 void Encoder_runSM() {

     if (accumulatorIndex < ACCUMULATOR_LENGTH) {
        accumulateAngle(currentReadAddress, currentWriteAddress);
        accumulatorIndex++;
     }
     else {
        // calculate and switch encoder choice, resetting index
         calculateAngle();
     }
 }

void Encoder_init() {

    useZeroPitchAngle = FALSE;
    useZeroYawAngle = FALSE;

    choosePitchEncoder();
}

void Encoder_setZeroPitch() {
    zeroPitchAngle = pitchAngle;
    useZeroPitchAngle = TRUE;
}


void Encoder_setZeroYaw() {
    zeroYawAngle = yawAngle;
    useZeroYawAngle = TRUE;
}
    
float Encoder_getPitch() {
    return pitchAngle;
}

float Encoder_getYaw() {
    // Invert yaw direction to be CW from north
    return 360.0f - yawAngle;
}



/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

static void choosePitchEncoder() {
    currentZeroAngle = zeroPitchAngle;
    currentReadAddress = SLAVE_PITCH_READ_ADDRESS;
    currentWriteAddress = SLAVE_PITCH_WRITE_ADDRESS;
    angleAccumulator = 0.0;
    accumulatorIndex = 0;
    accumulatePitch = TRUE;
}

static void chooseYawEncoder() {
    currentZeroAngle = zeroYawAngle;
    currentReadAddress = SLAVE_YAW_READ_ADDRESS;
    currentWriteAddress = SLAVE_YAW_WRITE_ADDRESS;
    angleAccumulator = 0.0;
    accumulatorIndex = 0;
    accumulatePitch = FALSE;
}

static void accumulateAngle(int READ_ADDRESS,int WRITE_ADDRESS) {
   float rawAngle = readSensor(READ_ADDRESS,WRITE_ADDRESS) * DEGREE_PER_NUMBER;

   
   // Decide which edge to stay on
   if (accumulatorIndex == 0) {
       accumulateOnLowEdge = (rawAngle <= (MIN_DEGREE + DELTA_DEGREE))?
           TRUE : FALSE;
   }
   
   // Did we start on 0 side, and teetered over to 360 side?
   if (accumulateOnLowEdge && (rawAngle >= (MAX_DEGREE - DELTA_DEGREE)))
       rawAngle -= MAX_DEGREE; // makes it negative

   // Did we start on 360 side and teetered over to the 0 side?
   if (!accumulateOnLowEdge && (rawAngle <= (MIN_DEGREE + DELTA_DEGREE)))
       rawAngle += MAX_DEGREE;
    
    /*
   if  (accumulateOnLowEdge && (rawAngle <= MAX_DEGREE && rawAngle >= (MAX_DEGREE - DELTA_DEGREE))
           || !accumulateOnLowEdge && (rawAngle >=  MIN_DEGREE && rawAngle <= (MIN_DEGREE + DELTA_DEGREE)))
       rawAngle = 0.0f;
   */
   angleAccumulator += rawAngle;

}


static float calculateAngle() {

    float finalAngle = (float)angleAccumulator/ACCUMULATOR_LENGTH;

    // Remove negative angles
    if (finalAngle < 0.0f)
        finalAngle += 360.0f;
    else if (finalAngle > 360.0f)
        finalAngle -= 360.0f;

    // Use zero angle point from calibration
    if((useZeroPitchAngle && accumulatePitch)
            || (useZeroYawAngle && !accumulatePitch)) {
        if(finalAngle >= currentZeroAngle)
            finalAngle = finalAngle - currentZeroAngle;
        else
            finalAngle = 360.0f - (currentZeroAngle - finalAngle);

        //if(finalAngle > 359.98f)
        //    finalAngle = 0.0f;
    }

    // Switch encoders for accumulation
    if (accumulatePitch) {
        pitchAngle = finalAngle;
        chooseYawEncoder();
    }
    else {
        yawAngle = finalAngle;
        choosePitchEncoder();
    }
 }


static uint16_t readSensor(int SLAVE_READ_ADDRESS,int SLAVE_WRITE_ADDRESS) {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(ENCODER_I2C_ID, I2C_WRITE )){
            DBPRINT("Encoder: FAILED initial transfer!\n");
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(ENCODER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(ENCODER_I2C_ID,SLAVE_ANGLE_ADDRESS)){
            DBPRINT("Encoder: Error: Sent byte was not acknowledged\n");
            break;
        }

        // Send a Repeated Started condition
        if(!I2C_startTransfer(ENCODER_I2C_ID,I2C_READ)){
            DBPRINT("Encoder: FAILED Repeated start!\n");
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
        DBPRINT("Encoder: Data transfer unsuccessful.\n");
        return FALSE;
    }
    return data;
}

//#define ENCODER_TEST
#ifdef ENCODER_TEST

#define PRINT_DELAY     50
#define STARTUP_DELAY   2000

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    dbprint("Starting encoders...\n");
    I2C_init(ENCODER_I2C_ID, I2C_CLOCK_FREQ);
    Encoder_init();
    Timer_new(TIMER_TEST, PRINT_DELAY );
    while (!Timer_isExpired(TIMER_TEST)) {
        Encoder_runSM();
    }

    // Not working?
    //Encoder_setZeroPitch();
    //Encoder_setZeroYaw();

    dbprint("Encoders initialized.\n");
    DELAY(STARTUP_DELAY)
    Timer_new(TIMER_TEST, PRINT_DELAY );


    LCD_setPosition(0,0);
    dbprint("Encoders:\n");
    while(1){
        if (Timer_isExpired(TIMER_TEST)) {
            LCD_setPosition(1,0);
            dbprint(" P=%.1f,\n Y=%.1f\n",Encoder_getPitch(), Encoder_getYaw());
            /*dbprint("Encoders:\n P=%d,\n Y=%d\n",
                readSensor(SLAVE_PITCH_READ_ADDRESS,SLAVE_PITCH_WRITE_ADDRESS),
                readSensor(SLAVE_YAW_READ_ADDRESS,SLAVE_YAW_WRITE_ADDRESS));*/
            /*dbprint("Encoders:\n P=%.1f,\n Y=%.1f\n",
                readSensor(SLAVE_PITCH_READ_ADDRESS,SLAVE_PITCH_WRITE_ADDRESS) * DEGREE_PER_NUMBER,
                readSensor(SLAVE_YAW_READ_ADDRESS,SLAVE_YAW_WRITE_ADDRESS) * DEGREE_PER_NUMBER);*/

            
            Timer_new(TIMER_TEST, PRINT_DELAY );
        }
        Encoder_runSM();
    }

    return (SUCCESS);
}

#endif
