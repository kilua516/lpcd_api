#ifndef MCU_H
#define  MCU_H

/********************************
DEFINE MCT TO RC523 PIN
********************************/
#define SPI0CS_EN (P4OUT &= ~BIT2) 
#define SPI0CS_DE (P4OUT |= BIT2)
#define RC523RST_L (P4OUT &= ~BIT1)
#define RC523RST_H (P4OUT |= BIT1)


/********************************
DEFINE MCT TO RC523 PIN
********************************/

#define THM_STANDBY_L P4OUT &=~ BIT6
#define THM_STANDBY_H P4OUT |= BIT6

#define THM_RST_H P4OUT |= BIT5
#define THM_RST_L P4OUT &=~ BIT5



#ifdef FPGA


#define SPI1CS_EN P5OUT &= ~BIT5 
#define SPI1CS_DE P5OUT |= BIT5

#define SL1523RST_H P4OUT |= BIT5
#define SL1523RST_L P4OUT &=~ BIT5

#define SPI2CS_EN P4OUT &= ~BIT7 
#define SPI2CS_DE P4OUT |= BIT7

#else

#define SPI1CS_EN      (P4OUT &= ~BIT7)
#define SPI1CS_DE      (P4OUT |= BIT7)
#define SL1523RST_L  (P4OUT &= ~BIT5)
#define SL1523RST_H (P4OUT |= BIT5)


#endif
#define STATUS_SUCCESS                  (0x00)            /*!< Returned in case of no error when there*/

#define STATUS_ERROR                 (0x01)            /*!< Returned in case of no error when there*/

uchar spi_rdreg( uchar addr );
void spi_wrreg( uchar addr, uchar value );
void spi_wrbuf( uchar addr, uint len, uchar* data );
void spi_rdbuf( uchar addr, uint len, uchar* data );
void spi_mode( uchar mode, uchar speed );
void init_mcu(void);

uint8_t uart1_rdreg(uchar addr);
void uart1_wrreg(uchar addr, uchar value);

#endif
// endfile

