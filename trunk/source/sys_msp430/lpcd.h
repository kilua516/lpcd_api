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
 *@Description: LPCD api code header file
 *@History:
 *@    <author>     <time>     <version >   <desc>
 *@    rpliu        20181225   V1.0.0       Initial Version
 *@    rpliu        20190110   V1.0.1       add phase auto calibration
 *@    rpliu        20190115   V2.0.0       new lpcd adjust algorithm
 *@    rpliu        20190116   V3.0.0       rewrite lpcd adjust algorithm again

*************************************************************
*/

#ifndef LPCD_H
#define LPCD_H

#include "sys_def.h"
#include "msp430x24x.h"

#define INDEX_NUM   45
#define THD_ADJ_CNT 5

#define SRCH_STEP 4

//#define LPCD_DEBUG

typedef struct
{
    unsigned char t1;
    unsigned char dc_shift_det_en;
    unsigned char phase;
    char phase_offset;
    unsigned char default_amp;
    unsigned char amp;
    unsigned char max_amp;
    unsigned char min_amp;
    unsigned char sense;
    unsigned char idx[8];
} lpcd_cfg_t;

extern lpcd_cfg_t lpcd_cfg;

extern double voltage[];

extern unsigned char lut[];

unsigned char lpcd_amp_test(unsigned char amp);

void osc_calib();

unsigned char phase_calib();

void lpcd_init();

void lpcd_entry();

void lpcd_exit();

int lpcd_sen_adj();

unsigned char lpcd_amp_search_floor(unsigned char lpcd_amp_target, unsigned char amp_low, unsigned char amp_high);
unsigned char lpcd_amp_search_ceil(unsigned char lpcd_amp_target, unsigned char amp_low, unsigned char amp_high);

#define ASSERT_SPI_CLK_LOW    \
    P5DIR |= BIT3;            \
    P5SEL &= ~BIT3;           \
    P5OUT &= ~BIT3;

#define RELEASE_SPI_CLK_LOW   \
    P5SEL = 0xE;

#if 0
#define LPCD_AMP_TEST_INFO(str, amp_info, rlt_info)  \
    printf(str);                                                  \
    printf("\n");                                                 \
    for (i = 0; i < 8; i++)                                       \
    {                                                             \
        printf("idx[%d]: %0.2d\t", i, lpcd_cfg.idx[i]);           \
    }                                                             \
    printf("\namp: %0.2x, phase: %0.2x, lpcd_amp_rlt: %x\n", amp_info, lpcd_cfg.phase, rlt_info);
#else
#define LPCD_AMP_TEST_INFO(str, amp_info, rlt_info)
#endif

#endif
// endfile
