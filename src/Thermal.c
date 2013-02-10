/*
 * File:   Thermal.c
 * Author: Shehadeh H. Dajani
 *
 * Created on January 21, 2013, 10:52 PM
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
    UINT8           eepromData[256];
    UINT16          rawTemp;
    float           V_th, K_t1, K_t2, chipTempC, chipTempF, emissivity;
    float           V_ir_tgc_comp[64], finalPixelTempF[64];
    int             pixelData[64], A_cp, B_cp, Tgc, B_iscale, CPixel;
    int             A_ij[64], B_ij[64];
    int             END_ROW_SEQUENCE = 0xFFFF;
    int             START_SEQUENCE = 0x1234;

    float alpha[64] = {1.5935E-09, 3.22331E-09, 2.99048E-09, 1.38305E-10, 3.9218E-09, 4.91133E-09, 4.44567E-09, 2.05916E-09,
                       4.91133E-09, 6.54115E-09, 6.07549E-09, 3.63077E-09, 5.84266E-09, 7.29785E-09, 6.30832E-09, 5.14416E-09,
                       6.89039E-09, 8.69483E-09, 8.69483E-09, 6.30832E-09, 7.76351E-09, 9.91719E-09, 9.80078E-09, 7.76351E-09,
                       8.92766E-09,  1.06157E-08, 8.40379E-09, 7.76351E-09, 7.87992E-09, 1.13142E-08, 1.05575E-08, 8.69483E-09,
                       8.40379E-09, 1.13142E-08, 1.08485E-08, 7.93813E-09, 7.99634E-09, 1.13142E-08, 1.1547E-08, 8.34559E-09,
                       8.40379E-09, 1.13142E-08, 1.10231E-08, 8.86945E-09, 5.60983E-09, 1.09649E-08, 1.10231E-08, 9.91719E-09,
                       6.65756E-09, 9.9754E-09, 9.50974E-09, 7.23964E-09, 6.48294E-09, 8.69483E-09, 8.63662E-09, 5.95907E-09,
                       4.50388E-09, 6.9486E-09, 6.71577E-09, 4.50388E-09, 3.22331E-09, 5.31879E-09, 5.31879E-09, 3.33973E-09};

    #define         slaveEEPROMWrite                0xA0
    #define         slaveEEPROMRead                 0xA1
    #define         slaveCameraWrite                0xC0
    #define         slaveCameraRead                 0xC1

    #define         slaveEEPROMReadCommand          0x00
    #define         slaveCameraReadRegisterCommand  0x02
    #define         slaveCameraWriteConfigCommand   0x03
    #define         slaveCameraWriteTrimCommand     0x04
    
void readEepromMemory(void){
    BOOL Success = TRUE;
    int Index;
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if (!I2C_sendData(I2C_ID,slaveEEPROMWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveEEPROMReadCommand)){
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
        if (!I2C_sendData(I2C_ID,slaveEEPROMRead)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        for(Index = 0; Index <=255; Index++){
            eepromData[Index] = I2C_getData(I2C_ID);
            if(Index < 255)
                I2CAcknowledgeByte(I2C1, TRUE);
            else
                I2CAcknowledgeByte(I2C1, FALSE);
            while(!I2CAcknowledgeHasCompleted(I2C1));
        }
    }
// Send the stop bit to finidh the transfer
    I2C_stopTransfer(I2C_ID);
    if(!Success){
        printf("Data transfer unsuccessful.\n");
        return;
    }
    I2C_stopTransfer(I2C_ID);
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
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(I2C_ID,slaveCameraWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveCameraReadRegisterCommand)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x92)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x01)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID,slaveCameraRead)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        configLSB = I2C_getData(I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);
        while(!I2CAcknowledgeHasCompleted(I2C_ID));
        configMSB = I2C_getData(I2C_ID);
        }
    I2C_stopTransfer(I2C_ID);
        while(!IsTransmitEmpty());
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
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(I2C_ID,slaveCameraWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveCameraWriteTrimCommand)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,LSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,LSByte)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,MSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,MSByte)){
        Success = FALSE;
    }
    I2C_stopTransfer(I2C_ID);
}

void writeConfigReg(void){
    BOOL Success = TRUE;
    UINT8 MSByte, LSByte, MSByteCheck, LSByteCheck;
    LSByte = eepromData[245];
    LSByteCheck = LSByte - 0x55;
    MSByte = eepromData[246];
    MSByteCheck = MSByte - 0x55;
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(I2C_ID,slaveCameraWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveCameraWriteConfigCommand)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,LSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,LSByte)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,MSByteCheck)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,MSByte)){
        Success = FALSE;
    }
    I2C_stopTransfer(I2C_ID);
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
        while(!IsTransmitEmpty());
        //printf("A_ij[%d]: %d\n",i,A_ij[i]);
        while(!IsTransmitEmpty());
        //printf("B_ij[%d]: %d\n",i,B_ij[i]);
    }
    while(!IsTransmitEmpty());
}

void Thermal_initSensor(void){
    readEepromMemory();
    writeTrimmingValue();
    writeConfigReg();
    readConfigReg();
    configCalculationData();
}

void readChipTemp(void){
    BOOL Success = TRUE;
    UINT8 tempMSB, tempLSB;
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(I2C_ID,slaveCameraWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveCameraReadRegisterCommand)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x90)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x01)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID,slaveCameraRead)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        tempLSB = I2C_getData(I2C_ID);
        I2CAcknowledgeByte(I2C_ID, TRUE);
        while(!I2CAcknowledgeHasCompleted(I2C_ID));
        tempMSB = I2C_getData(I2C_ID);
        I2CAcknowledgeByte(I2C_ID, TRUE);
        while(!I2CAcknowledgeHasCompleted(I2C_ID));
        }
    I2C_stopTransfer(I2C_ID);
        while(!IsTransmitEmpty());
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
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(I2C_ID,slaveCameraWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveCameraReadRegisterCommand)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x91)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x01)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID,slaveCameraRead)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        CPixelLSB = I2C_getData(I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);
        while(!I2CAcknowledgeHasCompleted(I2C_ID));
        CPixelMSB = I2C_getData(I2C_ID);
        I2CAcknowledgeByte(I2C1, TRUE);
        while(!I2CAcknowledgeHasCompleted(I2C_ID));
        CPixel = (CPixelMSB << 8) + CPixelLSB;
    }
    I2C_stopTransfer(I2C_ID);
    if(CPixel > 32767){
        CPixel = CPixel - 65536;
    }
    printf("CPixel: %d\n", CPixel);
}

void readPixelValue(void){
    int Index = 0;
    BOOL Success = TRUE;
    UINT8 pixelMSB, pixelLSB;
    if(!I2C_startTransfer(I2C_ID, FALSE)){
        printf("FAILED initial transfer!\n");
        Success = FALSE;
    }
// Transmit the slave's address to notify it
    if(!I2C_sendData(I2C_ID,slaveCameraWrite)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,slaveCameraReadRegisterCommand)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x00)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x01)){
        Success = FALSE;
    }
// Tranmit the read address module
    if(!I2C_sendData(I2C_ID,0x40)){
        Success = FALSE;
    }
    if(Success){
    // Send a Repeated Started condition
        if(!I2C_startTransfer(I2C_ID,TRUE)){
            printf("FAILED Repeated start!\n");
            Success = FALSE;
        }
    // Transmit the address with the READ bit set
        if (!I2C_sendData(I2C_ID,slaveCameraRead)){
            Success = FALSE;
        }
    }
    if(Success){
    // Read the I2C bus most significan byte and send an acknowledge bit
        for(Index = 0; Index <=63; Index++){
            pixelLSB = I2C_getData(I2C_ID);
            I2CAcknowledgeByte(I2C1, TRUE);
            while(!I2CAcknowledgeHasCompleted(I2C_ID));
            pixelMSB = I2C_getData(I2C_ID);
            I2CAcknowledgeByte(I2C1, TRUE);
            while(!I2CAcknowledgeHasCompleted(I2C_ID));
            pixelData[Index] = (pixelMSB << 8) + pixelLSB;
            if(pixelData[Index] > 32767)
                pixelData[Index] = pixelData[Index] - 65536;
            while(!IsTransmitEmpty());
            printf("V_ir[%d]: %d\n",Index,pixelData[Index]);
        }
    }
    I2C_stopTransfer(I2C_ID);
}

void calculateIRTemp(void){
    float V_cp_off_comp = (float)CPixel - (A_cp + (B_cp/pow(2,B_iscale))*(chipTempC - 25));
    //printf("V_cp_off_comp: %.2f\n",V_cp_off_comp);
    int i;
    for(i = 0; i < 64; i++){
        V_ir_tgc_comp[i] = pixelData[i] - (A_ij[i] + (float)(B_ij[i]/pow(2,B_iscale))*
                           (chipTempC - 25)) - (((float)Tgc/32)*V_cp_off_comp);
        while(!IsTransmitEmpty());
        //printf("V_ir_tgc_comp[%d]: %.2f\n",i,V_ir_tgc_comp[i]);
        finalPixelTempF[i] = sqrt(sqrt((V_ir_tgc_comp[i]/alpha[i]) +
                            pow((chipTempC + 273.15),4))) - 273.15;
    }
}

void sendData(int data){
    while (1) {
        if (IsTransmitEmpty() == TRUE)
            if (IsReceiveEmpty() == FALSE)
                 PutChar(GetChar());
    }  
    int index;
    for(index = 0; index < 4; index++){
        PutChar('c');
        while(!IsTransmitEmpty());
    }
}

void printTemp(void){
    int row, pixel;
    sendData(START_SEQUENCE);
    for(row = 0; row <=3; row++){
        for(pixel = row; pixel <=63;pixel = pixel + 4){
            sendData(pixelData[pixel]);
        }
        sendData(END_ROW_SEQUENCE);
    }
}

int main(void){
// Initialize the UART,Timers, and I2C1
    int count = 0;
    BOARD_Init();
    TIMERS_Init();
    I2C_Init(I2C_ID,I2C_CLOCK_FREQ);
    InitTimer(1,100);
    while(!IsTimerExpired(1));
    Thermal_initSensor();
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


