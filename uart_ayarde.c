#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>
#include <stdio.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"

void configPin();
void configUART();

int main(){
	configPin();
	configUART();
	uint8_t info[] = "Hola mundo\t-\tElectrónica Digital 3\t-\tFCEFyN-UNC \n\r";

	while(1){
		UART_Send(LPC_UART2, info, sizeof(info), BLOCKING);
	}
	return 0;
}

void configPin(){
	PINSEL_CFG_Type pinCfg;

	pinCfg.Portnum = 0;
	pinCfg.Pinnum = 10;
	pinCfg.Pinmode = 0;
	pinCfg.Funcnum = 1;
	pinCfg.OpenDrain = 0;
	PINSEL_ConfigPin(&pinCfg);
	pinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&pinCfg);
}

void configUART(){
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	UART_ConfigStructInit(&UARTConfigStruct);//configuración por defecto
	UART_Init(LPC_UART2, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(LPC_UART2,&UARTFIFOConfigStruct); //inicializa FIFO
	UART_TxCmd(LPC_UART2, ENABLE);
}


