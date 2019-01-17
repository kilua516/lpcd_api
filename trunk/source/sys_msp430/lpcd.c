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
 *@    rpliu        20190110   V1.0.1       add phase auto calibration
 *@    rpliu        20190115   V2.0.0       new lpcd adjust algorithm
 *@    rpliu        20190116   V3.0.0       rewrite lpcd adjust algorithm again
 *@    rpliu        20190117   V3.0.1       little change on lpcd flow
                                            replace SPI operation with macro define
 *@    rpliu        20190117   V3.0.2       replace set_bit_mask and clear_bit_mask
                                            with macro define
 *@    rpliu        20190117   V3.0.3       add timeout for sense adj loop

*************************************************************
*/

#include <stdio.h>
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

unsigned char lpcd_amp_test(unsigned char amp)
{
    unsigned char reg_14;
    unsigned char reg_15;
    unsigned char reg_28;
    unsigned char reg_29;
    unsigned char reg_65;
    unsigned char reg_66;
    unsigned char calib_rlt;
    unsigned char calib_rlt_filt;
    unsigned char found_1;
    unsigned char recv_data;
    unsigned char i;

    reg_14 = SL_RD_REG(0x14);
    reg_15 = SL_RD_REG(0x15);
    reg_28 = SL_RD_REG(0x28);
    reg_29 = SL_RD_REG(0x29);
    SL_WR_REG(TxASKReg, 0x00);  // Force100ASK = 0
    SL_WR_REG(TxControlReg, 0x80);
    SL_WR_REG(CWGsPReg, amp ^ 0x28);
    SL_WR_REG(ModGsPReg, amp ^ 0x28);

    SL_WR_REG(0x3f,0x01);

    reg_65 = SL_RD_REG(0x65);
    reg_66 = SL_RD_REG(0x66);
    SL_WR_REG(0x65, 0x00);
    SL_WR_REG(0x66, lpcd_cfg.phase);

    SL_WR_REG(0x54,0x81);         // lpcd calib mode

    SL_WR_REG(0x51,0x00);         // T1

    SL_WR_REG(0x3f,0x00);
    SL_WR_REG(0x05,0x20);
    SL_WR_REG(0x01,0x10);         // enter lpcd calib mode

    // wait lpcd calib done
    do
    {
        recv_data = SL_RD_REG(0x05);
    }
    while ((recv_data & 0x20) != 0x20);

    SL_WR_REG(0x01,0x00);

    // wait wakeup done
    do
    {
        SL_WR_REG(0x3f,0x01);
    }
    while (SL_RD_REG(0x3f) == 0x00);
    SL_WR_REG(0x3f,0x00);

    SL_WR_REG(0x05,0x20);
    SL_WR_REG(0x3f,0x01);

    calib_rlt = 0;
    for (i = 0; i < 8; i++)
    {
        calib_rlt >>= 1;
        calib_rlt |= (SL_RD_REG(0x5b+i) & 0x80);
//      printf("reg_%x: %x\n", 0x5b+i, SL_RD_REG(0x5b+i));
    }
    SL_WR_REG(0x3f,0x00);

    SL_WR_REG(0x3f,0x01);
    SL_WR_REG(0x65,reg_65);
    SL_WR_REG(0x66,reg_66);
    SL_WR_REG(0x51,lpcd_cfg.t1);

    SL_WR_REG(0x54,0x92);

    SL_WR_REG(0x3f,0x00);
    SL_WR_REG(TxASKReg, reg_15);
    SL_WR_REG(TxControlReg, reg_14 | 0x03);
    SL_WR_REG(ModGsPReg, reg_29);
    SL_WR_REG(CWGsPReg, reg_28);

    found_1 = 0;
    calib_rlt_filt = calib_rlt;
    for (i = 0; i < 8; i++)
    {
        if (found_1 == 1)
            calib_rlt_filt |= (0x01 << i);
        if ((calib_rlt >> i) & 0x01)
            found_1 = 1;
    }

//  if (calib_rlt != calib_rlt_filt)
//  {
//      printf("lpcd_amp_test: result been filtered, original result is %x\n", calib_rlt);
//  }

    return calib_rlt_filt;
}

