#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>
#include <stdio.h>
#include <math.h>
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pwm.h"

//#define STICK_PORT PINSEL_PORT_1 //pin analogico para movimiento en Y
//#define PIN_STICK_SWITCH ((uint32_t) (1<<4)) //pin digital para pulsador del stick
//#define PIN_STICK_X ((uint32_t) (1<<30))//pin analogico para movimiento en X (AD0.4)
//#define PIN_STICK_Y ((uint32_t) (1<<31))//pin analogico para movimiento en Y (AD0.5)

__IO uint16_t adc_val = 0;
__IO uint8_t valL = 0;
__IO uint8_t valH = 0;
__IO uint32_t mapped_val;

uint8_t i = 0;


void configGPIO();
void configADC();
void configUART();
void configTIMER0();
//void configI2C();
void configPWM();
int map(uint16_t value_to_map, uint32_t originalStart, uint32_t originalEnd, uint32_t newStart, uint32_t  newEnd);
void Servo_Write(uint8_t servoID, uint32_t value);//usa el valor para manipular la PWM asociada a la ID del servo
void ADC_IRQHandler();
void TIMER0_IRQHandler();

int main(){
	SystemInit();
	uint32_t clock = SystemCoreClock;
	configGPIO();
	configUART();
	configADC();
	configTIMER0();
	configPWM();

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

	// Por defecto el reloj de periferico del ADC tiene el valor CCLK/4 = 25 MHz pero system init lo configuro a 100MHz
	ADC_Init(LPC_ADC, 200000); //ajusta los bits CLKDIV para lograr la frecuencia de muestreo de 200KHz
//	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE); //configura interrupcion por canal 4
//	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE); //configura interrupcion por canal 5
//	ADC_ChannelCmd(LPC_ADC, 4, ENABLE); //habilita canal 4
//	ADC_ChannelCmd(LPC_ADC, 5, ENABLE); //habilita canal 5
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

void configTIMER0(){
	TIM_TIMERCFG_Type timerCFG;
	timerCFG.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timerCFG.PrescaleValue = 1;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&timerCFG); //setea a 25MHz el PCLK del timer haciendo CCLK/4

	TIM_MATCHCFG_Type matchCFG;
	matchCFG.MatchChannel = 1;
	matchCFG.IntOnMatch = ENABLE; // no hace falta que el timer interrumpa para generar toggleo en un mat
	matchCFG.ResetOnMatch = ENABLE;
	matchCFG.StopOnMatch = DISABLE;
	matchCFG.ExtMatchOutputType = TIM_EXTMATCH_NOTHING; //el mat0.1 no hace nada cada vez que desborda el timer
	matchCFG.MatchValue = 4999999; // Tint = 1/100MHz * (1) * (MAT + 1) = 0.05s (interrumpe y togglea a 20Hz)
	TIM_ConfigMatch(LPC_TIM0, &matchCFG); //configura el mat 0.1 del timer 0
	TIM_Cmd(LPC_TIM0, ENABLE); //habilita TC y PC
	TIM_ResetCounter(LPC_TIM0); //resetea TC y PC
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void configPWM(){
	PINSEL_CFG_Type PWM_PIN;
	PWM_PIN.Portnum = 2; //port 2
	PWM_PIN.Pinnum = 0;// pin 0
	PWM_PIN.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	PWM_PIN.Funcnum = PINSEL_FUNC_1; //PWM 1.1 function
	PWM_PIN.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&PWM_PIN);

	PWM_TIMERCFG_Type pwm_struct;
	pwm_struct.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
	pwm_struct.PrescaleValue = 1;
	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &pwm_struct); //setea a 25MHz el PCLK de pwm haciendo CCLK/4

	PWM_Cmd(LPC_PWM1, ENABLE); //habilita el periferico de PWM

	PWM_MATCHCFG_Type pwm_match0_struct;
	pwm_match0_struct.IntOnMatch = DISABLE;
	pwm_match0_struct.MatchChannel = 0;
	pwm_match0_struct.ResetOnMatch = ENABLE;
	pwm_match0_struct.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &pwm_match0_struct); //configura lo que pasa cuando ocurre el Match 0

	PWM_MATCHCFG_Type pwm_match1_struct;
	pwm_match1_struct.IntOnMatch = DISABLE;
	pwm_match1_struct.MatchChannel = 0;
	pwm_match1_struct.ResetOnMatch = DISABLE;;
	pwm_match1_struct.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &pwm_match1_struct); //configura lo que pasa cuando ocurre el Match 1

	PWM_MatchUpdate(LPC_PWM1, 0, 499999, PWM_MATCH_UPDATE_NOW); //configura el MR0 para periodo de 20ms
	PWM_MatchUpdate(LPC_PWM1, 1, 24999, PWM_MATCH_UPDATE_NOW); //configura el MR1 para que empieze en 0 grados (1ms)
}

