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

uint8_t recvdata=0,j=0,array[10];
uint16_t ty=0;
uint8_t speedflg=6;
extern char pcd_request(u8 req_code, u8 *ptagtype);

//#define _CALIB_
//#define LPCD_DEBUG

uchar volatile recv_data=0;
void main( void )
{
    volatile  uchar temp_buf[8]={0};
    uchar card_detect = 0;
    uchar upshift_det_cnt;
    uchar downshift_det_cnt;
    uchar dc_shift = 0;
    uchar lpcd_amp_rlt;
    int i = 0;
    int j = 0;
    int k = 0;
    unsigned long wait_cnt;
    unsigned char calib_rlt[8];
    unsigned char amp;
    unsigned char amp_mask = 0x28;

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

    osc_calib();
    
    lpcd_cfg.thd_idx = 12;
    lpcd_cfg.t1 = 0x12;
    lpcd_cfg.sense = 3;
    lpcd_cfg.dc_shift_det_en = 1;
    lpcd_cfg.amp = 0x20;
    lpcd_cfg.min_amp = 0x0d;
    lpcd_cfg.max_amp = 0x30;
    lpcd_cfg.phase_offset = 4;
    lpcd_init();
    
    upshift_det_cnt = 0;
    downshift_det_cnt = 0;
    
    write_reg(0x3f,0x01);
    write_reg(0x68,0x03);
    write_reg(0x3f,0x00);
    
//  while(1)
//  {
//      for (amp = 0x02; amp < 0x40; amp++)
//      {
//          write_reg(ModGsPReg, amp ^ amp_mask);
//          write_reg(CWGsPReg, amp ^ amp_mask);
//          pcd_antenna_on();
//          delay_1ms(1);
//          pcd_antenna_off();
//      }
//  }

//  pcd_antenna_on();
//  while(1)
//  {
//      test_a(1);
//      test_b(1);
//  }
    
    while (1) {
        card_detect = 0;
        dc_shift = 0;
        lpcd_entry();
        printf("ENTER LPCD PROC!\n");
        
        wait_cnt = 0;
        i = 0;
        k = 0;
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
                    printf("idx: %0.2d, amp: %0.2x, lpcd_amp_rlt: %x\n", lpcd_cfg.thd_idx, lpcd_cfg.amp, lpcd_amp_rlt);
                    printf("\n");
#endif

                }
            }
#ifndef NOT_IRQ
            irq_flag_io = 0;
#endif
            recv_data = read_reg(0x05);
            lpcd_exit();
            printf("\nEXIT LPCD PROC!\n");

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

        write_reg(0x3f, 0x01);
        recv_data = read_reg(0x59);
        write_reg(0x3f, 0x00);

        if ((recv_data & 0x04) == 0x04)
        {
            card_detect = 1;
        }
        else if ((recv_data & 0x02) == 0x02)
        {
            dc_shift = 1;
        }
        
        if (card_detect == 1) {
            card_detect = test_a(1);
            card_detect |= test_b(1);
            if (card_detect != 1)
            {
                printf("******* CARD READ FAIL! *******\n");
                upshift_det_cnt++;
                downshift_det_cnt = 0;
                if (upshift_det_cnt == THD_ADJ_CNT)
                {
                    printf("******* DC UP SHIFT DETECT! *******\n");
                    upshift_det_cnt = 0;
                    lpcd_sen_adj();
                }
            }
            else
            {
                printf("******* CARD DETECT! *******\n");
                upshift_det_cnt = 0;
                downshift_det_cnt = 0;
            }
        }
        else if (dc_shift == 1) {
            card_detect = test_a(1);
            card_detect |= test_b(1);
            if (card_detect != 1)
            {
                printf("******* CARD READ FAIL! *******\n");
                upshift_det_cnt = 0;
                downshift_det_cnt++;
                if (downshift_det_cnt == THD_ADJ_CNT)
                {
                    printf("******* DC DOWN SHIFT DETECT! *******\n");
                    downshift_det_cnt = 0;
                    lpcd_sen_adj();
                }
            }
            else
            {
                printf("******* CARD DETECT! *******\n");
                upshift_det_cnt = 0;
                downshift_det_cnt = 0;
            }
        }
        
    }

    while(1)
    {
        usmart_dev.scan();  // console
    }
}
//endfile
