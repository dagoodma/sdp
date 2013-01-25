/* 
 * File:   main.c
 * Author: dagoodma
 *
 * Created on January 24, 2013, 4:34 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "Serial.h"
#include "Board.h"

/*
 * 
 */
int main(void) {
    Board_init();

    while(1) {
        printf("Test\n");
        DELAY(1000);
    }


    return SUCCESS;
}

