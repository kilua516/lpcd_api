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

void lpcd_calib_callback(unsigned char proc);

void lpcd_init(unsigned char t1             ,
               unsigned char *idx           ,
               unsigned char sense          ,
               unsigned char dc_shift_det_en);

void lpcd_entry();

void lpcd_exit();

#endif
// endfile
