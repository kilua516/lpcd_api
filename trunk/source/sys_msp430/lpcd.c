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
 *@Description: LPCD api code
 *@History:
 *@    <author>     <time>     <version >   <desc>
 *@    rpliu        20181225   V1.0.0       Initial Version

*************************************************************
*/

#include <stdio.h>
#include "sys_def.h"
#include "lpcd.h"
#include "../iso14443/sl2523.h"

double voltage[] ={1.69    , // 0
                   1.73    , // 1
                   1.77    , // 2
                   1.81    , // 3
                   1.91    , // 4
                   1.9333  , // 5
                   2.0267  , // 6
                   2.05    , // 7
                   2.0933  , // 8
                   2.12    , // 9
                   2.1466  , // 10
                   2.1733  , // 11
                   2.1867  , // 12
                   2.2066  , // 13
                   2.2267  , // 14
                   2.2534  , // 15
                   2.28    , // 16
                   2.3067  , // 17
                   2.345   , // 18
                   2.3733  , // 19
                   2.395   , // 20
                   2.415   , // 21
                   2.435   , // 22
                   2.465   , // 23
                   2.485   , // 24
                   2.515   , // 25
                   2.536   , // 26
                   2.555   , // 27
                   2.592   , // 28
                   2.608   , // 29
                   2.636   , // 30
                   2.656   , // 31
                   2.688   , // 32
                   2.704   , // 33
                   2.7367  , // 34
                   2.7633  , // 35
                   2.7875  , // 36
                   2.8034  , // 37
                   2.83    , // 38
                   2.8675  , // 39
                   2.885   , // 40
                   2.9075  , // 41
                   2.9275  , // 42
                   2.9525  , // 43
                   2.9875  };// 44


unsigned char lut[] ={0x28, // 1.69    0 
                      0x38, // 1.73    1 
                      0x48, // 1.77    2 
                      0x58, // 1.81    3 
                      0x68, // 1.91    4 
                      0x71, // 1.9333  5 
                      0x61, // 2.0267  6 
                      0x78, // 2.05    7 
                      0x51, // 2.0933  8 
                      0x41, // 2.12    9 
                      0x31, // 2.1466  10
                      0x21, // 2.1733  11
                      0x11, // 2.1867  12
                      0x09, // 2.2066  13
                      0x29, // 2.2267  14
                      0x39, // 2.2534  15
                      0x49, // 2.28    16
                      0x59, // 2.3067  17
                      0x62, // 2.345   18
                      0x69, // 2.3733  19
                      0x52, // 2.395   20
                      0x42, // 2.415   21
                      0x32, // 2.435   22
                      0x12, // 2.465   23
                      0x1A, // 2.485   24
                      0x3A, // 2.515   25
                      0x63, // 2.536   26
                      0x5A, // 2.555   27
                      0x43, // 2.592   28
                      0x33, // 2.608   29
                      0x03, // 2.636   30
                      0x2B, // 2.656   31
                      0x4B, // 2.688   32
                      0x5B, // 2.704   33
                      0x24, // 2.7367  34
                      0x2C, // 2.7633  35
                      0x75, // 2.7875  36
                      0x4C, // 2.8034  37
                      0x5C, // 2.83    38
                      0x35, // 2.8675  39
                      0x05, // 2.885   40
                      0x3D, // 2.9075  41
                      0x5D, // 2.9275  42
                      0x6D, // 2.9525  43
                      0x7D};// 2.9875  44

lpcd_cfg_t lpcd_cfg;

