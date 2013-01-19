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
 * @return none
 * @remark Initializes the Xbee module. This function will also set
 *  the correct API mode for the Xbees to be paired with each other.
 *  Options:
 *      1: Ground Station Xbee
 *      2: Boat Xbee
 **********************************************************************/
void Xbee_init(uint8_t options) {
    if(options == GROUND_STATION_XBEE){
        Xbee_programInit(options);
        xbeeInitialized = TRUE;
    }else if(options == BOAT_XBEE){
        Xbee_programInit(options);
        xbeeInitialized = TRUE;
    }else{
        xbeeInitialized = FALSE;
    }
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
 * Function: Xbee_programInit()
 * @param Which Xbee is being programed, ground station or boat
 * @remark Puts the Xbee into API mode.
 **********************************************************************/
void Xbee_programInit(uint8 whichXbee){
    int count;
    Uart2_print("+++");
    for(count = 0; count < 14; count++){
        if(count % 3 == 0){
            Uart2_print("+++");
        }
        while(Uart2_recieve_isEmpty() == EMPTY){ };
        if(Uart2_recieve == "Ok\n"){
            break;
        }
    }
    Uart2_print("ATAP1\r"); //Why does it end in \r? // Puts it in API mode
    Uart2_print("ATCH14\r"); //sets the channel to something other than default
    Uart2_print("ATID7806\r"); //Pan ID, needs more research
    Uart2_print("ATDH"); //High & Low Xbee Destination addresses. 
    Uart2_print(whichXbee);
    Uart2_print("\r");
    Uart2_print("ATDL");
    Uart2_print(whichXbee);
    Uart2_print("\r");
    Uart2_print("ATBD5\r");//Sets the Baud Rate
    Uart2_print("ATWR\r");//Writes the command to memory
    Uart2_print("ATCN\r");//Leave the menu.
    xbeeApiMode = TRUE; 
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
    sendArray[2] = 0x0F; //LSB of the length
    sendArray[3] = 0x01; //API id
    sendArray[4] = 0x00; //Frame ID, uncertain of what this does.
    sendArray[5] = 0x00; //Destination Address, MSB
    sendArray[6] = 0x00; //Destination Address, LSB
    sendArray[7] = 0x04; //Acknowlegment, page 104, maybe it's 0x00
    sendArray[8] = 0x00; //Start of Data
    sendArray[9] = 0x00; //CHECK SUM
    
}


/**********************************************************************
 * Function: Xbee_sendString()
 * @param data that needs to be transmitted, split up into proper bytes
 * @remark Adds data to the sendArray, than sends that data over the 
 *  UART.
 **********************************************************************/
void Xbee_sendString(string data){
//load all the GPS data into the array
//compute the checksum
//send the array through the UART
//possibly load it intot the circular buffer
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