void osc_calib()
{
    SL_WR_REG(0x01,0x06);
    SL_WR_REG(0x3f,0x01);
    while (!(SL_RD_REG(0x4e) & 0x80));
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

    SL_WR_REG(0x3f,0x00);

    reg_14 = SL_RD_REG(TxControlReg);
    reg_15 = SL_RD_REG(TxASKReg);
    reg_29 = SL_RD_REG(ModGsPReg);

    SL_WR_REG(TxASKReg, 0x00);  // Force100ASK = 0
    SL_WR_REG(TxControlReg, 0x80);
    SL_WR_REG(ModGsPReg, SL_RD_REG(CWGsPReg));

    SL_WR_REG(0x3f,0x01);

    SL_WR_REG(0x54,0x81);       // lpcd calib mode

    SL_WR_REG(0x51,0x00);   // T1
    SL_WR_REG(0x52,0x1f);   // T2 = 0x1f
    SL_WR_REG(0x53,0xa3);   // T3 = 0x5 & Twait = 0x3

    reg_65 = SL_RD_REG(0x65);
    reg_66 = SL_RD_REG(0x66);

//  printf("auto delay phase: %x\n", reg_66);

    SL_WR_REG(0x65,0x00);   // disable auto phase

    for (i = 0; i < 8; i++)
    {
        idx[i] = (unsigned char)((INDEX_NUM-1) * i / 7);
    }

    // set threshold
    for (i = 0; i < 8; i++)
    {
        SL_WR_REG(0x5b+i, lut[idx[i]]);
//      printf("write %x to reg_%x\n", lut[idx[i]], 0x5b+i);
    }

    for (ph = 0; ph < 2; ph++)
    {
        SL_WR_REG(0x3f,0x01);

        ph_mask = (ph == 0) ? 0x80 : 0xc0;
        SL_WR_REG(0x66,reg_66 ^ ph_mask);   // set phase

        SL_WR_REG(0x3f,0x00);
        SL_WR_REG(0x05,0x20);
        SL_WR_REG(0x01,0x10);         // enter lpcd calib mode

        // wait lpcd calib done
        do
        {
            recv_data = SL_RD_REG(0x05);
        }
        while ((recv_data & 0x20) != 0x20);

        SL_WR_REG(0x01,0x00);

        // wait wakeup done
        do
        {
            SL_WR_REG(0x3f,0x01);
        }
        while (SL_RD_REG(0x3f) == 0x00);
        SL_WR_REG(0x3f,0x00);

        SL_WR_REG(0x05,0x20);
        SL_WR_REG(0x3f,0x01);

//        for (i = 0; i < 8; i++)
//        {
//            calib_rlt = SL_RD_REG(0x5b+i) & 0x80;
//            printf("reg:%x, calib_rlt[%d]: %d\n", SL_RD_REG(0x5b+i), i, calib_rlt >> 7);
//        }

        for (i = 0; i < 8; i++)
        {
            calib_rlt = SL_RD_REG(0x5b+i) & 0x80;
            if (calib_rlt != 0)
            {
                break;
            }
        }

        amp[ph] = i;
    }

    SL_WR_REG(0x3f,0x01);
    SL_WR_REG(0x65,reg_65);
    SL_WR_REG(0x66,reg_66);

    SL_WR_REG(0x3f,0x00);
    SL_WR_REG(TxASKReg, reg_15);
    SL_WR_REG(TxControlReg, reg_14 | 0x03);
    SL_WR_REG(ModGsPReg, reg_29);

    if (amp[0] < amp[1])
        ph = (reg_66 ^ 0xc0);
    else if (amp[0] > amp[1])
        ph = (reg_66 ^ 0x80);
    else
        return 0xff;

    if (lpcd_cfg.phase_offset >= 0)
    {
        if ((ph & 0x3f) > (0x3f - lpcd_cfg.phase_offset))
            return ((ph & 0xc0) | 0x3f);
        else
            return ((ph & 0xc0) | ((ph & 0x3f) + lpcd_cfg.phase_offset));
    }
    else
    {
        if ((ph & 0x3f) < (0x00 - lpcd_cfg.phase_offset))
            return ((ph & 0xc0) | 0x00);
        else
            return ((ph & 0xc0) | ((ph & 0x3f) + lpcd_cfg.phase_offset));
    }
}

