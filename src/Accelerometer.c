/*
 * File:   Accelerometer.c
 * Author: David Goodman
 *
 * Created on January 22, 2013, 6:19 PM
 */

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "I2C.h"
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "Accelerometer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define SLAVE_ADDRESS           0x1D // 0x1D if SA0 is high, 0x1C if low
#define SLAVE_WRITE_ADDRESS     0x3A//((SLAVE_ADDRESS << 1) + 0)
#define SLAVE_READ_ADDRESS      0x3B//((SLAVE_ADDRESS << 1) + 1)

// List of registers for accelerometer readings
#define WHO_AM_I_ADDRESS        0x0D
#define OUT_X_MSB_ADDRESS       0x01
#define XYZ_DATA_CFG_ADDRESS    0x0E
#define CTRL_REG1_ADDRESS       0x2A

#define WHO_AM_I_VALUE          0x2A // WHO_AM_I_ADDRESS should always be 0x2A

// Options
#define GSCALE          2 // Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.

#define UPDATE_DELAY    50 // (ms) delay for reading

// Printing debug messages over serial
#define DEBUG

//#define USE_ACCUMULATOR         // simple low-pass filter. comment line out to disable
#define ACCUMULATOR_LENGTH      2 // use a power of 2 and update shift too
#define ACCUMULATOR_SHIFT       1 // 2^shift = length

#define MINIMUM_LEVEL_ERROR     2 // (1e-1 G-counts)

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
// Pick the I2C_MODULE to initialize
I2C_MODULE      I2C_ID = I2C1;

struct {
    uint16_t x, y , z;
} gCount;

struct {
    uint32_t x , y, z;
} gAccumulator;

uint8_t accumulatorIndex = 0;



/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void updateReadings();
void setActiveMode();
void setStandbyMode();
int16_t readRegister( uint8_t address);
int16_t readRegisters( uint8_t address, uint16_t bytesToRead, uint8_t *dest );
int16_t writeRegister( uint8_t address, uint8_t data );
void resetAccumulator();

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

char Accelerometer_init() {
    int16_t c = readRegister(WHO_AM_I_ADDRESS);  // Read WHO_AM_I register
    if (c == WHO_AM_I_VALUE) //c != ERROR ||
    {
#ifdef DEBUG
        //printf("Accelerometer is online...\n");
#endif
    }
    else
    {
#ifdef DEBUG
        printf("Could not connect to Accelerometer: 0x%X\n",c);
#endif
        return FAILURE;
    }

    setStandbyMode(); // Must be in standby to change registers

    // Set up the full scale range to 2, 4, or 8g.
    char fsr = GSCALE;
    if(fsr > 8) fsr = 8; // Limit G-Scale
    fsr >>= 2; // 00 = 2G, 01 = 4A, 10 = 8G (pg. 20)
    writeRegister(XYZ_DATA_CFG_ADDRESS, fsr);

    setActiveMode();
    Timer_new(TIMER_ACCELEROMETER, UPDATE_DELAY);

#ifdef USE_ACCUMULATOR
    resetAccumulator();
#endif

    return SUCCESS;
}

int16_t Accelerometer_getX(){
    return gCount.x;
}

int16_t Accelerometer_getY(){
    return gCount.y;
}

int16_t Accelerometer_getZ(){
    return gCount.z;
}

void Accelerometer_runSM() {

    if (Timer_isExpired(TIMER_ACCELEROMETER)) {
        updateReadings();
        Timer_new(TIMER_ACCELEROMETER, UPDATE_DELAY);
    }
}

