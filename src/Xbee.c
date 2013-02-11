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
 12-29-12 2:10 PM    jash    Created file.
 1-17-13  4:10 PM    jash    Work on functions, add receieve pseudo code
 2-1-13   2:50 AM    jash    Complete functions and add comments.
 2-9-13   5:08 PM    jash    Added Mavlink functionality
***********************************************************************/

#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include <ports.h>
#include "Board.h"
#include "Uart.h"
#include "Mavlink.h"
#include "Timer.h"
#include "Xbee.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define UART_ID             UART2_ID
#define NUMBER_DATA_BYTES   1
#define OVERHEAD_BYTES      8
#define API_DELAY           1000

/*    FOR IFDEFS     */
//#define REPROGRAM_API

#define TIMER_TIMEOUT 2
#define DELAY_TIMEOUT 1000 // (ms)
/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static uint8_t Xbee_programMode();

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Xbee_message_data_test(mavlink_test_data_t* packet){
    Mavlink_send_Test_data(UART_ID, (packet->data+1)%255);
    Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
}


uint8_t Xbee_init(){
    UART_init(UART_ID,9600);
#ifdef REPROGRAM_API
    if( Xbee_programMode() == FAILURE){
        return FAILURE
    }
#endif
    return SUCCESS; 
}


void Xbee_runSM(){
    if(UART_isReceiveEmpty(UART_ID) == FALSE){
        Mavlink_recieve(UART_ID);
    }
}




/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

/**********************************************************************
 * Function: Xbee_programMode()
 * @return Success or Failure based on weather the mode could be set.
 * @remark Currently restores the XBEE to factory setting
 * @author John Ash
 * @date February 10th 2013
 **********************************************************************/
static uint8_t Xbee_programMode(){
    int i = 0;
    char confirm[3];
    UART_putString(UART_ID, "+++", 3);

    //wait for "OK\r"
    do {
        confirm[i] = UART_getChar(UART_ID);
        if (confirm[i] != 0)
            i++;
    } while(i < 3);

    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
    UART_putString(UART_ID, "ATRE\r", 5); // Puts it in API mode
    UART_putString(UART_ID, "ATWR\r", 5);//Writes the command to memory
    UART_putString(UART_ID, "ATCN\r", 5);//Leave the menu.
    return SUCCESS;
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
#define MASTER

int main(){
    Board_init();
    printf("Welcome to Xbee Test1\n");
    printf("UART INIT\n");
    Timer_init();

    printf("Timers Init\n");
    if(Xbee_init() == FAILURE){
        printf("Xbee Failed to initilize\n");
        return FAILURE;
    }
    printf("XBEE Initialized\n");

// Master sends packets and listens for responses
    #ifdef MASTER
    Mavlink_send_Test_data(UART2_ID, 1);
    Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
    while(1){
        Xbee_runSM();
        if(Timer_isActive(TIMER_TIMEOUT) != TRUE){
            Mavlink_send_Test_data(UART2_ID, 1);
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
            printf("lost_packet: %d\n", get_time());
        }
    }
    #else
    while(1){
        Xbee_runSM();
    }
    #endif
}
#endif
