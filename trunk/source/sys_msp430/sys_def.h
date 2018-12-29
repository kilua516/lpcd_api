
#ifndef _DEFINED_H
#define _DEFINED_H

#define VERSION "SL1523_V1.0.1"


#define DEBUG_PRINTF(fmt...)      do { printf("DEBUG  : [%s] ",__FUNCTION__);printf(fmt);} while (0)


typedef unsigned char uchar;
typedef unsigned int    uint;
typedef unsigned char u8;
typedef unsigned short int  u16;
typedef unsigned int  u32;
#if 0
/* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;

#endif
/* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;

#define     __IO    volatile  


/*@{*/
#define SUCCESS                  (0)            /*!< Returned in case of no error when there */
#define ERROR                      (!0)           /*                    isn't any more appropriate code.        */
#define FALSE	0
#define TRUE	1
//#define NULL		0

/*@}*/

#endif


