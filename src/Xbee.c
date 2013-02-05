/**********************************************************************
 Module
   Xbee.c

 Author: John Ash

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When               Who         What/Why
 --------------     ---         --------
 12-29-12 2:10 PM   jash    Created file.
 1-17-13 4:10 PM    jash    Work on functions, add receieve pseudo code
 2-1-13  2:50 AM    jash    Complete functions and add comments.
***********************************************************************/

#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include "Board.h"
#include "Uart.h"
#include "Xbee.h"

#include <ports.h>


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define UART_ID             UART2_ID
#define NUMBER_DATA_BYTES   1
#define OVERHEAD_BYTES      8
#define API_DELAY           1000

/*    FOR IFDEFS     */
#define REPROGRAM_API

/*Variables used within Xbee*/
uint8_t xbeeInitialized = FALSE;
uint8_t sendArray[OVERHEAD_BYTES+NUMBER_DATA_BYTES+1];
uint8_t recieveArray[OVERHEAD_BYTES+NUMBER_DATA_BYTES+1];
uint8_t packetWasRecieved; //1 a packet has been recieved, 0 no packet


/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/
void Xbee_initSend();

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

uint8_t Xbee_init(){
    #ifdef REPROGRAM_API
    if( Xbee_programApi() == FAILURE){
        if( Xbee_programApi() == FAILURE){
            return FAILURE;
        }
    }
    #endif
    Xbee_initSend();
    

    xbeeInitialized = TRUE;
    return SUCCESS; 
}


uint8_t Xbee_programApi(){
    int i = 0;
    char confirm[3];
    //empty recieve buffer before hand.  --NEED TO DO

    //This opens up command mode
    UART_putString(UART_ID, "+++", 3);
    DELAY(API_DELAY);
    //confirm we are in Xbee by recieving "OK\r"
    do {
        confirm[i] = UART_getChar(UART_ID);
        if (confirm[i] != 0)
            i++;
    } while(i < 3);
    //check the bytes to make sure it is "OK\r"
    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
    
    // Puts it in API mode
    UART_putString(UART_ID, "ATAP1\r", 6);
    DELAY(API_DELAY);
    //UART_putString(UART_ID, "ATID7806\r", 9); //Pan ID, needs more research
    //High & Low Xbee Destination addresses.
    
    /*
    UART_getChar(UART_ID, "ATDH");
    UART_getChar(UART_ID, whichXbee);
    UART_getChar(UART_ID, "\r");
    UART_getChar(UART_ID, "ATDL");
    UART_getChar(UART_ID, whichXbee);
    UART_getChar(UART_ID, "\r");
    //Sets the Baud Rate
    //1 = 2400 2 = 4800 3 = 9600 4 = 19200 5 = 38400 6 = 57600 7 = 11520
    UART_putString(UART_ID, "ATBD3\r", 6);
    */

    //Writes the changes to memory
    UART_putString(UART_ID, "ATWR\r", 5);
    DELAY(API_DELAY);
    //Leave the menu.
    UART_putString(UART_ID, "ATCN\r", 5);
    DELAY(API_DELAY);
    return SUCCESS;
}

uint8_t Xbee_sendData(char* data, int Length){
    uint8_t checkSum = 0xFF, x;
    /* check if sending this amount of data will
       cause an array out of bounds error */
    if(Length  > NUMBER_DATA_BYTES){
        return FAILURE;
    }
    //fill array with data
    for(x = OVERHEAD_BYTES; x <= Length+8; x++){
        sendArray[8] = data[x-8];
    }
    //calculate the CheckSum
    for(x=3; x <= OVERHEAD_BYTES; x++){
        checkSum -= sendArray[x];
    }
    sendArray[NUMBER_DATA_BYTES+OVERHEAD_BYTES] = checkSum;
    
    //send data
    for(x = 0; x <= OVERHEAD_BYTES + NUMBER_DATA_BYTES; x++){
        UART_putChar(UART_ID, sendArray[x]);
    }
}

void Xbee_runSM(){
    static int x = 0;
    uint16_t data;
    //get data from the recieve buffer
    data = UART_getChar(UART_ID);
    //check to make sure this is data
    if(data>>8 != 0)
        return;
    //load data into array recieve array
    recieveArray[x] = data;
    x++;
    //if we haven't found the start of a new packet, ABORT
    if(recieveArray[0] != 0x7E)
        x = 0;
   //if rx array is full set Flag
    if(x >= OVERHEAD_BYTES + NUMBER_DATA_BYTES){
       packetWasRecieved = 1;
       x=0;
    }
}

uint8_t Xbee_hasNewPacket(void){
    return packetWasRecieved;
}

uint8_t Xbee_isInitialized(void) {
    return xbeeInitialized;
}


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

/**********************************************************************
 * Function: Xbee_initSend()
 * @remark Initializes an array with data required for sending a
 *  message in API mode.
 * @return none
 **********************************************************************/
void Xbee_initSend(){
    sendArray[0] = 0x7E; //Start character
    sendArray[1] = 0x00; //MSB of the length
    sendArray[2] = 0x06; //LSB of the length
    sendArray[3] = 0x01; //API id
    sendArray[4] = 0x00; //Frame ID, 0x00 turns off ackowlegments
    sendArray[5] = 0x00; //Destination Address, MSB
    sendArray[6] = 0x00; //Destination Address, LSB
    sendArray[7] = 0x01; //Acknowlegment, page 104, maybe it's 0x04
    sendArray[8] = 0x00; //Start of Data
    //other Data Init should go here (For loop?)
    sendArray[(OVERHEAD_BYTES+NUMBER_DATA_BYTES)] = 0x00; //CHECK SUM

}





/*************************************************************
 * This test function will program two Xbees, Master & Slave.
 * The Master will send data packets 1-255. However before
 * sending the next packet in sequence it has to recieve a
 * return packet from Slave.
 *
 * Slave will look for packets, and then return the packet data
 * field in  a packet to the Master
 */
#define XBEE_TEST
#ifdef XBEE_TEST

//Comment this out to program the Slave mode
//#define MASTER

#ifndef MASTER
#define SLAVE
#endif

int main(){
    Board_init();
    printf("Welcome to Xbee Test\n");
    UART_init(UART2_ID,9600);
    printf("UART INIT\n");
    if(Xbee_init() == FAILURE){
        printf("Xbee Failed to initilize\n");
        DELAY(1000);
        return FAILURE;
    }
    printf("XBEE Initialized\n");
    uint8_t y= 0;
    uint8_t x = 1;

    #ifdef MASTER

        Xbee_sendData(&x, 1);
        Xbee_sendData(&x, 1);

   while(1){
        Xbee_sendData(&x, 1);
        printf("Sent Packet: %d", x);
        while(packetWasRecieved != 1){
            Xbee_getData();
        }
        if(packetWasRecieved == 1){
            packetWasRecieved = 0;
            y = recieveArray[8];
        }
        printf(" Receieved Packet: %d\n", y);
        x++;
        x %= 256;
        if(x == 0)
            x++;
    
    }
    #endif
    #ifdef SLAVE
    while(1){
        while(packetWasRecieved != 1){
            Xbee_runSM();
        }
        if(packetWasRecieved == 1){
            packetWasRecieved = 0;
            y = recieveArray[8];
        }
        printf("Recieved Packet: %d", y);
        Xbee_sendData(&y, 1);
        printf(" Sent Packet: %d\n", y);
   
    }
    #endif
}
#endif