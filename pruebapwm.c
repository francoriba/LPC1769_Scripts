#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>
#include <stdio.h>
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_uart.h"


uint32_t conta = 0;

void configPWM();
void configTimer0();
void TIMER0_IRQHandler();

int main(){
	SystemInit();
	configPWM();
	configTimer0();
	uint32_t clock = SystemCoreClock;
	while(1){

	}

	return 0;
}

void configPWM(){
	uint32_t aver = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_PWM1); // ver si queda a 25
	LPC_PINCON->PINSEL4 |= (1<<0); //Select PWM1.1 output for Pin2.0
	LPC_PWM1->PCR = 0x0; //Select Single Edge PWM - by default its single Edged so this line can be removed
	LPC_PWM1->PR = 24;
	LPC_PWM1->MR0 = 20000; // 20ms period duration
	LPC_PWM1->MR1 = 1000; //1ms - default pulse duration i.e. width
	LPC_PWM1->MCR = (1<<1); //Reset PWM TC on PWM1MR0 match
	LPC_PWM1->LER = (1<<1) | (1<<0); //update values in MR0 and MR1
	LPC_PWM1->PCR = (1<<9); //enable PWM output
	LPC_PWM1->TCR = 3; //Reset PWM TC & PR
	LPC_PWM1->TCR &= ~(1<<1); //libera la cuenta
}
void configTimer0();
void configTimer0(){
	TIM_TIMERCFG_Type timerCFG;
	timerCFG.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timerCFG.PrescaleValue = 1000;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&timerCFG); //setea a 25MHz el PCLK del timer haciendo CCLK/4

	TIM_MATCHCFG_Type matchCFG;
	matchCFG.MatchChannel = 1;
	matchCFG.IntOnMatch = ENABLE; // no hace falta 0que el timer interrumpa para generar toggleo en un mat
	matchCFG.ResetOnMatch = ENABLE;
	matchCFG.StopOnMatch = DISABLE;
	matchCFG.ExtMatchOutputType = TIM_EXTMATCH_NOTHING; //el mat0.1 no hace nada cada vez que desborda el timer
	matchCFG.MatchValue = 24999; // Tint = 1/100MHz * (1) * (MAT + 1) = 0.05s (interrumpe y togglea a 20Hz)
	TIM_ConfigMatch(LPC_TIM0, &matchCFG); //configura el mat 0.1 del timer 0
	TIM_Cmd(LPC_TIM0, ENABLE); //habilita TC y PC
	TIM_ResetCounter(LPC_TIM0); //resetea TC y PC
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(){

	if(TIM_GetIntStatus(LPC_TIM0, 1)){

		if(conta == 0){
			LPC_PWM1->MR1 = 1000; //1ms - default pulse duration i.e. width
			LPC_PWM1->LER = (1<<1); //update values in MR0 and MR1
			conta++;
		}
		else if(conta == 1){
			LPC_PWM1->MR1 = 1500; //1ms - default pulse duration i.e. width
			LPC_PWM1->LER = (1<<1); //update values in MR0 and MR1
			conta++;
		}
		else if(conta == 2){
			LPC_PWM1->MR1 = 2000; //1ms - default pulse duration i.e. width
			LPC_PWM1->LER = (1<<1); //update values in MR0 and MR1
			conta = 0;
		}

		TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);
	}

}
