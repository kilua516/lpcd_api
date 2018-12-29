#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "msp430x24x.h"
#include "../iso14443/sl2523.h"
//#include "eeprom.h"
void read_eeprom(u16 addr, u16 len,uchar* data)
{
    uint counter;
    uint i;
    uchar ee_temp[4];

    ee_temp[0] = (uchar)(addr & 0x00ff);
    ee_temp[1] = (uchar)((addr & 0xff00)>>8);
    ee_temp[2] = (uchar)(len & 0x00ff);
    ee_temp[3] = (uchar)((len & 0xff00)>>8);
    write_buf( WRBUF, ee_temp, 4 );
    M_ModifyReg( INTRQ, 0, BIT_IDLEI );
    //M_SetReg(INTEN, BIT_IDLEI);
    read_reg( COMMAND, PCD_REE );
    counter = 0xffff;
    while( !( read_reg( INTRQ )&BIT_IDLEI ) )
    {
        counter--;
        if( !counter )
        {
            counter = 0xffff;
            break;
        }
        //M_GetReg(0x01);
    }
    for(i=0; i<len; i++)
    {
        data[i] = read_reg(RDBUF);
    }

//    M_Getbuf( RDBUF, data, len );
}



void write_eeprom(uchar* data, uint len )
{
    uint RegVal;

    write_buf( WRBUF, data, len );
    RegVal = 14;
    M_ModifyReg( GLB_CTRL, 1, BIT_TMR_EN );
    M_ModifyReg( TMR_CTRL, 1, BIT_STAONBEGIN_WEEP );
    write_reg( TMR_CFGL, 0x4c );
    write_reg( TMR_CFGH, 0x5 );
    write_reg( TMR_RLDL, ( ( unsigned char )( RegVal & 0xff ) ) );
    write_reg( TMR_RLDH, ( ( unsigned char )( ( RegVal >> 8 ) & 0xff ) ) );
    //M_SetReg(INTEN, BIT_IDLEI | BIT_ERRI);
    write_reg( COMMAND, PCD_WEE );
}



