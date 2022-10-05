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

void configADC();
void configTiemer0(); //para trabajar con una frec de muestreo del ADC distinta de la frec maxima de 200KHz (10KHz)
void ADC_IRQHandler();
void map();

int main(){

	configADC();
	configTiemer();

	while(1){

	}

	return 0;
}

void configADC(){


	PINSEL_CFG_Type adc_signal;
	adc_signal.Portnum = STICK_PORT; //port 1
	adc_signal.Pinnum = PIN_STICK_X; //pin 30
	adc_signal.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	adc_signal.Funcnum = PINSEL_FUNC_3; //AD0.4 function
	adc_signal.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&adc_signal);

	LPC_SC->PCONP |= (1<<12) //enable peripheral adc
	LPC_SC->PCLKSEL0 |= (3<<24); //CCLK/8
	LPC_ADC->ADCR |= (1<<0); //enable sampling for ch1
	LPC_ADC->ADCR |= (255<<8); //ADC frecuency division of 1, this archives a working frequency of 12.5MHz for the ADC
	//LPC_ADC->ADCR |= (1<<16); //burst mode, each sample needs 64 cycles, so the sampling freq is 12.5Mhz/64 = 195312.5 samples/sec
	LPC_ADC->ADCR  &= ~(1<<16); //no burst, 12.5Mhz/65 = 192307.7 samples/sec, if we use 2 channels, 96153.85 samples/sec
	LPC_ADC->ADCR  |= (1<<21); //ADC converter is operational
	LPC_ADC->ADCR  |= (1<<26); //START = 100 -> start convertion when the edge selected occurs on MAT0.1
	LPC_ADC->ADCR  &= ~(0b11<<24);
	LPC_ADC->ADCR  |= (1<<27); //falling edge on match signal
	LPC_ADC->ADINTEN |= (0b11<<4); //interrupt on adc convertion compleiton for channels 4 and 5
	NVIC_EnableIRQ(ADC_IRQn);
}
void configTiemer0(){
	TIM_TIMERCFG_Type struct_config;//prescaler config
	TIM_MATCHCFG_Type struct_match; //match config

	struct_config.PrescaleOption =  TIM_PRESCALE_TICKVAL;
	struct_config.PrescaleValue = 1;

	struct_match.MatchChannel = 1;
	struct_match.IntOnMatch = ENABLE;
	struct_match.ResetOnMatch = ENABLE;
	struct_match.StopOnMatch = DISABLE;
	struct_match.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	struct_match.MatchValue = 9999;
	//struct_match.MatchValue = 1000;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config);
	TIM_ConfigMatch(LPC_TIM0, &struct_match);
	TIM_Cmd(LPC_TIM0, ENABLE); //habilita timer counter y prescaler counter
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void ADC_IRQHandler(){

	static uint16_t ADC0Value = 0;
	ADC0Value = ((LPC_ADC->ADDR0)>>4)& 0xFFF; //variable auxiliar para observar el valor del registro
	if(ADC0Value<2054){
		LPC_FIOSET = (1<<22);
	}
	else{
		LPC_GPIO->FIOCLR = (1<<22);
	}

}







