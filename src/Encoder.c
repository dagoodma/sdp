/*
 * File:   Encoder.c
 * Author: Darrel R. Deo
 *
 * Created on January 28, 2013, 9:14 PM
 */

#include <xc.h>
//#include <p32xxxx.h>
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
#define FOSC           80E6 //80 MHz Clockrate
#define PB_DIV         8    //Peripheral Bus Divider ==> 10Mhz
#define PRESCALE       1    //Prescale 1:1 ==> 1 timer tick = 1 peripheral clk cycle
#define MSEC           10E-3
#define T1_TICK        0xFFFF// (500 * MSEC * FOSC)/(PB_DIV * PRESCALE)
#define CaptureTimer   2
/***********************************************************************
 * PUBLIC FUNCTIONS                                                    *
 ***********************************************************************/



/***********************************************************************
 * GLOBAL VARIABLES                                                    *
 ***********************************************************************/
static int Rise = 0;
static int Fall = 0;
static unsigned int start = 0;
static unsigned int stop = 0;
static  uint16_t tempBuff = 0;
//static int pulsewidth = 0;
enum state {calculate, poll};
static int state = poll;
static int count = 0;
    int start_array[200];
    int stop_array[200];
    int pulsewidth[200];

int main(void){




    int i = 0;
    int j = 0;
    int pwidth = 0;
    //initialize modules
    Board_init();
    Timer_init();
    Serial_init();
    INTEnableSystemMultiVectoredInt();
    printf("OK WE ARE IN!\n");
while(!Serial_isTransmitEmpty()) ;

    //using J5, J5-03: RD3 IO input, J5-02: RD5: PORTY
    PORTY04_TRIS = 0;
    PORTZ08_TRIS = 1;
    PORTY04_LAT = 0;


      //clear interrupt flag
      mIC1ClearIntFlag();

      //setup Timer 3
      OpenTimer3(T3_ON | T1_PS_1_1, T1_TICK); //T3 on, Prescale 1:256,

      //Enable Capture Module and settings:
      // Capture every edge
      // Enable capture interrupts
      // Use timer 3 source
      //Capture rising edge first

      OpenCapture1(IC_EVERY_EDGE | IC_INT_1CAPTURE | IC_TIMER3_SRC | IC_FEDGE_RISE | IC_ON);
      //IC1CON &= ~(_IC1CON_C32_MASK);
      ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_1);
      mIC1IntEnable( 1);

      //}
    


    
    
    while(1){
        switch(state){
            case(calculate):
                i++;
               // if(Serial_isTransmitEmpty()){
                    //printf("CALCULATE\n\n\n");
                 //   if(Timer_isActive(5)) printf("YES TIMER\n\n");
                    //printf("start: %u  stop: %u\n\n\n",start,stop);
                //}
                //printf("CALCULATE\n\n\n");
//                    if (stop > start){
//                       pwidth = (stop - start);
//                       pulsewidth[j] = pwidth/10;
//                    }else{
//                       pwidth = (T1_TICK - start) + stop;
//                       pulsewidth[j] = pwidth/10;
//                    }
//

               // printf("YOU CAUGHT ONE PULSE!");
            //state = poll;
                if (i == 10000){
//                    exit(1);
                    i = 0;
                    Serial_putChar('I');
                    mIC1IntEnable(1);
                }
            break;

            case(poll):
                   //printf("Polling\n\n");
            break;
        }

    }
    return(SUCCESS);
    



}

void __ISR( _INPUT_CAPTURE_1_VECTOR, ipl1) IC1Interrupt( void){
    tempBuff = mIC1ReadCapture();
  //  tempBuff &= 0xFF;
    //mIC1ClearIntFlag();
    if (Rise == 0){
        PORTY04_LAT = 1;
        //Rising Edge
        Rise = 1;
        start = tempBuff;
        start_array[count];
        mIC1ClearIntFlag();
        Serial_putChar('R');
        
        
    }else{
        PORTY04_LAT = 0;
        //Falling Edge
        Rise = 0;
        state = calculate;
        stop = tempBuff;
        stop_array[count];
        mIC1IntEnable(0);
        //Timer_new(5,300);
        count = count + 1;
        mIC1ClearIntFlag();
        Serial_putChar('F');

    }
}
