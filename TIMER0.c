#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configTimer();

int main(){

    return 0;
}


void configTimer(){
    LPC_SC->PCONP |= (1<<1); //encendido del timer 0, aunque ya sale encendido del reset 
    LPC_SC->PCLKSEL0 |= (1<<2); // uso frec del timer0 = 1/CCLK, sin usar divisor de frecuencia externo
    LPC_PINCON->PINSEL3 |= (3<<24); //Asigno funcion de MAT0.0 (salida) P1.28
    LPC_TIM0->EMR |= (3<<4); //Asgina funcion de toggle al P1.28 cuando TC=MR0 (timer count = match reg)
    LPC_TIM0->MRO = 70000000; // valor de MATCH0
    LPC_TIM0->MCR = 0b10<<0; // cuando TC=MR0, no hay interrupcion y el TC se resetea (vuelve a 0)
    LPC_TIM0->TCR = 3; //habilita el contador y el prescaler y los mantiene en reset
    LPC_TIM0->TCR &= ~(1<<1);
    //NVIC_EnableIRQ(TIMER0_IRQn);
}
