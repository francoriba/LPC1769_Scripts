#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
void configPin();
void configUART();

int main(){
	configPin();
	configUART();
	uint8_t info[] = "Hola mundo \t - \t Electrónica Digital 3\t-\tFCEFyN-UNC \n\r";

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
}

void configUART(){
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	//configuración por defecto
	UART_ConfigStructInit(&UARTConfigStruct);
	//inicializa periférico
	UART_Init(LPC_UART2, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	//habilita transmisión
	UART_TxCmd(LPC_UART2, ENABLE);
}