void lpcd_init()
{
    unsigned char recv_data;
    int i;
#ifdef LPCD_DEBUG
    volatile unsigned char temp_value;
#endif
    osc_calib();

    lpcd_cfg.phase = phase_calib();
//    printf("lpcd_cfg.phase: %x\n", lpcd_cfg.phase);

    SL_WR_REG(0x3f,0x01);

    SL_WR_REG(0x54,0x92);

    SL_WR_REG(0x51,lpcd_cfg.t1);        // T1
#ifdef LPCD_DEBUG    
    temp_value = SL_RD_REG(0x51);
    printf("0x51: %x\r\n",temp_value);
#endif

    SL_WR_REG(0x52,0x1f);  // T2 = 0x1f
#ifdef LPCD_DEBUG    
    temp_value = SL_RD_REG(0x52);
    printf("0x52: %x\r\n",temp_value);
#endif

    SL_WR_REG(0x53,0xa3);  // T3 = 0x5 & Twait = 0x3
#ifdef LPCD_DEBUG    
    temp_value = SL_RD_REG(0x53);
    printf("0x53: %x\r\n",temp_value);
#endif
    
    for (i = 0; i < 8; i++)
    {
        SL_WR_REG(0x5b+i, lut[lpcd_cfg.idx[i]]);
//      printf("write %x to reg_%x\n", lut[lpcd_cfg.idx[i]], 0x5b+i);
    }

#ifdef LPCD_DEBUG    
    printf("initial lpcd parameters:\r\n");
    for (i=0;i<8;i++)
    {
        temp_value = SL_RD_REG(0x5b+i);
        printf("0x%0.2x: %0.2x\r\n",0x5b+i, temp_value);
    }
#endif

    // set card detect threshold
    SL_WR_REG(0x55, 0x00);
//  SL_WR_REG(0x56, 0xff);
    SL_WR_REG(0x56, 0xf1);
    
    SL_WR_REG(0x3f,0x00);

    // enable IRQ
    recv_data = SL_RD_REG(0x03);
    SL_WR_REG(0x03, recv_data | 0xA0);
}

unsigned char slm_reg_14, slm_reg_15, slm_reg_28, slm_reg_29, slm_reg_65, slm_reg_66;
// set 0x14/0x15/0x28/0x29/0x65/0x66 and turn off rf field
void lpcd_entry()
{
    SL_WR_REG(0x3f,0x00);
    slm_reg_14 = SL_RD_REG(0x14);
    slm_reg_15 = SL_RD_REG(0x15);
    slm_reg_28 = SL_RD_REG(0x28);
    slm_reg_29 = SL_RD_REG(0x29);
    SL_WR_REG(TxASKReg, 0x00);  // Force100ASK = 0
    SL_WR_REG(TxControlReg, 0x80);
    SL_WR_REG(CWGsPReg, lpcd_cfg.amp ^ 0x28);
    SL_WR_REG(ModGsPReg, lpcd_cfg.amp ^ 0x28);
    SL_WR_REG(0x3f, 0x01);
    slm_reg_65 = SL_RD_REG(0x65);
    slm_reg_66 = SL_RD_REG(0x66);
    SL_WR_REG(0x65, 0x00);
    SL_WR_REG(0x66, lpcd_cfg.phase);
    SL_WR_REG(0x3f, 0x00);
    SL_SET_BIT_MASK(DivIEnReg, BIT7 | BIT5);// enable LPCD IRQ

    SL_WR_REG(0x01,0x10);
    
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
    SL_WR_REG(0x01,0x00);

    // wait wakeup done
    do
    {
        SL_WR_REG(0x3f,0x01);
    }
    while (SL_RD_REG(0x3f) == 0x00);
    SL_WR_REG(0x3f,0x00);
    
    SL_WR_REG(TxASKReg, slm_reg_15);
    SL_WR_REG(TxControlReg, slm_reg_14 | 0x03);
    SL_WR_REG(CWGsPReg, slm_reg_28);
    SL_WR_REG(ModGsPReg, slm_reg_29);
    SL_WR_REG(0x3f, 0x01);
    SL_WR_REG(0x65, slm_reg_65);
    SL_WR_REG(0x66, slm_reg_66);
    SL_WR_REG(0x3f, 0x00);
    SL_CLR_BIT_MASK(DivIEnReg, BIT5);
}

