#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configExtInt();
void configSystick();
void SysTick_Handler();
void EINT2_IRQHandler();

uint8_t int_count_systick = 0;
uint8_t int_count_eint2 = 0;
uint16_t secuencia = 0x01;

int main (){
SystemInit();
configGPIO(); //configura P2.4 como GPIO de salida, sin pull ups ni pull downs
configExtInt(); //configura el P2.12 con su función como EINT2 y configura la interrupción
configSystick(); //configura systick

    while(1){
    }

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
    NVIC_SetPriority(EINT2_IRQn, 3); //prioridad 3
}

void EINT2_IRQHandler(){
    int_count_eint2++;
    if(int_count_eint2 == 9){
        int_count_eint2 = 0;
    }
    int_count_systick = 0; 
    LPC_SC->EXTINT |= (1<<2); //limpia flag de int externa
}

void configSystick(){
    SysTick->CTRL = (0b111<<0); //habilitamos, seleccionamos fuente interna, habilitamos interrupcion
    SysTick->LOAD =  0x189F;
    SysTick->VAL = 0;
    NVIC_SetPriority(SysTick_IRQn, 4); //prioridad 4
}

