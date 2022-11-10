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

#define PWM_dc_MAX	2500 // value of timer ticks for maximum servo displacement -> 2,5 ms
#define PWM_dc_MIN	500 // value of timer ticks for minimum servo displacement -> 0.5 ms
#define PWM_period 20000 //value of timer ticks for generation of 20 ms period for PWM signal

#define STEPPER_STEPS 100 // NEMA17 17HS4401S has a step angle of 1.8°

__IO uint16_t adc_val0 = 0, adc_val1 = 0, adc_val2 = 0, adc_val3 = 0, adc_val4 = 0, adc_val5 = 0, adc_val6 = 0;
__IO uint16_t phi0 = 0, phi1 = 0, phi2 = 0, phi3 = 0, phi4 = 0, phi5 = 0, phi6 = 0;
uint8_t accuracy_mode = 0;

void configGPIO();
void configADC();
void configUART();
void configPWM();
void configEINT();
int map(int x, int in_min, int in_max, int out_min, int out_max);
void Servo_Write(uint8_t servoID, uint32_t phi, uint32_t lowLimit, uint32_t highLimit);//usa el valor para manipular la PWM asociada a la ID del servo
/* IRS Handlers*/
void EINT0_IRQHandler();
void ADC_IRQHandler();

int main(){

	SystemInit();
	configGPIO();
	//configEINT();
	configUART();
	configPWM();
	configADC();
	uint32_t clock = SystemCoreClock; //check clock frequency while debugging

	while(1){

	}

	return 0;
}

void configGPIO(){
	/* LEDS */
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
	/* Step pin and Dir pin for controlling Stepper motor */
	PINSEL_CFG_Type stepper_steps_and_dir_pins;
	stepper_steps_and_dir_pins.Portnum = 0;
	stepper_steps_and_dir_pins.Pinnum = 4;
	stepper_steps_and_dir_pins.Pinmode = PINSEL_PINMODE_PULLUP;
	stepper_steps_and_dir_pins.Funcnum = PINSEL_FUNC_0;
	stepper_steps_and_dir_pins.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&stepper_steps_and_dir_pins); // P0.4 -> Step pin for stepper motor
	stepper_steps_and_dir_pins.Pinnum = 5;
	PINSEL_ConfigPin(&stepper_steps_and_dir_pins); // P0.5-> Direction pin for stepper motor
	LPC_GPIO0->FIODIR |= 0b11<<4; // P0.4 and P0.5 are outputs
}

void configADC(){
	/* --------------------ANALOG INPUT PINS CONFIGURATION----------------------------*/
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
	PINSEL_ConfigPin(&analog_pins_cfg_struct);
	/* Configure pin P0.3 as AD0.6 */
	analog_pins_cfg_struct.Portnum = 0;
	analog_pins_cfg_struct.Pinnum = 3;
	analog_pins_cfg_struct.Funcnum = PINSEL_FUNC_2;
	PINSEL_ConfigPin(&analog_pins_cfg_struct);


	/* -----------------------------ADC CONFIGURATION---------------------------------*/
	/* -------------------------------------------------------------------------------*/
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_ADC, 3); //ADC PCLK = CCLK/8
	uint32_t aver = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_ADC);

	ADC_Init(LPC_ADC, 120); //adjust CLKDIV bits of AD0R to achieve this sample rate 120

	/* enable channel 0 to channel 5 interrupts */
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN3, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN6, ENABLE);

	/* enable channel 0 to channel 5*/
	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 3, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 4, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 5, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 6, ENABLE);


	ADC_BurstCmd(LPC_ADC, 1); // Set Burst mode

	NVIC_EnableIRQ(ADC_IRQn); // Enable interrupt vector for ADC
}

void configUART(){
	/* ---------------------------UART TX PIN CONFIGURATION---------------------------*/
	/* -------------------------------------------------------------------------------*/
	PINSEL_CFG_Type TX_PIN;
	TX_PIN.Portnum = 0; //port 0
	TX_PIN.Pinnum = 2;// pin 2
	TX_PIN.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	TX_PIN.Funcnum = PINSEL_FUNC_1; //TXD0 function
	TX_PIN.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&TX_PIN);

	/* -------------------------------UART CONFIGURATION------------------------------*/
	/* -------------------------------------------------------------------------------*/
	UART_CFG_Type uart0;
	//UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	UART_ConfigStructInit(&uart0); //lo mismo que arriba, pero se hace automaticamente (config por defecto)
	UART_Init(LPC_UART0, &uart0); //inicializa periferico
	//UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	//UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct); //inicializa FIFO
	UART_TxCmd(LPC_UART0, ENABLE); //habilita transmision
}

