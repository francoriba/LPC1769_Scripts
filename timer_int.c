#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configTimer();
void TIMER0_IRQHandler();

int main(){

    configGPIO();
    configTimer();

    while(1){}
        return 0;
}

void configGPIO(){
    LPC_PINCON-> PINSEL3 &= ~(0b11<<8);
    LPC_PINCON->PINMODE3 |= (1<<9) & (~(1<<8));
    LPC_GPIO2->FIODIR |= (1<<4);

}

void configTimer(){
    LPC_SC->PCONP |= (1<<1); //encendido del timer 0, aunque ya sale encendido del reset
    LPC_SC->PCLKSEL0 |= (1<<2); // uso frec del timer0 = 1/CCLK, sin usar divisor de frecuencia externo
    LPC_PINCON->PINSEL3 |= (3<<24); //Asigno funcion de MAT0.0 (salida) P1.28
    LPC_TIM0->EMR |= (3<<4); //Asgina funcion de toggle al P1.28 cuando TC=MR0 (timer count = match reg)
    LPC_TIM0->MR0 = 70000000; // valor de MATCH0
    LPC_TIM0->MCR |= 0b11<<0; // cuando TC=MR0,hay interrupcion y el TC se resetea (vuelve a 0)
    //Prescaler -> PR
    LPC_TIM0->TCR = 3; //habilita el contador y el prescaler y los mantiene en estado de reset
    LPC_TIM0->TCR &= ~(1<<1); //vuelve el bit1 a 0 para que vuelva a empezar la cuenta (sale de reset)
    NVIC_EnableIRQ(TIMER0_IRQn);
}
void TIMER0_IRQHandler(){

    if(LPC_TIM0->IR  & (1<<0)){

        static uint8_t i = 0;

        if(i == 0){
            LPC_GPIO2->FIOSET |= (1<<4);
            i = 1;
        }else if (i == 1){
            LPC_GPIO2->FIOCLR = (1<<4);
            i = 0;
        }
        LPC_TIM0->IR |= 1; //limpia flag de interrupcion del match0
    }

}
