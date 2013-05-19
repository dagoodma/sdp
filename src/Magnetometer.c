/* 
 * File:   Magnetometer.c
 * Author: David Goodman, Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */
//#define DEBUG

#include <xc.h>
#include <stdio.h>
#include <math.h>
#include <plib.h>
#include <stdbool.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for Encoder

#define SLAVE_READ_ADDRESS          0x43
#define SLAVE_WRITE_ADDRESS         0x42

#define READ_EEPROM_ADDRESS         0x47
#define READ_SLAVE_ADDRESS          0x00
#define READ_DEGREE_ADDRESS         0x41

#define ACCUMULATOR_LENGTH          15

#define MAGNETIC_NORTH_OFFSET       13.7275f // (deg) offset eastward from true north
#define DEGREE_1E1_TO_DEGREE(d)    ((float)d/10.0f)

#define MINIMUM_NORTH_ERROR         1.2f // (degrees) minimum error from North

//#define FLIP_NORTH_SOUTH

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      MAGNETOMETER_I2C_ID = I2C1;
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

static uint16_t accumulatorIndex;
static uint32_t accumulator;
static float heading; // (degrees)

static bool haveReading;


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

static uint16_t readDevice(uint8_t dataAddress);
static uint16_t readDeviceEEPROM(uint8_t eeAddress);
static void calculateHeading();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

char Magnetometer_init() {
    accumulatorIndex = 0;
    accumulator = 0.0f;
    haveReading = FALSE;

    //(void)readDevice(READ_DEGREE_ADDRESS);
    /*
    uint8_t readAddress = readDeviceEEPROM(READ_SLAVE_ADDRESS);
    if (readAddress  != SLAVE_WRITE_ADDRESS) {
        DBPRINT("Magnetometer: Failed to read EEPROM address (0x%X)\n", readAddress );
        return FAILURE;
    }*/

    if (I2C_hasError())
        return FAILURE;

    return SUCCESS;
}

float Magnetometer_getHeading() {
#ifdef FLIP_NORTH_SOUTH
    if (heading > 180.0f)
        return (heading - 180.0f);
    else
        return (heading + 180.0f);
#else
    return heading;
#endif
}

void Magnetometer_runSM(){

    if (accumulatorIndex < ACCUMULATOR_LENGTH) {
        accumulator += readDevice(READ_DEGREE_ADDRESS);
        accumulatorIndex++;
    }

    if (accumulatorIndex >= ACCUMULATOR_LENGTH) {
        calculateHeading();
        accumulatorIndex = 0;
        accumulator = 0.0f;
        haveReading = TRUE;
    }
}


bool Magnetometer_isNorth() {
    return haveReading && (Magnetometer_getHeading() <= MINIMUM_NORTH_ERROR
            || Magnetometer_getHeading() >= (360.0 - MINIMUM_NORTH_ERROR));
}

void Magnetometer_enableSleepMode() {
    // TODO add this

}

void Magnetometer_disableSleepMode() {
    // TODO add this
}

/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

static void calculateHeading() {
    heading = (float)accumulator/ACCUMULATOR_LENGTH;
    heading = DEGREE_1E1_TO_DEGREE(heading);

    // Remove magnetic north offset
    heading -= MAGNETIC_NORTH_OFFSET;

    // Bound heading
    if (heading > 360.0f)
        heading -= 360.0f;
    else if (heading < 0.0f)
        heading += 360.0f;

}
 

