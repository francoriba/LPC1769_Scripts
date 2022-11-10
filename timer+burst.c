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
#define PWM_dc_MAX	2500
#define PWM_dc_MIN	500
#define PWM_period 20000


__IO uint16_t adc_val1 = 0, adc_val2 = 0, adc_val3 = 0, adc_val4 = 0, adc_val5 = 0, adc_val6 = 0;
__IO uint32_t mapped_val;




void configGPIO();
void configADC();
void configUART();
void configPWM();
int map(int x, int in_min, int in_max, int out_min, int out_max);
void Servo_Write(uint8_t servoID, uint32_t phi, uint32_t lowLimit, uint32_t highLimit);//usa el valor para manipular la PWM asociada a la ID del servo
void ADC_IRQHandler();

int main(){
    SystemInit();
	uint32_t clock = SystemCoreClock;
	configGPIO();
	configUART();
	configPWM();
	configADC();
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
	/* ----------------------ANALOG PINS CONFIGURATION--------------------------------*/
	/* -------------------------------------------------------------------------------*/
	PINSEL_CFG_Type analog_pins_cfg_struct; //struct for analog pins configuration
	/* Configure pins P0.23-P0.26 as AD0.0-AD0.3*/
	analog_pins_cfg_struct.Portnum = 0;
	analog_pins_cfg_struct.Pinnum = 23;
	analog_pins_cfg_struct.Funcnum = PINSEL_FUNC_1;
	analog_pins_cfg_struct.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	analog_pins_cfg_struct.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&analog_pins_cfg_struct);
	analog_pins_cfg_struct.Pinnum = 24;
	PINSEL_ConfigPin(&analog_pins_cfg_struct);
	analog_pins_cfg_struct.Pinnum = 25;
	PINSEL_ConfigPin(&analog_pins_cfg_struct);
	analog_pins_cfg_struct.Pinnum = 26;
	PINSEL_ConfigPin(&analog_pins_cfg_struct);

	/* Configure pins P1.30-P1.31 as AD0.4-AD0.5 */
	analog_pins_cfg_struct.Portnum = 1;
	analog_pins_cfg_struct.Pinnum = 30;
	analog_pins_cfg_struct.Funcnum = PINSEL_FUNC_3;
	PINSEL_ConfigPin(&analog_pins_cfg_struct);
	analog_pins_cfg_struct.Pinnum = 31;
	analog_pins_cfg_struct.Funcnum = PINSEL_FUNC_3;

	/* -----------------------------ADC CONFIGURATION---------------------------------*/
	/* -------------------------------------------------------------------------------*/
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_ADC, 3); //ADC PCLK = CCLK/8
	uint32_t aver = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_ADC);

	ADC_Init(LPC_ADC, 120); //ajusta los bits CLKDIV para lograr la frecuencia de muestreo de 8k

	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE); //configura interrupcion por canal 0
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE); //configura interrupcion por canal 1
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE); //configura interrupcion por canal 2
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN3, ENABLE); //configura interrupcion por canal 3
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE); //configura interrupcion por canal 4
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE); //configura interrupcion por canal 5

	ADC_ChannelCmd(LPC_ADC, 0, ENABLE); //habilita canal 0
	ADC_ChannelCmd(LPC_ADC, 1, ENABLE); //habilita canal 1
	ADC_ChannelCmd(LPC_ADC, 2, ENABLE); //habilita canal 2
	ADC_ChannelCmd(LPC_ADC, 3, ENABLE); //habilita canal 3
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
	LPC_PWM1->PR = 9; //99 si el PLL es de 100M

	LPC_PWM1->MR0 = PWM_period; // 20ms period duration 19999

	/*Defines the arm initial position*/
	LPC_PWM1->MR1 = PWM_dc_MIN; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR2 = PWM_dc_MIN; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR3 = PWM_dc_MIN; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR4 = PWM_dc_MIN; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR5 = PWM_dc_MIN; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR6 = PWM_dc_MIN; //1ms - default pulse duration - servo at 0 degrees

	LPC_PWM1->MCR = (1<<1); //Reset PWM TC on PWM1MR0 match
	LPC_PWM1->LER = (1<<1) | (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6); //update values in MR0 and MR1
	LPC_PWM1->PCR = (1<<9) | (1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14); //enable PWM outputs
	LPC_PWM1->TCR = 3; //Reset PWM TC & PR
	LPC_PWM1->TCR &= ~(1<<1); //libera la cuenta
	LPC_PWM1->TCR |= (1<<3); //enable counters and PWM Mode
}

int map(int x, int in_min, int in_max, int out_min, int out_max){
	return ((out_max-out_min)*x)/(in_max-in_min) + out_min;
}

void Servo_Write(uint8_t servoID, uint32_t phi, uint32_t lowLimit, uint32_t highLimit){
	float k = (highLimit - lowLimit)/180;	// 2500 - 500 = 2000, 2000/180 = 11.11
	float MRx_val = PWM_dc_MIN + (k * phi);

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
			LPC_PWM1->LER = (1<<3); //Load the MR3 new value at start of next cycle
			break;
		case 3:
			LPC_PWM1->MR4 = MRx_val;
			LPC_PWM1->LER = (1<<4); //Load the MR4 new value at start of next cycle
			break;
		case 4:
			LPC_PWM1->MR5 = MRx_val;
			LPC_PWM1->LER = (1<<5); //Load the MR5 new value at start of next cycle
			break;
		case 5:
			LPC_PWM1->MR6 = MRx_val;
			LPC_PWM1->LER = (1<<6); //Load the MR6 new value at start of next cycle
			break;
		default:
			break;
	}
}

void ADC_IRQHandler(){
//logica de control de servomotores

	if(ADC_ChannelGetStatus(LPC_ADC, 0, ADC_DATA_DONE)){ //canal 4-> mov en x
		adc_val1 = ADC_ChannelGetData(LPC_ADC, 1);
		mapped_val = map(adc_val1, 0, 4095, 0, 180);
		Servo_Write(0 , mapped_val, 500, 2500);
		UART_SendByte(LPC_UART0, mapped_val);
	}
	if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){ //canal 4-> mov en x

		adc_val4 = ADC_ChannelGetData(LPC_ADC, 4);
		mapped_val = map(adc_val4, 0, 4095, 0, 180);
		Servo_Write(4 , mapped_val, 500, 2500);
		UART_SendByte(LPC_UART0, mapped_val);
	}
	if(ADC_ChannelGetStatus(LPC_ADC, 5, ADC_DATA_DONE)){//canal 5-> mov en y

		adc_val5 = ADC_ChannelGetData(LPC_ADC, 5);
		mapped_val = map(adc_val5, 5, 4095, 0, 180);
		Servo_Write(5 , mapped_val, 500, 2500);
		UART_SendByte(LPC_UART0, mapped_val);
	}
}












