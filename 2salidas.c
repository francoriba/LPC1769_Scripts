/*
===============================================================================
 Name        : blink_led.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description :
===============================================================================
*/

#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configExtInt();
void configSystick(uint32_t load);
void SysTickHandler();
void EINT2_IRQHandler();
void waveform1();
void waveform2();

uint8_t wafevorm;
uint8_t systick_count;

int main(){

}

void configGPIO(){
    LPC_PINCON->PINSEL4 &= ~(0b1111<<0); // P2.0 y P2.1 como GPIO
    LPC_PINCON->PINMODE4 |= (1<<3) | (1<<1); // P2.0 y P2.1 sin pullup ni pulldown
    LPC_GPIO2->FIODIR |=  (0b11<<0);
}