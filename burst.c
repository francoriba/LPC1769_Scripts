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

#define ADC_avg	100

__IO uint16_t adc_val1 = 0;
__IO uint16_t adc_val2 = 0;
__IO uint8_t valL = 0;
__IO uint8_t valH = 0;
__IO uint32_t mapped_val;

uint8_t i = 0;
uint32_t previous4 = 0;
uint32_t previous5 = 0;



void configGPIO();
void configADC();
void configUART();
void configPWM();
int map(uint16_t value_to_map, uint32_t originalStart, uint32_t originalEnd, uint32_t newStart, uint32_t  newEnd);
void Servo_Write(uint8_t servoID, uint32_t value);//usa el valor para manipular la PWM asociada a la ID del servo
void ADC_IRQHandler();

int main(){
	SystemInit();
	configGPIO();
	configUART();
	configPWM();
	configADC();


	uint32_t clock = SystemCoreClock;
	while(1){
	}

	return 0;
}

void configGPIO(){
	PINSEL_CFG_Type leds;
	leds.Portnum = 2;
	leds.Funcnum = PINSEL_FUNC_0;
	leds.OpenDrain = PINSEL_PINMODE_NORMAL;

	for(uint8_t i = 6; i<10; i++){
		leds.Pinnum = i;
		PINSEL_ConfigPin(&leds);
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

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_ADC, 3); //ADC PCLK = CCLK/8
	uint32_t aver = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_ADC);

	ADC_Init(LPC_ADC, 1000); //ajusta los bits CLKDIV para lograr la frecuencia de muestreo de 10k

	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE); //configura interrupcion por canal 4
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE); //configura interrupcion por canal 5
	ADC_ChannelCmd(LPC_ADC, 4, ENABLE); //habilita canal 4
	ADC_ChannelCmd(LPC_ADC, 5, ENABLE); //habilita canal 5
	ADC_BurstCmd(LPC_ADC, 1); //1: Set Burst mode //si no se usa en modo burst usar ADC_StartCmd() en main
	NVIC_EnableIRQ(ADC_IRQn);
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
	//UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	UART_ConfigStructInit(&uart0); //lo mismo que arriba, pero se hace automaticamente (config por defecto)
	UART_Init(LPC_UART0, &uart0); //inicializa periferico

	//UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	//UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct); //inicializa FIFO

	UART_TxCmd(LPC_UART0, ENABLE); //habilita transmision
}

void configPWM(){
	PINSEL_CFG_Type PWM_pins_config;
	PWM_pins_config.Portnum = 2;
	PWM_pins_config.Pinmode = PINSEL_PINMODE_TRISTATE;
	PWM_pins_config.Funcnum  = 1;
	PWM_pins_config.OpenDrain = PINSEL_PINMODE_NORMAL;
	for(uint8_t i = 0; i< 6;i++){
		PWM_pins_config.Pinnum = i;
		PINSEL_ConfigPin(&PWM_pins_config);
	}

	/* PWM PCLK = CCLK/1 = 100MHz by default*/
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_1);
	//uint32_t checkfrec = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_PWM1);
	LPC_PWM1->PCR = 0x0; //Select Single Edge PWM - by default its single Edged so this line can be removed
	LPC_PWM1->PR = 99;

	LPC_PWM1->MR0 = 20000; // 20ms period duration

	/*Defines the arm initial position*/
	LPC_PWM1->MR1 = 999; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR2 = 999; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR3 = 999; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR4 = 999; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR5 = 999; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR6 = 999; //1ms - default pulse duration - servo at 0 degrees

	LPC_PWM1->MCR = (1<<1); //Reset PWM TC on PWM1MR0 match
	LPC_PWM1->LER = (1<<1) | (1<<0)|(1<<2); //update values in MR0 and MR1
	LPC_PWM1->PCR = (1<<9) | (1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14); //enable PWM outputs
	LPC_PWM1->TCR = 3; //Reset PWM TC & PR
	LPC_PWM1->TCR &= ~(1<<1); //libera la cuenta
	LPC_PWM1->TCR |= (1<<3); //enable counters and PWM Mode
}

int map(uint16_t value_to_map, uint32_t originalStart, uint32_t originalEnd, uint32_t newStart, uint32_t  newEnd ){
	int val =((newEnd-newStart)*value_to_map)/(originalEnd-originalStart);// 180*value/4096
	return val;
}

