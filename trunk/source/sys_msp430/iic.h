#ifndef IIC_H
#define  IIC_H

#include "sys_def.h"

void iic_test(void);
void write_word(uchar device_add,uchar word_add,uchar date) ;
uchar read_word(uchar device_add,uchar word_add);
#endif
// endfile