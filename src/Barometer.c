/* 
 * File:   Barometer.c
 * Author: Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */

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
#define BAROMETER_DATA_ADDRESS      0xF4
#define TEMPERATURE_DATA_ADDRESS    0x2E
#define PRESSURE_DATA_ADDRESS       0x34 // OSS=0
#define PRESSURE_DATA_ADDRESS_OSS1  0x74 // OSS=1
#define PRESSURE_DATA_ADDRESS_OSS2  0xB4 // OSS=2
#define PRESSURE_DATA_ADDRESS_OSS3  0xF4 // OSS=3

// Oversampling Setting
#define OSS             0

// Choose proper pressure address for given OSS
#if OSS == 1
#define PRESSURE_DATA_ADDRESS   PRESSURE_DATA_ADDRESS_OSS1
#elif OSS == 2
#define PRESSURE_DATA_ADDRESS   PRESSURE_DATA_ADDRESS_OSS2
#elif OSS == 3
#define PRESSURE_DATA_ADDRESS   PRESSURE_DATA_ADDRESS_OSS3
#endif


// Delay for updating (minimum of 10ms)
//  for each OSS add 15 ms; 0=15, 1=30 ,2=35
//  (see page 18 in datasheet)
#define UPDATE_DELAY    100 // (ms)

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

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// Pick the I2C_MODULE to initialize
I2C_MODULE      I2C_ID = I2C1;

// Calibration values
int16_t ac1;
int16_t ac2;
int16_t ac3;
uint16_t ac4;
uint16_t ac5;
uint16_t ac6;
int16_t b1;
int16_t b2;
int16_t mb;
int16_t mc;
int16_t md;

// Converted readings
int32_t temperature; // (degrees F)
int32_t pressure; // (Pascal)

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void updateReadings();
int16_t readData( uint8_t address);
int32_t readSensorValue(uint8_t dataAddress);

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

void Barometer_init() {
    ac1 = readData(AC1_ADDRESS);
    ac2 = readData(AC2_ADDRESS);
    ac3 = readData(AC3_ADDRESS);
    ac4 = readData(AC4_ADDRESS);
    ac5 = readData(AC5_ADDRESS);
    ac6 = readData(AC6_ADDRESS);
    b1 = readData(B1_ADDRESS);
    b2 = readData(B2_ADDRESS);
    mb = readData(MB_ADDRESS);
    mc = readData(MC_ADDRESS);
    md = readData(MD_ADDRESS);

    updateReadings();
    Timer_new(TIMER_BAROMETER,UPDATE_DELAY);

}

int32_t Barometer_getTemperatureData(void){
    return temperature;
}

int32_t Barometer_getPressureData(void){
    return pressure;
}

void Barometer_runSM() {
    
    if (Timer_isExpired(TIMER_BAROMETER)) {
        updateReadings();
        Timer_new(TIMER_BAROMETER,UPDATE_DELAY);
    }
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

/**
 * Function: readData
 * @param Address, The desired address to to ping for a read.
 * @return Data, (short) returns a 16-bit variable containing the desired data.
 * @remark Does the entire read process by first sending the start bit.
 * Then the slave's address and the read address is sent. Finally, the
 * Master sends a restart bit and then reads the incoming data to return.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
int16_t readData( uint8_t address) {
    BOOL Success = TRUE;
    int8_t     msb, lsb;
    int16_t   data;

// Send the start bit with the restart flag low
    if(!I2C_startTransfer(I2C_ID, FALSE)){
#ifdef DEBUG
        printf("FAILED initial transfer!\n");
#endif
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID, SLAVE_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,address)){
#ifdef DEBUG
        printf("Error: Sent byte was not acknowledged\n");
#endif
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
#ifdef DEBUG
            printf("FAILED Repeated start!\n");
#endif
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID, SLAVE_READ_ADDRESS)) {
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        msb = I2C_getData(I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);

        while(!I2CAcknowledgeHasCompleted(I2C_ID));

        if(Success){
        // Read the I2C bus lest significant byte
            lsb = I2C_getData(I2C_ID);
        }
    }
// Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!Success){
#ifdef DEBUG
        printf("Data transfer unsuccessful.\n");
#endif
        return FALSE;
    }
    data = (msb << 8) + lsb;
    return data;
}

/**
 * Function: readSensorValue
 * @param DataAddress, The I2C bus line that will be used
 * @return Data, (long) send back the temperature or pressure raw value
 * @remark Notifies the barometer to send back either temperature or pressure
 * data.
 * @author Shehadeh H. Dajani
 * @date 2013.01.21  */
int32_t readSensorValue(uint8_t dataAddress) {
    BOOL Success = TRUE;
    int32_t data = 0;

//Send the start bit to notify that a transmission is starting
    if(!I2C_startTransfer(I2C_ID, FALSE)){
#ifdef DEBUG
        printf("FAILED initial transfer!\n");
#endif
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID, SLAVE_WRITE_ADDRESS)){
        Success = FALSE;
    }
    if(!I2C_sendData(I2C_ID, BAROMETER_DATA_ADDRESS)){
#ifdef DEBUG
        printf("Error: Sent byte was not acknowledged\n");
#endif
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,dataAddress)){
#ifdef DEBUG
        printf("Error: Sent byte was not acknowledged\n");
#endif
        Success = FALSE;
    }
