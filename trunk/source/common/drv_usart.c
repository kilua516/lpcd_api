/*
*************************************************************
 *@Copyright (C), 2016-2020 smartlinkMicro. Co., Ltd.  All rights reserved.
 *@Filename:
 *@Complier:    IAR msp430 5.30
 *@Target OS:   NONE
 *@Target HW:  SL1523
 *@Author:        htyi
 *@Version :     V1.0.0
 *@Date:          2017.8
 *@Description:
 *@History:
 *@    <author>     <time>     <version >   <desc>

*************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "drv_usart.h"
#include "msp430x24x.h"
/** @addtogroup STM32F0xx_StdPeriph_Driver
  * @{
  */

/** @defgroup USART
  * @brief USART driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/*!< USART CR1 register clear Mask ((~(uint32_t)0xFFFFE6F3)) */
#define CR1_CLEAR_MASK            ((uint32_t)(USART_CR1_M | USART_CR1_PCE | \
                                              USART_CR1_PS | USART_CR1_TE | \
                                              USART_CR1_RE))

/*!< USART CR2 register clock bits clear Mask ((~(uint32_t)0xFFFFF0FF)) */
#define CR2_CLOCK_CLEAR_MASK      ((uint32_t)(USART_CR2_CLKEN | USART_CR2_CPOL | \
                                              USART_CR2_CPHA | USART_CR2_LBCL))

/*!< USART CR3 register clear Mask ((~(uint32_t)0xFFFFFCFF)) */
#define CR3_CLEAR_MASK            ((uint32_t)(USART_CR3_RTSE | USART_CR3_CTSE))

/*!< USART Interrupts mask */
#define IT_MASK                   ((uint32_t)0x000000FF)
/**
  * @brief  Initializes the USARTx peripheral according to the specified
  *         parameters in the USART_InitStruct .
  * @param  USARTx: where x can be 1 or 2 to select the USART peripheral.
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that contains
  *         the configuration information for the specified USART peripheral.
  * @retval None
  */

u8 USART_GetFlagStatus( uint32_t USART_FLAG)
{
    u8 bitstatus = 0;
    /* Check the parameters */

    if ((UCA0STAT & USART_FLAG) != 0)
    {
        bitstatus = 1;
    }
    else
    {
        bitstatus = 0;
    }
    return bitstatus;
}



void USART_ClearFlag(u8 set, uint32_t USART_FLAG)
{
    /* Check the parameters */
    if(set == 1)
    {
        UCA0STAT  |= USART_FLAG;
    }
    else
    {
        UCA0STAT  &= ~USART_FLAG;
    }
}

u8 USART_GetITStatus(uint32_t USART_IT)
{
    u8 bitstatus = 0;

    if ((IFG2 & USART_IT) != 0)
    {
        bitstatus = 1;
    }
    else
    {
        bitstatus = 0;
    }
    return bitstatus;
}

void USART_ClearITPendingBit(uint32_t USART_IT)
{
    IFG2 &= (uint32_t)USART_IT;
}


/**
  * @brief  Enables or disables the Auto Baud Rate.
  * @param  USARTx: where x can be 1 to select the USART peripheral.
  * @param  NewState: new state of the USARTx auto baud rate.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */


/**
  * @}
  */


/** @defgroup USART_Group4 Data transfers functions
 *  @brief   Data transfers functions
 *
@verbatim
 ===============================================================================
                    ##### Data transfers functions #####
 ===============================================================================
    [..] This subsection provides a set of functions allowing to manage
         the USART data transfers.
    [..] During an USART reception, data shifts in least significant bit first
         through the RX pin. When a transmission is taking place, a write instruction to
         the USART_TDR register stores the data in the shift register.
    [..] The read access of the USART_RDR register can be done using
         the USART_ReceiveData() function and returns the RDR value.
         Whereas a write access to the USART_TDR can be done using USART_SendData()
         function and stores the written data into TDR.

@endverbatim
  * @{
  */

/**
  * @brief  Transmits single data through the USARTx peripheral.
  * @param  USARTx: where x can be 1 or 2 to select the USART peripheral.
  * @param  Data: the data to transmit.
  * @retval None
  */
void USART_SendData(uint8_t Data)
{
    /* Check the parameters */
    /* Transmit Data */
    while (!(IFG2&UCA0TXIFG));    
    
    UCA0TXBUF = Data & (uint8_t)0xff;
}


/**
  * @brief  Returns the most recent received data by the USARTx peripheral.
  * @param  USARTx: where x can be 1 or 2 to select the USART peripheral.
  * @retval The received data.
  */
uint16_t USART_ReceiveData()
{
    /* Check the parameters */
    /* Receive Data */
    return (uint8_t)(UCA0RXBUF & (uint8_t)0xFF);
}

void USART1_SendData(uint8_t Data)
{
    /* Check the parameters */
    /* Transmit Data */
    while (!(UC1IFG&UCA1TXIFG));    
    
    UCA1TXBUF = Data & (uint8_t)0xff;
}
