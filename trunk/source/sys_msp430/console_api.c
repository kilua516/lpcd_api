/*
*************************************************************
 Copyright (C), 2016-2020 smartlinkMicro. Co., Ltd.  All rights reserved.
 Filename:
 Complier:    IAR msp430 5.30
 Target OS:   NONE
 Target HW:  SL1523
 Author:        htyi
 Version :     V1.0.0
 Date:          2017.8
 Description:
 History:
     <author>     <time>     <version >   <desc>

*************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "msp430x24x.h"
#include "../sys_msp430/sys_def.h"
#include "../iso14443/iso14443a.h"
#include "../iso14443/iso14443_4.h"
#include "../iso14443/Rfid_interface.h"
#include "../iso14443/sl2523.h"

#include "../common/usmart.h"

unsigned char sendBuffer[20];
unsigned char recvBuffer[20];
uchar UID[4];
uchar PUPI[5];  // fist byte if  code_flag 0x50
 int ok = 0;
void spi_wr_test()
{
    uchar  wdata[64];
    uchar  rdata[64];
   
    uchar err = 0;
    int i = 0; 
    int j = 0;
    uchar ret = 0xff;

    write_reg(0x0a,0x80);
    for(i=0; i<0xf; i++)
    {
       wdata[i]=i;
        write_reg(0x09,i);
        ret = read_reg(0x09);
        if(i !=ret)
        {
            err++;
        //    printf("i = 0x%x,errnum = 0x%x, data=0x%x\r\n",i,err,ret);
             //while(1);
        }
        else
        {
            ok++;
        }
    }
     //   write_reg(0x1a,0xc0);
    printf("test 0xf times ,oknum = 0x%x\r\n",ok);
 }

/*
     enable 1 rfon ; 0 rfoff
*/

void getbuf_printf( uchar Regaddr , uint lenth )
{
    uchar *data;
    u16  i = 0;
    printf("\r\n");
    data = (uchar *)malloc(lenth);
    if(data == 0)
    {
        printf("malloc fail\r\n");
    }
    read_buf(Regaddr,data, lenth);
    for(i=0; i<lenth; i++)
    {
        printf("0x%x ",data[i]);
    }
    printf("\r\n");
    free(data);
    data=0;
}

void eeprom_read(u16 addr, u16 length)
{
    uchar *data;
    u16  i = 0;
    printf("\r\n");
    data =  malloc(length);
    if(data == 0)
    {
        printf("malloc fail\r\n");
    }

    read_eeprom((u16)addr, (u16)length,(uchar*)data);

    for(i=0; i<length; i++)
    {
        printf("0x%x ",data[i]);
    }

    printf("\r\n");
    free(data);
    data=0;
}

void eeprom_write(u16 addr, uint length, uchar wr_data)
{
    uchar *data;
    u16  len = 0;
    printf("\r\n");
    if(addr > 512)
        return;
    data =  malloc(length);
    if(data == 0)
    {
        printf("malloc fail\r\n");
        return;
    }
    data[0] =  (uchar)(addr & 0x00ff);
    data[1] =  (uchar)((addr & 0xff00)>>8);
    memset((data+2),wr_data,length);
    len = length+2;

    write_eeprom(data,  len );

    free(data);
    data=0;
}




void wakeupA()//WUPA
{
    uchar atqa[2];
    int ret = -1;
    printf("\r\n");
    ret = pcd_request(0x52,atqa);
    if(ret != 0)
    {
        printf("wakup_A fail,erron:0x%x\r\n",ret);
        return;
    }
    printf("WKUP: 0x%x 0x%x\r\n",atqa[0],atqa[1]);
}


uchar snr[4];   //  save UID
uchar sak[4];


// receiv sak
void anticoll_selectA()//SELECT
{
    char status = MI_OK;
    u8 sak;
    u8 buf[10];

   //一次防冲突及选卡
    if (status == MI_OK)
    {
        g_tag_info.uid_length = UID_4;
        //make_packet(COM_PKT_CMD_CARD_TYPE, g_tag_info.tag_type_bytes, sizeof(g_tag_info.tag_type_bytes));

        status = pcd_cascaded_anticoll(PICC_ANTICOLL1, 0, &g_tag_info.serial_num[0]);
        if (status == MI_OK)
        {
            status = pcd_cascaded_select(PICC_ANTICOLL1, &g_tag_info.serial_num[0], &sak);
        }
    }
    //二次防冲突及选卡
    if(status == MI_OK && (sak & BIT2))
    {
        g_tag_info.uid_length = UID_7;
        status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
        if(status == MI_OK)
        {
            status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
        }
    }
    //回复uid
    if (status == MI_OK)
    {
        buf[0] = g_tag_info.uid_length;
        memcpy(buf+1, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0]:&g_tag_info.serial_num[1]), g_tag_info.uid_length);
        //make_packet(COM_PKT_CMD_REQA, buf, g_tag_info.uid_length + 1);
    }

    if(status == MI_OK)
    {
      printf("SAK: 0x%x\r\n",sak);
        return;
    }
    else
    {
              printf("SelectA fail,erron:0x%x\r\n",status);
        return;
       // delay_1ms(8);
    } 
}
void haltA()
{
    char ret = -1;
    printf("\r\n");
    ret = pcd_hlta();
    if(ret != 0)
    {
        printf("haltA fail\r\n");
        return;
    }
    printf("haltA !\r\n");
}


