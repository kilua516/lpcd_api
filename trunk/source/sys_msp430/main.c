/*
*************************************************************
 *@Copyright (C), 2016-2020 smartlinkMicro. Co., Ltd.  All rights reserved.
 *@Filename:
 *@Complier:    IAR msp430 5.30
 *@Target OS:   NONE
 *@Target HW:   SL1523
 *@Author:      rpliu
 *@Version :    V1.0.0
 *@Date:        2018.12.25
 *@Description: LPCD demo code
 *@History:
 *@    <author>     <time>     <version >   <desc>
 *@    rpliu        20181225   V1.0.0       Initial Version
 *@    rpliu        20190115   V2.0.0       new lpcd adjust algorithm
 *@    rpliu        20190116   V3.0.0       rewrite lpcd adjust algorithm again
 *@    rpliu        20190117   V3.0.1       simplify lpcd flow

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
#include "../iso14443/Rfid_interface.h"

#define _ENABLE_LPCD_


extern volatile u8 irq_flag_io;

// Timer A0 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void gpio_irq (void)
{

    if(P1IFG & BIT2)
    {
        P1IFG = 0;
        P1IFG &= ~(BIT2);// 清除中断标志位
        irq_flag_io = 1;
    }
}


#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    TACTL &= ~TAIFG;
}

uint8_t recvdata=0;
uint16_t ty=0;

extern char pcd_request(u8 req_code, u8 *ptagtype);

//#define _CALIB_
//#define LPCD_DEBUG

uchar volatile recv_data=0;
void main( void )
{
    volatile  uchar temp_buf[8]={0};
    uchar card_detect = 0;
    uchar false_det_cnt;
    uchar lpcd_amp_rlt;
    int i = 0;
    unsigned long wait_cnt;

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

    pcd_poweron();
    delay_1ms(10);

    pcd_reset();
    delay_1ms(10);
    
    pcd_init();

    Rfid_Init();
    pcd_default_info();

#ifdef _ENABLE_LPCD_
    //------------------ LPCD INITIALIZATION -------------------
    for (i = 0; i < 4; i++)
    {
        lpcd_cfg.idx[i] = 0;
    }
    for (; i < 8; i++)
    {
        lpcd_cfg.idx[i] = 1;
    }
    lpcd_cfg.t1 = 0x12;
    lpcd_cfg.sense = 2;
    lpcd_cfg.default_amp = 0x20;
    lpcd_cfg.amp = 0x20;
    lpcd_cfg.min_amp = 0x0d;
    lpcd_cfg.max_amp = 0x3f;
    lpcd_cfg.phase_offset = 0;
    lpcd_init();
    
    false_det_cnt = 0;
    //---------------- LPCD INITIALIZATION END -----------------
#endif
    
    write_reg(0x3f,0x01);
    write_reg(0x68,0x02);
    write_reg(0x3f,0x00);
    
#ifndef _ENABLE_LPCD_
    pcd_antenna_on();
    while(1)
    {
//      test_a(1);
//      test_b(1);
        test_auth();
    }
#endif
    
#ifdef _ENABLE_LPCD_
    while (1) {
        lpcd_entry();
        printf("ENTER LPCD PROC!\n");
        
        wait_cnt = 0;
        i = 0;
        while (1) {
#ifdef NOT_IRQ
            while(INT_PIN == 0)
#else
            while (irq_flag_io == 0)
#endif
            {
                wait_cnt++;
                if (wait_cnt == 500000) {
                    wait_cnt = 0;
                    printf(".");
#ifdef LPCD_DEBUG
                    write_reg(0x01,0x00);
                    delay_1ms(1);
                    write_reg(0x3f,0x01);
                    lpcd_amp_rlt = 0;
                    for (i = 0; i < 8; i++)
                    {
                        temp_buf[i] = read_reg(0x5b+i);
                        printf("reg0x%x: %x ",0x5b+i,(temp_buf[i]&0x80)>>7);
                        printf("%0.2x\t",(temp_buf[i]&0x7F));
                        lpcd_amp_rlt >>= 1;
                        lpcd_amp_rlt |= (read_reg(0x5b+i) & 0x80);
                    }
                    write_reg(0x3f,0x00);
                    write_reg(0x01,0x10);
                    LPCD_AMP_TEST_INFO("main", lpcd_cfg.amp, lpcd_amp_rlt)
                    printf("\n");
#endif

                }
            }
#ifndef NOT_IRQ
            irq_flag_io = 0;
#endif
            lpcd_exit();
            printf("\nEXIT LPCD PROC!\n");

            recv_data = read_reg(0x05);
            if ((recv_data & 0x20) == 0x20) {
                write_reg(0x05,0x20);
                break;
            }
            else
            {
                write_reg(0x04,0x7e);
                write_reg(0x05,0x7e);
                lpcd_entry();
                printf("ENTER LPCD PROC!\n");
            }
        }

//      lpcd_amp_rlt = 0;
//      write_reg(0x3f,0x01);
//      for (i = 0; i < 8; i++)
//      {
//          temp_buf[i] = read_reg(0x5b+i);
//          printf("%0.2x\t",(temp_buf[i]));
//          lpcd_amp_rlt >>= 1;
//          lpcd_amp_rlt |= (read_reg(0x5b+i) & 0x80);
//      }
//      write_reg(0x3f,0x00);
//      printf("\n");
//      LPCD_AMP_TEST_INFO("check lpcd result", lpcd_cfg.amp, lpcd_amp_rlt)
//
//      lpcd_amp_rlt = lpcd_amp_test(lpcd_cfg.amp);
//      LPCD_AMP_TEST_INFO("main", lpcd_cfg.amp, lpcd_amp_rlt)

        card_detect = test_a(1);
        card_detect |= test_b(1);
        if (card_detect != 1)
        {
            printf("******* CARD READ FAIL! *******\n");
            false_det_cnt++;
            if (false_det_cnt == THD_ADJ_CNT)
            {
                printf("********** LPCD AUTO ADJUST *******\n");
                false_det_cnt = 0;
                lpcd_sen_adj();
            }
        }
        else
        {
            printf("******* CARD DETECT! *******\n");
            false_det_cnt = 0;
        }
    }

    while(1)
    {
        usmart_dev.scan();  // console
    }
#endif
}
//endfile