// End the tranmission
    I2C_stopTransfer(I2C_ID);
    if(!Success){
#ifdef DEBUG
        printf("Data transfer unsuccessful.\n");
#endif
        return 1;
    }
// Wait while the sensor gets the desired data in the correct register
    Timer_new(TIMER_BAROMETER,UPDATE_DELAY);
    while(!Timer_isExpired(TIMER_BAROMETER));	// max time is 4.5ms

// Read the address that has the desired data: temperature or pressure
    data = readData(0xF6);
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
void updateReadings() {
    int32_t ut;
    int32_t up;
    int32_t x1, x2, b5, b6, x3, b3, p;
    uint32_t b4, b7;

// Read the pressure and temperature values from the sensor.
    up = readSensorValue(PRESSURE_DATA_ADDRESS);
    up &= 0x0000FFFF;
    ut = readSensorValue(TEMPERATURE_DATA_ADDRESS); // some bug here, have to read twice to get good data
#ifdef DEBUG
    printf("Raw Pressure: %ld\nRaw Temperature: %ld\n",up,ut);
#endif

 // Temperature conversion
    x1 = ((int32_t)ut - ac6) * ac5 >> 15;
    x2 = ((int32_t) mc << 11) / (x1 + md);
    b5 = x1 + x2;
    int32_t init_temperature = (b5 + 8) >> 4;
    temperature  = init_temperature/10 * 9/5 + 32;
    /*
    printf("UT = %d\n",ut);
    printf("X1 = %d\n",x1);
    printf("X2 = %d\n",x2);
    printf("b5 = %d\n\n",b5);
     */
// Presure conversions
    b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = ac2 * b6 >> 11;
    x3 = x1 + x2;
    b3 = (( ac1 * 4 + x3)<< OSS + 2) >> 2;
    x1 = ac3 * b6 >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (uint32_t) (x3 + 32768)) >> 15;
    b7 = ((uint32_t) up - b3) * (50000 >> OSS);
    if(b7 < 0x80000000){
        p = (b7 * 2)/b4;
    }
    else{
        p = (b7/b4)*2;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    pressure = p + ((x1 + x2 + 3791) >> 4);
#ifdef DEBUG_VERBOSE
    printf("B6 = %d\n",b6);
    printf("X1 = %d\n",x1);
    printf("X2 = %d\n",x2);
    printf("X3 = %d\n",x3);
    printf("B3 = %d\n",b3);
    printf("X1 = %d\n",x1);
    printf("X2 = %d\n",x2);
    printf("X3 = %d\n",x3);
    printf("B4 = %d\n",b4);
    printf("B7 = %d\n",b7);
    printf("p = %d\n",p);
    printf("X1 = %d\n",x1);
    printf("X1 = %d\n",x1);
    printf("X2 = %d\n",x2);
#endif
}


#define BAROMETER_TEST
#ifdef BAROMETER_TEST

// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)
#define PRINT_DELAY     1000 // (ms)

int main(void) {
    int32_t altitude = 0;
    double temp = 0;
// Initialize the UART,Timers, and I2C1
    Board_init();
    Timer_init();
    I2C_init(I2C_ID, I2C_CLOCK_FREQ);
    Barometer_init();

// Get all the calibration data.
    printf("\nCalibration Information:\n");
    printf("------------------------\n");

    printf("\tAC1 = %d\n", ac1);
    printf("\tAC2 = %d\n", ac2);
    printf("\tAC3 = %d\n", ac3);
    printf("\tAC4 = %d\n", ac4);
    printf("\tAC5 = %d\n", ac5);
    printf("\tAC6 = %d\n", ac6);
    printf("\tB1 = %d\n", b1);
    printf("\tB2 = %d\n", b2);
    printf("\tMB = %d\n", mb);
    printf("\tMC = %d\n", mc);
    printf("\tMD = %d\n", md);
    printf("------------------------\n\n");

    Timer_new(TIMER_TEST, PRINT_DELAY );

    while(1){
    // Convert the raw data to real values
        if (Timer_isExpired(TIMER_TEST)) {
            printf("Temperature: %ld (in deg F)\n", Barometer_getTemperatureData());
            printf("Pressure: %ld Pa\n", Barometer_getPressureData());
            temp = (double) pressure/101325;
            temp = 1-pow(temp, 0.19029);
            altitude = 44330*temp;
            printf("Altitude: %ld Meters\n\n", altitude);
            Timer_new(TIMER_TEST, PRINT_DELAY );

        }

        Barometer_runSM();
    }

    return (SUCCESS);
}

#endif
