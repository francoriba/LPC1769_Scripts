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

#define PIN_4 ((uint32_t) (1<<4))

void configGPIO(); //configura P2.4 como salida
void configTIMER0(); //configura el timer 0
void configEINT0();
void TIMER0_IRQHandler();
void EINT0_IRQHandler();

uint32_t pclktimer0 = 0;

int main(){
	uint32_t verClk = SystemCoreClock;
	SystemInit();
	configGPIO();
	configEINT0();
	configTIMER0();

	while(1){}
	return 0;
}

void configGPIO(){
	PINSEL_CFG_Type pin_configuration;
	pin_configuration.Portnum = 2; //puerto 2
	pin_configuration.Pinnum = 4; //pin 4
	pin_configuration.Pinmode = 0; //pull-up (no importa porque va a ser configurado como salida)
	pin_configuration.Funcnum = 0; // funcion de GPIO
	pin_configuration.OpenDrain = 0; // sin open drain
	PINSEL_ConfigPin(&pin_configuration);
	GPIO_SetDir(2, 4, 1); //P2.4 como salida
}

void configTIMER0(){
	PINSEL_CFG_Type pin_configuration; //P1.28 EXTMATCH 0.0
	TIM_TIMERCFG_Type struct_config;//prescaler config
	TIM_MATCHCFG_Type struct_match; //match config

	pin_configuration.Portnum = 1; //puerto 1
	pin_configuration.Pinnum = 28; //pin28
	pin_configuration.Pinmode = 0; //pull-up (no importa porque va a ser configurado como salida)
	pin_configuration.Funcnum = 3; //funcion de EXTMATCH0.0
	pin_configuration.OpenDrain = 0; //sin open drain

	//struct_config.PrescaleOption =  TIM_PRESCALE_USVAL;
	struct_config.PrescaleOption =  TIM_PRESCALE_TICKVAL;
	//struct_config.PrescaleValue = 500; //cantidad de microsegundos para incrementar el TC
	struct_config.PrescaleValue = 1;  // cantidad de tick

	struct_match.MatchChannel = 0; //cual de los 4 registros match usar para compararse
	struct_match.IntOnMatch = ENABLE;
	struct_match.ResetOnMatch = ENABLE;
	struct_match.StopOnMatch = DISABLE;
	struct_match.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	struct_match.MatchValue = 12499999;
	//struct_match.MatchValue = 1000;

	PINSEL_ConfigPin(&pin_configuration);
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);
	TIM_Cmd(LPC_TIM0, ENABLE); //habilita timer counter y prescaler counter
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void configEINT0(){
	PINSEL_CFG_Type pin_configuration;
	pin_configuration.Portnum = 2; //puerto 2
	pin_configuration.Pinnum = 10; //pin10
	pin_configuration.Pinmode = 0; //pull-up (no importa porque va a ser configurado como salida)
	pin_configuration.Funcnum = 1; // funcion de EINT0
	pin_configuration.OpenDrain = 0; // sin open drain
	PINSEL_ConfigPin(&pin_configuration);

	EXTI_SetMode(0, 1); // EINT 0 por flanco
	EXTI_SetPolarity(0, 0); //EINT0 polaridad flanco de bajada
	NVIC_EnableIRQ(EINT0_IRQn); //habilita vector de ISR de EINT0
	NVIC_SetPriority(EINT0_IRQn, 30); //asigna prioridad de EINT0
}

void TIMER0_IRQHandler(){
	if(TIM_GetIntStatus(LPC_TIM0, 0) ){ //pregunto por el flag de interrupción del timer0, canal 0, es decir por TC = MAT0.0
        if(GPIO_ReadValue(2) & PIN_4){
        	GPIO_ClearValue(2, PIN_4);
        }else{
        	GPIO_SetValue(2, PIN_4);
        }
        TIM_ClearIntPending(LPC_TIM0, 0); //limpia flag de interrupcion del timer0 por match del canal 0
	}
}
void EINT0_IRQHandler(){
	pclktimer0 ++;
	if(pclktimer0 == 4){pclktimer0 = 0;}
	CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER0, pclktimer0);
	EXTI_ClearEXTIFlag(0);
}

