/*
*************************************************************
 *@Copyright (C), 2016-2020 smartlinkMicro. Co., Ltd.  All rights reserved.
 *@Filename:
 *@Complier:    IAR msp430 5.30
 *@Target OS:   NONE
 *@Target HW:  SL1523
 *@Author:        htyi
 *@Version :     V1.0.0
 *@Date:          2017.8
 *@Description:
 *@History:
 *@    <author>     <time>     <version >   <desc>

*************************************************************
*/

#include "usmart.h"
#include "usmart_str.h"
#include "../iso14443/sl2523.h"
#include "../iso14443/Rfid_interface.h"


////////////////////////////用户配置区///////////////////////////////////////////////
//这下面要包含所用到的函数所申明的头文件(用户自己添加)


extern void System_GetClocks( void );
 #if 1
//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
struct _m_usmart_nametab usmart_nametab[] =
{
#if USMART_USE_WRFUNS==0    //如果使能了读写操作
    ( void* )read_addr, "u32 read_addr(u32 addr)",
    ( void* )write_addr, "void write_addr(u32 addr,u32 val)",
#endif

 /* base */

    //(void*)Testrfid ,               "void Testrfid(void)", 
    (void*)test_a ,       "void test_a(u8 rate)", 
    
    (void*)test_b,        "void test_b(uchar rate)",
    (void*)Detect_ContactlessCard, "char Detect_ContactlessCard(void)",
    (void*)read_reg,            "uchar read_reg( uchar Regaddr );",
    (void*)write_reg,            "void write_reg( uchar Regaddr, uchar RegValue );",

    //(void*)version,             "void version();",
    (void*)pcd_set_rate, "void pcd_set_rate( unsigned char baudindex )",
   // (void*)spi_mode, "void spi_mode(uchar mode,uchar speed)",
    (void*)pcd_config,       "void pcd_config( unsigned char type )",
  
   // (void*)Hard_Powerdown,     "void Hard_Powerdown( uchar enable)",
   
   // (void*)type_set,    "void type_set( unsigned char type);",

   // (void*)getbuf_printf,  "void getbuf_printf( uchar Regaddr , uint lenth )",
   // (void*)eeprom_read, "void eeprom_read(u16 addr, u16 length)",
    //(void*)eeprom_write,  "void eeprom_write(u16 addr, u16 length, uchar wr_data)",
   

    //(void*)reqA,             "void reqA()         ",
    //(void*)wakeupA,          "void wakeupA()      ",
    //(void*)anticollsionA,    "void anticollsionA()",
    //(void*)selectA,          "void selectA()      ",
    //(void*)haltA,            "void haltA()        ",
    //(void*)ratsA,            "void ratsA()        ",
    
   // (void*)loadkey,    "void loadkey(u8 load_type, u8 key_val,u16 key_addr);                      ",
   // (void*)authentication,    "void authentication(u8 block_id);                                         ",
   // (void*)secret_set ,   "void secret_set(u8 secret_en);                                            ",
   // (void*)write_block ,   "void write_block(u8 block_id, u8 wr_val);                                 ",
   // (void*)read_block ,   "void read_block(u8 block_id);                                             ",
   // (void*)typeA_APDU_random ,   "void typeA_APDU_random(uchar baud_speed,uint cnt);         ",
   // (void*)M1_operation_test,   "void M1_operation_test(uchar loadkey,uchar M1_en,uchar key, uchar secret);",
   // (void*)TypeB_APDU_random, "void TypeB_APDU_random(uchar baud_speed,uint cnt)"
    
};

///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//函数控制管理器初始化
//得到各个受控函数的名字
//得到函数总数量
struct _m_usmart_dev usmart_dev =
{
    usmart_nametab,
    usmart_init,
    usmart_cmd_rec,
    usmart_exe,
    usmart_scan,
    sizeof( usmart_nametab ) / sizeof( struct _m_usmart_nametab ), //函数数量
    0,      //参数数量
    0,      //函数ID
    1,      //参数显示类型,0,10进制;1,16进制
    0,      //参数类型.bitx:,0,数字;1,字符串
    0,      //每个参数的长度暂存表,需要MAX_PARM个0初始化
    0,      //函数的参数,需要PARM_LEN个0初始化
};
    #endif
//endfile

