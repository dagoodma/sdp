/*
 * File:   Barometer.c
 * Author: David Goodman
 *
 * Created on January 22, 2013, 6:19 PM
 */

#include <p32xxxx.h>
//#include <stdio.h>
#include <plib.h>
//#include <math.h>
#include "I2C.h"
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "Accelerometer.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define SLAVE_ADDRESS           0x1D // 0x1D if SA0 is high, 0x1C if low

// List of registers for accelerometer readings
#define WHO_AM_I_ADDRESS        0x0D
#define OUT_X_MSB_ADDRESS       0x01
#define XYZ_DATA_CFG_ADDRESS    0x0E
#define CTRL_REG1_ADDRESS       0x2A

#define WHO_AM_I_VALUE          0x2A // WHO_AM_I_ADDRESS should always be 0x2A


// Printing debug messages over serial
#define DEBUG

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// Acceleration data
int32_t accelerationX, accelerationY, accelerationZ; // (m/s^2)

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

void updateReadings();
int16_t readData( uint8_t address);
int32_t readSensorValue(uint8_t dataAddress);

/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

char Accelerometer_init() {
    uint8_t c = readRegister(WHO_AM_I_ADDRESS);  // Read WHO_AM_I register
    if (c == WHO_AM_I_VALUE)
    {
#ifdef DEBUG
        printf("Accelerometer is online...");
#endif
    }
    else
    {
#ifdef DEBUG
        printf("Could not connect to Accelerometer: 0x%X",c);
#endif
        return FAILURE;
    }

    MMA8452Standby();  // Must be in standby to change registers

    // Set up the full scale range to 2, 4, or 8g.
    byte fsr = GSCALE;
    if(fsr > 8) fsr = 8; //Easy error check
    fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
    writeRegister(XYZ_DATA_CFG, fsr);

    updateReadings();
    Timer_new(TIMER_BAROMETER,UPDATE_DELAY);

    return SUCCESS;
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
 * Function: readRegister
 * @param Address to read from.
 * @return Single byte register value, or -1 if an error occurs.
 * @remark Connects to the device and reads the given register address.
 * @author David Goodman
 * @date 2013.01.22  */
int16_t readRegister( uint8_t address) {
    int8_t  data = 0, success = TRUE;

// Send the start bit with the restart flag low
    if(!I2C_startTransfer(I2C_ID, FALSE)){
#ifdef DEBUG
        printf("FAILED initial transfer!\n");
#endif
        return ERROR;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID, SLAVE_ADDRESS)){
        success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,address)){
#ifdef DEBUG
        printf("Error: Sent byte was not acknowledged\n");
#endif
        success = FALSE;
    }
    if(success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
#ifdef DEBUG
            printf("FAILED Repeated start!\n");
#endif
            success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID, SLAVE_ADDRESS)) {
            success = FALSE;
        }
    }
    if(success){
    // Read the I2C bus most significant byte and send an acknowledge bit
        data = I2C_getData(I2C_ID);
    }
    // Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!success){
#ifdef DEBUG
        printf("Data transfer unsuccessful.\n");
#endif
        return ERROR;
    }
    return data;
}

/**
 * Function: writeRegister
 * @param Address to write to.
 * @return SUCCESS or ERROR.
 * @remark Connects to the device and writes to the given register address.
 * @author David Goodman
 * @date 2013.01.22  */
int16_t writeRegister( uint8_t address, uint8_t value) {
    int8_t  data = 0, success = TRUE;

// Send the start bit with the restart flag low
    if(!I2C_startTransfer(I2C_ID, FALSE)){
#ifdef DEBUG
        printf("FAILED initial transfer!\n");
#endif
        return ERROR;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID, SLAVE_ADDRESS)){
        success = FALSE;
    }
// Tranmit the write address
    if(!I2C_sendData(I2C_ID,address)){
#ifdef DEBUG
        printf("Error: Sent byte was not acknowledged\n");
#endif
        success = FALSE;
    }
    if(success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
#ifdef DEBUG
            printf("FAILED Repeated start!\n");
#endif
            success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID, SLAVE_ADDRESS)) {
            success = FALSE;
        }
    }
    if(success){
    // Read the I2C bus most significant byte and send an acknowledge bit
        data = I2C_getData(I2C_ID);
    }
    // Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!success){
#ifdef DEBUG
        printf("Data transfer unsuccessful.\n");
#endif
        return ERROR;
    }
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
