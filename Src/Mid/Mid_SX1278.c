/********************************************************
  * @Name	Mid_SX1278.c
  * @Brief	SX1278 module driver
  * Step 1: SPI Communication Setup
            -> Implement SPI communication between the SX1278 chip and the microcontroller to enable read and write operations.
            -> Configure the D0 pin of the microcontroller connected to the SX1278 chip as an input mode and enable the external interrupt for the D0 IO pin.
            Note: When the LORA wireless receives data, the D0 pin generates a rising edge.
            -> Upon power-up, call the function mid_sx1278_Reset to reset the SX1278 chip.
            -> Set up the configuration functions for the D0 pin in both transmission and reception modes.
    Step 2: Wireless Transmission Function
            -> Configure the D0 pin of the SX1278 chip as an input mode.
            -> Use the function void mid_Sx1278_LoRaSentBuffer(unsigned char *dat, unsigned char len) to send data.
            Note: After completing wireless data transmission, the D0 pin of LORA outputs a rising edge.
    Step 3: Wireless Reception Function
            -> After powering on and initializing the SPI driver, configure the SX1278 chip into receive mode using the function void mid_SX1278_LoRaRxConfig(void).
            -> Add the serial data processing function void mid_Sx1278RxDateGete(void) within the external interrupt handler for the D0 pin.
            -> In the mid_Sx1278RxDateGete(void) function, implement logic for handling received data.
**********************************************************/

#include "mid_sx1278.h"
#include "mid_usart.h"
#include "hal_spi.h"
#include "hal_usart.h"

Str_LoraVari LoraVari={1,LORAPOWER_13DBM,LORASF_7,LORA_RATE_250KHZ,LORA_CR_4_5,470000};
 
void mid_sx1278Vart(Str_LoraVari vari)
{
    LoraVari.RxPayLoadCrc = 1;
    LoraVari.loraCr = vari.loraCr;
    LoraVari.loraBw = vari.loraBw;
    LoraVari.loraFre = vari.loraFre;
    LoraVari.loraPower = vari.loraPower;
    LoraVari.loraSf = vari.loraSf; 
}

/*********************************************************************
 * Name         : mid_Delay1ms(void)
 * Function     : Millisecond delay function
 * Parameter    : None
**********************************************************************/
void mid_Delay1ms(void)
{
    unsigned short msdelay;
    msdelay = 2260;
    while(msdelay--);
}

/*********************************************************************
 * Name         : void mid_DelayMs(uint16_t t)
 * Function     : Millisecond delay function
 * Parameter    : t->xms, Value range: 0-65535
**********************************************************************/
void mid_DelayMs(uint16_t t)
{
  while(t--)
  {
    mid_Delay1ms();  
  }
}
 
/*********************************************************************
 * Name         : void mid_sx1278_Reset(void)
 * Function     : SX1278 chip reset operation function
 * Parameter    : None
**********************************************************************/
void mid_sx1278_Rest(void)
{
    RF_RST_LOW;
    mid_DelayMs(1);
    RF_RST_HIGH;
    mid_DelayMs(7);  
}



/*********************************************************************
 * Name         : void mid_sx1278_SetPowerConfig(unsigned char Power)
 * Function     : Configure the wireless transmission power of the SX1278 chip registers
 * Parameter    : Power->20dBm, Value range: 1-18
**********************************************************************/
void mid_sx1278_SetPowerConfig(unsigned char Power)
{
    if(Power == 20)
    {
        halSpi_WriteSX1276_RegDat(REG_LR_PADAC,0x87); 
        halSpi_WriteSX1276_RegDat(LR_RegPaConfig,0xFF); // 11dbm
    }
    else if((Power > 1)&&(Power < 18))
    {
        halSpi_WriteSX1276_RegDat(REG_LR_PADAC,0x84);   
        halSpi_WriteSX1276_RegDat(LR_RegPaConfig,(0xf0 + Power - 2)); // 11dbm
    }
}

/*********************************************************************
 * Name         : void mid_sx1278_SetFreConfig(unsigned char Power)
 * Function     : Configure the LoRa wireless operating frequency in the SX1278 chip registers
 * Parameter    : fre->Frequency (Range: 137,000,000 ~ 525,000,000 kHz)
**********************************************************************/
void mid_sx1278_SetFreConfig(unsigned long fre)  
{
    double fredat;
    static un_lorafre sx1278Fre;
    fredat = fre/0.06103515625L;
    sx1278Fre.fredat = (uint32_t)fredat;
    halSpi_WriteSX1276_RegDat(LR_RegFrMsb,sx1278Fre.Fre[1]);  
    halSpi_WriteSX1276_RegDat(LR_RegFrMid,sx1278Fre.Fre[2]); 
    halSpi_WriteSX1276_RegDat(LR_RegFrLsb,sx1278Fre.Fre[3]);  
}