void lpcd_find_edge(int lower_bound, int upper_bound, int *edge)
{
    int i = 0;
    int l_bound;
    int u_bound;
    int bound_upper;
    int bound_lower;
    unsigned int prev_lpcd_out;
    unsigned char calib_rlt[8];
    unsigned char idx[8];
    int edge_exist;
    
    u_bound = upper_bound;
    l_bound = lower_bound;
    do
    {
        while(1)
        {
            edge_exist = 0;
            
            do_lpcd_calib(u_bound, l_bound, calib_rlt, idx);

            // to deal with large signal that larger
            // than voltage[INDEX_NUM]
            if (u_bound == INDEX_NUM)
            {
                for (i = 0; i < 8; i++)
                {
                    if (calib_rlt[i] != 0)
                        break;
                }
                if (i == 8)
                {
                    bound_upper = INDEX_NUM;
                    bound_lower = INDEX_NUM;
                    edge_exist = 1;
                    break;
                }
            }
    
            // normal edge searching
            prev_lpcd_out = calib_rlt[0];
            for (i = 1; i < 8; i++)
            {
                if (prev_lpcd_out != calib_rlt[i])
                {
                    bound_upper = idx[i];
                    bound_lower = (i == 0) ? bound_upper : idx[i-1];
                    edge_exist = 1;
                    break;
                }
                prev_lpcd_out = calib_rlt[i];
            }
            if (edge_exist == 1)
            {
                break;
            }
        }
        
        if ((idx[7] - idx[0]) <= 7)
        {
            *edge = bound_upper;
            break;
        }
        else
        {
            u_bound = (u_bound >= (bound_upper + 2)) ? (bound_upper + 2) : u_bound;
            l_bound = (l_bound <= (bound_lower - 2)) ? (bound_lower - 2) : l_bound;
        }
    }
    while (1);
    
#ifdef LPCD_DEBUG
    printf("edge: %d\n", *edge);
#endif
}

void do_lpcd_calib(int upper_bound, int lower_bound, unsigned char *calib_rlt, unsigned char *idx)
{
    int i = 0;
    double step;
    volatile unsigned char temp_buf[8];
    
    step = ((upper_bound - lower_bound) / 7.0);
    
    for (i = 0; i < 8; i++)
    {
        idx[i] = (uint)(step * i + lower_bound + 0.5);
        idx[i] = (idx[i] > upper_bound) ? upper_bound :
                 (idx[i] < lower_bound) ? lower_bound :
                                          idx[i];
    }
    
    write_reg(0x3f, 0x01);
    for (i = 0; i < 8; i++)
    {
        write_reg(0x5b+i, lut[idx[i]]);
    }

    write_reg(0x3f,0x0);
    write_reg(0x01,0x10);
    delay_1ms(50);
    write_reg(0x01,0x00);
    delay_1ms(1);
    write_reg(0x3f,0x01);
    for (i = 0; i < 8; i++)
    {
        temp_buf[i] = read_reg(0x5b+i);
        calib_rlt[i] = (temp_buf[i]&0x80)>>7;
#ifdef LPCD_DEBUG
        printf("reg0x%x: %x ",0x5b+i,(temp_buf[i]&0x80)>>7);
        printf("%0.2x\t",(temp_buf[i]&0x7F));
        printf("%f\t",voltage[idx[i]]);
        printf("%d\n",idx[i]);
    }
    printf("\n");
#else
    }
#endif
}

unsigned char lpcd_amp_test(unsigned char amp)
{
    unsigned char reg_14;
    unsigned char reg_15;
    unsigned char reg_28;
    unsigned char reg_29;
    unsigned char reg_65;
    unsigned char reg_66;
    unsigned char calib_rlt;
    unsigned char recv_data;
    unsigned char i;

    reg_14 = read_reg(0x14);
    reg_15 = read_reg(0x15);
    reg_28 = read_reg(0x28);
    reg_29 = read_reg(0x29);
    write_reg(TxASKReg, 0x00);  // Force100ASK = 0
    write_reg(TxControlReg, 0x80);
    write_reg(CWGsPReg, amp ^ 0x28);
    write_reg(ModGsPReg, amp ^ 0x28);

    write_reg(0x3f,0x01);

    reg_65 = read_reg(0x65);
    reg_66 = read_reg(0x66);
    write_reg(0x65, 0x00);
    write_reg(0x66, lpcd_cfg.phase);

    write_reg(0x54,0x81);         // lpcd calib mode

    write_reg(0x51,0x00);         // T1

    write_reg(0x3f,0x00);
    write_reg(0x05,0x20);
    write_reg(0x01,0x10);         // enter lpcd calib mode

    // wait lpcd calib done
    do
    {
        recv_data = read_reg(0x05);
    }
    while ((recv_data & 0x20) != 0x20);

    write_reg(0x01,0x00);
    delay_1ms(1);

    write_reg(0x05,0x20);
    write_reg(0x3f,0x01);

    calib_rlt = 0;
    for (i = 0; i < 8; i++)
    {
        calib_rlt >>= 1;
        calib_rlt |= (read_reg(0x5b+i) & 0x80);
//      printf("reg_%x: %x\n", 0x5b+i, read_reg(0x5b+i));
    }
    write_reg(0x3f,0x00);

    write_reg(0x3f,0x01);
    write_reg(0x65,reg_65);
    write_reg(0x66,reg_66);
    write_reg(0x51,lpcd_cfg.t1);

    if (lpcd_cfg.dc_shift_det_en == 0)
        write_reg(0x54,0x82);
    else
        write_reg(0x54,0x92);

    write_reg(0x3f,0x00);
    write_reg(TxASKReg, reg_15);
    write_reg(TxControlReg, reg_14 | 0x03);
    write_reg(ModGsPReg, reg_29);
    write_reg(CWGsPReg, reg_28);

    return calib_rlt;
}