void Servo_Write(uint8_t servoID, uint32_t value){

	float tmp = 999 + (5.555 * value); //140
	uint32_t MRx_val = tmp;

	switch (servoID){
		case 0:
			LPC_PWM1->MR1 = MRx_val;
			LPC_PWM1->LER = (1<<1); //Load the MR1 new value at start of next cycle
			break;
		case 1:
			LPC_PWM1->MR2 = MRx_val;
			LPC_PWM1->LER = (1<<2); //Load the MR2 new value at start of next cycle
			break;
		case 2:
			LPC_PWM1->MR3 = MRx_val;
			LPC_PWM1->LER = (1<<2); //Load the MR3 new value at start of next cycle
			break;
		case 3:
			LPC_PWM1->MR4 = MRx_val;
			LPC_PWM1->LER = (1<<2); //Load the MR4 new value at start of next cycle
			break;
		case 4:
			LPC_PWM1->MR5 = MRx_val;
			LPC_PWM1->LER = (1<<2); //Load the MR5 new value at start of next cycle
			break;
		case 5:
			LPC_PWM1->MR6 = MRx_val;
			LPC_PWM1->LER = (1<<2); //Load the MR6 new value at start of next cycle
			break;
		default:
			break;
	}
}

void ADC_IRQHandler(){
//logica de control de servomotores
	if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){ //canal 4-> mov en x

		adc_val1 = ADC_ChannelGetData(LPC_ADC, 4);
		//adc_val1 = (previous4 * (ADC_avg-1) + adc_val1)/ ADC_avg;
		//previous4 = adc_val1;

		valL = adc_val1 & 0xFF;
		valH = (adc_val1 >> 8) & 0xFF;
		UART_SendByte(LPC_UART0, valL);
		UART_SendByte(LPC_UART0, valH);

		mapped_val = map(adc_val1, 0, 4095, 0, 180);
		Servo_Write(0 , mapped_val);

		if(adc_val1 >= 2500){
			LPC_GPIO2->FIOSET |= (1<<9);
			LPC_GPIO2->FIOCLR |= (1<<8);
			LPC_GPIO2->FIOCLR |= (1<<7);
			LPC_GPIO2->FIOCLR |= (1<<6);
		}
		else if (adc_val1 <= 1500){
			LPC_GPIO2->FIOSET |= (1<<8);
			LPC_GPIO2->FIOCLR |= (1<<9);
			LPC_GPIO2->FIOCLR |= (1<<7);
			LPC_GPIO2->FIOCLR |= (1<<6);
		}
		else{
			LPC_GPIO2->FIOCLR |= (1<<2);
			LPC_GPIO2->FIOCLR |= (1<<3);
			LPC_GPIO2->FIOCLR |= (1<<1);
			LPC_GPIO2->FIOCLR |= (1<<0);
		}
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 5, ADC_DATA_DONE)){//canal 5-> mov en y

		adc_val2 = ADC_ChannelGetData(LPC_ADC, 5);
		//adc_val2 = (previous5 * (ADC_avg-1) + adc_val2)/ ADC_avg;
		//previous5 = adc_val2;

		valL = adc_val2 & 0xFF;
		valH = (adc_val2>>8) & 0x00FF;
		UART_SendByte(LPC_UART0, valL);
		UART_SendByte(LPC_UART0, valH);

		mapped_val = map(adc_val2, 0, 4095, 0, 180);

		Servo_Write(1 , mapped_val);

		if(adc_val2 >= 2500){
			LPC_GPIO2->FIOSET |= (1<<7);
			LPC_GPIO2->FIOCLR |= (1<<6);
			LPC_GPIO2->FIOCLR |= (1<<8);
			LPC_GPIO2->FIOCLR |= (1<<9);
		}
		else if (adc_val2 <= 1500){
			LPC_GPIO2->FIOSET |= (1<<6);
			LPC_GPIO2->FIOCLR |= (1<<7);
			LPC_GPIO2->FIOCLR |= (1<<8);
			LPC_GPIO2->FIOCLR |= (1<<9);
		}
		else{
			LPC_GPIO2->FIOCLR |= (1<<9);
			LPC_GPIO2->FIOCLR |= (1<<8);
			LPC_GPIO2->FIOCLR |= (1<<7);
			LPC_GPIO2->FIOCLR |= (1<<6);
		}
	}
}












