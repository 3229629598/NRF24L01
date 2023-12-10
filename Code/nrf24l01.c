#include "nrf24l01.h"

uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0xa5,0xa5,0xa5,0xa5,0xa5}; //发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x5a,0x5a,0x5a,0x5a,0x5a}; //接收地址

nrf_status_str nrf_status;

uint8_t rx_data[RX_ADR_WIDTH]={0};
uint8_t tx_data[TX_ADR_WIDTH]={0};

#define rx_config 0x0f
#define tx_config 0x0e

uint8_t nrf_rx_init(void)
{
    NRF24L01_CE_LOW();

    nrf_set_addr(RX_ADDRESS,TX_ADDRESS); //配置收发地址
    nrf_write_reg(CONFIG,rx_config); //配置工作模式
    nrf_write_reg(EN_RXADDR,0x01); //使能通道0的接收地址

    nrf_write_reg(RF_CH,40); //设置RF通信频率
    nrf_write_reg(RF_SETUP,0x0f); //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
    nrf_write_reg(RX_PW_P0,RX_PLOAD_WIDTH); //选择通道0的有效数据宽度

    nrf_write_reg(EN_AA,0x01); //使能通道0的自动应答
    nrf_write_reg(FEATURE,0x06); //使能动态负载(收发数据)长度
    nrf_write_reg(DYNPD,0x01); //使能通道0动态负载长度
      
    NRF24L01_CE_HIGH(); //CE为高,进入接收模式
	return nrf_check();
}

uint8_t nrf_tx_init(void)
{
    NRF24L01_CE_LOW();

    nrf_set_addr(TX_ADDRESS,RX_ADDRESS); //配置收发地址
    nrf_write_reg(CONFIG,tx_config); //配置工作模式
    nrf_write_reg(EN_RXADDR,0x01); //使能通道0的接收地址

    nrf_write_reg(RF_CH,40); //设置RF通信频率
    nrf_write_reg(RF_SETUP,0x0f); //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
    nrf_write_reg(RX_PW_P0,RX_PLOAD_WIDTH); //选择通道0的有效数据宽度
    nrf_write_reg(SETUP_RETR, 0x03); //设置自动重发间隔时间:250us+86us;最大自动重发次数:3次

    nrf_write_reg(EN_AA,0x01); //使能通道0的自动应答
    nrf_write_reg(FEATURE,0x07); //使能动态负载(收发数据)长度
    nrf_write_reg(DYNPD,0x01); //使能通道0动态负载长度
      
    NRF24L01_CE_HIGH(); //CE为高,10us后启动发送
    return nrf_check();
}

uint8_t nrf_read_data(uint8_t command, uint8_t* buf, uint16_t len)
{
    uint8_t status;	    
	NRF24L01_SPI_CS_ENABLE();
    status=HAL_SPI_Transmit(&hspi1, &command, 1, 0xff); //发送读数据命令
    if(status==HAL_OK)
    {
        status=HAL_SPI_Receive(&hspi1, buf, len, 0xff); //接收fifo的数据
    }
    NRF24L01_SPI_CS_DISABLE();
    return status;
}

uint8_t nrf_write_data(uint8_t command, uint8_t* buf, uint16_t len)
{
    uint8_t status;
    NRF24L01_SPI_CS_ENABLE();
    status=HAL_SPI_Transmit(&hspi1, &command, 1, 0xff); //发送写数据命令
    if(status==HAL_OK)
    {
        if(len>32)
            len=32;
        status=HAL_SPI_Transmit(&hspi1, buf, len, 0xff); //发送数据到fifo
    }
    NRF24L01_SPI_CS_DISABLE();
    return status;
}

uint8_t nrf_send(uint8_t command, uint8_t* buf, uint16_t len)
{
    uint8_t status;
    NRF24L01_CE_LOW();
    status=nrf_write_data(command,buf,len);
    NRF24L01_CE_HIGH();
    return status;
}

uint8_t nrf_config(nrf_config_str nrf_config)
{
    return nrf_config.prim_rx+nrf_config.pwr_up+nrf_config.crco+nrf_config.en_crc+nrf_config.mask_max_rt+nrf_config.mask_tx_ds+nrf_config.mask_rx_dr;
}

uint8_t nrf_set_addr(uint8_t* rx_addr, uint8_t* tx_addr)
{
    uint8_t status;
    status=nrf_write_data(NRF_WRITE_REG+RX_ADDR_P0,rx_addr,RX_ADR_WIDTH);
    if(status==HAL_OK)
    {
        status=nrf_write_data(NRF_WRITE_REG+TX_ADDR,tx_addr,TX_ADR_WIDTH);
    }
    return status;
}

uint8_t nrf_read_reg(uint8_t reg, uint8_t* value)
{
    return nrf_read_data(NRF_READ_REG+reg,value,1);
}

uint8_t nrf_write_reg(uint8_t reg, uint8_t value)
{
    return nrf_write_data(NRF_WRITE_REG+reg,&value,1);
}

uint8_t nrf_check(void)
{
	uint8_t tx_addr_read[TX_ADR_WIDTH]={0};
    nrf_read_data(NRF_READ_REG+TX_ADDR,tx_addr_read,TX_ADR_WIDTH);
	int i;
	for(i=0; i<TX_ADR_WIDTH; ++i)
	{
		if(tx_addr_read[i]!=TX_ADDRESS[i])  
            break;
	}
	if(i!=5)
        return HAL_ERROR;
    return HAL_OK;
}

uint8_t nrf_low_power_mode(void)
{
    NRF24L01_CE_LOW();
    return nrf_write_reg(CONFIG, 0x00);
}

void nrf_IRQHandler(void)
{
    uint8_t status;
    nrf_read_reg(STATUS, &status);
    nrf_status.tx_full=status&0x01;
    nrf_status.rx_p_no=status&0x0e;
    nrf_status.max_rt=status&MAX_TX;
    nrf_status.tx_ds=status&TX_OK;
    nrf_status.rx_dr=status&RX_OK;

    if(nrf_status.tx_ds)
    {
        NRF24L01_CE_LOW();
        nrf_write_reg(STATUS, TX_OK);
        nrf_write_reg(FLUSH_TX, NOP);
        NRF24L01_CE_HIGH();
    }
    if(nrf_status.rx_dr)
    {
        NRF24L01_CE_LOW();
        uint8_t len;
        nrf_read_reg(R_RX_PL_WID,&len);
        if(len<33)
        {
            nrf_read_data(RD_RX_PLOAD,rx_data,len);
			CDC_Transmit_FS(rx_data,len);
        }
        nrf_write_data(W_ACK_PAYLOAD(0),tx_data,sizeof(tx_data));
        nrf_write_reg(STATUS,RX_OK);
        nrf_write_reg(FLUSH_RX, NOP);
        NRF24L01_CE_HIGH();
    }
    if(nrf_status.max_rt)
    {
        NRF24L01_CE_LOW();
        nrf_write_reg(STATUS, MAX_TX);
        nrf_write_reg(FLUSH_TX, NOP);
        NRF24L01_CE_HIGH();
    }
}
