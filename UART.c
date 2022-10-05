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

//#define STICK_PORT PINSEL_PORT_1 //pin analogico para movimiento en Y
//#define PIN_STICK_SWITCH ((uint32_t) (1<<4)) //pin digital para pulsador del stick
//#define PIN_STICK_X ((uint32_t) (1<<30))//pin analogico para movimiento en X (AD0.4)
//#define PIN_STICK_Y ((uint32_t) (1<<31))//pin analogico para movimiento en Y (AD0.5)




void configGPIO();
void configADC();
//void configTiemer0(); //para trabajar con una frec de muestreo del ADC distinta de la frec maxima de 200KHz (10KHz)
//void configUART();
void ADC_IRQHandler();

int main(){

	configGPIO();
	configUART();
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
		GPIO_SetDir(2, i, 1); //salidas
		//LPC_GPIO2->FIODIR |= 1<<i;
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
void configUART(){

	PINSEL_CFG_Type TX_PIN;
	TX_PIN.Portnum = 0; //port 0
	TX_PIN.Pinnum = 2;// pin 2
	TX_PIN.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	TX_PIN.Funcnum = PINSEL_FUNC_1; //TXD0 function
	TX_PIN.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&TX_PIN);

	UART_CFG_Type uart0;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

//	uart1.Baud_rate = 9600;
//	uart1.Databits = UART_DATABIT_8;
//	uart1.Parity = UART_PARITY_NONE;
//	uart1.Stopbits = UART_STOPBIT_1;

	UART_ConfigStructInit(&uart0); //lo mismo que arriba, pero se hace automaticamente (config por defecto)
	UART_Init(LPC_UART0, &uart0); //inicializa periferico

	//UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	//UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct); //inicializa FIFO

	UART_TxCmd(LPC_UART0, ENABLE); //habilita transmision
}

void ADC_IRQHandler(){
//envio de datos por UART
//logica de control de servomotores
	__IO uint16_t adc_val = 0;
	__IO uint8_t valL = 0;
	__IO uint8_t valH = 0;

	if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){ //canal 4-> mov en x

		adc_val = ADC_ChannelGetData(LPC_ADC, 4);

		valL = adc_val & 0xFF;
		valH = (adc_val>>8) & 0xFF;
		UART_SendByte(LPC_UART0, valL);
		UART_SendByte(LPC_UART0, valH);

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

		valL = adc_val & 0xFF;
		valH = (adc_val>>8) & 0x00FF;
		UART_SendByte(LPC_UART0, valL);
		UART_SendByte(LPC_UART0, valH);

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