void configPWM(){
	/* ----------------------------PWM PINS CONFIGURATION-----------------------------*/
	/* -------------------------------------------------------------------------------*/
	PINSEL_CFG_Type PWM_pins_config;
	PWM_pins_config.Portnum = 2;
	PWM_pins_config.Pinmode = PINSEL_PINMODE_TRISTATE;
	PWM_pins_config.Funcnum  = 1;
	PWM_pins_config.OpenDrain = PINSEL_PINMODE_NORMAL;
	for(uint8_t i = 0; i< 6;i++){
		PWM_pins_config.Pinnum = i;
		PINSEL_ConfigPin(&PWM_pins_config);
	}

	/* --------------------------------PWM CONFIGURATION------------------------------*/
	/* -------------------------------------------------------------------------------*/
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_1);
	//uint32_t checkfrec = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_PWM1);
	LPC_PWM1->PCR = 0x0; //Select Single Edge PWM - by default its single Edged so this line can be removed
	LPC_PWM1->PR = 9; //99 si el PLL es de 100M
	LPC_PWM1->MR0 = PWM_period;

	/*Defines the arm initial position*/
	LPC_PWM1->MR1 = 1000; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR2 = 1000; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR3 = 1000; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR4 = 1000; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR5 = 1000; //1ms - default pulse duration - servo at 0 degrees
	LPC_PWM1->MR6 = 1000; //1ms - default pulse duration - servo at 0 degrees

	LPC_PWM1->MCR = (1<<1); //Reset PWM TC on PWM1MR0 match
	LPC_PWM1->LER = (1<<1) | (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6); //update values in MR0 and MR1
	LPC_PWM1->PCR = (1<<9) | (1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14); //enable PWM outputs
	LPC_PWM1->TCR = 3; //Reset PWM TC & PR
	LPC_PWM1->TCR &= ~(1<<1); //libera la cuenta
	LPC_PWM1->TCR |= (1<<3); //enable counters and PWM Mode
}

void configEINT(){
	/* P2.10 will function as External interrupt EINT0 */
	PINSEL_CFG_Type extint0_pin;
	extint0_pin.Portnum = 2;
	extint0_pin.Pinnum = 10;
	extint0_pin.Pinmode = PINSEL_PINMODE_PULLUP;
	extint0_pin.Funcnum = 1;
	extint0_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&extint0_pin);

	EXTI_SetMode(EXTI_EINT0, EXTI_MODE_EDGE_SENSITIVE);
	EXTI_SetPolarity(EXTI_EINT0, EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE);
	NVIC_EnableIRQ(EINT0_IRQn);
	NVIC_SetPriority(EINT0_IRQn, 0);
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

void EINT0_IRQHandler(){
	accuracy_mode ^= 1<<0; //toggle this flag state
	EXTI_ClearEXTIFlag(EXTI_EINT0);
}

