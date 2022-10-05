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

//#define STICK_PORT PINSEL_PORT_1 //pin analogico para movimiento en Y
//#define PIN_STICK_SWITCH ((uint32_t) (1<<4)) //pin digital para pulsador del stick
//#define PIN_STICK_X ((uint32_t) (1<<30))//pin analogico para movimiento en X (AD0.4)
//#define PIN_STICK_Y ((uint32_t) (1<<31))//pin analogico para movimiento en Y (AD0.5)

__IO uint16_t adc_val = 0;

void configGPIO();
void configADC();
//void configTiemer0(); //para trabajar con una frec de muestreo del ADC distinta de la frec maxima de 200KHz (10KHz)
//void configUART();
void ADC_IRQHandler();

int main(){

	configGPIO();
	configADC();

	while(1){
	}

	return 0;
}

void configGPIO(){
	PINSEL_CFG_Type leds[4];

	for(uint8_t i = 0; i<4; i++){
		leds[i].Portnum = 2;
		leds[i].Pinnum = i;
		leds[i].Funcnum = PINSEL_FUNC_0;
		leds[i].OpenDrain = PINSEL_PINMODE_NORMAL;
		PINSEL_ConfigPin(&leds[i]);
		//GPIO_SetDir(2, i, 1); //salidas
		LPC_GPIO2->FIODIR |= 1<<i;
		LPC_GPIO2->FIOCLR = (1<<i);
	}

}

void configADC(){

	PINSEL_CFG_Type pin_stick_x_configuration; //config AD0.4
	PINSEL_CFG_Type pin_stick_y_configuration; //config AD0.5
	pin_stick_x_configuration.Portnum = 1; //port 1
	pin_stick_x_configuration.Pinnum = 30;//PIN_STICK_X; //pin 30
	pin_stick_x_configuration.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	pin_stick_x_configuration.Funcnum = PINSEL_FUNC_3; //AD0.4 function
	pin_stick_x_configuration.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	pin_stick_y_configuration.Portnum = 1; //port 1
	pin_stick_y_configuration.Pinnum = 31; //pin 31
	pin_stick_y_configuration.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	pin_stick_y_configuration.Funcnum = PINSEL_FUNC_3; //AD0.5 function
	pin_stick_y_configuration.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&pin_stick_x_configuration);
	PINSEL_ConfigPin(&pin_stick_y_configuration);

	ADC_Init(LPC_ADC, 10000); //ajusta los bits CLKDIV para lograr la frecuencia de muestreo de 10k
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE); //configura interrupcion por canal 4
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE); //configura interrupcion por canal 5
	ADC_ChannelCmd(LPC_ADC, 4, ENABLE); //habilita canal 4
	ADC_ChannelCmd(LPC_ADC, 5, ENABLE); //habilita canal 5
	ADC_BurstCmd(LPC_ADC, 1); //1: Set Burst mode //si no se usa en modo burst usar ADC_StartCmd() en main
	NVIC_EnableIRQ(ADC_IRQn);

	/*
	LPC_SC->PCONP |= (1<<12); //enable peripheral adc
	LPC_SC->PCLKSEL0 |= (3<<24); //CCLK/8
	LPC_ADC->ADCR |= (0b11<<4); //enable sampling for ch4 and ch5
	LPC_ADC->ADCR |= (255<<8); //ADC frecuency division of 1, this archives a working frequency of 12.5MHz for the ADC
	//LPC_ADC->ADCR |= (1<<16); //burst mode, each sample needs 64 cycles, so the sampling freq is 12.5Mhz/64 = 195312.5 samples/sec
	LPC_ADC->ADCR  &= ~(1<<16); //no burst, 12.5Mhz/65 = 192307.7 sampples/sec, if we use 2 channels, 96153.85 samples/sec
	LPC_ADC->ADCR  |= (1<<21); //ADC converter is operational
	LPC_ADC->ADCR  |= (1<<26); //START = 100 -> start convertion when the edge selected occurs on MAT0.1
	LPC_ADC->ADCR  &= ~(0b11<<24);
	LPC_ADC->ADCR  |= (1<<27); //falling edge on match signal
	LPC_ADC->ADINTEN |= (0b11<<4); //interrupt on adc convertion compleiton for channels 4 and 5
	 */
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

//envio de datos por UART
//logica de control de servomotores


	if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){ //canal 4-> mov en x
		adc_val = ADC_ChannelGetData(LPC_ADC, 4);
		//printf("%d", adc_val); //para que funcione recordar cambiar redlib(semihost) en
		//Project > Properties > C/C++ Build > Settings > Tool Settings > MCU Linker > Managed Linker script > Library
		if(adc_val>=2500){
			LPC_GPIO2->FIOSET |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<1);
			LPC_GPIO2->FIOCLR |= (1<<0);
		}
		else if (adc_val<=1500){
			LPC_GPIO2->FIOSET |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<1);
			LPC_GPIO2->FIOCLR |= (1<<0);
		}
		else{
			LPC_GPIO2->FIOCLR |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<1);
			LPC_GPIO2->FIOCLR |= (1<<0);
		}
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 5, ADC_DATA_DONE)){//canal 5-> mov en y
		adc_val = ADC_ChannelGetData(LPC_ADC, 5);
				//printf("%d", adc_val); //para que funcione recordar cambiar redlib(semihost) en
		//Project > Properties > C/C++ Build > Settings > Tool Settings > MCU Linker > Managed Linker script > Library
		if(adc_val>=2500){
			LPC_GPIO2->FIOSET |= (1<<1);
			LPC_GPIO2->FIOCLR |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<0);
		}
		else if (adc_val<1500){
			LPC_GPIO2->FIOSET |= (1<<0);
			LPC_GPIO2->FIOCLR |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<1);
		}
		else{
			LPC_GPIO2->FIOCLR |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<1);
			LPC_GPIO2->FIOCLR |= (1<<0);
		}
	}
}

//void configUART(){
//
//}