// send rats ,  receiv ats , sel_code 0xe0; fsid 0x8(256); cid 0x1
void ratsA()
{
    u8 ats[15];
    int ret = -1;
    printf("\r\n");
    ret =  pcd_rats_a(0x1,ats);
    if(ret != 0)
    {
        printf("ratsA fail erron:0x%x\r\n",ret);
        return;
    }
    printf("TL:0x%x  T0:0x%x TA(1):0x%x TB(1):0x%x TC(1):0x%x T1:0x%x Tk:0x%x\r\n",\
           ats[0],ats[1],ats[2],ats[3],ats[4],ats[5],ats[6]);
}


// loadtype 0:eeprom  1:sram ; key 0x0  0xff , key_addr:epprom add tobe write




// secret_en 0 ,clear the authentication ,trancefrom by no secret



void write_block(u8 block_id, u8 wr_val)
{
    u8 wr_buf[16];
    u8 ret;
    u16 i = 0;
    for(i=0; i<16; i++)
    {
        wr_buf[i] = wr_val;
    }
    printf("\r\n");
    ret = pcd_write(block_id,wr_buf);
    if(ret != 0x00)
    {
        printf("write_block  fail erron:0x%x\r\n",ret);
        return;
    }
    printf("write_block successful!\r\n");
}

void read_block(u8 block_id)
{
    u8 rd_buf[16];
    u8 ret;
    u16 i = 0;
    printf("\r\n");
    ret = pcd_read( block_id, rd_buf);
    if(ret != 0x00)
    {
        printf("read_block  fail erron:0x%x\r\n",ret);
        return;
    }
    printf("read_block data is: \r\n");
    for(i=0; i<16; i++)
    {
        printf("0x%x \r\n",rd_buf[i]);
    }
    printf("\r\n");

}


uchar M1_buffer_write[16];
uchar M1_buffer_read[16];
// loadkey 0 eeprom 1 dsram; M1_en 1 enable mifare ; key 0x0 or 0xff ;secret : 1 keep secret after auth




void TypeB_APDU_random(uchar baud_speed,uint cnt)
{
    uint test_num;
    uchar i,I_block,ret;
    u16  rdlen;
    u16 len = 0;
    S_ATTRIB attrib_val;
    printf("\r\n");

 //   write_reg(COMMAND,0x00);
 //   M_ModifyReg(DLENH,1,BIT_CLR_BUF);//flush buffer
  //  write_reg(INTRQ,0x00);
    pcd_set_rate(0);
    //M_ModifyReg(TYPEB_CON1,1,0x15);


    sendBuffer[0]=0x05;
    sendBuffer[1]=0x00;
    sendBuffer[2]=0x8;

    /* REQB*/
    len = 3;
    crc_set(1);
    M_SetTimeOut(5000);
    ret = iISO14443TransMit( sendBuffer,len , PUPI, &rdlen );  // warning: pupi[0] is the ATQB flag 0x50
    if(ret != 0)
    {
        printf("REQB  fail, erron:0x%x\r\n",ret);
        return;
    }

    printf("PUPI: ");
    for(i=0; i<4; i++)
    {
        printf("0x%x ", PUPI[1+i]);
    }
    printf("\r\n");

     /* ATTRIB */
    len = 9;
    attrib_val.start_code = 0x1D;
    for(i=0; i<4; i++)
    {
        attrib_val.flag[i] = PUPI[1+i];
    }
    attrib_val.param[0] = 0x00;
    switch(baud_speed)
    {
        case 0x00:
            attrib_val.param[1]=0x08;
            break;
        case 0x01:
            attrib_val.param[1]=0x58;
            break;
        case 0x02:
            attrib_val.param[1]=0xa8;
            break;
        case 0x03:
            attrib_val.param[1]=0xf8;
            break;
        default:
            attrib_val.param[1]=0x08;
            break;
    }
    attrib_val.param[2] =0x01;
    attrib_val.cid =0x04;
    crc_set(1);
    M_SetTimeOut(5000);
    ret = iISO14443TransMit( (uchar *)&attrib_val,len , recvBuffer, &rdlen );
    if(ret != 0)
    {
        printf("ATTRIB  fail, erron:0x%x\r\n",ret);
        return;
    }
    printf("ATTRIB  respone CID: 0x%x\r\n",recvBuffer[0]);  // should be equate attrib_val.cid

     /* I-BLOCK exchange data */
    I_block =0x0b;
    switch(baud_speed)
    {
        case 0x00:
            pcd_set_rate(0);
            break;
        case 0x01:
            pcd_set_rate(1);
            break;
        case 0x02:
            pcd_set_rate(2);
            break;
        case 0x03:
            pcd_set_rate(3);
            break;
        default:
            pcd_set_rate(0);
            break;
    }
    test_num = cnt;
    while(test_num)
    {
        len =7;
        if(I_block==0x0b)
        {
            I_block = 0x0a;
        }
        else
        {
            I_block = 0x0b;
        }
        sendBuffer[0] = I_block;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x00;
        sendBuffer[3] = 0x84;
        sendBuffer[4] = 0x00;
        sendBuffer[5] = 0x00;
        sendBuffer[6] = 0x04;
        crc_set(1);
        M_SetTimeOut(5000);
        ret = iISO14443TransMit( sendBuffer,len , recvBuffer, &rdlen );
        if(ret!=0)
        {
            break;
        }
        test_num--;
    }
    if(test_num)
    {
        printf("leave test_num :%d not pass\r\n",test_num);
        return;
    }
    else
    {       
        printf(" pass all test_num:%d\r\n",cnt);
    }
}

//endfile

