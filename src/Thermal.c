/*
 * File:   Thermal.c
 * Author: Shehadeh H. Dajani
 *
 * Created on January 21, 2013, 10:52 PM
 */

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "Board.h"
#include "I2C.h"
#include "Serial.h"
#include "Timer.h"
#include <math.h>

//#define DEBUG

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define EEPROM_WRITE_ADDRESS            0xA0
#define EEPROM_READ_ADDRESS             0xA1
#define CAMERA_WRITE_ADDRESS            0xC0
#define CAMERA_READ_ADDRESS             0xC1

#define EEPROM_READ_COMMAND             0x00
#define CAMERA_READ_COMMAND             0x02
#define CAMERA_WRITE_CONFIG_COMMAND     0x03
#define CAMERA_WRITE_TRIM_COMMAND       0x04


#define TOTAL_PIXEL_ROWS    4
#define TOTAL_PIXEL_COLS    16
#define TOTAL_PIXELS        (TOTAL_PIXEL_ROWS * TOTAL_PIXEL_COLS)

#define READ_DELAY      100

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

// Pick the I2C_MODULE to initialize
I2C_MODULE      THERMAL_I2C_ID = I2C1;
// Set Desired Operation Frequency
UINT32          I2C_CLOCK_FREQ = 100000;
UINT8           eepromData[256];
UINT16          rawTemp;
float           V_th, K_t1, K_t2, chipTempC, chipTempF, emissivity;
float           V_ir_tgc_comp[TOTAL_PIXELS], finalPixelTempF[TOTAL_PIXELS];
int             pixelData[TOTAL_PIXELS], A_cp, B_cp, Tgc, B_iscale, CPixel;
int             A_ij[TOTAL_PIXELS], B_ij[TOTAL_PIXELS];

float alpha[TOTAL_PIXELS] = {1.5935E-09, 3.22331E-09, 2.99048E-09, 1.38305E-10, 3.9218E-09, 4.91133E-09, 4.44567E-09, 2.05916E-09,
                   4.91133E-09, 6.54115E-09, 6.07549E-09, 3.63077E-09, 5.84266E-09, 7.29785E-09, 6.30832E-09, 5.14416E-09,
                   6.89039E-09, 8.69483E-09, 8.69483E-09, 6.30832E-09, 7.76351E-09, 9.91719E-09, 9.80078E-09, 7.76351E-09,
                   8.92766E-09,  1.06157E-08, 8.40379E-09, 7.76351E-09, 7.87992E-09, 1.13142E-08, 1.05575E-08, 8.69483E-09,
                   8.40379E-09, 1.13142E-08, 1.08485E-08, 7.93813E-09, 7.99634E-09, 1.13142E-08, 1.1547E-08, 8.34559E-09,
                   8.40379E-09, 1.13142E-08, 1.10231E-08, 8.86945E-09, 5.60983E-09, 1.09649E-08, 1.10231E-08, 9.91719E-09,
                   6.65756E-09, 9.9754E-09, 9.50974E-09, 7.23964E-09, 6.48294E-09, 8.69483E-09, 8.63662E-09, 5.95907E-09,
                   4.50388E-09, 6.9486E-09, 6.71577E-09, 4.50388E-09, 3.22331E-09, 5.31879E-09, 5.31879E-09, 3.33973E-09};

uint8_t count = 0;


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/
void readEeprom(void);
void readConfigReg(void);
void writeTrimmingValue(void);

void writeConfigReg(void);

void configCalculationData(void);


void readChipTemp(void);

void calculateChipTemp(void);

void readCPixelValue(void);
void readPixelValue(void);

void calculateIRTemp(void);
/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/


void Thermal_init(){
    I2C_init(THERMAL_I2C_ID,I2C_CLOCK_FREQ);
    count = 0;

    readEeprom();
    writeTrimmingValue();
    writeConfigReg();
    readConfigReg();
    configCalculationData();

    Timer_new(TIMER_THERMAL,READ_DELAY);
}

