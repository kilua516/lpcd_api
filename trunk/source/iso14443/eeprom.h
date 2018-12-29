/**
 ****************************************************************
 * @file mifare.h
 *
 * @brief 
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 
#ifndef MIFARE_H
#define MIFARE_H
#include "sl2523.h"


void write_eeprom(uchar* data, uint len );
extern void read_eeprom(u16 addr, u16 len,uchar* data);

#endif
