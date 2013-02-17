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
#include <stdint.h>
#include <ports.h>
#include "Board.h"
#include "Sonar.h"
#include "Timer.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define TIMER_NUMBER 4
#define TIMER_TIME 100


/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/
static uint32_t getAnalog();

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

void Sonar_init(){
    AD_init(SONAR_AD_PIN);
    Timer_init();
    Timer_new(TIMER_NUMBER, TIMER_TIME);
}

uint32_t Xbee_runSM(){
    if(Timer_isExpired()){
        uint32_t analogData = getAnalog();
        Timer_new(TIMER_NUMBER, TIMER_TIME);
        return analogData;
    }else{
        return 0x000000;
    }
}

/*
 void Sonar_getPW(uint16_t* data){
    
}
*/



/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


static uint32_t getAnalog(){
    return AD_readPin(AD_PIN);
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
//#define SONAR_TEST
#ifdef SONAR_TEST

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