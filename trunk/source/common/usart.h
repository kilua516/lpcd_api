/**
  *****************************************************************************
  *                           ϵͳ����������صĺ���
  *
  *                       (C) Copyright 2000-2020, ***
  *                            All Rights Reserved
  *****************************************************************************
**/

#ifndef _usart_h
#define _usart_h


/**
  ****************************** Support C++ **********************************
**/
#ifdef __cplusplus
	extern "C"{
#endif		
/**
  *****************************************************************************
**/
		

/******************************************************************************
                               �ⲿ����ͷ�ļ�                        
******************************************************************************/

#include <stdio.h>	//printf����ͷ�ļ�

#include "../sys_msp430/sys_def.h"

/******************************************************************************
                         ���崮���Ƿ�����ⲿ����
******************************************************************************/

#define Printf_Support_En                        1  //�Ƿ�֧��C��׼��printf��������
                                                    // 0: ��֧��
                                                    // 1: ֧��
#define __ICCARM__              1

#define EN_USART1_RX                             1  //���ڽ���ʹ�ܡ�0����ֹ����
                                                    //             1����������
#define USART_REC_LEN                            200  //�����������ֽ���
#define USART1_REC_LEN                            20   //����uart1�������ֽ���

/******************************************************************************
                              �ⲿ���ù��ܺ���
******************************************************************************/


extern uint8_t  USART_RX_BUF[USART_REC_LEN];  //���ջ���,���USART_REC_LEN���ֽ�
extern uint16_t USART_RX_STA;	//����״̬���

extern uint8_t  USART1_RX_BUF[20];  //���ջ���,���USART_REC_LEN���ֽ�
extern uint16_t USART1_RX_STA;	//����״̬���


void uart_init                                   (uint32_t bound);  //��ʼ��IO ����

#if Printf_Support_En == 1
void uart_sendbyte                               (uint8_t byte);  //���ڷ���һ���ֽ�
void uart_senfstring                             (uint8_t * str);  //���ڷ����ַ���
void uart_printf                                 (char *format, ...);  //���ڸ�ʽ����ӡ
#endif


/**
  ****************************** Support C++ **********************************
**/
#ifdef __cplusplus
	}
#endif
/**
  *****************************************************************************
**/


#endif  /* end usart.h */