/*********************************************************************
 * Name         : void mid_sx1278_Config(Str_LoraVari loraPara)
 * Function     : Configure SX1278 registers
 * Parameter    : loraPara->Configuration parameters for the registers
**********************************************************************/
void mid_sx1278_Config(Str_LoraVari loraPara)
{
    uint8_t temp;
    
    halSpi_WriteSX1276_RegDat(LR_RegOpMode,0x00);   // set to Sleep mode
    halSpi_WriteSX1276_RegDat(REG_LR_TCXO,0x09);    // set system clock from external clock
    halSpi_WriteSX1276_RegDat(LR_RegOpMode,0x88);   // set to LoRa mode
   
    // Frequency config
    mid_sx1278_SetFreConfig(loraPara.loraFre.fredat);
    
    // Power config
    mid_sx1278_SetPowerConfig(loraPara.loraPower);
    
    halSpi_WriteSX1276_RegDat(LR_RegOcp,0x0B);
    halSpi_WriteSX1276_RegDat(LR_RegLna,0x23);  

    if(loraPara.loraSf==6)  
    {   //spreading factor SFactor=6
        halSpi_WriteSX1276_RegDat(LR_RegModemConfig1,((loraPara.loraBw<<4)+(loraPara.loraCr<<1)+0x01));         //Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
        halSpi_WriteSX1276_RegDat(LR_RegModemConfig2,((loraPara.loraSf<<4)+(loraPara.RxPayLoadCrc<<2)+0x03));
        temp = halSpi_ReadSX1276_RegDat(LR_RegPacketConfig2);
        temp &= 0xF8;
        temp |= 0x05;  
        halSpi_WriteSX1276_RegDat(LR_RegPacketConfig2,temp); 
        halSpi_WriteSX1276_RegDat(LR_RegSeqConfig2,0x0C); 
    } 
    else
    {
        halSpi_WriteSX1276_RegDat(LR_RegModemConfig1,((loraPara.loraBw<<4)+(loraPara.loraCr<<1)+0x00));         //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
        halSpi_WriteSX1276_RegDat(LR_RegModemConfig2,((loraPara.loraSf<<4)+(loraPara.RxPayLoadCrc<<2)+0x03));   //SFactor & LNA gain set by the internal AGC loop 
        halSpi_WriteSX1276_RegDat(LR_RegModemConfig3,0x08);     //LowDataRateOptimize enable
    }
    
    halSpi_WriteSX1276_RegDat(LR_RegSymbTimeoutLsb,0xFF);   
    halSpi_WriteSX1276_RegDat(LR_RegPreambleMsb, 0);       
    halSpi_WriteSX1276_RegDat(LR_RegPreambleLsb,16);       
    
    halSpi_WriteSX1276_RegDat(REG_LR_DIOMAPPING2,0x01);   // DIO mapping 
    halSpi_WriteSX1276_RegDat(LR_RegOpMode,0x89);         // set to standby mode                                     
}



/**********************************************************
 * Name         : void mid_Sx1278_RxGpioGonfig(void)
 * Function     : config Lora receiving mode
 * Parameter    : None
**********************************************************/
void mid_Sx1278_RxGpioGonfig(void)
{
    GPIO_Init(RF_IRQ_DIO0_PORT, RF_IRQ_DIO0_PIN, GPIO_Mode_In_FL_IT);  // D0 external interrupt pin
}

/**********************************************************
 * Name         : void mid_Sx1278TxGpioConfig(void)
 * Function     : config Lora to transmit mode(DIO input, disable interrupt)
 * Parameter    : None
**********************************************************/
void mid_Sx1278TxGpioConfig(void)
{
    GPIO_Init(RF_IRQ_DIO0_PORT, RF_IRQ_DIO0_PIN, GPIO_Mode_In_PU_No_IT); 
}

