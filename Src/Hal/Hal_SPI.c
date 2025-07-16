/****************************************************
  * @Name	Hal_SPI.c
  * @Brief	Lora SPI interface
  ***************************************************/

#include "hal_spi.h"

static void hal_Sx1278_GpioInt(void);

/*----------------------------------------------------------------------------
@Name		: hal_spi_Config(void)
@Function	: SPI_Lora config
@Parameter	: None
------------------------------------------------------------------------------*/
void hal_spi_Config(void)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_SPI, ENABLE);  
  hal_Sx1278_GpioInt();   
  SPI_Init(SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master,SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_2Lines_FullDuplex,SPI_NSS_Soft);
  SPI_Cmd(ENABLE);
}

/*----------------------------------------------------------------------------
@Name		: hal_Sx1278_GpioInt(void)
@Function	: GPIO config
@Parameter	: None
------------------------------------------------------------------------------*/
static void hal_Sx1278_GpioInt(void)
{
  GPIO_Init(RF_RST_PORT,RF_RST_PIN , GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(SCK_PORT,SCK_PIN , GPIO_Mode_Out_PP_Low_Fast);  
  GPIO_Init(MISO_PORT,MISO_PIN , GPIO_Mode_In_PU_No_IT);
  GPIO_Init(MOSI_PORT,MOSI_PIN , GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(CS_PORT,CS_PIN , GPIO_Mode_Out_PP_Low_Fast);  
  //*****RF_MISO       PC_IDR_IDR5 //INPUT
  GPIO_Init(RF_IRQ_DIO0_PORT,RF_IRQ_DIO0_PIN,GPIO_Mode_In_PU_No_IT);  
  EXTI_SetPinSensitivity (EXTI_Pin_3,EXTI_Trigger_Rising);           //Lora receive interrupt 
  enableInterrupts(); 
  CS_HIGH;
  RF_RST_HIGH;
}

/*----------------------------------------------------------------------------
@Name		: SpiInOut(uint8_t outData)
@Function	: write/read SPI data
@Parameter	: outData->data to be written
@Return		: data received through SPI
------------------------------------------------------------------------------*/
uint8_t SpiInOut(uint8_t outData)
{
    while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET);
    SPI_SendData(outData);
    while(SPI_GetFlagStatus(SPI_FLAG_RXNE ) == RESET);
    return SPI_ReceiveData();
}

/*----------------------------------------------------------------------------
@Name		: halSpi_ReadSX1276_RegDat(uint8_t addr)
@Function	: read single register of SX1278
@Parameter	: addr->register address
@Return		: data received through SPI
------------------------------------------------------------------------------*/
uint8_t halSpi_ReadSX1276_RegDat(uint8_t addr)
{
    uint8_t dat;
    CS_LOW;
    SpiInOut(addr & 0x7F);
    dat = SpiInOut(0xFF); // 0xFF->dummy byte, to generate clock
    CS_HIGH;
    return dat; 
}

/*----------------------------------------------------------------------------
@Name		: halSpi_WriteSX1276_RegDat( uint8_t addr, uint8_t dat)
@Function	: write single register of SX1278
@Parameter	: addr->register address
              dat->data to be written
------------------------------------------------------------------------------*/
void halSpi_WriteSX1276_RegDat( uint8_t addr, uint8_t dat)
{
    CS_LOW;
    SpiInOut(addr | 0x80);
    SpiInOut(dat);
    CS_HIGH;
}

/*----------------------------------------------------------------------------
@Name		: halSpi_ReadSX1276_MultiRegDat(uint8_t addr, uint8_t *buffer, uint8_t size)
@Function	: read multiple registers of SX1276
@Parameter	: addr->register address
              buffer->point to the buffer to save the data
              size->size of data to read
------------------------------------------------------------------------------*/
void halSpi_ReadSX1276_MultiRegDat(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    uint8_t i;
    CS_LOW;
    SpiInOut(addr & 0x7F);
    for(i=0; i<size; i++)
    {
        buffer[i] = SpiInOut(0xFF);
    }
    CS_HIGH;
}

/*----------------------------------------------------------------------------
@Name		: halSpi_WriteSX1276_MultiRegDat(uint8_t addr, uint8_t *buffer, uint8_t size)
@Function	: write multiple registers of SX1276
@Parameter	: addr->register address
              buffer->point to the data to be written
              size->size of data  to be written
------------------------------------------------------------------------------*/
void halSpi_WriteSX1276_MultiRegDat(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    uint8_t i;
    CS_LOW;
    SpiInOut(addr | 0x80);
    for(i=0; i<size; i++)
    {
        SpiInOut(buffer[i]);
    }
    CS_HIGH;
}