void osc_calib()
{
    write_reg(0x01,0x06);
    delay_1ms(1);
}

void lpcd_calib(unsigned char *idx)
{
    int i;
    volatile uchar temp_buf[8];
    int edge_without_card;
    int edge_with_card;
    double step;
    unsigned char volatile recv_data;
    unsigned char wdata;
    unsigned char calib_rlt[8];

#if 0
    clear_bit_mask(Status2Reg, BIT3);
    write_reg(RxSelReg, 0x88);//RxWait
    write_reg(CWGsPReg, 0x17);  //
    write_reg(RFCfgReg, 0x38); //
    write_reg(ControlReg, 0x10);  //default val
    write_reg(TxASKReg, 0x40);//15  //typeA
    write_reg(TxModeReg, 0x00);//12 //Tx Framing A
    write_reg(RxModeReg, 0x00);//13 //Rx framing A

    pcd_antenna_on();
    delay_1ms(2);

    write_reg(0x3f, 0x01);
    write_reg(0x66, 0x80);
    write_reg(0x3f, 0x00);

    wdata = 0x0;
    write_reg(FIFODataReg, wdata);
    write_reg(CommandReg, PCD_TRANSCEIVE);
    write_reg(BitFramingReg, 0x80);

    do
    {
        recv_data = read_reg(ComIrqReg);
    } while(!(recv_data & 0x40));
    clear_bit_mask(ComIrqReg, 0x40);

    delay_1ms(100);
    write_reg(CommandReg, PCD_IDLE);

    write_reg(0x3f, 0x01);
#ifdef LPCD_DEBUG
    recv_data = read_reg(0x66);
    printf("EXT_RXCLK_CTRL: 0x%x\n", recv_data);
#endif
    write_reg(0x3f, 0x00);
    pcd_antenna_off();
#endif
    
    write_reg(0x3f, 0x01);
    write_reg(0x65, 0x00);
//  recv_data = read_reg(0x66);
//  write_reg(0x66, recv_data ^ 0x80 ^ 0x40);
    write_reg(0x66, 0x15);
    write_reg(0x3f, 0x00);

    write_reg(TxASKReg, 0x00);  // Force100ASK = 0

    lpcd_calib_callback(0);

    write_reg(0x3f,0x1);
    write_reg(0x54,0x81);
    write_reg(0x51,0x0);   // T1
    write_reg(0x52,0x1f);  // T2 = 0x7
    write_reg(0x53,0xa3);  // T3 = 0x5 & Twait = 0x3

    lpcd_find_edge(0, INDEX_NUM, &edge_without_card);
    
    lpcd_calib_callback(1);
    
    lpcd_find_edge(0, INDEX_NUM, &edge_with_card);

    lpcd_calib_callback(2);
#ifdef LPCD_DEBUG    
    printf("edge_without_card: %d\n", edge_without_card);
    printf("edge_with_card   : %d\n", edge_with_card);
#endif

    step = (edge_with_card - edge_without_card) / 5.0;
//  idx[0] = (uint)(edge_without_card - 1.0 * step) > 0 ? (uint)(edge_without_card - 1.0 * step) : 0;
    idx[0] = edge_without_card - 1;
    idx[1] = edge_without_card;
    idx[2] = (uint)(edge_without_card + 1.0 * step);
    idx[3] = (uint)(edge_without_card + 2.0 * step);
    idx[4] = (uint)(edge_with_card - 2.0 * step);
    idx[5] = (uint)(edge_with_card - 1.0 * step);
    idx[6] = edge_with_card;
    idx[7] = (uint)(edge_with_card + 1.0 * step) < INDEX_NUM ? (uint)(edge_with_card + 1.0 * step) : INDEX_NUM;

#ifdef LPCD_DEBUG    
    for (i = 0; i < 8; i++)
    {
        printf("idx[%d] = %d\n", i, idx[i]);
    }
#endif
}