/**********************************************************
 * Name         : SX1276_LoRaRxConfig(void)
 * Function     : config Lora receive mode
 * Parameter    : None
**********************************************************/
void mid_SX1278_LoRaRxConfig(void)
{
    uint8_t addr; 
 
    mid_sx1278_Config(LoraVari);
                              
    halSpi_WriteSX1276_RegDat(LR_RegHopPeriod,0);           // disable frequency modulation
    halSpi_WriteSX1276_RegDat(REG_LR_DIOMAPPING1,0x00);     // D0 set to Rx mode
    halSpi_WriteSX1276_RegDat(LR_RegIrqFlagsMask,0xBF);     // enable RxDone interrupt
    halSpi_WriteSX1276_RegDat(LR_RegIrqFlags,0xFF);         // clear interrupt flag
    addr = halSpi_ReadSX1276_RegDat(LR_RegFifoRxBaseAddr);  // read the data receive base address
    halSpi_WriteSX1276_RegDat(LR_RegFifoAddrPtr,addr);      // set the data receive base address　                    
    halSpi_WriteSX1276_RegDat(LR_RegOpMode,0x8D);           // config Lora to continuous receive mode
    mid_Sx1278_RxGpioGonfig();
}




/**********************************************************
 * Name         : void mid_Sx1278_LoRaEntryTxConfig(void)
 * Function     : Configure registers for SX1278 wireless transmission
 * Parameter    : None
**********************************************************/
void mid_Sx1278_LoRaEntryTxConfig(void)
{
    uint8_t addr;
    mid_sx1278_Config(LoraVari);                  
    halSpi_WriteSX1276_RegDat(LR_RegHopPeriod,0x00);        // disable frequency modulation
    halSpi_WriteSX1276_RegDat(REG_LR_DIOMAPPING1,0x40);     // D0 set to Tx mode
    halSpi_WriteSX1276_RegDat(LR_RegIrqFlags,0xFF);         // clear interrupt flag
    halSpi_WriteSX1276_RegDat(LR_RegIrqFlagsMask,0xF7);     // enable TxDone interrupt
    addr = halSpi_ReadSX1276_RegDat(LR_RegFifoTxBaseAddr);  // read the data transmission base address
    halSpi_WriteSX1276_RegDat(LR_RegFifoAddrPtr,addr);      // set the data transmission base address
}

/**********************************************************
 * Name         : void mid_Sx1278_LoRaSentBuffer(unsigned char *dat, unsigned char len)
 * Function     : SX1278 wireless data transmission function
 * Parameters   :
 *   -> dat : Pointer to the LoRa transmission data
 *   -> len : Length of the data to be transmitted
**********************************************************/
void mid_Sx1278_LoRaSentBuffer(unsigned char *dat,unsigned char len)
{ 
    mid_Sx1278TxGpioConfig();                               // config Lora transmit mode(DIO0 input, disable interrupt)
    mid_Sx1278_LoRaEntryTxConfig();                         // Configure registers for SX1278 wireless transmission
    halSpi_WriteSX1276_RegDat(LR_RegPayloadLength,len);     // set the payload length
    halSpi_WriteSX1276_MultiRegDat(0,dat,len);              // write the data to be transmitted
    halSpi_WriteSX1276_RegDat(LR_RegOpMode,0x8b);           // set to transmit mode
    while(!(RF_IRQ_DIO0_DATABIT));                          // wait for data transmission to complete
    mid_SX1278_LoRaRxConfig();                              // restore to receive mode
}

/**********************************************************
 * Name         : void mid_Sx1278RxDateGete(void)
 * Function     : Lora wireless data receive
 * Parameter    : None
**********************************************************/
void mid_Sx1278RxDateGet(void)
{
    uint8_t addr,packet_size;
    uint8_t lorarxbuf[256];
    addr = halSpi_ReadSX1276_RegDat(LR_RegFifoRxCurrentaddr);   // get the Rx starting address
    halSpi_WriteSX1276_RegDat(LR_RegFifoAddrPtr,addr);          // set address
    if(LoraVari.loraSf == 6)                                    // set the packet length = 21
    {
        packet_size=21;   
    }  
    else
    {
        packet_size = halSpi_ReadSX1276_RegDat(LR_RegRxNbBytes);    // check the packet length
    }            
    halSpi_ReadSX1276_MultiRegDat(0x00,&lorarxbuf[0],packet_size);  // readout data
    
    if(SystemVari.com_Mode == COM_MODE_AT)
    {
        HAL_USART_PutInTxQueue("DATA:",5);
    } 
    HAL_USART_PutInTxQueue(&lorarxbuf[0],packet_size);
    halSpi_WriteSX1276_RegDat(LR_RegIrqFlags,0xFF);     // clear flag
    mid_SX1278_LoRaRxConfig();                          // restore to receive mode
}

void mid_Sx1278_Sleep(void)
{
    halSpi_WriteSX1276_RegDat(LR_RegOpMode,0x08);       // set to Lora mode
}
