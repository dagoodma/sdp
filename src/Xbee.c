/**********************************************************************
 Module
   Xbee.c

 Author: John Ash

 Description
	Code to initialzie and control the Xbee modules in API mode
   
 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
 12-29-12 2:10  PM      jash        Created file.
 1-17-13  4:10  PM      jash        Work on functions, add receieve pseudo code
 2-1-13   2:50  AM      jash        Complete functions and add comments.
 2-9-13   5:08  PM      jash        Added Mavlink functionality
 2-9-15   12:40 PM      jash        MAVLink test up and running, and set channel
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

#define API_DELAY           1000
// note, need to reprogram Xbee for different Baud Rates.
//  factory settings are 9600 baud rate
#define XBEE_BAUD_RATE      9600

/*    FOR IFDEFS     */
//#define XBEE_RESET_FACTORY

//Leave uncommented when programming XBEE_1, comment out when programming XBEE_2
//#define XBEE_1

#define TIMER_HEARTBEAT 2
#ifdef XBEE_1
#define DELAY_HEARTBEAT 4000 //time to wait for recieve
#else
#define DELAY_HEARTBEAT 1000 //Time to wait before send heartbeat
#endif


#define ACK_WAIT_TIME 3000
/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static uint8_t Xbee_programMode();

void check_ACK(ACK *message);

/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

uint8_t Xbee_init(){
    UART_init(XBEE_UART_ID,XBEE_BAUD_RATE);
#ifdef XBEE_RESET_FACTORY
    if( Xbee_programMode() == FAILURE){
        return FAILURE;
    }
#endif
    Timer_new(TIMER_HEARTBEAT, DELAY_HEARTBEAT);
    return SUCCESS; 
}


void Xbee_runSM(){
    //Recieve bytes if they are available
    if(UART_isReceiveEmpty(XBEE_UART_ID) == FALSE){
        Mavlink_recieve(XBEE_UART_ID);
    }
    //send and recieve heart beat message
#ifndef XBEE_1 //if Xbee 2
    //Sends out a HEARTBEAT every 1000 ms.
    if(Timer_isActive(TIMER_HEARTBEAT) != TRUE){
        Mavlink_send_xbee_heartbeat(XBEE_UART_ID, 1);
        Timer_new(TIMER_HEARTBEAT, DELAY_HEARTBEAT);
        //printf("Heart beat sent");
    }

#else
    //we have not heard a heartbeat message for 4 s, LOST CONNECTION
    if(Timer_isActive(TIMER_HEARTBEAT) != TRUE){
        Timer_new(TIMER_HEARTBEAT, DELAY_HEARTBEAT);
        printf("XBEE LOST CONNECTION\n");
    }
#endif
    
    //check ACKS
    check_ACK(&start_rescue);


}



void Xbee_recieved_message_heartbeat(mavlink_xbee_heartbeat_t* packet){
#ifdef XBEE_1 
    Timer_new(TIMER_HEARTBEAT, DELAY_HEARTBEAT); //still have a connection
#endif
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
    DELAY(2000);
    UART_putString(XBEE_UART_ID, "+++", 3);
    DELAY(1000);
    //wait for "OK\r"
    do {
        confirm[i] = UART_getChar(XBEE_UART_ID);
        if (confirm[i] != 0)
            i++;
    } while(i < 3);

    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATRE\r", 5);// Resets to Factory settings
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATCH15\r", 7);
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATDH0\r", 6);
    DELAY(1000);
   #ifdef XBEE_1
    UART_putString(XBEE_UART_ID, "ATDLAAC3\r", 9);
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATMYBC64\r", 9);
    #else
    UART_putString(XBEE_UART_ID, "ATDLBC64\r", 9);
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATMYAAC3\r", 9);
    #endif
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATWR\r", 5);//Writes the command to memory
    DELAY(1000);
    UART_putString(XBEE_UART_ID, "ATCN\r", 5);//Leave the menu.
    return SUCCESS;
}