void lpcd_calib_callback(unsigned char proc)
{
    if (proc == 0)
    {
        printf("Remove Card\n");
        delay_1ms(1000);
    }
    else if (proc == 1)
    {
        printf("Put Card In The Reader Area\n");
        delay_1ms(1000);
    }
    else if (proc == 2)
    {
        printf("Auto Calib Done\n");
    }
}

unsigned char phase_calib()
{
    unsigned char reg_14;
    unsigned char reg_15;
    unsigned char reg_29;
    unsigned char reg_65;
    unsigned char reg_66;
    unsigned char recv_data;
    unsigned char idx[8];
    unsigned char ph;
    unsigned char ph_mask;
    unsigned char calib_rlt;
    unsigned char amp[2];
    unsigned char i;

    write_reg(0x3f,0x00);

    reg_14 = read_reg(TxControlReg);
    reg_15 = read_reg(TxASKReg);
    reg_29 = read_reg(ModGsPReg);

    write_reg(TxASKReg, 0x00);  // Force100ASK = 0
    write_reg(TxControlReg, 0x80);
    write_reg(ModGsPReg, read_reg(CWGsPReg));

    write_reg(0x3f,0x01);

    write_reg(0x54,0x81);       // lpcd calib mode

    write_reg(0x51,0x00);   // T1
    write_reg(0x52,0x1f);   // T2 = 0x1f
    write_reg(0x53,0xa3);   // T3 = 0x5 & Twait = 0x3

    reg_65 = read_reg(0x65);
    reg_66 = read_reg(0x66);

//    printf("auto delay phase: %x\n", reg_66);

    write_reg(0x65,0x00);   // disable auto phase

    for (i = 0; i < 8; i++)
    {
        idx[i] = (unsigned char)((INDEX_NUM-1) * i / 7);
    }

    // set threshold
    for (i = 0; i < 8; i++)
    {
        write_reg(0x5b+i, lut[idx[i]]);
    }

    for (ph = 0; ph < 2; ph++)
    {
        write_reg(0x3f,0x01);

        ph_mask = (ph == 0) ? 0x80 : 0xc0;
        write_reg(0x66,reg_66 ^ ph_mask);   // set phase

        write_reg(0x3f,0x00);
        write_reg(0x05,0x20);
        write_reg(0x01,0x10);         // enter lpcd calib mode

        // wait lpcd calib done
        do
        {
            recv_data = read_reg(0x05);
        }
        while ((recv_data & 0x20) != 0x20);

        write_reg(0x01,0x00);
        delay_1ms(1);

        write_reg(0x05,0x20);
        write_reg(0x3f,0x01);

//        for (i = 0; i < 8; i++)
//        {
//            calib_rlt = read_reg(0x5b+i) & 0x80;
//            printf("reg:%x, calib_rlt[%d]: %d\n", read_reg(0x5b+i), i, calib_rlt >> 7);
//        }

        for (i = 0; i < 8; i++)
        {
            calib_rlt = read_reg(0x5b+i) & 0x80;
            if (calib_rlt != 0)
            {
                break;
            }
        }

        amp[ph] = i;
    }

    write_reg(0x3f,0x01);
    write_reg(0x65,reg_65);
    write_reg(0x66,reg_66);

    write_reg(0x3f,0x00);
    write_reg(TxASKReg, reg_15);
    write_reg(TxControlReg, reg_14 | 0x03);
    write_reg(ModGsPReg, reg_29);

    if (amp[0] < amp[1])
        return (reg_66 ^ 0xc0);
    else if (amp[1] > amp[0])
        return (reg_66 ^ 0x80);
    else
        return 0xff;
}

