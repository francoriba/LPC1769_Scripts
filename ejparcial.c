/*
===============================================================================
 Name        : blink_led.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description :  produzca una interrupción por flanco descendente en EINT2 sacar
                por pin P2.4 la secuencia . En caso que se produzca una nueva
                interrupción por EINT2 mientras se está realizando la secuencia,
                se pondrá en uno la salida P2.4 y se dará por finalizada
                la secuencia. El programa NO debe hacer uso de retardos por
                software y deben enmascararse los pines del puerto 2 que no van
                 a ser utilizados. Suponer una frecuencia de reloj cclk de 60Mhz.
                 Se pide el programa completo debidamente comentado y los
                 respectivos cálculos de tiempo asignados al Systick.
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configExtInt();
void outputWaveform();
void configSystick();
void SysTick_Handler();
void EINT2_IRQHandler();

uint8_t int_count = 0;
uint8_t waveform_flag = 0;
uint32_t timer = 0;

int main (){
SystemInit();
configGPIO(); //configura P2.4 como GPIO de salida, sin pull ups ni pull downs
configExtInt(); //configura el P2.12 con su función como EINT2 y configura la interrupción

while(1){
    if(waveform_flag){
        outputWaveform();
    }
    else{
        LPC_GPIO2-> FIOPIN |= (1<<4);
    }
}
    return 0;
}

void configGPIO(){
    LPC_PINCON->PINSEL4 &= ~(0b11<<8); //P2.4 como GPIO
    LPC_PINCON->PINMODE4 |= (1<<9); //P2.4 sin pull up ni pull down
    LPC_GPIO2->FIODIR |= (1<<4); //P2.4 como output
    LPC_GPIO2->FIOMASK = 0xFFFFFFFF; //enmascara todos los bits del P2
    LPC_GPIO2->FIOMASK &= ~ ((1<<4)|(1<<12)); //desenmascara los bits 4 y 12 del P2
}
void configExtInt(){
    LPC_PINCON->PINSEL4 |= (1<<24); //P2.12 como EINT2
    LPC_SC->EXTMODE = (1<<2); //edge
    LPC_SC->EXTPOLAR &= ~(1<<2); // falling edge por defecto
    LPC_SC->EXTINT |= (1<<2); //limpia flag de int externa
    NVIC_EnableIRQ(EINT2_IRQn); //habilita vector de interrupcion
}
void outputWaveform(){
    if(int_count < 4){
        LPC_GPIO2->FIOPIN &= ~(1<<4);
    }
    else if(int_count == 4){
        LPC_GPIO2->FIOPIN |= (1<<4);
    }
    else if(int_count == 5){
        LPC_GPIO2->FIOPIN &= ~(1<<4);
    }
    else if(int_count == 6){
        LPC_GPIO2->FIOPIN |= (1<<4);
    }
    else if(int_count == 7){
        LPC_GPIO2->FIOPIN &= ~(1<<4);
    }
    else if(int_count == 8){
        LPC_GPIO2->FIOPIN |= (1<<4);
    }
    else if(int_count == 9){
        LPC_GPIO2->FIOPIN &= ~(1<<4);
    }
    else if(int_count == 10){
        LPC_GPIO2->FIOPIN |= (1<<4);
    }
    else if((int_count >= 11) && (int_count<=15)){
        LPC_GPIO2->FIOPIN &= ~(1<<4);
    }
}

void EINT2_IRQHandler(){
    if(waveform_flag == 0){
        waveform_flag = 1;
        configSystick();
    }
    else{
        waveform_flag = 0;
        SysTick->CTRL  &= ~(0b1<<0); //deshabilito Systick
    }
    LPC_SC->EXTINT |= (1<<2); //limpia flag de int externa
}

void configSystick(){
    SysTick->CTRL = (0b111<<0); //habilitamos, seleccionamos fuente interna, habilitamos interrupcion
    SysTick->LOAD =  0x5B8D7F; // 1 tick -> 1/60MHz entonces 10ms -> 10ms*60MHz -1 = 5999999 = 0x5B8D7F
    SysTick->VAL = 0;
    NVIC_SetPriority(SysTick_IRQn, 3);
}

void SysTick_Handler(){
    timer++; //variable testigo usada para el debug, no hace falta
    int_count++;
    if(int_count == 16){
        int_count = 0;
    }
    SysTick->CTRL &= SysTick->CTRL; //limpio flag de desborde del systick
}
