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

#define TIMER_NUMBER 8
#define TIMER_TIME 100/NUMBER_OF_SAMPLES
#define INIT_THRESHOLD 1022



/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static uint32_t getAnalog();
static uint32_t getAnalogWindow();

/**********************************************************************
 * PRIVATE VARIABLES                                                 *
 **********************************************************************/

//uint32_t rawAnalogData[100] = 0;
//uint32_t averagedAnalogData[100]=0;
uint8_t count = 0;
uint8_t print_count = 0;
uint32_t oldWindowValue=0, windowValue=0;

uint32_t incoming[NUMBER_OF_SAMPLES],completed[NUMBER_OF_SAMPLES],deleting[NUMBER_OF_SAMPLES];

uint32_t *incoming_sonar_data = (incoming);
uint32_t *completed_sonar_data = (completed);
uint32_t *deleting_sonar_data = (deleting);
/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

void Sonar_init(){
    AD_init(ANALOG_WINDOW_PIN);
    Timer_init();
    Timer_new(TIMER_NUMBER, TIMER_TIME);
}

void Sonar_runSM(void){

    //Filling incoming array with data
    windowValue = getAnalogWindow();
    //if we have hit the peak value(every 100 ms)
    if(windowValue > INIT_THRESHOLD && count > 5){
        //make sure everything was deleted
        while(count < NUMBER_OF_SAMPLES)
            deleting_sonar_data[count++] = 0;
        count = 0;
        
        //Switch the pointers
        //printf("\n%d Compared to ",incoming_sonar_data[7]);
        uint32_t *temp;
        temp = completed_sonar_data;
        completed_sonar_data = incoming_sonar_data;
        incoming_sonar_data = deleting_sonar_data;
        deleting_sonar_data = temp;
        //printf("%d\n",completed_sonar_data[7]);

        //delete first value
        deleting_sonar_data[count] = 0;
        //fill in first value
        incoming_sonar_data[count++] = windowValue;
        //begin timer
        Timer_new(TIMER_NUMBER, TIMER_TIME);
    }else if(Timer_isExpired(TIMER_NUMBER) && count < NUMBER_OF_SAMPLES){
        deleting_sonar_data[count] = 0;
        incoming_sonar_data[count++] = windowValue;
        Timer_new(TIMER_NUMBER, TIMER_TIME);
    }
}

void Sonar_getRawData(uint32_t *output){
    int x;
    for(x = 0; x < NUMBER_OF_SAMPLES; x++){
        output[x] = completed_sonar_data[x];
    }
}

/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/

static uint32_t getAnalog(){
    return AD_readPin(ANALOG_PIN);
}

static uint32_t getAnalogWindow(){
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
    //mJTAGPortEnable(0);
    Sonar_init();
    Timer_new(4, 1000);
    printf("Sonar Init\n");

    uint32_t rawAnalogWindow[NUMBER_OF_SAMPLES] = {0};
    while(1){
        Sonar_runSM();
        if(Timer_isExpired(4)){
            Sonar_getRawData(rawAnalogWindow);
            printf("\nDATA\n");
            int x;
            for(x = 0; x < NUMBER_OF_SAMPLES; x++){
                printf("%d\t",rawAnalogWindow[x]);
                rawAnalogWindow[x] = 0;
            }
            Timer_new(4, 1000);
        }

    }
}
#endif