void Thermal_runSM() {
    if (Timer_isExpired(TIMER_THERMAL)) {
        #ifdef DEBUG
        printf("Reading sensor...\n");
        #endif
        if(count == 0){
            readChipTemp();
            calculateChipTemp();
        }
        count++;
        if(count >= 16){
            count = 0;
        }
        readPixelValue();
        readCPixelValue();
        calculateIRTemp();

        Timer_new(TIMER_THERMAL,READ_DELAY);
    }
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

void readEeprom(void){
    BOOL success = FALSE;
    int index;

    do {
        if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
            #ifdef DEBUG
            printf("FAILED initial transfer!\n");
            #endif
            break;
        }
        // Transmit the slave's address to notify it
        if (!I2C_sendData(THERMAL_I2C_ID,EEPROM_WRITE_ADDRESS))
            break;

        // Tranmit the read address module
        if(!I2C_sendData(THERMAL_I2C_ID,EEPROM_READ_COMMAND)){
            #ifdef DEBUG
            printf("Error: Sent byte was not acknowledged\n");
            #endif
            break;
        }

        // Send a Repeated Started condition
        if(!I2C_startTransfer(THERMAL_I2C_ID,TRUE)){
            #ifdef DEBUG
                printf("FAILED Repeated start!\n");
            #endif
            break;
        }
        // Transmit the address with the READ bit set
        if (!I2C_sendData(THERMAL_I2C_ID,EEPROM_READ_ADDRESS))
            break;

        // Read the I2C bus most significant byte and send an acknowledge bit
        for(index = 0; index <=0xFF; index++){
            eepromData[index] = I2C_getData(THERMAL_I2C_ID);
            if(index < 0xFF)
                I2CAcknowledgeByte(I2C1, TRUE);
            else
                I2CAcknowledgeByte(I2C1, FALSE);
            while(!I2CAcknowledgeHasCompleted(I2C1));
        }
        success = TRUE;
    } while(0);
    // Send the stop bit to finish the transfer
    I2C_stopTransfer(THERMAL_I2C_ID);
    if(!success){
        printf("Data transfer unsuccessful.\n");
        return;
    }
    // Stop transfer twice?
    I2C_stopTransfer(THERMAL_I2C_ID);
  /*
    for(Index = 0; Index <=255; Index++){
        while(!IsTransmitEmpty());
        printf("EEPROM %x / %d: %x\n", Index, Index, eepromData[Index]);
    }
     */
}

void readConfigReg(void){
    BOOL Success = TRUE;
    UINT8 configMSB, configLSB;
    if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_COMMAND)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x92)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x01)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(THERMAL_I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_ADDRESS)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        configLSB = I2C_getData(THERMAL_I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);
        while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
        configMSB = I2C_getData(THERMAL_I2C_ID);
        }
    I2C_stopTransfer(THERMAL_I2C_ID);
        //while(!IsTransmitEmpty());
        //configMSB = configMSB & 0x04;
        //printf("Config Data %x %x\n", configMSB, configLSB);
}

void writeTrimmingValue(void){
    BOOL Success = TRUE;
    UINT8 MSByte, LSByte, MSByteCheck, LSByteCheck;
    LSByte = eepromData[247];
    LSByteCheck = LSByte - 0xAA;
    MSByte = 0x00;
    MSByteCheck = 0x56;
    if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_TRIM_COMMAND)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,LSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,LSByte)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,MSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,MSByte)){
        Success = FALSE;
    }
    I2C_stopTransfer(THERMAL_I2C_ID);
}

void writeConfigReg(void){
    BOOL Success = TRUE;
    UINT8 MSByte, LSByte, MSByteCheck, LSByteCheck;
    LSByte = eepromData[245];
    LSByteCheck = LSByte - 0x55;
    MSByte = eepromData[246];
    MSByteCheck = MSByte - 0x55;
    if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_CONFIG_COMMAND)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,LSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,LSByte)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,MSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,MSByte)){
        Success = FALSE;
    }
    I2C_stopTransfer(THERMAL_I2C_ID);
}