bool Accelerometer_isLevel() {
    return abs(gCount.x - gCount.y) <= MINIMUM_LEVEL_ERROR;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/


/**
 * Function: updateReadings
 * @return None
 * @remark Records the current G-values from the sensor. Accumulation (low-pass)
 *  filtering will occur if USE_ACCUMULATOR is defined.
 * @author David Goodman
 * @date 2013.01.23  */
void updateReadings() {
    uint8_t rawData[6];  // x/y/z accel register data stored here

    readRegisters(OUT_X_MSB_ADDRESS, 6, rawData);  // Read the six raw data registers into data array

    int i;
    // Loop to calculate 12-bit ADC and g value for each axis
    for(i = 0; i < 3 ; i++)
    {
        int16_t gCountI = (rawData[i*2] << 8) | rawData[(i*2)+1];  //Combine the two 8 bit registers into one 12-bit number
        gCountI >>= 4; //The registers are left align, here we right align the 12-bit integer

        // If the number is negative, we have to make it so manually (no 12-bit data type)
        if (rawData[i*2] > 0x7F)
        {
            gCountI = ~gCountI + 1;
            gCountI *= -1;  // Transform into negative 2's complement #
        }

        //Record this gCount into the struct or short ints
        switch (i) {
            #ifndef USE_ACCUMULATOR
            case 0:
                gCount.x = gCountI;
                break;
            case 1:
                gCount.y = gCountI;
                break;
            case 2:
                gCount.z = gCountI;
                break;
            #else
            case 0:
                gAccumulator.x += gCountI;
                break;
            case 1:
                gAccumulator.y += gCountI;
                break;
            case 2:
                gAccumulator.z += gCountI;
                break;
            #endif
        }
    }
    // Update gCounts if accumulating
    #ifdef USE_ACCUMULATOR
    accumulatorIndex++;
    if (accumulatorIndex >= ACCUMULATOR_LENGTH) {
        gCount.x = (uint16_t)(gAccumulator.x >> ACCUMULATOR_SHIFT);
        gCount.y = (uint16_t)(gAccumulator.y >> ACCUMULATOR_SHIFT);
        gCount.z = (uint16_t)(gAccumulator.z >> ACCUMULATOR_SHIFT);

        resetAccumulator();
    }
    #endif
}



/**
 * Function: resetAccumulator
 * @return None
 * @remark Resets the accumulator.
 * @author David Goodman
 * @date 2013.01.23  */
void resetAccumulator() {
    gAccumulator.x = 0;
    gAccumulator.y = 0;
    gAccumulator.z = 0;
    accumulatorIndex = 0;
}

/**
 * Function: setActiveMode
 * @return Single byte register value, or -1 if an error occurs.
 * @remark Sets the device into active mode. It must be in standby to change
 *       most register settings.
 * @author David Goodman
 * @date 2013.01.23  */
void setActiveMode() {
  char c = readRegister(CTRL_REG1_ADDRESS);
  writeRegister(CTRL_REG1_ADDRESS, c | 0x01); //Set the active bit to begin detection
}

/**
 * Function: setStandbyMode
 * @return None.
 * @remark Sets the device into standby mode. It must be in standby to change
 *       most register settings.
 * @author David Goodman
 * @date 2013.01.23  */
void setStandbyMode() {
  char c = readRegister(CTRL_REG1_ADDRESS);
  writeRegister(CTRL_REG1_ADDRESS, c & ~(0x01)); //Clear the active bit to go into standby
}

/**
 * Function: readRegister
 * @param Address to read from.
 * @return Single byte register value, or -1 if an error occurs.
 * @remark Connects to the device and reads the given register address.
 * @author David Goodman
 * @date 2013.01.22  */
int16_t readRegister( uint8_t address ) {
    int8_t data = 0, success = FALSE;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(I2C_ID, I2C_WRITE )) {
            return ERROR;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(I2C_ID, SLAVE_WRITE_ADDRESS))
            break;
        // Transmit the read address
        if(!I2C_sendData(I2C_ID,address))
            break;
        // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,I2C_READ))
            break;
        // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID, SLAVE_READ_ADDRESS))
            break;

        // Read the I2C bus 
        data = I2C_getData(I2C_ID);

        success = TRUE;
    } while(0);

    // Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!success){
#ifdef DEBUG
        printf("Failed to read register 0x%X.\n", address);
#endif
        return ERROR;
    }
    return data;
}

/**
 * Function: readRegisters
 * @param Address to read from.
 * @param Number of bytes to read.
 * @param Address of array to store the bytes in.
 * @return SUCCESS or ERROR.
 * @remark Connects to the device and reads the given register address.
 * @author David Goodman
 * @date 2013.01.22  */
int16_t readRegisters( uint8_t address, uint16_t bytesToRead, uint8_t *dest ) {
    int8_t success = FALSE;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(I2C_ID, I2C_WRITE))
            return ERROR;
        // Transmit the slave's address to notify it
        if (!I2C_sendData(I2C_ID, SLAVE_WRITE_ADDRESS))
            break;
        // Transmit the read address
        if(!I2C_sendData(I2C_ID,address))
            break;
        // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,I2C_READ))
            break;
        // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID, SLAVE_READ_ADDRESS))
            break;

        int i;
        // Read the I2C bus and send an acknowledge bit each time
        for(

                i = 0 ; i < bytesToRead ; i++) {
            dest[i] = I2C_getData(I2C_ID);

            // Only send and wait for Ack if there's more to read
            if (i < (bytesToRead - 1)) {
                I2CAcknowledgeByte(I2C1, TRUE);
                //I2C_acknowledgeRead(I2C1, I2C_ACK);

                //while(!I2C_hasAcknowledged(I2C_ID));
                while(!I2CAcknowledgeHasCompleted(I2C_ID));
            }
        }
        success = TRUE;
    } while(0);

    // Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!success){
#ifdef DEBUG
        printf("Failed to read registers 0x%X.\n", address);
#endif
        return ERROR;
    }
    return SUCCESS;
}

/**
 * Function: writeRegister
 * @param Address to write to.
 * @return SUCCESS or ERROR.
 * @remark Connects to the device and writes to the given register address.
 * @author David Goodman
 * @date 2013.01.22  */
