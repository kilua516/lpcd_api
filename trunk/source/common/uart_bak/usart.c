/**
  *****************************************************************************
  *                           系统串口设置相关的函数
  *
  *                       (C) Copyright 2000-2020, ***
  *                            All Rights Reserved
  *****************************************************************************
**/
#include "msp430x24x.h"
#include "usart.h"
#include <stdio.h>
#include "drv_usart.h"

#if Printf_Support_En == 0
#include <stdarg.h>
#endif


/******************************************************************************
           加入以下代码，支持printf函数，而不需要选择use MicroLIB
******************************************************************************/

#if Printf_Support_En == 1

#ifdef __ICCARM__  //IAR支持

/**
  *****************************************************************************
  * @Name   : 重定义out_char函数
  *
  * @Brief  : none
  *
  * @Input  : ch：发送字符
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
int putchar(int c)
{
    uint16_t timeout = 0;
   
    USART_SendData((uint8_t)c);
    while(USART_GetFlagStatus( USART_FLAG_TC) == 0) 
    {
        timeout++;
        if (timeout > 800)
        {
            timeout = 0;
            break;
        }
    }

    return c;
}


#else  //MDK支持

//
//标准库需要的支持函数
//
struct __FILE
{
    int handle;

};
/* FILE is typedef’ d in stdio.h. */
FILE __stdout;
//
//定义_sys_exit()以避免使用半主机模式
//
_sys_exit(int x)
{
    x = x;
}

/**
  *****************************************************************************
  * @Name   : 重定义fputc函数
  *
  * @Brief  : none
  *
  * @Input  : ch：发送字符
  *           *f: FILE指针
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
int fputc(int ch, FILE *f)
{
    uint16_t timeout = 0;

    while(USART_GetFlagStatus(USART_FLAG_TC) == RESET)  //循环发送，直到发送完毕
    {
        timeout++;
        if (timeout > 800)
        {
            timeout = 0;
            break;
        }
    }
    USART_SendData((uint16_t)ch);  //发送数据

    return ch;
}

#endif

#else

/**
  *****************************************************************************
  * @Name   : 串口发送一个字节
  *
  * @Brief  : none
  *
  * @Input  : byte：发送字符
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void uart_sendbyte(uint8_t byte)
{
    while(USART_GetFlagStatus( USART_FLAG_TC) == RESET);
    USART_SendData( byte);
    while(USART_GetFlagStatus( USART_FLAG_TC) == RESET);
}

/**
  *****************************************************************************
  * @Name   : 串口发送字符串
  *
  * @Brief  : none
  *
  * @Input  : *str：发送字符串
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void uart_senfstring(uint8_t * str)
{
    while(USART_GetFlagStatus( USART_FLAG_TC) == RESET);
    while (*str != '\0')
    {
        USART_SendData( *(str++));
        while(USART_GetFlagStatus( USART_FLAG_TC) == RESET);
    }
}

/**
  *****************************************************************************
  * @Name   : 串口格式化打印
  *
  * @Brief  : none
  *
  * @Input  : *format：格式化字符串
  *           ...:     变长参数
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
void uart_printf(char *format, ...)
{
    va_list ap;
    char string[512];

    va_start(ap, format);
    vsprintf(string, format, ap);
    va_end(ap);

    uart_senfstring((uint8_t *)string);
}

#endif

/*************************************   end   *******************************/


//注意,读取USARTx->SR能避免莫名其妙的错误

uint8_t USART_RX_BUF[USART_REC_LEN];  //接收缓冲,最大USART_REC_LEN个字节

uint8_t USART1_RX_BUF[20];  //接收缓冲,最大USART1_REC_LEN个字节

//接收状态
//bit15：接收完成标志
//bit14：接收到0x0d
//bit13~0：接收到的有效字节数目，最大512字节

uint16_t USART_RX_STA=0;    //接收状态标记

//uint16_t   cnt_irq = 0;
/**
  *****************************************************************************
  * @Name   : 串口1接收中断服务程序
  *
  * @Brief  : none
  *
  * @Input  : none
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
u8 rdbuf[128]={0};
   uint8_t i;
//void USART1_IRQHandler(void)
#if 1
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    uint8_t res;
 
#if 0
    if (USART_GetFlagStatus( USART_FLAG_PE) == 0)
    {
        USART_ReceiveData();   
        USART_ClearFlag(1, USART_FLAG_PE);
    }

    if (USART_GetFlagStatus( USART_FLAG_ORE) == 0)
    {
        res = USART_ReceiveData();
        USART_ClearFlag( 1,USART_FLAG_ORE);
    }

    if (USART_GetFlagStatus( USART_FLAG_FE) != 0)
    {
        USART_ReceiveData();
        USART_ClearFlag( 0,USART_FLAG_FE);
    }
#endif
    if(USART_GetITStatus( USART_IT_RXNE) != 0)  //接收到数据
    {

       // USART_ClearFlag( 0,USART_IT_RXNE);
       // USART_ClearITPendingBit( USART_IT_RXNE);
        
        res = USART_ReceiveData();  //读取接收到的数据
        
        if((USART_RX_STA & 0x8000) == 0)//接收未完成
        {
            if(USART_RX_STA & 0x4000)//接收到了0x0d
            {
                /***********************************************
                                  修改内容如下
                    当用户数据当中有0x0d的时候修正的错误的判断
                ***********************************************/

                if(res != 0x0a)
                {
                    USART_RX_BUF[USART_RX_STA & 0x3fff] = 0x0d; //补上丢失的0x0d数据
                    USART_RX_STA++;
                    USART_RX_BUF[USART_RX_STA & 0x3fff] = res;  //继续接收数据
                    USART_RX_STA++;
                    USART_RX_STA &= 0xbfff;                     //清除0x0d标志
                }

                /***********************************************
                                      修改完成
                ***********************************************/
                else    USART_RX_STA |= 0x8000; //接收完成了
            }
            else //还没收到0x0d
            {
                if(res == 0x0d) USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0x3fff] = res;
                    USART_RX_STA++;
                    if(USART_RX_STA > (USART_REC_LEN - 1))  USART_RX_STA = 0;//接收数据错误,重新开始接收
                }
            }
        }   //end 接收未完成
    }   //end 接收到数据
}
////////////////
uint16_t USART1_RX_STA=0;    //接收状态标记
#pragma vector=USCIAB1RX_VECTOR
__interrupt void USCI1RX_ISR(void)
{
  if ((UC1IFG&UCA1RXIFG))
  {
    USART1_RX_BUF[0] = UCA1RXBUF;                     // TX -> RXed character
    USART1_RX_STA =1;
  }
//     while (!(UC1IFG&UCA1TXIFG));               // USCI_A1 TX buffer ready?
 
//     UCA1TXBUF = UCA1RXBUF;                     // TX -> RXed character

}
#endif

