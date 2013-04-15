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
#define INIT_THRESHOLD 1020

#define DO_AVERAGE



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
uint32_t averaging[NUMBER_OF_SAMPLES][10];
//uint32_t incoming_average[NUMBER_OF_SAMPLES], completed_average[NUMBER_OF_SAMPLES],deleting_average[NUMBER_OF_SAMPLES];

uint32_t *incoming_sonar_data = (incoming);
uint32_t *completed_sonar_data = (completed);
uint32_t *deleting_sonar_data = (deleting);
/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

void Sonar_init(){
    AD_init(ANALOG_WINDOW_PIN|ANALOG_PIN);
    Timer_init();
    Timer_new(TIMER_NUMBER, TIMER_TIME);
}

void Sonar_runSM(void){
    static int x=0;
    static uint32_t old_windowValue = 0;
    //Filling incoming array with data
    windowValue = getAnalogWindow();
    //if we have hit the peak value(every 100 ms)
    if(old_windowValue > INIT_THRESHOLD && old_windowValue > windowValue){
        //printf("Anaglog: %d\n", getAnalog());
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
        incoming_sonar_data[count++] = old_windowValue;
        incoming_sonar_data[count++] = windowValue;
        //begin timer
        Timer_new(TIMER_NUMBER, TIMER_TIME);
    }else if(Timer_isExpired(TIMER_NUMBER) && count < NUMBER_OF_SAMPLES){
        deleting_sonar_data[count] = 0;
#ifdef DO_AVERAGE
        averaging[count][x++%10] = windowValue;
        //average it
        int y;
        int total=0;
        for(y = 0; y < 10; y++){
            total += averaging[count][y];
        }
        total = total/10;
        incoming_sonar_data[count++] = total;

#else
        incoming_sonar_data[count++] = windowValue;
#endif
        Timer_new(TIMER_NUMBER, TIMER_TIME);
    }
    old_windowValue = windowValue;
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
//#define SONAR_TEST
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
            printf("\n");
            int x;
            //find average:
            /*
            float average = 0;
            for(x = 10; x< NUMBER_OF_SAMPLES -4; x++){
                average += rawAnalogWindow[x];
            }
            average = average/(NUMBER_OF_SAMPLES -14);
            printf("AVERAGE: %f\n", average);
            */
            for(x = 0; x < NUMBER_OF_SAMPLES; x++){
              /*  if(rawAnalogWindow[x]+10-((int)average) > 30 && x > 14){
                    printf("HIT\t");
                }
                else{
                    printf("0\t");
                }*/
             
                printf("%d\t",rawAnalogWindow[x]);
                //if(x > 10 && rawAnalogWindow[x]+10-((int)average) > 20)
                //    printf("<--hit\t");*/
                rawAnalogWindow[x] = 0;
               
            }
            Timer_new(4, 1000);
        }

    }
}
#endif

#define SONAR_MATLAB_TEST
#ifdef SONAR_MATLAB_TEST


//#include "Board.h"

#define START_SEQUENCE      (0xFFFF1234)


void sendSerial32(uint32_t data);
void sendSerialFloat(float data) ;

int main(){
    Board_init();
    Serial_init();
    Sonar_init();
    Timer_new(4, 200);

    uint32_t rawAnalogWindow[NUMBER_OF_SAMPLES] = {0};
    while(1){
        Sonar_runSM();
        if(Timer_isExpired(4)){
            Sonar_getRawData(rawAnalogWindow);
            int x;

            sendSerial32(START_SEQUENCE);
            for(x = 0; x < NUMBER_OF_SAMPLES; x++){
        
                sendSerial32(rawAnalogWindow[x]);
                rawAnalogWindow[x] = 0;
            }
            Timer_new(4, 200);
        }

    }
}

void sendSerial32(uint32_t data) {
    #ifndef DEBUG_TEST2
    int i;
    for (i = 0; i < 4; i++) {
        //while (!Serial_isTransmitEmpty()) { asm("nop"); }
            Serial_putChar((uint8_t)(data >>  (8 * i)));
    }
    #else
    printf("0x%X",data);
    #endif
}

void sendSerialFloat(float data) {
    #ifndef DEBUG_TEST2
    int i;
    for (i = 0; i < 4; i++) {
        //while (!Serial_isTransmitEmpty()) { asm("nop"); }
            Serial_putChar(((uint8_t)data >>  (8 * i)));
    }
    #else
    printf("_%.8f/",data);
    #endif
}

#endif