#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

#define DMA_SIZE 60
#define NUM_SINE_SAMPLE 60
#define SINE_FREQ_IN_HZ 50
#define PCLK_DAC_IN_MHZ 25 //CCLK divided by 4

void confPin();
void confDac();
void confDMA();

GPDMA_Channel_CFG_Type GPDMACfg;

uint32_t dac_sine_lut[NUM_SINE_SAMPLE]; //lo que se va a transferir por DMA al DAC

int main(){

	uint32_t i;
	uint32_t sin_0_to_90_16_samples[16] = {0, 1045, 2079, 3090, 4067, 5000, 5877, 6691, 7431, 8090, 8660, 9135, 9510, 9781, 9945, 10000};

	confPin();
	confDac();
	//Prepare DAC sine look up table
	for(i = 0; i < NUM_SINE_SAMPLE; i++)
	{
		if(i <= 15){
			dac_sine_lut[i] = 512 + 512*sin_0_to_90_16_samples[i]/10000;
			if(i == 15) dac_sine_lut[i] = 1023;
		}
		else if(i <= 30){
			dac_sine_lut[i] = 512 + 512*sin_0_to_90_16_samples[30-i]/10000;
		}
		else if(i<=45){
			dac_sine_lut[i] = 512 - 512*sin_0_to_90_16_samples[i-30]/10000;
		}
		else{
			dac_sine_lut[i] = 512 - 512*sin_0_to_90_16_samples[60-i]/10000;
		}
		dac_sine_lut[i] = (dac_sine_lut[i]<<6);
	}
	confDMA();
	//Enable GPDMA channel 0
	while(1);
	return 0;
}

void confPin(){
	PINSEL_CFG_Type PinCfg;
	/*
	*	Init DAC pin connect
	*	AOUT on P0.26
	*/
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 26;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.OpenDrain = 0;
	PINSEL_ConfigPin(&PinCfg);
}

void confDac(){
	uint32_t tmp;
	DAC_CONVERTER_CFG_Type DAC_ConverterConfigStruct;
	DAC_ConverterConfigStruct.CNT_ENA = SET;
	DAC_ConverterConfigStruct.DMA_ENA = SET;
	DAC_Init(LPC_DAC);
	// set time out for dac
	tmp = (PCLK_DAC_IN_MHZ*1000000)/(SINE_FREQ_IN_HZ*NUM_SINE_SAMPLE);
	DAC_SetDMATimeOut(LPC_DAC, tmp);
	DAC_ConfigDAConverterControl(LPC_DAC, &DAC_ConverterConfigStruct);
	return;
}

void confDMA(){
	GPDMA_LLI_Type DMA_LLI_Struct;
	DMA_LLI_Struct.SrcAddr = (uint32_t) dac_sine_lut;
	DMA_LLI_Struct.DstAddr = (uint32_t) & (LPC_DAC->DACR);
	DMA_LLI_Struct.NextLLI = (uint32_t)&DMA_LLI_Struct; //se enlaza a si misma
	DMA_LLI_Struct.Control = DMA_SIZE | (2<<18) | (2<<21) | (1<<26);


	/*
	GPDMA_LLI_Type DMA_LLI2_Struct;
	DMA_LLI2_Struct.SrcAddr = (uint32_t) proxRegion;
	DMA_LLI2_Struct.DstAddr = (uint32_t) & (LPC_DAC->DACR);
	DMA_LLI2_Struct.NextLLI = 0; //termina
	DMA_LLI2_Struct.Control = DMA_SIZE | (2<<18) | (2<<21) | (1<<26);

	*/

	/* GPDMA block section-------------*/
	/*Initialize GPDMA controller */
	GPDMA_Init();
	//Setup GPDMA channel
	//channel 0
	GPDMACfg.ChannelNum = 0;
	//Source memory
	GPDMACfg.SrcMemAddr = (uint32_t) (dac_sine_lut);
	//Destination memory - unused
	GPDMACfg.DstMemAddr = 0;
	//Transfer size
	GPDMACfg.TransferSize = DMA_SIZE;
	//Transfer widht -unused
	GPDMACfg.TransferWidth = 0;
	//Transfer type
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	//Source connection
	GPDMACfg.SrcConn = 0;
	//Destination connection
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	// Linked List Item 
	GPDMACfg.DMALLI = (uint32_t) &DMA_LLI_Struct;
	//Setup channel with given parameter
	GPDMA_Setup(&GPDMACfg);
	GPDMA_ChannelCmd(0, ENABLE);


}