void configI2C(){
	PINSEL_CFG_Type I2C_SDA_PIN;
	PINSEL_CFG_Type I2C_SCL_PIN;
	I2C_SDA_PIN.Portnum = 0; //port 0
	I2C_SDA_PIN.Pinnum = 27;// pin 27
	I2C_SDA_PIN.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	I2C_SDA_PIN.Funcnum = PINSEL_FUNC_1; //PWM 1.1 function
	I2C_SDA_PIN.OpenDrain =  PINSEL_PINMODE_OPENDRAIN; //no open drain
	I2C_SCL_PIN.Portnum = 0; //port 0
	I2C_SCL_PIN.Pinnum = 28;// pin 28
	I2C_SCL_PIN.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	I2C_SCL_PIN.Funcnum = PINSEL_FUNC_1; //PWM 1.1 function
	I2C_SCL_PIN.OpenDrain =  PINSEL_PINMODE_OPENDRAIN; //no open drain
	PINSEL_ConfigPin(&I2C_SDA_PIN);
	PINSEL_ConfigPin(&I2C_SCL_PIN);
}
int map(uint16_t value_to_map, uint32_t originalStart, uint32_t originalEnd, uint32_t newStart, uint32_t  newEnd ){

	float val =((newEnd-newStart)*value_to_map)/(originalEnd-originalStart);
	int output = round(val);
	return output;
}

void Servo_Write(uint8_t servoID, uint32_t value){

	//uint32_t MRx_val  = 99999 + (555 * value);
	uint32_t MRx_val  = 24999 + (138 * value);
	PWM_MatchUpdate(LPC_PWM1, 1, MRx_val, PWM_MATCH_UPDATE_NOW); //configura el MR1 para lograr el duty cycle que corresponde

//	switch (servoID){
//		case 0:
//
//		case 1:
//			//carga el MR2 con MRx_val
//	}

}

void TIMER0_IRQHandler(){

	if(TIM_GetIntStatus(LPC_TIM0, 1)){
		if(i%2 == 0){
			ADC_ChannelCmd(LPC_ADC, 5, DISABLE); //deshabilita canal 5
			ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, DISABLE); //configura interrupcion por canal 5
			ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE); //configura interrupcion por canal 5
			ADC_ChannelCmd(LPC_ADC, 4, ENABLE); //habilita canal 4
			i = 1;
		}
		else{
			ADC_ChannelCmd(LPC_ADC, 4, DISABLE); //deshabilita canal 4
			ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, DISABLE); //configura interrupcion por canal 4
			ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE); //configura interrupcion por canal 5
			ADC_ChannelCmd(LPC_ADC, 5, ENABLE); //habilita canal 5
			i = 0;
		}
		ADC_StartCmd(LPC_ADC, ADC_START_NOW);
		TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);
	}
}

void ADC_IRQHandler(){
//logica de control de servomotores
	if(ADC_ChannelGetStatus(LPC_ADC, 4, ADC_DATA_DONE)){ //canal 4-> mov en x

		adc_val = ADC_ChannelGetData(LPC_ADC, 4);

		valL = adc_val & 0xFF;
		valH = (adc_val>>8) & 0xFF;
		UART_SendByte(LPC_UART0, valL);
		UART_SendByte(LPC_UART0, valH);

		mapped_val = map(adc_val, 0, 4095, 0, 180);

		Servo_Write(0 , mapped_val);

//		if(adc_val>=2500){
//			LPC_GPIO2->FIOSET |= (1<<3);
//			LPC_GPIO2->FIOCLR |= (1<<2);
//			LPC_GPIO2->FIOCLR |= (1<<1);
//			LPC_GPIO2->FIOCLR |= (1<<0);
//		}
//		else if (adc_val<=1500){
//			LPC_GPIO2->FIOSET |= (1<<2);
//			LPC_GPIO2->FIOCLR |= (1<<3);
//			LPC_GPIO2->FIOCLR |= (1<<1);
//			LPC_GPIO2->FIOCLR |= (1<<0);
//		}
//		else{
//			LPC_GPIO2->FIOCLR |= (1<<2);
//			LPC_GPIO2->FIOCLR |= (1<<3);
//			LPC_GPIO2->FIOCLR |= (1<<1);
//			LPC_GPIO2->FIOCLR |= (1<<0);
//		}
	}
	else if(ADC_ChannelGetStatus(LPC_ADC, 5, ADC_DATA_DONE)){//canal 5-> mov en y

		adc_val = ADC_ChannelGetData(LPC_ADC, 5);

		valL = adc_val & 0xFF;
		valH = (adc_val>>8) & 0x00FF;
		UART_SendByte(LPC_UART0, valL);
		UART_SendByte(LPC_UART0, valH);

//		if(adc_val>=2500){
//			LPC_GPIO2->FIOSET |= (1<<1);
//			LPC_GPIO2->FIOCLR |= (1<<3);
//			LPC_GPIO2->FIOCLR |= (1<<2);
//			LPC_GPIO2->FIOCLR |= (1<<0);
//		}
//		else if (adc_val<1500){
//			LPC_GPIO2->FIOSET |= (1<<0);
//			LPC_GPIO2->FIOCLR |= (1<<3);
//			LPC_GPIO2->FIOCLR |= (1<<2);
//			LPC_GPIO2->FIOCLR |= (1<<1);
//		}
//		else{
//			LPC_GPIO2->FIOCLR |= (1<<2);
//			LPC_GPIO2->FIOCLR |= (1<<3);
//			LPC_GPIO2->FIOCLR |= (1<<1);
//			LPC_GPIO2->FIOCLR |= (1<<0);
//		}
	}
}







