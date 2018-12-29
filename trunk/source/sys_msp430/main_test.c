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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "msp430x24x.h"
#include "sys_def.h"
#include "lpcd.h"
#include "../sys_msp430/mcu.h"
#include "../iso14443/sl2523.h"
#include "../common/usmart.h"
#include "../common/Usart.h"


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR //1/12ms
__interrupt void Timer_A (void)
{
    TACTL &= ~TAIFG;
}

uint8_t recvdata=0,j=0,array[10];
uint16_t ty=0;
uint8_t speedflg=6;

#define _CALIB_

uchar volatile recv_data=0;
void main( void )
{
  volatile  uchar temp_value=0,temp_buf[8]={0},prev_temp_value = 0;
  volatile uchar autodly_test[1000];
  uchar card_detect = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  int upper_bound;
  int lower_bound;
  uint edge_found;
  int bound0;
  int bound1;
  double step;
  uint prev_lpcd_out;
  uchar idx[8];
  uchar wdata[64];
    init_mcu();
    //SL1523rf_init();
    _EINT();
    
    for(ty=0;ty<2000;ty++)
    {
       for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);
       for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);
       for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);
       for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);for(ty=0;ty<2000;ty++);  
    }
   
    printf("welocom to SL1523! smartlinkMicro. Co., Ltd.\r\n");

    write_reg(0x3f,0x00);
    write_reg(0x01,0x00);
    delay_1ms(100);
    write_reg(0x3f,0x00);

    spi_wr_test();

    pcd_init();
    pcd_antenna_on();

    write_reg(0x3f, 0x01);

    write_reg(0x63, 0x84);
    write_reg(0x64, 0x3f);
    
//    write_reg(0x68,0x02); // observe field strength
    write_reg(0x68,0x03); // observe field strength
//    write_reg(0x68,0x04); // comparator output
//    write_reg(0x68,0x0e); // observe threshold
//    write_reg(0x68,0x0c); // lpcd out
//    write_reg(0x68,0x06); // bg main

    while(1)
    {
    write_reg(0x3f, 0x01);
    write_reg(0x65, 0x00);
    recv_data = read_reg(0x66);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
    write_reg(0x65, 0x80);
    recv_data = read_reg(0x66);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
    write_reg(0x66, 0x80);
    recv_data = read_reg(0x66);
    write_reg(0x3f, 0x00);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);

    wdata[0] = 0x0;
    write_buf(FIFODataReg, wdata, 1);
    write_reg(CommandReg, PCD_TRANSCEIVE);
    write_reg(BitFramingReg, 0x80);
    
    write_reg(0x3f, 0x01);
    for (i = 0; i < 60; i++);
    for (i = 0; i < 10; i++)
    {
        autodly_test[i] = read_reg(0x66);
    }
    write_reg(0x3f, 0x00);

//    i = 0;
//    do
//    {
//        autodly_test[i] = read_reg(0x66);
//        i++;
//        recv_data = read_reg(ComIrqReg);
//    } while(!(recv_data & 0x40));

    delay_1ms(100);
    write_reg(CommandReg, PCD_IDLE);

    write_reg(0x3f, 0x01);
    recv_data = read_reg(0x66);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
    write_reg(0x3f, 0x00);
    for (j = 0; j < i; j++)
    {
        printf("%x\n", autodly_test[j]);
    }
    }

    pcd_antenna_off();
    
    write_reg(0x3f, 0x01);
    write_reg(0x65, 0x00);
    recv_data = read_reg(0x66);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
    write_reg(0x66, recv_data ^ 0x80);
    recv_data = read_reg(0x66);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
    write_reg(0x3f, 0x00);
    
    write_reg(TxASKReg, 0x00);  // Force100ASK = 0
    
    write_reg(0x01,0x06);
    
    write_reg(0x3f,0x1);
    write_reg(0x54,0x81);
    write_reg(0x51,0x5);   // T1
    write_reg(0x52,0x3f);  // T2 = 0x7
    write_reg(0x53,0xff);  // T3 = 0x5 & Twait = 0x3
    write_reg(0x58,0x1);

#if 0
    printf("
    
    //-----------------------------------------------------------
    // find lower edge
    //-----------------------------------------------------------
    lower_bound = 0;
    upper_bound = 13;
    
    lpcd_find_edge(lower_bound, upper_bound, LOWER_EDGE);

    //-----------------------------------------------------------
    // find upper edge
    //-----------------------------------------------------------
    lower_bound = 13;
    upper_bound = 95;

    lpcd_find_edge(lower_bound, upper_bound, UPPER_EDGE);
    
#endif
    
#if 0
    prev_lpcd_out = 0;
//    for (i = 0; i < 12; i++)
    for (i = 1; ; )
    {
        write_reg(0x3f,0x1);
        
        idx[0] = i*8+0;
        idx[1] = i*8+1;
        idx[2] = i*8+2;
        idx[3] = i*8+3;
        idx[4] = i*8+4;
        idx[5] = i*8+5;
        idx[6] = i*8+6;
        idx[7] = i*8+7;

        write_reg(0x5b,lut[idx[0]]);
        write_reg(0x5c,lut[idx[1]]);
        write_reg(0x5d,lut[idx[2]]);
        write_reg(0x5e,lut[idx[3]]);
        write_reg(0x5f,lut[idx[4]]);
        write_reg(0x60,lut[idx[5]]);
        write_reg(0x61,lut[idx[6]]);
        write_reg(0x62,lut[idx[7]]);

        write_reg(0x3f,0x0);
        write_reg(0x01,0x10);
        delay_1ms(100);
        write_reg(0x01,0x00);
        delay_1ms(100);
        write_reg(0x3f,0x01);
        for(j=0;j<8;j++)
        {
            temp_buf[j] = read_reg(0x5b+j);
            printf("reg0x%x: %x ",0x5b+j,(temp_buf[j]&0x80)>>7);
            printf("%0.2x\t",(temp_buf[j]&0x7F));
            printf("%f\r\n",voltage[idx[j]]);
        }
        printf("\r\n");
        
        recv_data = read_reg(0x66);
        printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
        write_reg(0x66, recv_data ^ 0x40);
    }
#endif
    while(1)
    {
        usmart_dev.scan();  // console
    }
}
//endfile




