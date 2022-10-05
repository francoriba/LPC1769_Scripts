#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>
#include <stdio.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"

void confPin();
void confDAC();
void delay();

int main(){
	uint32_t dac_value[11] = {0, 132, 223, 323, 423, 523, 623, 723, 823, 923, 1023};
	confPin();
	confDAC();

	while(1){
		for (uint8_t conta = 0; conta < 11; conta++){
			DAC_UpdateValue(LPC_DAC, dac_value[conta]);
			delay(); //este retardo debe asegurar que el tiempo entre conversiones sea mayor a 1/1MHz
		}
	}
}

void confPin(){
	PINSEL_CFG_Type pin_dac; //config AOUT P0.26
	pin_dac.Portnum = 0; //port 0
	pin_dac.Pinnum = 26; //pin 26
	pin_dac.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	pin_dac.Funcnum = PINSEL_FUNC_2; //AOUT  function
	pin_dac.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&pin_dac);
}

void confDAC(){DAC_Init(LPC_DAC);} // DAC init configura por defecto con la maxima frec de conv (1MHz)

void delay(){
	for (uint32_t i = 0; i < 4000000;  i++) {}
}
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>
#include <stdio.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"

void confPin();
void confDAC();
void delay();

int main(){
	uint32_t dac_value[11] = {0, 132, 223, 323, 423, 523, 623, 723, 823, 923, 1023};
	confPin();
	confDAC();

	while(1){
		for (uint8_t conta = 0; conta < 11; conta++){
			DAC_UpdateValue(LPC_DAC, dac_value[conta]);
			delay();
		}
	}
}

void confPin(){
	PINSEL_CFG_Type pin_dac; //config AOUT P0.26
	pin_dac.Portnum = 0; //port 0
	pin_dac.Pinnum = 26; //pin 26
	pin_dac.Pinmode = PINSEL_PINMODE_TRISTATE; //neither pull-up nor pull-down
	pin_dac.Funcnum = PINSEL_FUNC_2; //AOUT  function
	pin_dac.OpenDrain = PINSEL_PINMODE_NORMAL; //no open drain
	PINSEL_ConfigPin(&pin_dac);
}

void confDAC(){DAC_Init(LPC_DAC);} // DAC init configura por defecto con la maxima frec (1MHz)

void delay(){
	for (uint32_t i = 0; i < 4000000;  i++) {}
}
