/**********************************************************************
 Module
   Uart.c

 Revision
   1.0.0

 Description
 Code for initilazing and running the UART

 Notes

 History
 When           Who         What/Why
 -------------- ---         --------
 12-18-13 8:10 PM jash    Create File
***********************************************************************/

/**********************************************************************
 * Function: UART_init()
 * @param id: identifies the UART module we want to initialize
 *        baudRate: The baudRate we want to initialize the function too.
 * @return a UINT32 value with the actual baudrate initialized. Check against
 *  desired to see if the system can handle the desired baud rate.
 * @remark Initializes the UART to the specific ID and BaudRate given as
 *  functions to the system. Will return the actual baudrate initialzied.
 **********************************************************************/

UINT32 UART_init(UART_MODULE id, UINT32 baudRate);

/**********************************************************************
 * Function: UART_sendData()
 * @param id: identifies the UART module we want to send data over
 *        data: The location of the bytes of data we want to send
 *        size: The number of bytes we want to send over the UART
 * @remark Uses blocking code to send data over the UART. Does NOT
 *  check the bytes to make sure they are not junk.
 **********************************************************************/
void UART_sendData(UART_MODULE id, UINT8* data, int size);

/**********************************************************************
 * Function: UART_getByte()
 * @param id: identifies the UART module we want to get data from
 * @remark 
 **********************************************************************/
UINT16 UART_getByte(UART_MODULE id);

