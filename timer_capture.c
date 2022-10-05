
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configTimer();
void TIMER0_IRQHandler();

uint8_t i = 0;

int main(){
    SystemInit();
    configGPIO();
    configTimer();

    while(1){}
        return 0;
}

void configGPIO(){
    LPC_PINCON-> PINSEL4 &= ~(0b11<<8); //P2.4 gpio
    LPC_GPIO2->FIODIR |= (1<<4);    //P2.4 salida
    LPC_PINCON->PINMODE3 &= ~(0b11<<20); //pull up P1.26
}

void configTimer(){
    LPC_SC->PCONP |= (1<<1); //encendido del timer 0, aunque ya sale encendido del reset
    LPC_SC->PCLKSEL0 |= (1<<2); // uso frec del timer0 = 1/CCLK, sin usar divisor de frecuencia externo
    LPC_PINCON->PINSEL3 |= (0b11<<20); //Asigno funcion de CAP0.0 a P1.26
    LPC_TIM0->CCR |= (1<<1) | (1<<2); //captura por flanco descendente e interrumpe    //Prescaler -> PR
    LPC_TIM0->TCR = 3; //habilita el contador y el prescaler y los mantiene en estado de reset
    LPC_TIM0->TCR &= ~(1<<1); //vuelve el bit1 a 0 para que vuelva a empezar la cuenta (sale de reset)
    NVIC_EnableIRQ(TIMER0_IRQn);
}
void TIMER0_IRQHandler(){

    if(i == 0){
        LPC_GPIO2->FIOSET |= (1<<4);
        i = 1;
    }else if (i == 1){
        LPC_GPIO2->FIOCLR = (1<<4);
        i = 0;
    }
    LPC_TIM0->IR |= 1; //limpia flag de interrupcion del match0

}