int lpcd_sen_adj()
{
    const unsigned char lpcd_amp_thd = 0xf0;
    lpcd_cfg_t lpcd_cfg_bk;
    char lpcd_idx_adj_val;
    unsigned char amp_srch_rlt1;
    unsigned char amp_srch_rlt2;
    unsigned char lpcd_amp_rlt;
    unsigned char adj_fail;
    unsigned long adj_to_cnt;
    unsigned char i;

    // backup original lpcd_cfg
    lpcd_cfg_bk = lpcd_cfg;
    for (i = 0; i < 8; i++)
    {
        lpcd_cfg_bk.idx[i] = lpcd_cfg.idx[i];
    }

    lpcd_cfg.phase = phase_calib();

    // index expand
    switch (lpcd_cfg.sense)
    {
        case 0:
            lpcd_cfg.idx[1] = lpcd_cfg.idx[0]+1;
            lpcd_cfg.idx[2] = lpcd_cfg.idx[0]+2;
            lpcd_cfg.idx[3] = lpcd_cfg.idx[0]+3;
            lpcd_cfg.idx[4] = lpcd_cfg.idx[7]-3;
            lpcd_cfg.idx[5] = lpcd_cfg.idx[7]-2;
            lpcd_cfg.idx[6] = lpcd_cfg.idx[7]-1;
            break;
        case 1:
            lpcd_cfg.idx[0] = lpcd_cfg.idx[1]-1;
            lpcd_cfg.idx[2] = lpcd_cfg.idx[1]+1;
            lpcd_cfg.idx[3] = lpcd_cfg.idx[1]+2;
            lpcd_cfg.idx[4] = lpcd_cfg.idx[6]-2;
            lpcd_cfg.idx[5] = lpcd_cfg.idx[6]-1;
            lpcd_cfg.idx[7] = lpcd_cfg.idx[6]+1;
            break;
        case 2:
            lpcd_cfg.idx[0] = lpcd_cfg.idx[2]-2;
            lpcd_cfg.idx[1] = lpcd_cfg.idx[2]-1;
            lpcd_cfg.idx[3] = lpcd_cfg.idx[2]+1;
            lpcd_cfg.idx[4] = lpcd_cfg.idx[5]-1;
            lpcd_cfg.idx[6] = lpcd_cfg.idx[5]+1;
            lpcd_cfg.idx[7] = lpcd_cfg.idx[5]+2;
            break;
        default:
            lpcd_cfg.idx[0] = lpcd_cfg.idx[3]-3;
            lpcd_cfg.idx[1] = lpcd_cfg.idx[3]-2;
            lpcd_cfg.idx[2] = lpcd_cfg.idx[3]-1;
            lpcd_cfg.idx[5] = lpcd_cfg.idx[4]+1;
            lpcd_cfg.idx[6] = lpcd_cfg.idx[4]+2;
            lpcd_cfg.idx[7] = lpcd_cfg.idx[4]+3;
            break;
    }

    // large step adjust threshold
    adj_to_cnt = ADJ_TO_TIME;   // adj timeout
    while (adj_to_cnt--)
    {
        SL_WR_REG(0x3f, 0x01);
        for (i = 0; i < 8; i++)
        {
            SL_WR_REG(0x5b+i, lut[lpcd_cfg.idx[i]]);
//          printf("write %x to reg_%x\n", lut[lpcd_cfg.idx[i]], 0x5b+i);
        }
        SL_WR_REG(0x3f, 0x00);

        lpcd_amp_rlt = lpcd_amp_test(lpcd_cfg.default_amp);
        LPCD_AMP_TEST_INFO("lpcd_sen_adj", lpcd_cfg.default_amp, lpcd_amp_rlt)

        if (lpcd_amp_rlt == 0xff)
        {
            if (lpcd_cfg.idx[0] >= 6)
                lpcd_idx_adj_val = -6;
            else if (lpcd_cfg.idx[0] >= 5)
                lpcd_idx_adj_val = -5;
            else if (lpcd_cfg.idx[0] >= 4)
                lpcd_idx_adj_val = -4;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0xfe)
        {
            if (lpcd_cfg.idx[0] >= 5)
                lpcd_idx_adj_val = -5;
            else if (lpcd_cfg.idx[0] >= 4)
                lpcd_idx_adj_val = -4;
            else if (lpcd_cfg.idx[0] >= 3)
                lpcd_idx_adj_val = -3;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0xfc)
        {
            if (lpcd_cfg.idx[0] >= 3)
                lpcd_idx_adj_val = -3;
            else if (lpcd_cfg.idx[0] >= 2)
                lpcd_idx_adj_val = -2;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0xf8)
        {
            if (lpcd_cfg.idx[0] >= 1)
                lpcd_idx_adj_val = -1;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0xf0)
        {
            break;
        }
        else if (lpcd_amp_rlt == 0xe0)
        {
            if ((lpcd_cfg.idx[7]) < (INDEX_NUM-1))
                lpcd_idx_adj_val = 1;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0xc0)
        {
            if ((lpcd_cfg.idx[7]) < (INDEX_NUM-3))
                lpcd_idx_adj_val = 3;
            else if ((lpcd_cfg.idx[7]) < (INDEX_NUM-2))
                lpcd_idx_adj_val = 2;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0x80)
        {
            if ((lpcd_cfg.idx[7]) < (INDEX_NUM-5))
                lpcd_idx_adj_val = 5;
            else if ((lpcd_cfg.idx[7]) < (INDEX_NUM-4))
                lpcd_idx_adj_val = 4;
            else if ((lpcd_cfg.idx[7]) < (INDEX_NUM-3))
                lpcd_idx_adj_val = 3;
            else
                break;
        }
        else if (lpcd_amp_rlt == 0x00)
        {
            if ((lpcd_cfg.idx[7]) < (INDEX_NUM-6))
                lpcd_idx_adj_val = 6;
            else if ((lpcd_cfg.idx[7]) < (INDEX_NUM-5))
                lpcd_idx_adj_val = 5;
            else if ((lpcd_cfg.idx[7]) < (INDEX_NUM-4))
                lpcd_idx_adj_val = 4;
            else
                break;
        }
        else
        {
            break;
        }

        // adjust idx according to previous test results
        for (i = 0; i < 8; i++)
        {
            lpcd_cfg.idx[i] += lpcd_idx_adj_val;
        }
    }

    adj_fail = (adj_to_cnt == 0);

    if (adj_fail == 0)
    {
        amp_srch_rlt1 = lpcd_amp_search_floor(lpcd_amp_thd, lpcd_cfg.min_amp, lpcd_cfg.default_amp);
        if (amp_srch_rlt1 != 0)
        {
            amp_srch_rlt2 = lpcd_amp_search_ceil(lpcd_amp_thd, lpcd_cfg.default_amp, lpcd_cfg.max_amp);
            adj_fail = (amp_srch_rlt2 == 0);
        }

        lpcd_cfg.amp = (amp_srch_rlt1 + amp_srch_rlt2) / 2;

        switch (lpcd_cfg.sense)
        {
            case 0:
                lpcd_cfg.idx[1] = lpcd_cfg.idx[0];
                lpcd_cfg.idx[2] = lpcd_cfg.idx[0];
                lpcd_cfg.idx[3] = lpcd_cfg.idx[0];
                lpcd_cfg.idx[4] = lpcd_cfg.idx[7];
                lpcd_cfg.idx[5] = lpcd_cfg.idx[7];
                lpcd_cfg.idx[6] = lpcd_cfg.idx[7];
                break;
            case 1:
                lpcd_cfg.idx[0] = lpcd_cfg.idx[1];
                lpcd_cfg.idx[2] = lpcd_cfg.idx[1];
                lpcd_cfg.idx[3] = lpcd_cfg.idx[1];
                lpcd_cfg.idx[4] = lpcd_cfg.idx[6];
                lpcd_cfg.idx[5] = lpcd_cfg.idx[6];
                lpcd_cfg.idx[7] = lpcd_cfg.idx[6];
                break;
            case 2:
                lpcd_cfg.idx[0] = lpcd_cfg.idx[2];
                lpcd_cfg.idx[1] = lpcd_cfg.idx[2];
                lpcd_cfg.idx[3] = lpcd_cfg.idx[2];
                lpcd_cfg.idx[4] = lpcd_cfg.idx[5];
                lpcd_cfg.idx[6] = lpcd_cfg.idx[5];
                lpcd_cfg.idx[7] = lpcd_cfg.idx[5];
                break;
            default:
                lpcd_cfg.idx[0] = lpcd_cfg.idx[3];
                lpcd_cfg.idx[1] = lpcd_cfg.idx[3];
                lpcd_cfg.idx[2] = lpcd_cfg.idx[3];
                lpcd_cfg.idx[5] = lpcd_cfg.idx[4];
                lpcd_cfg.idx[6] = lpcd_cfg.idx[4];
                lpcd_cfg.idx[7] = lpcd_cfg.idx[4];
                break;
        }
    }

    if (adj_fail == 1)
    {
//      printf("lpcd_sen_adj: fail\n");
        lpcd_cfg = lpcd_cfg_bk;
        SL_WR_REG(0x3f,0x01);
        for (i = 0; i < 8; i++)
        {
            lpcd_cfg.idx[i] = lpcd_cfg_bk.idx[i];
            SL_WR_REG(0x5b+i, lut[lpcd_cfg.idx[i]]);
//          printf("write %x to reg_%x\n", lut[lpcd_cfg.idx[i]], 0x5b+i);
        }
        SL_WR_REG(0x3f,0x00);
    }

//  printf("lpcd_info:\n");
//  printf("lpcd_cfg.t1: %x\n", lpcd_cfg.t1);
//  printf("lpcd_cfg.phase: %x\n", lpcd_cfg.phase);
//  printf("lpcd_cfg.amp: %x\n", lpcd_cfg.amp);
//  printf("lpcd_cfg.sense: %x\n", lpcd_cfg.sense);
//  for (i = 0; i < 8; i++)
//  {
//      printf("lpcd_cfg.idx[%d]: %x\n", i, lpcd_cfg.idx[i]);
//  }

    return ((adj_fail == 1) ? -1 : 0);
}

