/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <cr_section_macros.h>



enum states{
	INIT= 0x0,
	SENDING_1 = 0x1,
	SENDING_2 = 0x2,
	CHECKING_PIN = 0x3,
	DELAY = 0x4,
};

void check_pin(uint8_t* pin);

void check_pin(uint8_t* pin){
	*pin =(LPC_GPIO0->FIOPIN2 >>3)&1;
}

int main(void) {
	uint8_t* pin =0;
	uint16_t signal_1= 0x256;
	uint16_t signal_2= 0x3AA;//cambiar a bytes.
	uint8_t rotate=0;
	uint8_t previous_state=SENDING_1;
	uint8_t state=INIT;
	SystemInit();
	LPC_GPIO0->FIODIR |= ((1<<22)&(~(1<<19)));//output port0 pin 22 input port0 pin 1
	LPC_GPIO0->FIOMASK = ~(1<<22);//
	LPC_GPIO0->FIOMASK &= ~(1<<19);//

    while(1) {
    	switch(state){
    	case INIT:
				rotate=0;
				state= CHECKING_PIN;
				break;

    	case CHECKING_PIN:
				check_pin(pin);

				if(*pin==1){
					state= SENDING_1;
				}
				else{
					state= SENDING_2;
				}
				if (previous_state!=state){
					rotate=0;
					}

				break;

    	case SENDING_1:
    			previous_state=SENDING_1;
    			LPC_GPIO0->FIOPIN = ((signal_1>>rotate) & 1 )<< 22;//
				rotate++;
				//call delay function;
				state = DELAY;
				break;
    	case SENDING_2:
    			previous_state=SENDING_2;
				LPC_GPIO0->FIOPIN = ((signal_2>>rotate) & 1 )<< 22;//
				rotate++;
				state = DELAY;
				break;

    	case DELAY:
    			//call delay function here;
    		if (rotate==11)
    			state=INIT;
    		else
    			state=CHECKING_PIN;
    		break;
    	default:
    		state=INIT;
    		break;
    }
    return 0;
    }
}








