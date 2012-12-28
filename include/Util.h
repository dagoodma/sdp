/*
 * File: Util.h
 *
 * Utilities and useful definitions.
 *
 */

#ifndef Util_H
#define Util_H

#include <stdint.h>
#include "Error.h"
#include "Util.h"


/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

typedef char bool;

// constants
#define SUCCESS 0
#define ERROR   -1
#define FAILURE ERROR

#define TRUE    ((bool)1)
#define FALSE   ((bool)0)

#define ON      ((bool)1)
#define OFF     ((bool)0)

#define UP      ((bool)0)
#define DOWN    ((bool)1)

// DDRx register
#define INPUT   0
#define OUTPUT  1

// delay times (ms)
#define MICRO_DELAY 1 
#define TINY_DELAY 2
#define SHORT_DELAY 4
#define MEDIUM_DELAY 250
#define LONG_DELAY 750


// functions
#define SECOND_TO_MILLISECOND(a)   (a * 1000)
#define MINUTE_TO_MILLISECOND(a)   (SECOND_TO_MILLISECOND(a) * 60)

#define BYTE_TO_BIT(a)             (a * 8)

#define ONE_BYTE                   8


#define STRING_GET_BYTE(sp,i)      ((uint8_t)sp[i])
#define STRING_GET_WORD(sp,i)      ((((uint16_t)STRING_GET_BYTE(sp,i)) << 8) \
    + STRING_GET_BYTE(sp,i+1))

#define STRING_SET_BYTE(sp,i,val)  sp[i] = (char)val
#define STRING_SET_WORD(sp,i,val)  do { STRING_SET_BYTE(sp,i+1,val); \
    STRING_SET_BYTE(sp,i,((char)(val >> ONE_BYTE))); } while(0)

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

#endif 
