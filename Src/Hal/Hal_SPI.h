#ifndef ____HAL_SPI_H
#define ____HAL_SPI_H_

#include "stm8l10x.h"

#define RF_RST_PORT  GPIOC   
#define RF_RST_PIN   GPIO_Pin_0
#define RF_RST_HIGH  GPIO_SetBits(RF_RST_PORT,RF_RST_PIN)
#define RF_RST_LOW   GPIO_ResetBits(RF_RST_PORT,RF_RST_PIN)

#define MISO_PORT    GPIOB
#define MISO_PIN     GPIO_Pin_7
#define MISO_DATABIT GPIO_ReadInputDataBit(MISO_PORT,MISO_PIN)

#define MOSI_PORT  GPIOB
#define MOSI_PIN   GPIO_Pin_6
#define MOSI_HIGH  GPIO_SetBits(MOSI_PORT,MOSI_PIN)
#define MOSI_LOW   GPIO_ResetBits(MOSI_PORT,MOSI_PIN)

#define SCK_PORT  GPIOB
#define SCK_PIN   GPIO_Pin_5
#define SCK_HIGH  GPIO_SetBits(SCK_PORT,SCK_PIN)
#define SCK_LOW   GPIO_ResetBits(SCK_PORT,SCK_PIN)

#define CS_PORT  GPIOB
#define CS_PIN   GPIO_Pin_4
#define CS_HIGH  GPIO_SetBits(CS_PORT,CS_PIN)
#define CS_LOW   GPIO_ResetBits(CS_PORT,CS_PIN)
 
#define RF_IRQ_DIO0_PORT     GPIOB
#define RF_IRQ_DIO0_PIN      GPIO_Pin_3
#define RF_IRQ_DIO0_DATABIT  GPIO_ReadInputDataBit(RF_IRQ_DIO0_PORT,RF_IRQ_DIO0_PIN)

void hal_spi_Config(void);
uint8_t halSpi_ReadSX1276_RegDat(uint8_t addr);
void halSpi_WriteSX1276_RegDat(uint8_t addr, uint8_t dat);
void halSpi_ReadSX1276_MultiRegDat(uint8_t addr, uint8_t *buffer, uint8_t size);
void halSpi_WriteSX1276_MultiRegDat(uint8_t addr, uint8_t *buffer, uint8_t size);

#endif
//
