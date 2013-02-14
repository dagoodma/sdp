/*
 * File:   Encoder.c
 * Author: Darrel R. Deo
 *
 * Created on January 28, 2013, 9:14 PM
 */

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "Ports.h"
#include "Encoder.h"


// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 10 MHz Peripheral Clock
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
// WDT OFF
// Other options are don't care
//

#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_8


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define T3_TICK        0xFFFF// (500 * MSEC * FOSC)/(PB_DIV * PRESCALE)
#define CaptureTimer   2

#define PRINT_DELAY    100//1 second
#define AngleScalar    2195E-6
#define DEBUG

/*DEFINE IO BUTTONS*/
//LockOn Button
#define LockOn_DIR PORTY05_TRIS
#define LockOn PORTY05_BIT


/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/
 void Encoder_init(){
      //setup Timer 3
      OpenTimer3(T3_ON | T3_PS_1_1, T3_TICK);

      //Enable Capture Module and settings:
      // Capture every edge
      // Enable capture interrupts
      // Use timer 3 source
      //Capture rising edge first
      OpenCapture1(IC_EVERY_EDGE | IC_INT_1CAPTURE | IC_TIMER3_SRC | IC_FEDGE_RISE | IC_ON);

      //Enable the Interrupt for input capture, set priority level 1
      ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_1);
      mIC1IntEnable(0);

      //clear interrupt flag
      mIC1ClearIntFlag();
    }

 float calculate_Angle(uint16_t pwidth){
     return (pwidth * AngleScalar);
 }

 BOOL Is_lockOnButtonPressed(){
     if(LockOn == 1)
         return TRUE;
     else
         return FALSE;
 }


/***********************************************************************
 * GLOBAL VARIABLES                                                    *
 ***********************************************************************/
//Rising and Falling edge flags
static int16_t Rise = 0;
static int16_t Fall = 0;

//start,stop, temporary buffer for captured time
static uint16_t start = 0;
static uint16_t stop = 0;
static uint16_t Pulse = 0;
static uint16_t tempBuff = 0;
static uint16_t accum_count = 0;
static float    angle = 0;


//State declarations
enum state {calculate, poll, event_checker, take_avg};
static int16_t state = poll;

/************************************************************************/
//#define State_Machine
#ifdef State_Machine

int main(void){
     //Module Initializations
    Board_init();
    Timer_init();
    Serial_initSM();
    Encoder_init();
    LockOn_DIR = 1;
    float accumulator;

    state = event_checker;
    while(1){
        switch(state){
            case(event_checker):
                //
                //printf("EventCheck\n\n");
                if(Is_lockOnButtonPressed()){
                    mIC1IntEnable(1);
                    state = poll;
                }

            break;

            case(calculate):
                state = poll;
                if(stop > start)
                {
                    Pulse = stop - start;
                }else{
                    Pulse = (T3_TICK - start) + stop;
                }

                accumulator += calculate_Angle(Pulse);
                accum_count++;
                if(accum_count == 300){
                    mIC1IntEnable(0);
                    angle = accumulator/300;
                    printf("ANGLE: %.3f\n\n\n",angle);
                    state = event_checker;
                    accum_count = 0;
                    accumulator = 0;
                }
                
            break;

            case(poll):
                ;
            break;


        }


    }





}
#endif

#define Test_Harness
#ifdef Test_Harness
int main(void){

    //Module Initializations
    Board_init();
    Timer_init();
    Serial_initSM();
    Encoder_init();
    mIC1IntEnable(1);


   // printf("Encoder Test Harness!\n");
   Timer_new(TIMER_TEST,PRINT_DELAY);


    //Initialize PortY04 for digital input
    PORTY04_TRIS = 0;
    PORTY04_LAT = 0;

    //State Machine
    while(1){

        switch(state){

            case(calculate):
                //printf("Calculate");
                //Serial_putChar('C');
                if(stop > start)
                {
                    Pulse = stop - start;
                }else{
                    Pulse = (T3_TICK - start) + stop;
                }

                angle = calculate_Angle(Pulse);
                //state = poll;

                printf("START: %u      STOP:  %u     Pulse: %u      Angle: %.3f degrees\n\n\n", start, stop, Pulse, angle);
                //Serial_runSM();
                while (!Serial_isTransmitEmpty()) {
                    ;
                }
                state = poll;

                break;

            case(poll):
                   //printf("Polling\n\n");
                //Serial_putChar('P');
                break;
        }
        //Serial_runSM();
    }
    return(SUCCESS);
}
#endif



/*********************************************************************
 *                  INTERRUPT SERVICE ROUTINES                       *
 * *******************************************************************/
void __ISR( _INPUT_CAPTURE_1_VECTOR, ipl1) IC1Interrupt( void){
    //Read Timer Buffer as soon as you enter

    tempBuff = mIC1ReadCapture();
    mIC1IntEnable(0);
    //Serial_putChar('I');
    //Rising Edge
    if (Rise == 0){
        PORTY04_LAT = 1;
        //Serial_putChar('R');                                                        //DEBUG PWM
        Rise = 1;                                                               //Set to Rise
        start = tempBuff;                                                       //Start holds timer buffer of rising edge
        mIC1IntEnable(1);
    //Falling Edge
    }else{
        PORTY04_LAT = 0;
        //mIC1IntEnable(1);
        //Serial_putChar('F');                                                        //DEBUG PWM
        Rise = 0;                                                               //Set to fall
        state = calculate;                                                      //Change state to Calculate Pulsewidth
        stop = tempBuff;                                                        //Stop holds timer buffer of falling edge
                                                                                  //Disable IC interrupt
      
    }


    mIC1IntEnable(1);
    //Clear Interrupt Flag
    mIC1ClearIntFlag();
}
