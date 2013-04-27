/*
 * File:   Override.c
 * Author: David Goodman, Darrel Deo
 *
 * Created on April 25, 2013, 3:27 PM
 */
#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "Board.h"
#include "Serial.h"
#include "Timer.h"
#include "Drive.h"
//#include "Logger.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define DEBUG
#define USE_DRIVE


#define OVERRIDE_TIMEOUT_DELAY  2000 //  (ms) receiver turns off


// Ports
#define ENABLE_OUT_TRIS PORTX12_TRIS // J5-06
#define ENABLE_OUT_LAT  PORTX12_LAT // J5-06
#define MICRO_HAS_CONTROL       0
#define RECIEVER_HAS_CONTROL    1


/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/


/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/

/**********************************************************************
 * Function: Override_init
 * @return None
 * @remark Initializes the override board and the interrupt to detect
 *  when the receiver comes online.
 * @author Darrel Deo
 * @date 2013.04.01  
 **********************************************************************/
void Override_init() {

    // Initialize override board pins to give Micro control
    ENABLE_OUT_TRIS = OUTPUT;  // Set pin to be an output (fed to the AND gates)
    ENABLE_OUT_LAT = MICRO_HAS_CONTROL;  // Initialize control for Microcontroller

    // Let override timer expire to disable override
    Timer_new(TIMER_OVERRIDE, 1);
    while (Override_isTriggered()) {
        asm("nop");
    }

    //Enable the interrupt for the override feature
    mPORTBSetPinsDigitalIn(BIT_0); // CN2

    mCNOpen(CN_ON | CN_IDLE_CON , CN2_ENABLE , CN_PULLUP_DISABLE_ALL);
    uint16_t value = mPORTDRead(); //?
    ConfigIntCN(CHANGE_INT_ON | CHANGE_INT_PRI_2);

    //CN2 J5-15

    //DBPRINT("Override Function has been Initialized\n");

    INTEnable(INT_CN,1);
}

/**********************************************************************
 * Function: Override_giveReceiverControl
 * @return None
 * @remark Passes motor control over to the receiver.
 * @author David Goodman
 * @date 2013.04.01 
 **********************************************************************/
void Override_giveReceiverControl() {
    //DBPRINT("Reciever has control\n");
#ifdef USE_DRIVE
    Drive_stop();
#endif

    //Give control over to Reciever using the enable line
    ENABLE_OUT_LAT = RECIEVER_HAS_CONTROL;
}

/**********************************************************************
 * Function: Override_giveMicroControl
 * @return None
 * @remark Passes motor control over to the micro.
 * @author David Goodman
 * @date 2013.04.01  
 **********************************************************************/
void Override_giveMicroControl() {
    //DBPRINT("Micro has control\n");
#ifdef USE_DRIVE
    Drive_stop();
#endif

    //Give control over to Micro using the enable line
    ENABLE_OUT_LAT = MICRO_HAS_CONTROL;      
}


/**********************************************************************
 * Function: Override_isTriggered
 * @return TRUE or FALSE if the receiver has come online.
 * @remark The receiver will only be considered offline (FALSE), if 
 *  it is turned off for longer and 2 seconds.
 * @author David Goodman
 * @date 2013.04.01  
 **********************************************************************/
bool Override_isTriggered() {

    return !Timer_isExpired(TIMER_OVERRIDE);
}



/**********************************************************************
 * Function: Interrupt Service Routine for Override board
 * @return None
 * @remark ISR that is called when CH3 pings external interrupt
 * @author Darrel Deo
 * @date 2013.04.01 
 **********************************************************************/
void __ISR(_CHANGE_NOTICE_VECTOR, ipl2) ChangeNotice_Handler(void){
    mPORTDRead(); //?

    Timer_new(TIMER_OVERRIDE, OVERRIDE_TIMEOUT_DELAY);

    //Clear the interrupt flag that was risen for the external interrupt
    //might want to set a timer in here

    mCNClearIntFlag();
    //INTEnable(INT_CN,0);
}

