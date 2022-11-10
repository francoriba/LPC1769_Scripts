#include "lpc17xx.h"
#include "lpc17xx_gpdma.h"
/* interrupcion cuando termina la transferencia */
#define DMA_SIZE 60

void confDac();
void confDMA();

const uint32_t DMASrc_Buffer[DMA_SIZE] = 
{
	0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10,
	0x11121314, 0x15161718, 0x191A1B1C, 0x1D1E1F20,
	0x21222324, 0x25262728, 0x292A2B2C, 0x2D2E2F20,
	0x31323334, 0x35363738, 0x393A3D3C, 0x3D3E3F30
};
uint32_t DMADest_Buffer[DMA_SIZE];

volatile uint32_t Channel0_TC; //Terminal Counter flag for Channel 0 //cuando termina la transf
volatile uint32_t Channel0_Err;//Error Counter flag For Channel 0 //indica si hubo error en la transf

void DMA_IRQHandler();
void BufferVerify(); //verifica que los datos del destino sean los mismos que los de la fuente


int main(){
	confDMA();
	Channel0_TC = 0; //reset terminal counter
	Channel0_Err = 0; //reset error counter
	GPDMA_ChannelCmd(0, ENABLE); //Enable GPDMA channel 0
	NVIC_EnableIRQ(DMA_IRQn); // enable GPDMA interrupt

	while((Channel0_TC == 0)&&C(hannel0_Err == 0));
	Buffer_Verify(); //verify buffer
	while(1); //loop forever
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

void confDMA(){

	NVIC_DisableIRQ(DMA_IRQn);
	GPDMA_Init(); 	/*Initialize GPDMA controller */

	//Setup GPDMA channel
	GPDMACfg.ChannelNum = 0; 	//channel 0
	GPDMACfg.SrcMemAddr = (uint32_t) DMASrc_Buffer; 	//Source memory
	GPDMACfg.DstMemAddr = (uint32_t) DMADest_Buffer; 	//Destination memory 
	GPDMACfg.TransferSize = DMA_SIZE; 	//Transfer size
	GPDMACfg.TransferWidth = GPDMA_WIDHT_WORD; 	//Transfer widht 
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2M; 	//Transfer type
	GPDMACfg.SrcConn = 0; //Source connection
	GPDMACfg.DstConn = 0; //Destination connection
	GPDMACfg.DMALLI = 0; 	// Linked List Item - unused
	GPDMA_Setup(&GPDMACfg); 	//Setup channel with given parameter
}

void BufferVerify(){
	uint8_t i;
	uint8_t *src_addr = (uint32_t *) DMASrc_Buffer;
	uint32_t *dest_addr = (uint32_t *) DMADest_Buffer;

	for(i = 0; i<DMA_SIZE; i++){
		if(*src_addr++ != *dest_addr++){
			while(1){}
		}
	}	
}

void DMA_IRQHandler(){
	if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){
		if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)){
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
			Channel0_TC++;
		}
		if(GPDMA_IntGetStatus(GPDMA_sTAT_INTERR, 0)){
			GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
			Channel0_Err++;
		}
	}
}

