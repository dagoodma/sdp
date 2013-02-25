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
#include <Ports.h>
#include "Board.h"
#include "Uart.h"
//#include "Mavlink.h"
#include <Timer.h>
#include <AD.h>
#include "Sonar.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
//Cannot be greater than 100
#define NUMBER_OF_SAMPLES 100

#define TIMER_NUMBER 4
#define TIMER_TIME 100/NUMBER_OF_SAMPLES



/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static float getAnalog();
static float getAnalogWindow();

/**********************************************************************
 * PRIVATE VARIABLES                                                 *
 **********************************************************************/

//uint32_t rawAnalogData[100] = 0;
//uint32_t averagedAnalogData[100]=0;
uint8_t count = 0;

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

void Sonar_init(){
    AD_init(ANALOG_WINDOW_PIN | ANALOG_PIN);
    Timer_init();
    Timer_new(TIMER_NUMBER, TIMER_TIME);
}

 BOOL Sonar_runSM(float* rawAnalogWindowData, float* rawAnalogData){
    if(Timer_isExpired(TIMER_NUMBER)){

        Timer_new(TIMER_NUMBER, TIMER_TIME);
        *rawAnalogData = getAnalog();
        rawAnalogWindowData[count++%NUMBER_OF_SAMPLES] = getAnalogWindow();
        if (count == NUMBER_OF_SAMPLES){
            count = 0;
            return TRUE;
        }
    }
    return FALSE;
}




/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

static float getAnalog(){
    return AD_readPin(ANALOG_PIN);
}

static float getAnalogWindow(){
    return AD_readPin(ANALOG_WINDOW_PIN);
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
#define SONAR_TEST
#ifdef SONAR_TEST


//#include "Board.h"



int main(){
    Board_init();
    Serial_init();
    mJTAGPortEnable(0);
    Sonar_init();
    printf("Sonar Init");

    uint8_t x;
    float rawAnalogWindow[NUMBER_OF_SAMPLES] = {0};
    float analog;
    while(1){
        if(Sonar_runSM(rawAnalogWindow, &analog) == TRUE){
            printf("Analog: %f\n", analog);
            printf("Raw Data:");
            for(x=0; x < NUMBER_OF_SAMPLES; x++){
                printf("\t %f", rawAnalogWindow[x]);
            }
        }
    }
}
#endif