void configCalculationData(void){
    V_th = (eepromData[219] << 8) + eepromData[218];
    K_t1 = ((eepromData[221] << 8) + eepromData[220])/1024.0;
    K_t2 = ((eepromData[223] << 8) + eepromData[222])/1048576.0;

    //printf("V_th: %.2f\n",V_th);
    //printf("K_t1: %.2f\n",K_t1);
    //printf("K_t2: %.2f\n",K_t2);

    A_cp = eepromData[212];
    if(A_cp > 127){
        A_cp = A_cp -256;
    }
    //printf("A_cp: %d\n",A_cp);

    B_cp = eepromData[213];
    if(B_cp > 127){
        B_cp = B_cp -256;
    }
    //printf("B_cp: %d\n",B_cp);

    Tgc = eepromData[216];
    if(Tgc > 127){
        Tgc = Tgc -256;
    }
    //printf("Tgc: %d\n",Tgc);

    B_iscale = eepromData[217];
    //printf("B_iscale: %d\n",B_iscale);

    emissivity = (((unsigned int)eepromData[229] << 8) + eepromData[228])/32768.0;
    int i;
    for(i = 0; i <= 63; i++){
        A_ij[i] = eepromData[i];
        if(A_ij[i] > 127){
            A_ij[i] = A_ij[i] - 256;
        }
        B_ij[i] = eepromData[64 + i];
        if(B_ij[i] > 127){
            B_ij[i] = B_ij[i] - 256;
        }
        //while(!IsTransmitEmpty());
        //printf("A_ij[%d]: %d\n",i,A_ij[i]);
        //while(!IsTransmitEmpty());
        //printf("B_ij[%d]: %d\n",i,B_ij[i]);
    }
    //while(!IsTransmitEmpty());
}


void readChipTemp(void){
    BOOL Success = TRUE;
    UINT8 tempMSB, tempLSB;
    if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_COMMAND)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x90)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x01)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(THERMAL_I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_ADDRESS)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        tempLSB = I2C_getData(THERMAL_I2C_ID);
        I2CAcknowledgeByte(THERMAL_I2C_ID, TRUE);
        while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
        tempMSB = I2C_getData(THERMAL_I2C_ID);
        I2CAcknowledgeByte(THERMAL_I2C_ID, TRUE);
        while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
        }
    I2C_stopTransfer(THERMAL_I2C_ID);
        //while(!IsTransmitEmpty());
        rawTemp = (tempMSB << 8) + tempLSB;
}

void calculateChipTemp(void){
    chipTempC = ((-K_t1+sqrt(pow(K_t1,2)-(4*K_t2*(V_th-(float)rawTemp))))/(2*K_t2))+25;
    chipTempF = (chipTempC*9/5)+32;
    //printf("Chip TempC: %.2f\nChip TempF: %.2f\n",chipTempC, chipTempF);
}

void readCPixelValue(void){
    BOOL Success = TRUE;
    UINT8 CPixelMSB, CPixelLSB;
    if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_COMMAND)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x91)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x01)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(THERMAL_I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_ADDRESS)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        CPixelLSB = I2C_getData(THERMAL_I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);
        while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
        CPixelMSB = I2C_getData(THERMAL_I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);
        while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
        CPixel = (CPixelMSB << 8) + CPixelLSB;
    }
    I2C_stopTransfer(THERMAL_I2C_ID);
    if(CPixel > 32767){
        CPixel = CPixel - 65536;
    }
    //printf("CPixel: %d\n", CPixel);
}

void readPixelValue(void){
    int Index = 0;
    BOOL Success = TRUE;
    UINT8 pixelMSB, pixelLSB;
    if(!I2C_startTransfer(THERMAL_I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_WRITE_ADDRESS)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_COMMAND)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x01)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(THERMAL_I2C_ID,0x40)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(THERMAL_I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(THERMAL_I2C_ID,CAMERA_READ_ADDRESS)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        for(Index = 0; Index <=63; Index++){
            pixelLSB = I2C_getData(THERMAL_I2C_ID);
            I2CAcknowledgeByte(I2C1, TRUE);
            while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
            pixelMSB = I2C_getData(THERMAL_I2C_ID);
            I2CAcknowledgeByte(I2C1, TRUE);
            while(!I2CAcknowledgeHasCompleted(THERMAL_I2C_ID));
            pixelData[Index] = (pixelMSB << 8) + pixelLSB;
            if(pixelData[Index] > 32767)
                pixelData[Index] = pixelData[Index] - 65536;
            //while(!IsTransmitEmpty());
           //printf("V_ir[%d]: %d\n",Index,pixelData[Index]);
        }
    }
    I2C_stopTransfer(THERMAL_I2C_ID);
}