void lpcd_init()
{
    uint8_t recv_data;
    int i;
#ifdef LPCD_DEBUG
    volatile unsigned char temp_value;
#endif
    lpcd_cfg.phase = phase_calib();
    lpcd_cfg.phase += lpcd_cfg.phase_offset;
//    printf("lpcd_cfg.phase: %x\n", lpcd_cfg.phase);

    write_reg(0x3f,0x01);

    if (lpcd_cfg.dc_shift_det_en == 0)
        write_reg(0x54,0x82);
    else
        write_reg(0x54,0x92);

    write_reg(0x51,lpcd_cfg.t1);        // T1
#ifdef LPCD_DEBUG    
    temp_value = read_reg(0x51);
    printf("0x51: %x\r\n",temp_value);
#endif

    write_reg(0x52,0x1f);  // T2 = 0x1f
#ifdef LPCD_DEBUG    
    temp_value = read_reg(0x52);
    printf("0x52: %x\r\n",temp_value);
#endif

    write_reg(0x53,0xa3);  // T3 = 0x5 & Twait = 0x3
#ifdef LPCD_DEBUG    
    temp_value = read_reg(0x53);
    printf("0x53: %x\r\n",temp_value);
#endif
    
    for (i = 0; i < 8; i++)
    {
        write_reg(0x5b+i, lut[lpcd_cfg.thd_idx+i]);
    }

#ifdef LPCD_DEBUG    
    printf("initial lpcd parameters:\r\n");
    for (i=0;i<8;i++)
    {
        temp_value = read_reg(0x5b+i);
        printf("0x%0.2x: %0.2x\r\n",0x5b+i, temp_value);
    }
#endif

    // set card detect threshold
    switch (lpcd_cfg.sense)
    {
        case 0:
            write_reg(0x55, 0x80);
            write_reg(0x56, 0xff);
            break;
        case 1:
            write_reg(0x55, 0xc0);
            write_reg(0x56, 0xfe);
            break;
        default:
            write_reg(0x55, 0xe0);
            write_reg(0x56, 0xfc);
            break;
    }
    
    write_reg(0x3f,0x00);

    // enable IRQ
    recv_data = read_reg(0x03);
    write_reg(0x03, recv_data | 0xA0);
}

unsigned char slm_reg_14, slm_reg_15, slm_reg_28, slm_reg_29, slm_reg_65, slm_reg_66;
// set 0x14/0x15/0x28/0x29/0x65/0x66 and turn off rf field
void lpcd_entry()
{
    slm_reg_14 = read_reg(0x14);
    slm_reg_15 = read_reg(0x15);
    slm_reg_28 = read_reg(0x28);
    slm_reg_29 = read_reg(0x29);
    write_reg(TxASKReg, 0x00);  // Force100ASK = 0
    write_reg(TxControlReg, 0x80);
    write_reg(CWGsPReg, lpcd_cfg.amp ^ 0x28);
    write_reg(ModGsPReg, lpcd_cfg.amp ^ 0x28);
    write_reg(0x3f, 0x01);
    slm_reg_65 = read_reg(0x65);
    slm_reg_66 = read_reg(0x66);
    write_reg(0x65, 0x00);
    write_reg(0x66, lpcd_cfg.phase);
    write_reg(0x3f, 0x00);
    set_bit_mask(DivIEnReg, BIT7 | BIT5);// enable LPCD IRQ

    write_reg(0x01,0x10);
    
#ifndef NOT_IRQ
    ASSERT_SPI_CLK_LOW;
#endif
}

// restore 0x14/0x15/0x28/0x29/0x65/0x66
void lpcd_exit()
{
#ifndef NOT_IRQ
    RELEASE_SPI_CLK_LOW;
#endif
    write_reg(0x01,0x00);
    delay_1ms(10);
    
    write_reg(TxASKReg, slm_reg_15);
    write_reg(TxControlReg, slm_reg_14 | 0x03);
    write_reg(CWGsPReg, slm_reg_28);
    write_reg(ModGsPReg, slm_reg_29);
    write_reg(0x3f, 0x01);
    write_reg(0x65, slm_reg_65);
    write_reg(0x66, slm_reg_66);
    write_reg(0x3f, 0x00);
    clear_bit_mask(DivIEnReg, BIT5);
}