void ADC_IRQHandler(){
	/* -----------------------------SERVOS CONTROL LOGIC------------------------------*/
	/* -------------------------------------------------------------------------------*/
	if(ADC_ChannelGetStatus(LPC_ADC, 0, ADC_DATA_DONE)){ //channel 0 -> used for stepper

		uint32_t i;
		adc_val0 = ADC_ChannelGetData(LPC_ADC, 0);
		if (adc_val0 < 1500){
			LPC_GPIO0->FIOCLR |= (1<<5); //set direction of rotation on P0.5

			LPC_GPIO0->FIOSET |= (1<<4);
			//for(i = 0; i<100; i++);
			LPC_GPIO0->FIOCLR |= (1<<4);
			//for(i = 0; i<100; i++);

			//adc_val0 = ADC_ChannelGetData(LPC_ADC, 0);
			//phi0 = map(adc_val0, 0, 4095, 0, 180);
			//UART_SendByte(LPC_UART0, phi0);
		}
		else if (adc_val0 > 2500){
			LPC_GPIO0->FIOSET |= (1<<5); //set direction of rotation on P0.5

			LPC_GPIO0->FIOSET |= (1<<4);
			//for(i = 0; i<100; i++);
			LPC_GPIO0->FIOCLR |= (1<<4);
			//for(i = 0; i<100; i++);

			//adc_val0 = ADC_ChannelGetData(LPC_ADC, 0);
			//phi0 = map(adc_val0, 0, 4095, 0, 180);
			//UART_SendByte(LPC_UART0, phi0);
		}
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 1, ADC_DATA_DONE)){//channel 1 -> used for servo 1 - shouler joint 1 and 2

		adc_val1 = ADC_ChannelGetData(LPC_ADC, 1);

		if(accuracy_mode){ //check if accuracy mode is set
			if(adc_val1 > 3000){
				if (phi1 == 180){Servo_Write(0 , 180, 500, 2500);} //llego al tope maximo, permanece ahi
				else{Servo_Write(0 , phi1+1, 500, 2500);} //no llego al tope, incrementar un grado
			}
			else if (adc_val1<3000 && adc_val1 > 1000){ // no se esta moviendo el joystick, mantener en pos actual
				Servo_Write(0 , phi1, 500, 2500);}
			else{
				if(phi1 == 0){Servo_Write(0 , 0, 500, 2500);}  //llego al minimo, permanece ahi
				else{Servo_Write(0 , phi1-1, 500, 2500); }//decrementar un grado
			}
		}
		else{
			phi1 = map(adc_val1, 0, 4095, 0, 180);
			Servo_Write(0 , phi1, 500, 2500); //usamos el valor mapeado para el servo0 directamente
			//UART_SendByte(LPC_UART0, phi1); //mandomos el angulo 'original'

			//morroring
			if(phi1>90){ //por ej si phi1 era 100, el valor mirroreado va a ser 80 y si era 80 el valor mirroreado va a ser 100
				phi1 = 90 - (phi1-90);
			}
			else {
				phi1 = 90 + (90-phi1);
			}
			//usamos el valor mirroreado para el servo 1
			Servo_Write(1 , phi1, 500, 2500); // mirrored
			//UART_SendByte(LPC_UART0, phi1); //mandamos el angulo espjeado
		}
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 2, ADC_DATA_DONE)){//channel 2 -> used for servo 3 - elbow joint

		adc_val2 = ADC_ChannelGetData(LPC_ADC, 2);

		if(accuracy_mode){ //check if accuracy mode is set
			if(adc_val2 > 3000){
				if (phi2 == 180){Servo_Write(0 , 180, 500, 2500);} //llego al tope maximo, permanece ahi
				else{Servo_Write(0 , phi2+1, 500, 2500);} //no llego al tope, incrementar un grado
			}
			else if (adc_val2<3000 &&adc_val2 > 1000){ // no se esta moviendo el joystick, mantener en pos actual
				Servo_Write(0 , phi2, 500, 2500);}
			else{
				if(phi1 == 0){Servo_Write(0 , 0, 500, 2500);}  //llego al minimo, permanece ahi
				else{Servo_Write(0 , phi2-1, 500, 2500); }//decrementar un grado
			}
		}
		else{
			phi2 = map(adc_val2, 0, 4095, 0, 180);
			Servo_Write(2 , phi2, 500, 2500);
		}
		UART_SendByte(LPC_UART0, phi2);

	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 3, ADC_DATA_DONE)){//channel 3 -> used for servo 4 - wrist joint 1

		adc_val3 = ADC_ChannelGetData(LPC_ADC, 3);

		if(accuracy_mode){ //check if accuracy mode is set
			if(adc_val3 > 3000){
				if (phi3 == 180){Servo_Write(0 , 180, 500, 2500);} //llego al tope maximo, permanece ahi
				else{Servo_Write(0 , phi3+1, 500, 2500);} //no llego al tope, incrementar un grado
			}
			else if (adc_val3<3000 && adc_val3 > 1000){ // no se esta moviendo el joystick, mantener en pos actual
				Servo_Write(0 , phi3, 500, 2500);}
			else{
				if(phi3 == 0){Servo_Write(0 , 0, 500, 2500);}  //llego al minimo, permanece ahi
				else{Servo_Write(0 , phi3-1, 500, 2500); }//decrementar un grado
			}
		}
		else{
			phi3 = map(adc_val3, 0, 4095, 0, 180);
			Servo_Write(3 , phi3, 500, 2500);

		}
		UART_SendByte(LPC_UART0, phi3);
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){//channel 4 -> used for servo 5 - wrist joint 2

		adc_val4 = ADC_ChannelGetData(LPC_ADC, 4);

		if(accuracy_mode){ //check if accuracy mode is set
			if(adc_val4 > 3000){
				if (phi4 == 180){Servo_Write(0 , 180, 500, 2500);} //llego al tope maximo, permanece ahi
				else{Servo_Write(0 , phi4+1, 500, 2500);} //no llego al tope, incrementar un grado
			}
			else if (adc_val4<3000 && adc_val4 > 1000){ // no se esta moviendo el joystick, mantener en pos actual
				Servo_Write(0 , phi4, 500, 2500);}
			else{
				if(phi4 == 0){Servo_Write(0 , 0, 500, 2500);}  //llego al minimo, permanece ahi
				else{Servo_Write(0 , phi4-1, 500, 2500); }//decrementar un grado
			}
		}
		else{
			phi4 = map(adc_val4, 0, 4095, 0, 180);
			Servo_Write(4 , phi4, 500, 2500);
		}
		UART_SendByte(LPC_UART0, phi4);
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 5, ADC_DATA_DONE)){ //channel 5 -> used for servo 6 - gripper joint

		adc_val5 = ADC_ChannelGetData(LPC_ADC, 5);

		if(accuracy_mode){ //check if accuracy mode is set
			if(adc_val5 > 3000){
				if (phi5 == 180){Servo_Write(0 , 180, 500, 2500);} //llego al tope maximo, permanece ahi
				else{Servo_Write(0 , phi5+1, 500, 2500);} //no llego al tope, incrementar un grado
			}
			else if (adc_val5<3000 && adc_val5 > 1000){ // no se esta moviendo el joystick, mantener en pos actual
				Servo_Write(0 , phi5, 500, 2500);}
			else{
				if(phi5 == 0){Servo_Write(0 , 0, 500, 2500);}  //llego al minimo, permanece ahi
				else{Servo_Write(0 , phi5-1, 500, 2500); }//decrementar un grado
			}
		}
		else{
			phi5 = map(adc_val5, 0, 4095, 0, 180);
			Servo_Write(5 , phi5, 500, 2500);
		}
		UART_SendByte(LPC_UART0, phi5);
	}
}













