/**********************************************************************
 Module
   Compas.c

 Author:
    David Goodman
    John Ash
    Shehadeh Dajani

 Description
    Main state machine for the COMPAS (command center), which acts as an
    interface between the lifeguard and the boat.

 Notes

 History
 When                   Who         What/Why
 --------------         ---         --------
4/29/2013   3:08PM      dagoodma    Started new Compas module.
***********************************************************************/
#define IS_COMPAS
#define DEBUG
//#define DEBUG_VERBOSE

#include <xc.h>
#include <stdio.h>
#include <plib.h>
#include "I2C.h"
#include "Serial.h"
#include "Board.h"
#include "Encoder.h"
#include "Ports.h"
#include "TiltCompass.h"
#include "Xbee.h"
#include "UART.h"
#include "Gps.h"
#include "Navigation.h"
#include "Barometer.h"
#include "Override.h"