int lpcd_sen_adj()
{
    unsigned char amp_mask = 0x28;
    unsigned char amp;
    unsigned char lpcd_amp_target;
    unsigned char lpcd_amp_rlt;
    unsigned char i;

    lpcd_cfg.phase = phase_calib();
    lpcd_cfg.phase += lpcd_cfg.phase_offset;

    amp = lpcd_cfg.amp;
  
    lpcd_amp_target = 0xf8;
  
    // large step adjust threshold
    while (1)
    {
        write_reg(0x3f, 0x01);
        for (i = 0; i < 8; i++)
        {
            write_reg(0x5b+i, lut[lpcd_cfg.thd_idx+i]);
        }
        write_reg(0x3f, 0x00);

        lpcd_amp_rlt = lpcd_amp_test(amp);
        if (lpcd_amp_rlt == 0xff)
        {
            if (lpcd_cfg.thd_idx >= 3)
            {
                lpcd_cfg.thd_idx -= 3;
            }
            else
                return -1;
        }
        else if (lpcd_amp_rlt == 0x00)
        {
            if ((lpcd_cfg.thd_idx+7) < (INDEX_NUM-3))
            {
                lpcd_cfg.thd_idx += 3;
            }
            else
                return -1;
        }
        else
        {
            break;
        }
    }

    while (1)
    {
        lpcd_amp_rlt = lpcd_amp_test(amp);

        if (lpcd_amp_rlt > lpcd_amp_target)   // current amp smaller than target amp
        {
            if (lpcd_amp_search(lpcd_amp_target, amp, 1) == 0)
                return 0;
          
            if (lpcd_cfg.thd_idx > 0)
            {
                amp = lpcd_cfg.amp;
                lpcd_cfg.thd_idx--;
          
                write_reg(0x3f, 0x01);
                for (i = 0; i < 8; i++)
                {
                    write_reg(0x5b+i, lut[lpcd_cfg.thd_idx+i]);
                }
                write_reg(0x3f, 0x00);
          
                if (lpcd_amp_search(lpcd_amp_target, amp, 0) == 0)
                    return 0;
            }
            else
                return -1;
        }
        else                                  // current amp larger than target amp
        {
            if (lpcd_amp_search(lpcd_amp_target, amp, 0) == 0)
                return 0;
          
            if ((lpcd_cfg.thd_idx+7) < (INDEX_NUM-1))
            {
                amp = lpcd_cfg.amp;
                lpcd_cfg.thd_idx++;
          
                write_reg(0x3f, 0x01);
                for (i = 0; i < 8; i++)
                {
                    write_reg(0x5b+i, lut[lpcd_cfg.thd_idx+i]);
                }
                write_reg(0x3f, 0x00);
          
                if (lpcd_amp_search(lpcd_amp_target, amp, 1) == 0)
                    return 0;
            }
            else
                return -1;
        }
    }

    return -1;
}

int lpcd_amp_search(unsigned char lpcd_amp_target, unsigned char amp, unsigned char dir)
{
    unsigned char max_amp;
    unsigned char min_amp;
    unsigned char cur_amp;
    unsigned char lpcd_amp_rlt;

    max_amp = dir ? lpcd_cfg.max_amp : amp;
    min_amp = dir ? amp : lpcd_cfg.min_amp;

    lpcd_amp_rlt = lpcd_amp_test(max_amp);
    if (lpcd_amp_rlt > lpcd_amp_target)     // max_amp too small
        return -1;

    lpcd_amp_rlt = lpcd_amp_test(min_amp);
    if (lpcd_amp_rlt < lpcd_amp_target)     // min_amp too large
        return -1;

    while (1)
    {
        cur_amp = (max_amp + min_amp) / 2;
        lpcd_amp_rlt = lpcd_amp_test(cur_amp);
        printf("idx: %0.2d, amp: %0.2x, lpcd_amp_rlt: %x\n", lpcd_cfg.thd_idx, cur_amp, lpcd_amp_rlt);
        if (lpcd_amp_rlt > lpcd_amp_target) // cur_amp smaller than target_amp
            min_amp = cur_amp;
        else
            max_amp = cur_amp;

        if ((max_amp - min_amp) <= 1)
        {
            lpcd_cfg.amp = cur_amp;
            return 0;
        }
    }
}
