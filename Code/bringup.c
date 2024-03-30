#include "bringup.h"

void bringup_init(void)
{
  while(NRF24L01_Check())
	{
 		HAL_Delay(1000);
	}
  HAL_TIM_Base_Start_IT(&htim3);
}

void bringup_tim3_loop(void)
{
//	#if nrf_mode
//	#else
//		if(NRF24L01_RxPacket(Receive)==0)
//    {      
//      CDC_Transmit_FS(Receive,sizeof(Receive));
//    }
//	#endif
}

uint8_t usb_buf[TX_PLOAD_WIDTH];
uint8_t Receive[RX_PLOAD_WIDTH];
uint8_t datacom;

void bringup_main_loop(void)
{
	#if nrf_mode
	if(datacom)
	{
		NRF24L01_MTX_Mode();
		NRF24L01_TxPacket(usb_buf);
		NRF24L01_MRX_Mode();
		while(NRF24L01_IRQ_PIN_READ()!=0);
		NRF24L01_RxPacket(Receive);
		CDC_Transmit_FS(Receive,sizeof(Receive));
		datacom=0;
	}
	
	#else
	NRF24L01_SRX_Mode();
	while(NRF24L01_IRQ_PIN_READ()!=0);
	NRF24L01_RxPacket(Receive);
	NRF24L01_STX_Mode();
	NRF24L01_TxPacket(Receive);
	CDC_Transmit_FS(Receive,sizeof(Receive));
	
	#endif
}