void check_ACK(ACK *message){
    if(message->ACK_status == ACK_STATUS_WAIT){
        if(message->ACK_time == 0) {
            message->ACK_time = get_time();

        }
        else if((get_time() - message->ACK_time) >= ACK_WAIT_TIME){
            Mavlink_resend_message(message);
            message->ACK_time = 0;
        }

    }else if(message->ACK_status == ACK_STATUS_RECIEVED){
            message->ACK_status = ACK_STATUS_NO_ACK;
            message->ACK_time = 0;
    }
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
#ifdef XBEE_TEST

#include "Serial.h"

#define TIMER_TIMEOUT 4
#define DELAY_TIMEOUT 1000
#define TIMER_STATUS 3
#define DELAY_STATUS 4000

uint32_t count_recieved = 0, count_lost = 0;

int main(){
    Board_init();
    Serial_init();
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
    #ifdef XBEE_1
    Mavlink_send_Test_data(XBEE_UART_ID, 1);
    Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
    Timer_new(TIMER_STATUS, DELAY_STATUS);
    while(1){
        Xbee_runSM();
        //lost a packet, report it, and restart
        if(Timer_isActive(TIMER_TIMEOUT) != TRUE){
            Mavlink_send_Test_data(XBEE_UART_ID, 1);
            count_lost++;
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
            printf("lost_packet: %d\n", get_time());
        }
        //Printout the status
        if(Timer_isActive(TIMER_STATUS) != TRUE){
            Timer_new(TIMER_STATUS, DELAY_STATUS);
            printf("Status: %d,%d [Recieved,Lost] TIME: %d\n", count_recieved, count_lost, get_time());
        }

    }
    #else
    while(1){
        Xbee_runSM();
    }
    #endif
}


void Xbee_message_data_test(mavlink_test_data_t* packet){
    Mavlink_send_Test_data(XBEE_UART_ID, (packet->data+1)%255);
    count_recieved++;
    Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
}

#endif


//#define XBEE_TEST_2
#ifdef XBEE_TEST_2

#define TIMER_TIMEOUT 4
#define DELAY_TIMEOUT 4000

int main(){
    Board_init();
    Serial_init();
    Timer_init();
    Xbee_init();
    printf("Xbee Test 2\n");
    while(1){
        Xbee_runSM();
        //if(!UART_isTransmitEmpty(UART1_ID));
        //printf("%d\t",Mavlink_returnACKStatus(messageName_start_rescue));
        if(!UART_isReceiveEmpty(UART1_ID)){
            Serial_getChar();
            Mavlink_send_start_rescue(UART2_ID, TRUE, 0xFF, 0x34FD, 0xAB54);
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
            printf("\nSENT\n");
        }
        else if(Timer_isActive(TIMER_TIMEOUT) != TRUE && Mavlink_returnACKStatus(messageName_start_rescue) == ACK_STATUS_WAIT){
        //else if(Timer_isActive(TIMER_TIMEOUT) != TRUE){
            Mavlink_editACKStatus(messageName_start_rescue, ACK_STATUS_DEAD);
            printf("ACK DEAD\n");
        }
        else if(Mavlink_returnACKStatus(messageName_start_rescue) == ACK_STATUS_DEAD){
            Mavlink_send_start_rescue(UART2_ID, TRUE, 0xFF, 0x34FD, 0xAB54);
            Timer_new(TIMER_TIMEOUT, DELAY_TIMEOUT);
        }
        else if(Mavlink_returnACKStatus(messageName_start_rescue) == ACK_STATUS_RECIEVED){
            Mavlink_editACKStatus(messageName_start_rescue, ACK_STATUS_NO_ACK);
            printf("GPS SENT AND ACKOWLEGED\n");
        } 
    }
}

#endif

//#define XBEE_TEST_3
#ifdef XBEE_TEST_3
int main(){
    Board_init();
    Serial_init();
    Timer_init();
    Xbee_init();
    printf("Xbee Test 2\n");
    while(1){
        Xbee_runSM();
        //if(!UART_isTransmitEmpty(UART1_ID));
        //printf("%d\t",Mavlink_returnACKStatus(messageName_start_rescue));
        if(!UART_isReceiveEmpty(UART1_ID)){
            Serial_getChar();
            Mavlink_send_start_rescue(UART2_ID, TRUE, 0xFF, 0x34FD, 0xAB54);
            printf("\nSENT\n");
        }
    }

}
#endif