/**
  *****************************************************************************
  *                           ϵͳ����������صĺ���
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
           �������´��룬֧��printf������������Ҫѡ��use MicroLIB
******************************************************************************/

#if Printf_Support_En == 1

#ifdef __ICCARM__  //IAR֧��

/**
  *****************************************************************************
  * @Name   : �ض���out_char����
  *
  * @Brief  : none
  *
  * @Input  : ch�������ַ�
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


#else  //MDK֧��

//
//��׼����Ҫ��֧�ֺ���
//
struct __FILE
{
    int handle;

};
/* FILE is typedef�� d in stdio.h. */
FILE __stdout;
//
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
//
_sys_exit(int x)
{
    x = x;
}

/**
  *****************************************************************************
  * @Name   : �ض���fputc����
  *
  * @Brief  : none
  *
  * @Input  : ch�������ַ�
  *           *f: FILEָ��
  *
  * @Output : none
  *
  * @Return : none
  *****************************************************************************
**/
int fputc(int ch, FILE *f)
{
    uint16_t timeout = 0;

    while(USART_GetFlagStatus(USART_FLAG_TC) == RESET)  //ѭ�����ͣ�ֱ���������
    {
        timeout++;
        if (timeout > 800)
        {
            timeout = 0;
            break;
        }
    }
    USART_SendData((uint16_t)ch);  //��������

    return ch;
}

#endif

#else

/**
  *****************************************************************************
  * @Name   : ���ڷ���һ���ֽ�
  *
  * @Brief  : none
  *
  * @Input  : byte�������ַ�
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
  * @Name   : ���ڷ����ַ���
  *
  * @Brief  : none
  *
  * @Input  : *str�������ַ���
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
  * @Name   : ���ڸ�ʽ����ӡ
  *
  * @Brief  : none
  *
  * @Input  : *format����ʽ���ַ���
  *           ...:     �䳤����
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


//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���

uint8_t USART_RX_BUF[USART_REC_LEN];  //���ջ���,���USART_REC_LEN���ֽ�

uint8_t USART1_RX_BUF[20];  //���ջ���,���USART1_REC_LEN���ֽ�

//����״̬
//bit15��������ɱ�־
//bit14�����յ�0x0d
//bit13~0�����յ�����Ч�ֽ���Ŀ�����512�ֽ�

uint16_t USART_RX_STA=0;    //����״̬���

//uint16_t   cnt_irq = 0;
/**
  *****************************************************************************
  * @Name   : ����1�����жϷ������
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
    if(USART_GetITStatus( USART_IT_RXNE) != 0)  //���յ�����
    {

       // USART_ClearFlag( 0,USART_IT_RXNE);
       // USART_ClearITPendingBit( USART_IT_RXNE);
        
        res = USART_ReceiveData();  //��ȡ���յ�������
        
        if((USART_RX_STA & 0x8000) == 0)//����δ���
        {
            if(USART_RX_STA & 0x4000)//���յ���0x0d
            {
                /***********************************************
                                  �޸���������
                    ���û����ݵ�����0x0d��ʱ�������Ĵ�����ж�
                ***********************************************/

                if(res != 0x0a)
                {
                    USART_RX_BUF[USART_RX_STA & 0x3fff] = 0x0d; //���϶�ʧ��0x0d����
                    USART_RX_STA++;
                    USART_RX_BUF[USART_RX_STA & 0x3fff] = res;  //������������
                    USART_RX_STA++;
                    USART_RX_STA &= 0xbfff;                     //���0x0d��־
                }

                /***********************************************
                                      �޸����
                ***********************************************/
                else    USART_RX_STA |= 0x8000; //���������
            }
            else //��û�յ�0x0d
            {
                if(res == 0x0d) USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0x3fff] = res;
                    USART_RX_STA++;
                    if(USART_RX_STA > (USART_REC_LEN - 1))  USART_RX_STA = 0;//�������ݴ���,���¿�ʼ����
                }
            }
        }   //end ����δ���
    }   //end ���յ�����
}
////////////////
uint16_t USART1_RX_STA=0;    //����״̬���
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

