/* 
 * File:   Barometer.c
 * Author: Shehadeh, David Goodman
 *
 * Created on January 21, 2013, 11:52 AM
 */
#define DEBUG

#include <p32xxxx.h>
#include <stdio.h>
#include <plib.h>
#include <math.h>
#include "I2C.h"
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "Barometer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// List of registers for barometer readings
#define SLAVE_READ_ADDRESS          0xEF
#define SLAVE_WRITE_ADDRESS         0xEE

// Oversampling setting
#define OSS                         3
#define SENSOR_SELECT_ADDRESS       0xF4 // either the temp. or pressure
#define SENSOR_DATA_ADDRESS         0xF6

#define TEMPERATURE_DATA_ADDRESS    0x2E
#define PRESSURE_DATA_ADDRESS       0x34 + (OSS << 6)


// Delay for baro. ADC to sample sensor
//  temp: 4.5 ms, pressure: 4.5, 7.5, 13.5 or 25.5 ms
//  (pressure delay depends on OSS see page 18 in datasheet)
#define UPDATE_DELAY    	100 // (ms)
#define READ_SENSOR_DELAY	25 // (ms) pause for sampling of sensor

// Calibration variable addresses
#define AC1_ADDRESS     0xAA
#define AC2_ADDRESS     0xAC
#define AC3_ADDRESS     0xAE
#define AC4_ADDRESS     0xB0
#define AC5_ADDRESS     0xB2
#define AC6_ADDRESS     0xB4
#define B1_ADDRESS      0xB6
#define B2_ADDRESS      0xB8
#define MB_ADDRESS      0xBA
#define MC_ADDRESS      0xBC
#define MD_ADDRESS      0xBE

// Printing debug messages over serial
#define DEBUG

// Reference pressure at sea level (changes with weather)
#define PRESSURE_P0		102201.209f // (Pa)


/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// Pick the I2C_MODULE to initialize
I2C_MODULE      BAROMETER_COMPAS_I2C_ID = I2C1;
//I2C_MODULE      BAROMETER_ATLAS_I2C_ID = I2C2;

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

// Calibration values
static int16_t ac1;
static int16_t ac2;
static int16_t ac3;
static uint16_t ac4;
static uint16_t ac5;
static uint16_t ac6;
static int16_t b1;
static int16_t b2;
static int16_t mb;
static int16_t mc;
static int16_t md;

// Converted readings
static int32_t temperature; // (0.1 degrees C)
static int32_t pressure; // (Pascal)

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

static int16_t readTwoDataBytes( uint8_t address, int BAROMETER_I2C_ID) ;
static int32_t readThreeDataBytes( uint8_t address, int BAROMETER_I2C_ID);
static int32_t readSensor(uint8_t sensorSelectAddress, int BAROMETER_I2C_ID);
static void updateReadings(int BAROMETER_I2C_ID);

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

