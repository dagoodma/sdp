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
 1-18-13 8:10 PM jash    Create File
 1-22-13 5:24 PM jash    Move functions from Serial.c to Uart.c
 1-23-13 11:20AM jash    Make two circular buffers
***********************************************************************/


#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include "Uart.h"
#include "Board.h"


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define F_PB (Board_GetPBClock())
#define QUEUESIZE 512
#define UART2       UART3A

/*******************************************************************************
 * PRIVATE DATATYPES                                                           *
 ******************************************************************************/
typedef struct CircBuffer {
    unsigned char buffer[QUEUESIZE];
    int head;
    int tail;
    unsigned int size;
    unsigned char overflowCount;
} CircBuffer;
typedef struct CircBuffer* CBRef;


/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/
void newCircBuffer(CBRef cB);
void freeCircBuffer(CBRef* cB);
unsigned int getLength(CBRef cB);
int readHead(CBRef cB);
int readTail(CBRef cB);
unsigned char peak(CBRef cB);
unsigned char readFront(CBRef cB);
unsigned char writeBack(CBRef cB, unsigned char data);

/*******************************************************************************
 * PRIVATE VARIABLES                                                           *
 ******************************************************************************/
struct CircBuffer outgoingUart1;
CBRef transmitBufferUart1;
struct CircBuffer incomingUart1;
CBRef receiveBufferUart1;
struct CircBuffer outgoingUart2;
CBRef transmitBufferUart2;
struct CircBuffer incomingUart2;
CBRef receiveBufferUart2;



/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: UART_init()
 * @param id: identifies the UART module we want to initialize
 *        baudRate: The baudRate we want to initialize the function too.
 * @return a uint32_t value with the actual baudrate initialized. Check against
 *  desired to see if the system can handle the desired baud rate.
 * @remark Initializes the UART to the specific ID and BaudRate given as
 *  functions to the system. Will return the actual baudrate initialzied.
 *
 * Written by John Ash, help from Max Dunne 1/20/2013
 **********************************************************************/

