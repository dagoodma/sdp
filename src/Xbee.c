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

#include "Xbee.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define NUMBER_UART 1
#define REPROGRAM_API
 
uint8 xbeeInitialized = FALSE;
uint8 xbeeApiMode = FALSE;

uint8 sendArray[10];


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
uint8_t Xbee_init(uint8_t options){
    #ifdef REPORGRAM_XBEE
    if( Xbee_programApi(options) == FAILURE){
        if( Xbee_programApi(options) == FAILURE){
            return FAILURE
        }
    }
    #endif
    
    
    
    xbeeInitialized = TRUE;
}

/**********************************************************************
 * Function: Xbee_isInitialized()
 * @return Whether the Xbee was initialized.
 * @remark none
 **********************************************************************/
bool Xbee_isInitialized() {
    return xbeeInitialized;
}


/**********************************************************************
 * Function: Xbee_programApi()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Puts the Xbee into API mode.
 **********************************************************************/
uint8_t Xbee_programApi(uint8 whichXbee){
    int i = 0;
    char confirm[3];
    //empty recieve buffer before hand.
    
    
    Uart2_print("+++");
    //wait for "OK\r"
    do {
        confirm[i] = UART_XBee_GetChar();
        if (confirm[i] != 0)
            i++;
    } while(i < 3);
    
    if (!(confirm[0] == 0x4F && confirm[1] == 0x4B && confirm[2] == 0x0D)){
        return FAILURE;
    }
    
    //may need delays between each command
    
    UART_sendData(NUMBER_UART, "ATAP1\r", 6); // Puts it in API mode
    UART_sendData(NUMBER_UART, "ATCH14\r", 7); //sets the channel to something other than default
    //UART_sendData(NUMBER_UART, "ATID7806\r", 9); //Pan ID, needs more research
    //High & Low Xbee Destination addresses. 
    /*
    UART_sendData(NUMBER_UART, "ATDH", 4); 
    UART_sendData(NUMBER_UART, whichXbee, ?????);
    UART_sendData(NUMBER_UART, "\r", 1);
    UART_sendData(NUMBER_UART, "ATDL", 4);
    UART_sendData(NUMBER_UART, whichXbee, ??????);
    UART_sendData(NUMBER_UART, "\r", 1);
    */
    UART_sendData(NUMBER_UART, "ATBD5\r", 6);//Sets the Baud Rate
    UART_sendData(NUMBER_UART, "ATWR\r", 5);//Writes the command to memory
    UART_sendData(NUMBER_UART, "ATCN\r", 5);//Leave the menu.
    xbeeApiMode = TRUE; 
    return SUCCESS
}


/**********************************************************************
 * Function: Xbee_isApi()
 * @return Whether the Xbee was succesfully put into API mode
 * @remark none
 **********************************************************************/
bool Xbee_isApi(){
    return xbeeApiMode;
}

/**********************************************************************
 * Function: Xbee_initSend()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Initializes an array with data required for sending a
 *  message in API mode.
 **********************************************************************/
void Xbee_initSend(uint8 whichXbee){
    sendArray[0] = 0x7E; //Start character
    sendArray[1] = 0x00; //MSB of the length
    sendArray[2] = 0x07; //LSB of the length
    sendArray[3] = 0x01; //API id
    sendArray[4] = 0x01; //Frame ID, 0x00 turns off ackowlegments
    sendArray[5] = 0x00; //Destination Address, MSB
    sendArray[6] = 0x00; //Destination Address, LSB
    sendArray[7] = 0x00; //Acknowlegment, page 104, maybe it's 0x04
    sendArray[8] = 0x00; //Start of Data
    sendArray[9] = 0x00; //CHECK SUM
    
}


/**********************************************************************
 * Function: Xbee_sendString()
 * @param data that needs to be transmitted, split up into proper bytes
 * @remark Adds data to the sendArray, than sends that data over the 
 *  UART.
 **********************************************************************/
void Xbee_sendString(uint8_t data){
//load all the GPS data into the array
//compute the checksum
//send the array through the UART
//possibly load it into the circular buffer
    uint8_t checkSum = 0xFF, x;

    //fill array with data
    sendArray[8] = data[0];

    //calculate the CheckSum
    for(x=3; x <= 8; x++){
        checkSum -= sendArray[x];
    }
    sendArray[9] = checkSum;
    
    //send data
    for(x = 0; x <= 9; x++){
        UART_putChar(sendArray[x]);
    }
}


/**********************************************************************
 * Function: Xbee_recieveData()
 * @remark Beings taking in data, once a packet has been fully read in,
 * we will analyze that packet for it's useful information, namely
 * the GPS cordinate we should be driving towards.
 **********************************************************************/
void Xbee_recieveData(){
//load all the GPS data into the array
//compute the checksum
//send the array through the UART
//possibly load it intot the circular buffer

}


