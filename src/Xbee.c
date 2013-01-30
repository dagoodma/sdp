/**********************************************************************
 Module
   Xbee.c

 Revision
   1.0.0

 Description
	Code to initialzie and control the Xbee modules for both 
	ground station and boat.
   
 Notes

 History
 When           Who         What/Why
 -------------- ---         --------
 12-29-12 2:10 jash    Created file.
 12-17-13 4:10 jash    Work on functions, add receieve pseudo code
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

#define NUMBER_UART     1
#define UART_ID         UART2_ID
/*    FOR IFDEFS     */
#define REPROGRAM_API
 
uint8_t xbeeInitialized = FALSE;
uint8_t xbeeApiMode = FALSE;

uint8_t sendArray[10];
uint8_t recieveArray[10];

uint8_t FLAG_PACKET_RECIEVED; //1 a packet has been recieved, 0 no packet


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: Xbee_init()
 * @param An options bitfield.
 * @return Failure or Success
 * @remark Initializes the Xbee module. This function will also set
 *  the correct API mode for the Xbees to be paired with each other.
 *  Options:
 *      1: Ground Station Xbee
 *      2: Boat Xbee
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

/**********************************************************************
 * Function: Xbee_isInitialized()
 * @return Whether the Xbee was initialized.
 * @remark none
 **********************************************************************/
uint8_t Xbee_isInitialized(void) {
    return xbeeInitialized;
}


/**********************************************************************
 * Function: Xbee_programApi()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Puts the Xbee into API mode.
 **********************************************************************/
uint8_t Xbee_programApi(){
    int i = 0;
    char confirm[3];
    //empty recieve buffer before hand.
    
    
    UART_putString(UART_ID, "+++", 3);
    DELAY(1000);
    do {
        confirm[i] = UART_getChar(UART_ID);
        if (confirm[i] != 0)
            i++;
    } while(i < 3);
    //check the bytes to make sure it is "OK\r"
    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
    
    //may need delays between each command
    
    UART_putString(UART_ID, "ATAP1\r", 6); // Puts it in API mode
    DELAY(1000);
    //UART_putString(UART_ID, "ATID7806\r", 9); //Pan ID, needs more research
    //High & Low Xbee Destination addresses. 
    /*
    UART_getChar(UART_ID, "ATDH");
    UART_getChar(UART_ID, whichXbee);
    UART_getChar(UART_ID, "\r");
    UART_getChar(UART_ID, "ATDL");
    UART_getChar(UART_ID, whichXbee);
    UART_getChar(UART_ID, "\r");
    */
    //1 = 2400 2 = 4800 3 = 9600 4 = 19200 5 = 38400 6 = 57600 7 = 11520
    //UART_putString(UART_ID, "ATBD3\r", 6);//Sets the Baud Rate

    UART_putString(UART_ID, "ATWR\r", 5);//Writes the command to memory
    DELAY(1000);
    UART_putString(UART_ID, "ATCN\r", 5);//Leave the menu.
    DELAY(1000);
    xbeeApiMode = TRUE; 
    return SUCCESS;
}



/**********************************************************************
 * Function: Xbee_isApi()
 * @return Whether the Xbee was succesfully put into API mode
 * @remark none
 **********************************************************************/
uint8_t Xbee_isApi(){
    return xbeeApiMode;
}

/**********************************************************************
 * Function: Xbee_initSend()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Initializes an array with data required for sending a
 *  message in API mode.
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
    sendArray[9] = 0x00; //CHECK SUM
    
}


/**********************************************************************
 * Function: Xbee_sendString()
 * @param data that needs to be transmitted, split up into proper bytes
 * @remark Adds data to the sendArray, than sends that data over the 
 *  UART.
 **********************************************************************/
void Xbee_sendData(char* data, int Length){
//load all the GPS data into the array
//compute the checksum
//send the array through the UART
//possibly load it into the circular buffer
    uint8_t checkSum = 0xFF, x;
    DELAY(1000);
    //fill array with data
    sendArray[8] = data[0];

    //calculate the CheckSum
    for(x=3; x <= 8; x++){
        checkSum -= sendArray[x];
    }
    sendArray[9] = checkSum;
    
    //send data
    //printf("\nSend Data Function: ");
    for(x = 0; x <= 9; x++){
        UART_putChar(UART_ID, sendArray[x]);
        //printf("%x\t",sendArray[x]);
    }
    //printf("\n");
}


/**********************************************************************
 * Function: Xbee_recieveData()
 * @remark Beings taking in data, once a packet has been fully read in,
 * we will analyze that packet for it's useful information, namely
 * the GPS cordinate we should be driving towards.
 **********************************************************************/
void Xbee_getData(){
    //load all the GPS data into the array
    //compute the checksum
    //send the array through the UART
    //possibly load it intot the circular buffer
    static int x = 0;
    uint16_t data;
    data = UART_getChar(UART_ID);
    //check to make sure this is data
    if(data>>8 != 0)
        return;
    recieveArray[x] = data;
    printf("%X\t", recieveArray[x]);
    x++;
    if(recieveArray[0] != 0x7E)
        x = 0;
   //if rx array is full
    if(x >= 9){
       FLAG_PACKET_RECIEVED = 1;
       x=0;
    }
}









#define XBEE_TEST
#ifdef XBEE_TEST

#define MASTER
//#define SLAVE

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
//        Xbee_sendData(&x, 1);
//        Xbee_sendData(&x, 1);

   while(1){
        Xbee_sendData(&x, 1);
        printf("Sent Packet: %d", x);
        //DELAY(1000);
        while(FLAG_PACKET_RECIEVED != 1){
            Xbee_getData();
            //DELAY(1000);
        }
        //DELAY(1000);
        if(FLAG_PACKET_RECIEVED == 1){
            FLAG_PACKET_RECIEVED = 0;
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
        while(FLAG_PACKET_RECIEVED != 1){
            Xbee_getData();
            //DELAY(1000);
        }
        if(FLAG_PACKET_RECIEVED == 1){
            FLAG_PACKET_RECIEVED = 0;
            y = recieveArray[8];
            /*
            for(x = 0; x <= 9; x++){
                printf("%X ", recieveArray[x]);
            }
            printf("\n");
             */
        }
        printf("Recieved Packet: %d", y);
        Xbee_sendData(&y, 1);
        printf(" Sent Packet: %d\n", y);
   
    }
    #endif
    printf("WTF");
    DELAY(1000);
}
#endif

//#define XBEE_TEST_2
#ifdef XBEE_TEST_2

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
    int x = 1, y= 0;
    DELAY(5000);
    #ifdef MASTER
    while(1){
        Xbee_sendData((char*)x, 1);
    }
    printf("GOT HERE");
    #endif
    #ifdef SLAVE
    while(1){
        while(FLAG_PACKET_RECIEVED != 1){
            Xbee_getData();
            DELAY(1000);
        }
        printf("DATA: %c\n", recieveArray[8]);
    }
    #endif

}
#endif



