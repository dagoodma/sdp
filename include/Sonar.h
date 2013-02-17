/*
 * @file  Xbee.h
 *
 * @author John Ash
 *
 * @brief
 * State machine for Xbee module.
 *
 * @details
 * Module that wraps the bee in a statemachine that
 * reads from the UART.
 *
 * @date February 1, 2013 2:59 AM -- created
 *
 */
#ifndef SONAR_H
#define SONAR_H

#define AD_PIN AD_PORTV4
#define SONAR_AD_PIN        AD_PIN

void Sonar_init();


uint32_t Sonar_runSM();



#endif