uint32_t UART_init(UART_MODULE id, uint32_t baudRate){

    //will need buffers for both the UARTS
    if(id == 1){
        transmitBufferUart1 = (struct CircBuffer*) &outgoingUart1; //set up buffer for receive
        newCircBuffer(transmitBufferUart1);

        receiveBufferUart1 = (struct CircBuffer*) &incomingUart1; //set up buffer for transmit
        newCircBuffer(receiveBufferUart1);
    }else if(id == 2){
        transmitBufferUart2 = (struct CircBuffer*) &outgoingUart2; //set up buffer for receive
        newCircBuffer(transmitBufferUart2);

        receiveBufferUart2 = (struct CircBuffer*) &incomingUart2; //set up buffer for transmit
        newCircBuffer(receiveBufferUart2);

        mU2SetIntPriority(4); //set the interrupt priority
        mU2RXIntEnable(1);
        mU2TXIntEnable(1);
    }


    UARTConfigure(id,UART_ENABLE_PINS_TX_RX_ONLY);
    uint32_t actual_baudRate = UARTSetDataRate(id, F_PB, baudRate);
    UARTSetFifoMode(id, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    
    mU1SetIntPriority(4); //set the interrupt priority
     
    UARTEnable(id, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
    
    mU1RXIntEnable(1);
    mU1TXIntEnable(1);   
    
    return actual_baudRate;
}


/****************************************************************************
 Function
     UART_putChar

 Parameters
    char ch, the char to be sent out the serial port

 Returns
     None.

 Description
    adds char to the end of the circular buffer and forces the interrupt flag high is nothing is currently transmitting
 Notes


 Author
    Max Dunne, 2011.11.10
 ****************************************************************************/



void UART_putChar(uint8_t id, char ch)
{
    if(id == 1){
        if (getLength(transmitBufferUart1) != QUEUESIZE) {
            writeBack(transmitBufferUart1, ch);
            if (U1STAbits.TRMT) {
                IFS0bits.U1TXIF = 1;
            }
        }
    }else{
        if (getLength(transmitBufferUart2) != QUEUESIZE) {
            writeBack(transmitBufferUart2, ch);
            if (U2STAbits.TRMT) {
                IFS0bits.U2TXIF = 1;
            }
        }
    }
}

/****************************************************************************
 Function
     UART_getChar

 Parameters
     None.

 Returns
    ch - char from the serial port

 Description
    reads first character from buffer or returns 0 if no chars available
 Notes
     

 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
char UART_getChar(uint8_t id)
{
    if(id == 1){
        char ch;
        if (getLength(receiveBufferUart1) == 0) {
            ch = 0;
        } else {
            ch = readFront(receiveBufferUart1);
        }
        return ch;
    }else if(id == UART2){
        char ch;
        if (getLength(receiveBufferUart2) == 0) {
            ch = 0;
        } else {
            ch = readFront(receiveBufferUart2);
        }
        return ch;
    }
}

/****************************************************************************
 Function
     _mon_putc

 Parameters
    c - char to be sent

 Returns
    None.

 Description
    overwrites weakly define extern to use circular buffer instead of Microchip functions
 
 Notes
     

 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void _mon_putc(char c)
{
    UART_putChar(1, c);
}

/****************************************************************************
 Function
     _mon_puts

 Parameters
    s - pointer to the string to be sent

 Returns
    None.

 Description
    overwrites weakly defined extern to use circular buffer instead of Microchip functions

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void _mon_puts(const char* s)
{
    int i;
    for (i = 0; i<sizeof (s); i++)
        UART_putChar(1, s[i]);
}

/****************************************************************************
 Function
     _mon_getc

 Parameters
    canblock - unused variable but required to match Microchip form

 Returns
    None.

 Description
    overwrites weakly defined extern to use circular buffer instead of Microchip functions

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
int _mon_getc(int canblock)
{
    if (getLength(receiveBufferUart1) == 0)
        return -1;
    return UART_getChar(1);
}

/****************************************************************************
 Function
    UART_isReceiveEmpty

 Parameters
     None.

 Returns
    TRUE or FALSE

 Description
    returns the state of the receive buffer
 Notes
     

 Author
 Max Dunne, 2011.12.15
 ****************************************************************************/
char UART_isReceiveEmpty(uint8_t id)
{
    if(id == 1){
        if (getLength(receiveBufferUart1) == 0)
            return TRUE;
        return FALSE;
    } else if(id == UART2){
        if (getLength(receiveBufferUart2) == 0)
            return TRUE;
        return FALSE;
    }
}

/****************************************************************************
 Function
    UART_isTransmitEmpty

 Parameters
     None.

 Returns
    TRUE or FALSE

 Description
    returns the state of the transmit buffer
 Notes


 Author
 Max Dunne, 2011.12.15
 ****************************************************************************/
char UART_isTransmitEmpty(uint8_t id)
{
    if(id == 1){
        if (getLength(transmitBufferUart1) == 0)
            return TRUE;
        return FALSE;
    } else if(id == UART2){
        if (getLength(transmitBufferUart2) == 0)
            return TRUE;
        return FALSE;
    }
}

/****************************************************************************
 Function
    IntUart1Handler

 Parameters
    None.

 Returns
    None.

 Description
    Interrupt Handle for the uart. with the PIC32 architecture both send and receive are handled within the same interrupt

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void __ISR(_UART1_VECTOR, ipl4) IntUart1Handler(void)
{
    if (mU1RXGetIntFlag()) {
        mU1RXClearIntFlag();
        writeBack(receiveBufferUart1, (unsigned char) U1RXREG);
    }
    if (mU1TXGetIntFlag()) {
        mU1TXClearIntFlag();
        if (!(getLength(transmitBufferUart1) == 0)) {
            U1TXREG = readFront(transmitBufferUart1);
        }
    }

    if (mU2RXGetIntFlag()) {
        mU2RXClearIntFlag();
        writeBack(receiveBufferUart2, (unsigned char) U1RXREG);
    }
    if (mU2TXGetIntFlag()) {
        mU2TXClearIntFlag();
        if (!(getLength(transmitBufferUart2) == 0)) {
            U1TXREG = readFront(transmitBufferUart2);
        }
    }

}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

void newCircBuffer(CBRef cB)
{

    // initialize to zero
    int i;
    for (i = 0; i < QUEUESIZE; i++) {
        cB->buffer[i] = 0;
    }

    // initialize the data members
    cB->head = 0;
    cB->tail = 0;
    cB->size = QUEUESIZE;
    cB->overflowCount = 0;

}

// this function frees the Circular Buffer CB Ref

void freeCircBuffer(CBRef* cB)
{
    // if it is already null, nothing to free
    if (cB == NULL || *cB == NULL) {
        return;
    }

    // free and nil the pointer
    //free(*cB);
    *cB = NULL;
}




// Accesor Methods
// ===============

// returns the amount of unread bytes in the circular buffer

unsigned int getLength(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        if (cB->head <= cB->tail) {
            return (cB->tail - cB->head);
        } else {
            return (cB->size + cB->tail - cB->head);
        }
    } else {
        return 0;
    }


}

// returns the actual index of the head

int readHead(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        return (cB->head);
    } else {
        return 0;
    }

}

// returns the actual index of the tail

int readTail(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        return (cB->tail);
    } else {
        return 0;
    }

}

// returns the byte (actual value) that the head points to. this
// does not mark the byte as read, so succesive calls to peak will
// always return the same value

unsigned char peak(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        // if there are bytes in the buffer
        if (getLength(cB) > 0) {
            return cB->buffer[cB->head];
        }
    }
    return 0;
}


// Manipulation Procedures
// ======================
// returns the front of the circular buffer and marks the byte as read

unsigned char readFront(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        char retVal;
        // if there are bytes in the buffer
        if (getLength(cB) > 0) {
            retVal = cB->buffer[cB->head];
            cB->head = cB->head < (cB->size - 1) ? cB->head + 1 : 0;
            return retVal;
        }
        return 128;
    }
    return 254;
}

// writes one byte at the end of the circular buffer,
// increments overflow count if overflow occurs

unsigned char writeBack(CBRef cB, unsigned char data)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        if (getLength(cB) == (cB->size - 1)) {
            cB->overflowCount++;
            //return 1;
        } else {
            cB->buffer[cB->tail] = data;
            cB->tail = cB->tail < (cB->size - 1) ? cB->tail + 1 : 0;
            //return 0;
        }
        //return 0;
    } else {
        return 1;
    }
    return 0;
}

// empties the circular buffer. It does not change the size. use with caution!!

void makeEmpty(CBRef cB)
{
    if (cB != NULL) {
        int i;
        for (i = 0; i < cB->size; ++i) {
            cB->buffer[i] = 0;
        }
        cB->head = 0;
        cB->tail = 0;
        cB->overflowCount = 0;
    }
}

// returns the amount of times the CB has overflown;

unsigned char getOverflow(CBRef cB)
{
    if (cB != NULL) {
        return cB->overflowCount;
    }
    return 0;
}




//#define UART_TEST
#ifdef UART_TEST
#include "Serial.h"
#include "Board.h"
#include <GenericTypeDefs.h>
#include <p32xxxx.h>
#include <stdio.h>
#include <plib.h>


int main(void)
{

    Board_init();
    UART_init(UART2,115200);
    printf("\r\nUno Serial Test Harness\r\nAfter this Message the terminal should mirror anything you type.\r\n");
    unsigned char ch = 0;
    int x;
    while (1) {
        if (UART_isTransmitEmpty(1) == TRUE){
            if (UART_isReceiveEmpty(1) == FALSE){
                ch = UART_getChar(1);
                UART_putChar(1, ch);
                printf("  Through UART2: ");
                UART_putChar(2, ch);
                x = 1024;
                while(--x){
                    if(UART_isReceiveEmpty(2) ==FALSE)
                        break;
                }
                if(UART_isReceiveEmpty(2) == FALSE){
                    printf("&c\n", UART_getChar(2));
                } else{
                    printf("FAILED\n");
                }
                //DELAY(1000)
            }
        }
    }

    return 0;
}




#endif

#define UART2_TEST
#ifdef UART2_TEST

#define DELAY(count)  do { int i; for(i = 0; i <= count << 8; i++){ asm("nop");} } while(0);

#include<ports.h>
int main(void)
{
    UART_init(UART2,9600);
    PORTX05_TRIS = 0;
  /*  while(1){
    PORTX05_LAT = 1;
    DELAY(1000);
    PORTX05_LAT = 0;
    DELAY(1000);
    }
    */
    
    uint8_t x;
    uint16_t y;
    
    for(x=1; ;x = ((x++)%5) +1){
        if (UART_isTransmitEmpty(UART2) == TRUE)
            UART_putChar(UART2, x);
        y = 1024;
        while(--y){
            if(UART_isReceiveEmpty(UART2) == FALSE)
                break;
        }
        if(UART_isReceiveEmpty(UART2) == FALSE){
            for(y = UART_getChar(UART2); y != 0; y--){
                DELAY(1000);
                PORTX05_LAT = 1;
                DELAY(1000);
                PORTX05_LAT = 0;
            }
        }else{
            PORTX05_LAT = 1;
            DELAY(5000);
            PORTX05_LAT = 0;
        }
        DELAY(5000);
    }

    return 0;
}




#endif