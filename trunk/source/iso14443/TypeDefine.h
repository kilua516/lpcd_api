/** ----------------------------------------------------------------------------
 *         Nationz Technology Software Support  -  NATIONZ  -
 * -----------------------------------------------------------------------------
 * Copyright (c) 2013－2018, Nationz Corporation  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaiimer below.
 * 
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the disclaimer below in the documentation and/or
 * other materials provided with the distribution. 
 * 
 * Nationz's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission. 
 * 
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONZ "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
 * -----------------------------------------------------------------------------
 */

/** ****************************************************************************
 * @copyright      Nationz Co.,Ltd 
 *        Copyright (c) 2013－2018 All Rights Reserved 
 *******************************************************************************
 * @file     TypeDefine.h
 * @author   nationz
 * @date     Tue Jan 21 17:58:51 2014
 * @version  v1.2
 * @brief    type define 
 ******************************************************************************/
#ifndef __TYPEDEFINE_H__
#define __TYPEDEFINE_H__

//------------------------------------------------------------------
//                        Headers
//------------------------------------------------------------------
/* #include "core_cm0.h" */

//------------------------------------------------------------------
//                        Definitions
//------------------------------------------------------------------
//#define	FALSE   	0
//#define	TRUE    	1
//
#define NULL		0


//------------------------------------------------------------------------------
#define DEBUG_PRINT_EN  			//printf enable for debug
//#define DEBUG_PRINT_EN  			//printf enable for debug
#define UART_SIMULATE_OPEN		//蓝牙通讯开启GPIO1模拟串口打印
#define YY_TEST								//YY调试用
#define uprintf printf

#ifdef DEBUG
#define printfdbg printf
#else
#define printfdbg /\
/printfdbg
#endif

#ifndef DEBUG_PRINT_EN
#define uprintf /\
/uprintf

#endif
#define SCREEN_FLAG
//#define    APPLICATION_DEBUG (1)
//end changed
//------------------------------------------------------------------------------




//------------------------------------------------------------------
//                        TypeDefs
//------------------------------------------------------------------
typedef	unsigned char			UINT8;	///<unsigned char
typedef	unsigned short			UINT16;	///<unsigned char

//------- UINT32 type definition --------------
#ifndef	_UINT32_TYPE_
#define _UINT32_TYPE_
typedef unsigned int            UINT32;	///<unsigned int
#endif


typedef	unsigned char			U08;	///<unsigned char
typedef	unsigned short			U16;	///<unsigned char
typedef unsigned int            U32;	///<unsigned int

//-------- INT8 type definition --------------
#ifndef	_INT8_TYPE_
#define _INT8_TYPE_
typedef	signed char             INT8;	///< char
#endif

typedef	signed short            INT16;	///<short

//-------- INT32 type definition --------------
#ifndef	_INT32_TYPE_
#define _INT32_TYPE_
typedef	signed int              INT32;	///<int
#endif

typedef	signed char             I08;	///< char
typedef	signed short            I016;	///<short
typedef	signed int              I032;	///<int


typedef unsigned char			BOOL;	///<BOOL

//typedef unsigned int 			U32;
//typedef unsigned short 			U16;
typedef unsigned char 			U8;
typedef signed short 			S16;
typedef signed int 				S32;
typedef signed char 			S8;
typedef unsigned long long 		U64;

typedef unsigned int 			u32;
typedef unsigned short 			u16;
typedef unsigned char 			u8;

typedef signed int 			    i32;
typedef signed short 			i16;
typedef signed char 			i8;

typedef unsigned char  T_Bool;
typedef unsigned char  T_U08;
typedef unsigned short T_U16;
typedef unsigned long  T_U32;

typedef char  T_S08;
typedef short T_S16;
typedef long  T_S32;

#define REG8(addr)		  (*(volatile UINT8 *) (addr))
#define REG08(addr)		  (*(volatile UINT8 *) (addr))
#define REG16(addr)		  (*(volatile UINT16 *)(addr))
#define REG32(addr)		  (*(volatile UINT32 *)(addr))

//------------------------------------------------------------------
//                        Exported variables
//------------------------------------------------------------------

//------------------------------------------------------------------
//                        Exported functions
//------------------------------------------------------------------


#define DATA_X  
#define DATA_I  
#define CONSTA   const

#define bool_t u8


/////////////////////////////////////////


#define HIBYTE(word)				((U08)((word) >> 8))
#define LOBYTE(word)				((U08)(word))
#define MAKEWORD(HiByte, LoByte)	((((U16)(HiByte)<<8)&0xFF00) | ((U16)(LoByte)&0x00FF))
#define BYTE2WORD(HiByte, LoByte)	((((U16)(HiByte)<<8)&0xFF00) | ((U16)(LoByte)&0x00FF))




//#define  NULL (void*)0

//------- VUINT8 type definition --------------
#ifndef	_VUINT8_TYPE_
#define _VUINT8_TYPE_
typedef volatile unsigned char  VUINT8;
#endif

//------- VUINT16 type definition --------------
#ifndef	_VUINT16_TYPE_
#define _VUINT16_TYPE_
typedef volatile unsigned short 	VUINT16;
#endif

//------- VUINT32 type definition --------------
#ifndef	_VUINT32_TYPE_
#define _VUINT32_TYPE_
typedef volatile unsigned long  VUINT32;
#endif

#ifndef	_PFILE_TYPE_
#define _PFILE_TYPE_
typedef UINT8*					   PFILE;
#endif

//-------------- General Return Code -------------
#define RT_OK            0x00 //success
#define RT_FAIL          0x01  //fail
#define RT_COMMAND_ERR   0x02  //command error
#define RT_PARAM_ERR     0x03  //param error
#define RT_OVERTIME      0x04  //over time
#define RT_ECC_ERR       0x05  //ecc error
#define RT_WRITE_ERR     0x06  //write flash err
#define RT_READ_ERR      0x07  //read flash err

// define BOOL value
#define FALSE	0
#define TRUE	1
//#define false 	0
//#define true  	1

#endif