unsigned char lpcd_amp_search_floor(unsigned char lpcd_amp_target, unsigned char amp_low, unsigned char amp_high)
{
    unsigned char cur_amp;
    unsigned char max_amp;
    unsigned char min_amp;
    unsigned char lpcd_amp_rlt;
    int i;

    max_amp = amp_high;
    min_amp = amp_low;

    while (1)
    {
        cur_amp = (max_amp + min_amp) / 2;
        lpcd_amp_rlt = lpcd_amp_test(cur_amp);
        LPCD_AMP_TEST_INFO("lpcd_amp_search_floor", cur_amp, lpcd_amp_rlt)

        if (lpcd_amp_rlt > lpcd_amp_target)
            min_amp = cur_amp;
        else if (lpcd_amp_rlt == lpcd_amp_target)
            max_amp = cur_amp;
        else
            return 0;

        if ((max_amp - min_amp) <= 1)
            return max_amp;
    }
}

unsigned char lpcd_amp_search_ceil(unsigned char lpcd_amp_target, unsigned char amp_low, unsigned char amp_high)
{
    unsigned char cur_amp;
    unsigned char max_amp;
    unsigned char min_amp;
    unsigned char lpcd_amp_rlt;
    int i;

    max_amp = amp_high;
    min_amp = amp_low;

    while (1)
    {
        cur_amp = (max_amp + min_amp) / 2;
        lpcd_amp_rlt = lpcd_amp_test(cur_amp);
        LPCD_AMP_TEST_INFO("lpcd_amp_search_ceil", cur_amp, lpcd_amp_rlt)

        if (lpcd_amp_rlt < lpcd_amp_target)
            max_amp = cur_amp;
        else if (lpcd_amp_rlt == lpcd_amp_target)
            min_amp = cur_amp;
        else
            return 0;

        if ((max_amp - min_amp) <= 1)
            return min_amp;
    }
}