void Barometer_init() {

    ac1 = readTwoDataBytes(AC1_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    ac2 = readTwoDataBytes(AC2_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    ac3 = readTwoDataBytes(AC3_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    ac4 = readTwoDataBytes(AC4_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    ac5 = readTwoDataBytes(AC5_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    ac6 = readTwoDataBytes(AC6_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    b1 = readTwoDataBytes(B1_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    b2 = readTwoDataBytes(B2_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    mb = readTwoDataBytes(MB_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    mc = readTwoDataBytes(MC_ADDRESS, BAROMETER_COMPAS_I2C_ID);
    md = readTwoDataBytes(MD_ADDRESS, BAROMETER_COMPAS_I2C_ID);

    updateReadings(BAROMETER_COMPAS_I2C_ID);
    Timer_new(TIMER_BAROMETER,UPDATE_DELAY);

}

float Barometer_getTemperature() {
    return (float)temperature/10;
}

float Barometer_getTemperatureFahrenheit() {
    return CELCIUS_TO_FAHRENHEIT(Barometer_getTemperature());
}

int32_t Barometer_getPressure(){
    return pressure;
}

void Barometer_runSM() {
    if (Timer_isExpired(TIMER_BAROMETER)) {
        updateReadings(BAROMETER_COMPAS_I2C_ID);
        Timer_new(TIMER_BAROMETER,UPDATE_DELAY);
    }
}

float Barometer_getAltitude() {
    return (float)((44330.0f*(1.0f - powf(((float)pressure/PRESSURE_P0),0.19029f))));
}


/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

/**
 * Function: readTwoDataBytes
 * @param Desired address to ping for a read.
 * @return Desired data at requested address.
 * @remark Sends start bit to the slave's address, then the address of the data,
 *      and finally a restart bit before reading the incoming data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
static int16_t readTwoDataBytes( uint8_t address, int BAROMETER_I2C_ID) {
    int8_t success = FALSE;
    int16_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(BAROMETER_I2C_ID, I2C_WRITE )){
            DBPRINT("Barometer: FAILED initial transfer!\n");
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(BAROMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(BAROMETER_I2C_ID,address)){
            DBPRINT("Barometer: Error: Sent byte was not acknowledged\n");
            break;
        }

        // Send a Repeated Started condition
        if(!I2C_startTransfer(BAROMETER_I2C_ID,I2C_READ)){
            DBPRINT("Barometer: FAILED Repeated start!\n");
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(BAROMETER_I2C_ID, SLAVE_READ_ADDRESS))
            break;
        
        // Read the I2C bus twice
        data = (I2C_getData(BAROMETER_I2C_ID) << 8);
        I2C_acknowledgeRead(BAROMETER_I2C_ID, TRUE);
        while(!I2C_hasAcknowledged(BAROMETER_I2C_ID));
        data += I2C_getData(BAROMETER_I2C_ID);

        // Send the stop bit to finish the transfer
        I2C_stopTransfer(BAROMETER_I2C_ID);

        success = TRUE;
    } while(0);
    if (!success) {
        DBPRINT("Barometer: Data transfer unsuccessful.\n");
        return FALSE;
    }
    return data;
}

/**
 * Function: readThreeDataBytes
 * @param Desired address to ping for a read.
 * @return Desired data at requested address.
 * @remark Sends start bit to the slave's address, then the address of the data,
 *      and finally a restart bit before reading the incoming data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
static int32_t readThreeDataBytes( uint8_t address, int BAROMETER_I2C_ID) {
    int8_t success = FALSE;
    int32_t data = 0;

    do {
        // Send the start bit with the restart flag low
        if(!I2C_startTransfer(BAROMETER_I2C_ID, I2C_WRITE )){
            DBPRINT("Barometer: FAILED initial transfer!\n");
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(BAROMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(BAROMETER_I2C_ID,address)){
            DBPRINT("Barometer: Error: Sent byte was not acknowledged\n");
            break;
        }

        // Send a Repeated Started condition
        if(!I2C_startTransfer(BAROMETER_I2C_ID,I2C_READ)){
            DBPRINT("Barometer: FAILED Repeated start!\n");
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(BAROMETER_I2C_ID, SLAVE_READ_ADDRESS))
            break;
        
        // Read the I2C bus twice
        data = (I2C_getData(BAROMETER_I2C_ID) << 16);
        I2C_acknowledgeRead(BAROMETER_I2C_ID, TRUE);
        while(!I2C_hasAcknowledged(BAROMETER_I2C_ID));
		
        data += (I2C_getData(BAROMETER_I2C_ID) << 8);
        I2CAcknowledgeByte(BAROMETER_I2C_ID, TRUE);
        while(!I2CAcknowledgeHasCompleted(BAROMETER_I2C_ID));

        data += I2C_getData(BAROMETER_I2C_ID);
	I2CAcknowledgeByte(BAROMETER_I2C_ID, FALSE);
        while(!I2CAcknowledgeHasCompleted(BAROMETER_I2C_ID));\

        // Roll off extra
        data = data >> (8 - OSS);
		
        // Send the stop bit to finish the transfer
        I2C_stopTransfer(BAROMETER_I2C_ID);

        success = TRUE;
    } while(0);
    if (!success) {
        DBPRINT("Barometer: Data transfer unsuccessful.\n");
        return FALSE;
    }
    return data;
}
/**
 * Function: readSensor
 * @param Sensor to select for sampling and reading.
 * @return Raw temperature or pressure value.
 * @remark Notifies the barometer to send back either temperature or pressure
 *      data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
static int32_t readSensor(uint8_t sensorSelectAddress, int BAROMETER_I2C_ID) {
    bool success = FALSE;
    int32_t data = 0;

    do {
        //Send the start bit to notify that a transmission is starting
        if(!I2C_startTransfer(BAROMETER_I2C_ID, I2C_WRITE)){
            DBPRINT("Barometer: FAILED initial transfer!\n");
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(BAROMETER_I2C_ID, SLAVE_WRITE_ADDRESS))
            break;

        // Designate the sensor select register for writing
        if(!I2C_sendData(BAROMETER_I2C_ID, SENSOR_SELECT_ADDRESS)){
            DBPRINT("Barometer: Error: Sent byte was not acknowledged\n");
            break;
        }

        // Tranmit the sensor to read's address
        if(!I2C_sendData(BAROMETER_I2C_ID,sensorSelectAddress)){
            DBPRINT("Barometer: Error: Sent byte was not acknowledged\n");
            break;
        }
        // End the tranmission
        I2C_stopTransfer(BAROMETER_I2C_ID);

        // Wait while the sensor gets the desired data in the correct register
		// TODO remove this blocking code
        Timer_new(TIMER_BAROMETER2,READ_SENSOR_DELAY);
        while(!Timer_isExpired(TIMER_BAROMETER2));	// max time is 4.5ms

        // Read the address that has the desired data: temperature or pressure
        if(sensorSelectAddress == PRESSURE_DATA_ADDRESS)
            data = readThreeDataBytes(SENSOR_DATA_ADDRESS, BAROMETER_I2C_ID);
        else
            data = readTwoDataBytes(SENSOR_DATA_ADDRESS, BAROMETER_I2C_ID);

        success = TRUE;
    } while(0);

    if(!success){
        DBPRINT("Barometer: Data transfer unsuccessful.\n");
        return ERROR;
    }
        
    return data;
}

/**
 * Function: updateReadings
 * @return
 * @remark Gets the raw temperature and pressure readings and then converts
 * them into actual readings. The final readings are stored into the
 * temperature and pressure variables for future access.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
static void updateReadings(int BAROMETER_I2C_ID) {
    int32_t ut;
    int32_t up;
    int32_t x1, x2, b5, b6, x3, b3, p;
    uint32_t b4, b7;

    // Read the pressure and temperature values from the sensor.
    up = readSensor(PRESSURE_DATA_ADDRESS,BAROMETER_I2C_ID);
    //up &= 0x0000FFFF;
    ut = readSensor(TEMPERATURE_DATA_ADDRESS,BAROMETER_I2C_ID);

	// Temperature conversion
    x1 = (((long)ut - ac6) * ac5) >> 15;
    x2 = ((long) mc << 11) / (x1 + md);
    b5 = x1 + x2;
    temperature = (b5 + 8) >> 4;

	// Presure conversions
    b6 = b5 - 4000;
    x1 = (b2 * (b6 * (b6/pow(2,12))))/pow(2,11);
    x2 = (ac2 * b6)/(pow(2,11));
    x3 = x1 + x2;
    b3 = ((((ac1 * 4) + x3)<< OSS) + 2)/4;
    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (unsigned long)(x3 + 32768)) >> 15;
    b7 = ((unsigned long)up - b3) * (50000 >> OSS);
    if(b7 < 0x80000000){
        p = (b7 * 2)/b4;
    }
    else{
        p = (b7/b4)*2;
    }
    x1 = (p/(pow(2,8))) * (p/(pow(2,8)));
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    pressure = p + ((x1 + x2 + 3791) >> 4);

}

//#define BAROMETER_TEST
#ifdef BAROMETER_TEST

#define PRINT_DELAY     1 // (ms)

int main(void) {
// Initialize the UART,Timers, and I2C1
    Board_init();
    Timer_init();
    float accumulator = 0;
    Serial_init();
    I2C_init(BAROMETER_COMPAS_I2C_ID, I2C_CLOCK_FREQ);
    I2C_init(BAROMETER_ATLAS_I2C_ID, I2C_CLOCK_FREQ);
    //Barometer_init();
    Timer_new(TIMER_TEST, PRINT_DELAY );

    while(1){
        int i;
        for (i = 0; i < 100; ++i){
            Barometer_init(BAROMETER_COMPAS_I2C_ID);
            // Convert the raw data to real values
            while (!Timer_isExpired(TIMER_TEST));
                //printf("Altitude1: %.1f (ft)\n\n", Barometer_getAltitude());
            float baro1 = Barometer_getAltitude();
                Timer_new(TIMER_TEST, PRINT_DELAY );

            Barometer_init(BAROMETER_ATLAS_I2C_ID);
            // Convert the raw data to real values
            while (!Timer_isExpired(TIMER_TEST));
                //printf("Altitude2: %.1f (ft)\n\n", Barometer_getAltitude());
            float baro2 = Barometer_getAltitude();
                Timer_new(TIMER_TEST, PRINT_DELAY );
            accumulator += (baro2 - baro1);
            //Barometer_runSM();
        }
        printf("%.1f\n",7 - (accumulator/100));
        accumulator = 0;
    }

    return (SUCCESS);
}

#endif


//#define BAROMETER_TEST_2
#ifdef BAROMETER_TEST_2

#define PRINT_DELAY     100 // (ms)

int main(void) {
// Initialize the UART,Timers, and I2C1
    Board_init();

    Timer_init();
    Serial_init();
    printf("Serial init done\n");
    I2C_init(BAROMETER_COMPAS_I2C_ID, I2C_CLOCK_FREQ);
    printf("I2C Init done\n");
    Barometer_init();
    printf("Baro init\n");
    Timer_new(TIMER_TEST, PRINT_DELAY );
    printf("Timer \n");

    while(1){

            //Barometer_init(BAROMETER_COMPAS_I2C_ID);
            // Convert the raw data to real values
            while (!Timer_isExpired(TIMER_TEST)){
                Barometer_runSM();
                asm("nop");
            }
                //printf("Altitude1: %.1f (ft)\n\n", Barometer_getAltitude());
            float baro1 = Barometer_getAltitude();
            Timer_new(TIMER_TEST, PRINT_DELAY );
            printf("%f\n", baro1);
            Barometer_runSM();
    }
    return (SUCCESS);
}

#endif