static uint16_t readDevice(uint8_t dataAddress) {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID, I2C_WRITE )){
            DBPRINT("Magnetometer: Failed to start transfer.\n");
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(MAGNETOMETER_I2C_ID, dataAddress)){
            DBPRINT("Magnetometer: Sent byte was not acknowledged\n");
            break;
        }
        // Send a Repeated Started condition
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID,I2C_READ)){
            DBPRINT("Magnetometer: Failed repeated start condition.\n");
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_READ_ADDRESS))
            break;
        
        // Read the I2C bus twice
        data = (I2C_getData(MAGNETOMETER_I2C_ID) << 8);
        I2C_acknowledgeRead(MAGNETOMETER_I2C_ID, TRUE);
        while(!I2C_hasAcknowledged(MAGNETOMETER_I2C_ID));
        data += (I2C_getData(MAGNETOMETER_I2C_ID));

        // Send the stop bit to finish the transfer
        I2C_stopTransfer(MAGNETOMETER_I2C_ID);

        success = TRUE;
    } while(0);
    if (!success) {
        DBPRINT("Magnetometer: Data transfer unsuccessful.\n");
        return FALSE;
    }
    return data;
}

static uint16_t readDeviceEEPROM(uint8_t eeAddress) {
    int8_t success = FALSE;
    uint16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID, I2C_WRITE )){
            DBPRINT("Magnetometer: Failed to start transfer.\n");
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Wait for ack here?

        // Transmit 'r' command
        if(!I2C_sendData(MAGNETOMETER_I2C_ID, READ_EEPROM_ADDRESS)){
            DBPRINT("Magnetometer: Sent byte was not acknowledged.\n");
            break;
        }

        // Wait for slave's ack
        /*
        if (I2C_waitForAcknowledgement(MAGNETOMETER_I2C_ID) != TRUE)
            break;
        */
        // Transmit the address of EEPROM data
        if(!I2C_sendData(MAGNETOMETER_I2C_ID, eeAddress)){
            DBPRINT("Magnetometer: Second sent byte was not acknowledged.\n");
            break;
        }

        // Wait for slave's ack
        /*
        if (I2C_waitForAcknowledgement(MAGNETOMETER_I2C_ID) != TRUE)
            break;
        */
        // Start a read from device EEPROM
        if(!I2C_startTransfer(MAGNETOMETER_I2C_ID,I2C_READ)){
            DBPRINT("Magnetometer: Failed repeated start condition.\n");
            break;
        }

        // Transmit the address with the READ bit set
        if (!I2C_sendData(MAGNETOMETER_I2C_ID, SLAVE_READ_ADDRESS))
            break;

        // Read the I2C bus twice
        data = I2C_getData(MAGNETOMETER_I2C_ID);
        I2C_stopTransfer(MAGNETOMETER_I2C_ID);

        success = TRUE;

    } while(0);
    if (!success) {
        DBPRINT("Magnetometer: Data transfer unsuccessful.\n");
        return FALSE;
    }
    return data;
}

//#define MAGNETOMETER_TEST
#ifdef MAGNETOMETER_TEST

#define PRINT_DELAY         500 // (ms)
#define STARTUP_DELAY       1000

int main(void) {
// Initialize the UART,Timers, and I2C1v
    Board_init();
    Board_configure(USE_SERIAL | USE_LCD | USE_TIMER);
    I2C_init(MAGNETOMETER_I2C_ID, I2C_CLOCK_FREQ);
    dbprint("Initializing mag.\n");
    if (Magnetometer_init() != SUCCESS) {
        dbprint("Failed mag. init\n");
        //return FAILURE;
    }
    dbprint("Mag. initialized.\n");
    DELAY(STARTUP_DELAY);

    LCD_clearDisplay();
    Timer_new(TIMER_TEST, PRINT_DELAY);
    while(1){
        Magnetometer_runSM();
        if (Timer_isExpired(TIMER_TEST)) {
            LCD_setPosition(0,0);
            char *debug = (Magnetometer_isNorth())?
                "(N)" : "";
            dbprint("Mag: %.1f %s, Raw=%d\n", Magnetometer_getHeading(),
                    debug, readDevice(READ_DEGREE_ADDRESS));
            Timer_new(TIMER_TEST, PRINT_DELAY);
        }
    }

    return (SUCCESS);
}

#endif