void calculateIRTemp(void){
    float V_cp_off_comp = (float)CPixel - (A_cp + (B_cp/pow(2,B_iscale))*(chipTempC - 25));
    //printf("V_cp_off_comp: %.2f\n",V_cp_off_comp);
    int i;
    for(i = 0; i < TOTAL_PIXELS; i++) {
        V_ir_tgc_comp[i] = pixelData[i] - (A_ij[i] + (float)(B_ij[i]/pow(2,B_iscale))*
                           (chipTempC - 25)) - (((float)Tgc/32)*V_cp_off_comp);
        //while(!IsTransmitEmpty());
        //printf("V_ir_tgc_comp[%d]: %.2f\n",i,V_ir_tgc_comp[i]);
        finalPixelTempF[i] = sqrt(sqrt((V_ir_tgc_comp[i]/alpha[i]) +
                            pow((chipTempC + 273.15),4))) - 273.15;
    }
}


//#define THERMAL_TEST
#ifdef THERMAL_TEST
int main(void){
// Initialize the UART,Timers, and I2C1
    int count = 0;
    Board_init();
    Timer_init();
    Serial_init();
    //I2C_init(THERMAL_I2C_ID,I2C_CLOCK_FREQ);
   // InitTimer(1,100);
    //while(!IsTimerExpired(1));
    Thermal_init();
    while(1){
        if(count == 0){
            readChipTemp();
            calculateChipTemp();
        }
        count++;
        if(count >= 16){
            count = 0;
        }
        readPixelValue();
        //readCPixelValue();
        //calculateIRTemp();
        //printTemp();
    }
}
#endif



#define THERMAL_TEST2
#ifdef THERMAL_TEST2

//#define DEBUG_TEST2         1

#define END_ROW_SEQUENCE    (0xFFFFAAAA)
#define START_SEQUENCE      (0xFFFF1234)

#define PRINT_DELAY         200

void sendSerial32(uint32_t data);
void sendSerialFloat(float data) ;

int main(void) {
// Initialize the UART,Timers, and I2C1
    //int row = 0;
    Board_init();
    Timer_init();
    Thermal_init();
    Serial_init();

    /*Timer_new(TIMER_TEST,100);
    while(!Timer_isExpired(TIMER_TEST);*/
    Thermal_init();
#ifdef DEBUG_TEST2
    printf("Initialized Thermal module.\n");
#endif

    uint8_t row = 0, col = 0;
    Timer_new(TIMER_TEST,PRINT_DELAY);

    while(1) {
        if (Timer_isExpired(TIMER_TEST)) {
            #ifdef DEBUG_TEST2
                printf("Sending thermal data...\n");
            #endif
            sendSerial32(START_SEQUENCE);

            for(row = 0; row < TOTAL_PIXEL_ROWS; row++) {
                for(col = 0; col < TOTAL_PIXEL_COLS; col++){
                    uint8_t pixel = col + (row * TOTAL_PIXEL_COLS);
                    sendSerialFloat((float)pixelData[pixel]);
                }
                sendSerial32(END_ROW_SEQUENCE);
            }

            #ifdef DEBUG_TEST2
                printf("\nFinished sending thermal data.\n");
            #endif
            Timer_new(TIMER_TEST,PRINT_DELAY);
        } // Timer_isExpired
        Thermal_runSM();
    } // while
}

void sendSerial32(uint32_t data) {
    #ifndef DEBUG_TEST2
    int i;
    for (i = 0; i < 4; i++) {
        //while (!Serial_isTransmitEmpty()) { asm("nop"); }
            Serial_putChar((uint8_t)(data >>  (8 * i)));
    }
    #else
    printf("0x%X",data);
    #endif
}

void sendSerialFloat(float data) {
    #ifndef DEBUG_TEST2
    int i;
    for (i = 0; i < 4; i++) {
        //while (!Serial_isTransmitEmpty()) { asm("nop"); }
            Serial_putChar(((uint8_t)data >>  (8 * i)));
    }
    #else
    printf("_%.8f/",data);
    #endif
}

#endif