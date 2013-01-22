/* 
 * File:   Barometer.c
 * Author: Shehadeh
 *
 * Created on January 21, 2013, 11:52 AM
 */

#include <p32xxxx.h>
#include <stdio.h>
#include <plib.h>
#include "I2C.h"
#include "serial.h"
#include "timers.h"
#include <math.h>

// Pick the I2C_MODULE to initialize
    I2C_MODULE      I2C_ID = I2C1;
// Set Desired Operation Frequency
    UINT32          I2C_CLOCK_FREQ = 100000;
// List of registers for barometer readings
    #define         slaveReadAddress 0xEF
    #define         slaveWriteAddress 0xEE
    #define         barometerDataAddress 0xF4
    #define         temperatureDataAddress 0x2E
    #define         pressureDataAddress 0x34
// Oversampling Setting
    #define         OSS 0
// Calibration values
    short           ac1;
    short           ac2;
    short           ac3;
    unsigned short  ac4;
    unsigned short  ac5;
    unsigned short  ac6;
    short           b1;
    short           b2;
    short           mb;
    short           mc;
    short           md;
// Converted temperature values in fahrenheit and pressure readings in pascals
    long            temperature;
    long            pressure;

short readData( unsigned char address){
    BOOL Success = TRUE;
    char     msb, lsb;
    short   data;

// Send the start bit with the restart flag low
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID,slaveWriteAddress)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,address)){
        printf("Error: Sent byte was not acknowledged\n");
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID,slaveReadAddress)){
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
        printf("Data transfer unsuccessful.\n");
        return 1;
    }
    data = (msb << 8) + lsb;
    return data;
}

long readSensorValue(unsigned char dataAddress){
    BOOL Success = TRUE;
    long data = 0;

//Send the start bit to notify that a transmission is starting
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID,slaveWriteAddress)){
        Success = FALSE;
    }
    if(!I2C_sendData(I2C_ID,barometerDataAddress)){
        printf("Error: Sent byte was not acknowledged\n");
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,dataAddress)){
        printf("Error: Sent byte was not acknowledged\n");
        Success = FALSE;
    }
// End the tranmission
    I2C_stopTransfer(I2C_ID);
    if(!Success){
        printf("Data transfer unsuccessful.\n");
        return 1;
    }
// Wait while the sensor gets the desired data in the correct register
    InitTimer(1,10);
    while(!IsTimerExpired(1));	// max time is 4.5ms

// Read the address that has the desired data: temperature or pressure
    data = readData(0xF6);
    return data;
}

long Barometer_getTemperatureData(void){
    return temperature;
}

long Barometer_getPressureData(void){
    return pressure;
}

void convert(long* temperature, long* pressure){
    long ut;
    long up;
    int x1, x2, b5, b6, x3, b3, p;
    unsigned int b4, b7;

// Read the pressure and temperature values from the sensor.
    up = readSensorValue(pressureDataAddress);
    up &= 0x0000FFFF;
    ut = readSensorValue(temperatureDataAddress);	// some bug here, have to read twice to get good data

    printf("Raw Pressure: %ld\nRaw Temperature: %ld\n",up,ut);

 // Temperature conversion
    x1 = ((long)ut - ac6) * ac5 >> 15;
    x2 = ((long) mc << 11) / (x1 + md);
    b5 = x1 + x2;
    long init_temperature = (b5 + 8) >> 4;
    *temperature  = init_temperature/10 * 9/5 + 32;
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
    b3 = (( ac1 * 4 + x3)<< OSS + 2)/4;
    x1 = ac3 * b6 >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;
    b7 = ((unsigned long) up - b3) * (50000 >> OSS);
    if(b7 < 0x80000000){
        p = (b7 * 2)/b4;
    }
    else{
        p = (b7/b4)*2;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    *pressure = p + ((x1 + x2 + 3791) >> 4);
/*
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
*/
}

int main(void) {
    long altitude = 0;
    double temp = 0;
// Initialize the UART,Timers, and I2C1
    BOARD_Init();
    TIMERS_Init();
    I2C_Init(I2C_ID,I2C_CLOCK_FREQ);

// Get all the calibration data.
    printf("\nCalibration Information:\n");
    printf("------------------------\n");
    ac1 = readData(0xAA);
    ac2 = readData(0xAC);
    ac3 = readData(0xAE);
    ac4 = readData(0xB0);
    ac5 = readData(0xB2);
    ac6 = readData(0xB4);
    b1 = readData(0xB6);
    b2 = readData(0xB8);
    mb = readData(0xBA);
    mc = readData(0xBC);
    md = readData(0xBE);
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

    while(1){
    // Convert the raw data to real values
        convert(&temperature, &pressure);
        printf("Temperature: %ld (in deg F)\n", Barometer_getTemperatureData());
        printf("Pressure: %ld Pa\n", Barometer_getPressureData());
        temp = (double) pressure/101325;
        temp = 1-pow(temp, 0.19029);
        altitude = 44330*temp;
        printf("Altitude: %ld Meters\n\n", altitude);

        InitTimer(1,1000);
        while(!IsTimerExpired(1));
    }

    return (EXIT_SUCCESS);
}

