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

*************************************************************
*/

#ifndef LPCD_H
#define LPCD_H

#include "sys_def.h"

#define INDEX_NUM   45
#define THD_ADJ_CNT 5

//#define LPCD_DEBUG

extern double voltage[];

extern unsigned char lut[];

void lpcd_find_edge(int lower_bound, int upper_bound, int *lower_edge);

void do_lpcd_calib(int upper_bound, int lower_bound, uchar *calib_rlt, uchar *idx);

void osc_calib();

void lpcd_calib(unsigned char *idx);

unsigned char phase_calib();

void lpcd_calib_callback(unsigned char proc);

void lpcd_init(unsigned char t1             ,
               unsigned char *idx           ,
               unsigned char sense          ,
               unsigned char dc_shift_det_en);

void lpcd_entry();

void lpcd_exit();

#endif
// endfile