int16_t writeRegister( uint8_t address, uint8_t data ) {
    int8_t  success = FALSE;

// Send the start bit with the restart flag low
    do {
        if(!I2C_startTransfer(I2C_ID, I2C_WRITE))
            return ERROR;
    // Transmit the slave's address to notify it
        if (!I2C_sendData(I2C_ID, SLAVE_WRITE_ADDRESS))
            break;
    // Tranmit the write address
        if(!I2C_sendData(I2C_ID,address))
            break;
    // Transmit the data to write
        if(!I2C_sendData(I2C_ID,data))
            break;

        success = TRUE;
    } while(FALSE);

    // Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!success){
#ifdef DEBUG
        printf("Failed to write to register 0x%X.\n", address);
#endif
        return ERROR;
    }
    return success;
}





//#define ACCELEROMETER_TEST
#ifdef ACCELEROMETER_TEST

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)
#define PRINT_DELAY     250 // (ms)

int main(void) {

    // Initialize the modules
    Board_init();
    Timer_init();
    Serial_init();
    I2C_init(I2C_ID, I2C_CLOCK_FREQ);

    //printf("Who am I: 0x%X\n", readRegister(WHO_AM_I_ADDRESS));

    if (Accelerometer_init() != SUCCESS) {
        printf("Failed to initialize the accelerometer.\n");
        return FAILURE;
    }
    printf("Initialized the accelerometer.\n");
    Timer_new(TIMER_TEST, PRINT_DELAY );

    while(1){
    // Convert the raw data to real values
        if (Timer_isExpired(TIMER_TEST)) {
            printf("**G-Counts: x=%.2f, y=%.2f, z=%.2f\n\n",
                (float)Accelerometer_getX()/1000,
                (float)Accelerometer_getY()/1000,
                (float)Accelerometer_getZ()/1000);
            Timer_new(TIMER_TEST, PRINT_DELAY );
        }

        Accelerometer_runSM();
    }

    return (SUCCESS);

}

#endif

//#define CC_CALIBRATION_TEST
#ifdef CC_CALIBRATION_TEST

#include "Ports.h"

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  80000 // (Hz)
#define LED_DELAY     1 // (ms)

#define G_DELTA         20 // (0.001 G) scaled by 1e-3 == 0.02 G
#define G_X_DESIRED     0
#define G_Y_DESIRED     0
#define G_Z_DESIRED     1000 // (0.001 G) scaled by 1e-3 == 1 G

#define LED_N           PORTZ06_LAT // RD0
#define LED_S           PORTZ04_LAT // RF1
#define LED_E           PORTY12_LAT // RD1
#define LED_W           PORTY10_LAT // RD2

#define LED_N_TRIS      PORTZ06_TRIS // RD0
#define LED_S_TRIS      PORTZ04_TRIS // RF1
#define LED_E_TRIS      PORTY12_TRIS // RD1
#define LED_W_TRIS      PORTY10_TRIS // RD2

int main(void) {

    // Initialize the modules
    Board_init();
    Timer_init();
    Serial_init();
    I2C_init(I2C_ID, I2C_CLOCK_FREQ);

    //printf("Who am I: 0x%X\n", readRegister(WHO_AM_I_ADDRESS));

    if (Accelerometer_init() != SUCCESS) {
        printf("Failed to initialize the accelerometer.\n");
        return FAILURE;
    }
    printf("Initialized the accelerometer.\n");

    // Configure ports as outputs
    LED_N_TRIS = OUTPUT;
    LED_S_TRIS = OUTPUT;
    LED_E_TRIS = OUTPUT;
    LED_W_TRIS = OUTPUT;
    
    Timer_new(TIMER_TEST, LED_DELAY );

    while(1){
    // Convert the raw data to real values
        if (Timer_isExpired(TIMER_TEST)) {
            // X-Axis
            if (Accelerometer_getX() <= (G_X_DESIRED - G_DELTA)) {
                LED_N = ON;
                LED_S = OFF;
            }
            else if (Accelerometer_getX() >= (G_X_DESIRED + G_DELTA)) {
                LED_N = OFF;
                LED_S = ON;
            }
            else {
                LED_N = OFF;
                LED_S = OFF;
            }

            // Y-Axis
            if (Accelerometer_getY() <= (G_Y_DESIRED - G_DELTA)) {
                LED_E = OFF;
                LED_W = ON;
            }
            else if (Accelerometer_getY() >= (G_Y_DESIRED + G_DELTA)) {
                LED_E = ON;
                LED_W = OFF;
            }
            else {
                LED_E = OFF;
                LED_W = OFF;
            }
            
            Timer_new(TIMER_TEST, LED_DELAY );
        }

        Accelerometer_runSM();
    }

    return (SUCCESS);

}

#endif

