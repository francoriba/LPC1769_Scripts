#include "lpc17xx.h"
#include "lpc17xx_gpdma.h"

#define DMA_SWAP_SIZE 10
#define TABLE_LEN 1034

void confDMA();
void confADC();

GPDMA_Channel_CFG_Type GPDMACfg;

uint32_t table[TABLE_LEN];
uint32_t random[2];

uint32_t aux_buffer[DMA_SWAP_SIZE];

GPDMA_LLI_Type DMA_LLI_Struct1;
GPDMA_LLI_Type DMA_LLI_Struct2;
GPDMA_LLI_Type DMA_LLI_Struct3;

uint32_t bitcount;

int main(){
	uint32_t i;
	random[0] = 545;
	random[1] = 433;
	bitcount = 0;

	//prepare table
	for(i = 0; i<TABLE_LEN; i++){
		table[i]=i;
	}
	confADC(); //se genera un valor aleatorio usando el ADC
	while(1); //loop forever
	return 0;
}


void confDMA(){
	 //Prepare DMA link list item structure

	DMA_LLI_Struct1.SrcAddr = (uint32_t) table + 4*random[0];
	DMA_LLI_Struct1.DstAddr = (uint32_t) aux_buffer;
	DMA_LLI_Struct1.NextLLI = (uint32_t)&DMA_LLI_Struct2;
	DMA_LLI_Struct1.Control = DMA_SWAP_SIZE | (2<<18) | (2<<21) | (1<<26) | (1<<27); //src 32 dst 32 src++ dst++

	DMA_LLI_Struct2.SrcAddr = (uint32_t) table + 4*random[1];
	DMA_LLI_Struct2.DstAddr = (uint32_t) table + 4*random[0];
	DMA_LLI_Struct2.NextLLI = &DMA_LLI_Struct3;
	DMA_LLI_Struct2.Control = DMA_SWAP_SIZE | (2<<18) | (2<<21) | (1<<26) | (1<<27); //src 32 dst 32 src++ dst++

	DMA_LLI_Struct3.SrcAddr = (uint32_t) aux_buffer;
	DMA_LLI_Struct3.DstAddr = (uint32_t) table + 4*random[1];
	DMA_LLI_Struct3.NextLLI = 0;
	DMA_LLI_Struct3.Control = DMA_SWAP_SIZE | (2<<18) | (2<<21) | (1<<26) | (1<<27); //src 32 dst 32 src++ dst++


	GPDMA_Init(); 	/*Initialize GPDMA controller */

	//Setup GPDMA channel
	GPDMACfg.ChannelNum = 7; 	//channel 0
	GPDMACfg.SrcMemAddr = DMA_LLI_Struct1.SrcAddr; 	//Source memory
	GPDMACfg.DstMemAddr = DMA_LLI_Struct1.DstAddr; 	//Destination memory 
	GPDMACfg.TransferSize = DMA_SWAP_SIZE; 	//Transfer size
	GPDMACfg.TransferWidth = GPDMA_WIDHT_WORD; 	//Transfer widht 
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2M; 	//Transfer type
	GPDMACfg.SrcConn = 0; //Source connection
	GPDMACfg.DstConn = 0; //Destination connection
	GPDMACfg.DMALLI = (uint32_t) &DMA_LLI_Struct2;
	GPDMA_Setup(&GPDMACfg); 	//Setup channel with given parameter
}

void confADC(){
	ADC_Init(lpc_adc, 2000);
	LPC_SC->PCOMP = (1<<12);
	LPC_ADC->ADCR = (1<<21);
	LPC_SC->PCLKSEL0 |= (3<<24); //CCLK/8
	LPC_ADC->ADCR &= ~(255<<8); //[15:8] CCLKDIV
	LPC_ADC->ADCR |= (1<<16); 
	LPC_PINCON->PINMODE1 |= (1<<15); //neither pullup nor pull down
	LPC_PINCON->PINSEL1 |= (1<<14); //0.23 AD0.0
	LPC_ADC->ADINTEN = 1;
	NVIC_SetPriority(ADC_IRQn, 3);
	NVIC_Enable(ADC_IRQn);
}