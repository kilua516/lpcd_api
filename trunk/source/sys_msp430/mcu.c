/*
*************************************************************
 *@Copyright (C), 2016-2020 smartlinkMicro. Co., Ltd.  All rights reserved.
 *@Filename:
 *@Complier:    IAR msp430 5.30
 *@Target OS:   NONE
 *@Target HW:  SL1523
 *@Author:        htyi
 *@Version :     V1.0.0
 *@Date:          2018.5
 *@Description:
 *@History:
 *@    <author>     <time>     <version >   <desc>

*************************************************************
*/
#include "msp430x24x.h"
#include "sys_def.h"
//#include "../ios14443/Mh523.h"
#include "mcu.h"
#include "iic.h"



/*
*            3060 (0)-- SL1523
*            FPGA (1)
             523  --  ex
*      
        sl1523  spi1
*      p4.5  --rst     need modify (p5.7)
        p1.2 -- irq     need modify(p1.0)
        p5.1     simo
        p5.2     somi
        p5.3     clk    
        p4.7     ssn    need modify (p5.5)

ext    spi0  not need modify
        p4.1  --rst
        p1.1 -- irq
        p3.1     simo
        p3.2     somi
        p3.3     clk
        p4.2     ssn          
*
*
*/

extern uint USART1_RX_STA;
extern uint8_t USART1_RX_BUF[20];  //接收缓冲,最大USART1_REC_LEN个字节
/*************************************************
Function:       init_port
Description:
     setting the fuction of muc's io-pins
    such as interrupt/direction/special fuction(spi/uart and so on)
Parameter:
     None
Return:
     None
**************************************************/
#ifdef FPGA
void init_port(void) 
{
    uint i;
    
  //qianfeng RST p4.5
    
//    P4DIR |=BIT5;		    //设置P5为输出   
//    P4REN |= BIT5;                 // ENABLE PULL RESISTOR
// 
//    P4OUT &= ~BIT5;
//    for(i=0;i<0xffff;i++);
//    P4OUT |= BIT5;
    
    
  
    P1DIR &= ~(BIT0+BIT1+BIT2);
    P1IES |= BIT0+BIT1+BIT2;//high to low 
    P1REN |= BIT0+BIT1+BIT2; // ENABLE PULL RESISTOR
    P1OUT |= BIT0+BIT1+BIT2; // PULLUP RESISTOR
    //P3SEL = 0x3E;           // SPI0 UART0
    //P3SEL = 0x3E;
   // P3DIR |= BIT1+BIT3;
//#ifdef IO_SELECT
    //P3SEL = 0x3E;                 //P3的p31,p32,p33,p34,p35 做外设功能模块
//#else
    P3SEL = 0xFE;                 //P3的(p31,p32,p33 spi0),(p34,p35 uart0),(p36,p37 uart1)做外设功能模块
//#endif
    
    P3DIR |= BIT1+BIT3+BIT6+BIT7;           //P31 和 P33输出方向
    P1OUT &= ~0xc0;
    //P4.1 RST_B;P4.2 NSS_B;P4.5 RST_AO; P4.6 STANDBY_A0; P4.7 NSS_A0
    P4DIR |= BIT1+BIT2+BIT5+BIT6+BIT7; 
    P5SEL = 0xE;           // SPI1 
    //p5.5 NSS_A1;P5.6 STANDBY_A1;P5.7 RST_A1;
    P5DIR |= BIT1+BIT3+BIT5+BIT6+BIT7;
    P5OUT |= BIT5;
    P5OUT &= ~BIT7;
    for(i=0;i<0xffff;i++);
    P5OUT |= BIT7;
    P1DIR |= (BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
    P1OUT |= BIT7;//同步
    //P1OUT &=~ BIT7;
    P1OUT &=~ (BIT3 + BIT4 );
    P2DIR |= 0xff;
    P6DIR |= 0xff;
      
    //P6DIR |= 0xff;
}
#else
void init_port(void) 
{
    uint i;

/* init irq & rst */

    /* yht P1.0~2  ==  IRQ0 A B INT */
    
    P1DIR &= ~(BIT0+BIT1+BIT2);  // set bit0 1 2 input
    	// 将中断寄存器清零

    P1SEL &= ~(BIT0+BIT1+BIT2);   // input
     
    P1IES = 0x04;               // 1 选择下降沿中断
    P1IE  = 0x04;               // 打开中断使能
     P1IFG = 0;
    //   P1IES |= BIT0+BIT1+BIT2;//high to low 
    P1REN |= BIT0+BIT1+BIT2; // ENABLE PULL RESISTOR
    P1OUT |= BIT0+BIT1+BIT2; // PULLUP RESISTOR

/* rst 1 5 & nss 2 7 */
    P4DIR |=( BIT1|BIT5|BIT2|BIT7);
    for(i=0;i<0xffff;i++);
    P4OUT |=BIT1; 
    P4OUT |= BIT5;

    /*  yht P3  spi0 for ext */
/* YHT p3 0~4 spi_B for backup    spiB  p3.1.2.3 p4.2,  p4.1 p1.1*/
/* ext_spi p3.1 .2 .3 ; UTXD0 p3.4 URXD0 P3.5 */        
    P3SEL = 0x3E;
    P4OUT |= BIT2;   // nss
  
/* yht spi_A sl1523 */
/* p5.1 .2.3  p4,7 */
    P5SEL = 0xE;           // SPI1 
    P4OUT |= BIT7;     // nss 
}
#endif




/*************************************************
Function:       init_crystal
Description:
     setting the fuction of crystal
     xt2 is 16mhz smclk = xt2  
     LFXT1 is Low frequency mode
     ACLK is vloclk 12khz
     mclk is 16mhz
Parameter:
     None
Return:
     None
**************************************************/
void init_crystal(void) 
{     
    uchar i;
    _DINT();                          // disable interrupt     
    WDTCTL=WDTPW+WDTHOLD;             // stop watch_dog timer
    IE1&=~OFIE;                       // forbid oscillation interrupt when unvalid
    BCSCTL1&=~(XT2OFF+XTS);           // XT2 is 16MHZ ;?????and ACLK is XT1(32768HZ)
                                      // no XT1 in test board and add it in formal board
    BCSCTL3 |= XT2S_2 + LFXT1S_2;     // 3 ?16MHz crystal or resonator
                                      // VLOCLK ACLK = 12khz
    do
    {
        IFG1 &= ~OFIFG;                         // Clear OSCFault flag
        for (i = 0xFF; i > 0; i--);             // Time for flag to set
    }
    while (IFG1 & OFIFG);                     // OSCFault flag still set?
    
    BCSCTL2 |= SELM_2 + SELS;                        // MCLK = XT2 HF XTAL (safe) smclk = xt2
 
}
/*************************************************
Function:       init_uart
Description:
     setting the fuction of uart
     baud is 115200  
     enable rx interrupt
Parameter:
     None
Return:
     None
**************************************************/
void init_uart(void)//usci_a0 uart 115200
{
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK 0x80
    UCA0BR0 = 0x8a;                            // 16MHz 115200   UCA0BR0+UCA0BR1*256 = F/rate
    UCA0BR1 = 0x0;                              // 16MHz 115200
//    UCA0BR0 = 0x82;                             // 16MHz 9600   UCA0BR0+UCA0BR1*256 = F/rate
//    UCA0BR1 = 0x6;                              // 16MHz 9600
    UCA0MCTL = UCBRS1;//UCBRS2 + UCBRS1 + UCBRS0;      // Modulation UCBRSx = 7 not Oversampling
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

/*************************************************
Function:       init_uart1
Description:
     setting the fuction of uart
     baud is 115200  
     enable rx interrupt
Parameter:
     None
Return:
     None
**************************************************/
void init_uart1(void)//usci_a0 uart 115200
{
    UCA1CTL1 |= UCSSEL_2;                     // SMCLK 0x80
//    UCA1BR0 = 0x05;                           // 16MHz 4800   UCA0BR0+UCA0BR1*256 = F/rate
//    UCA1BR1 = 0xd;                            // 16MHz 4800
    UCA1BR0 = 0x82;                            // 16MHz 9600   UCA0BR0+UCA0BR1*256 = F/rate
    UCA1BR1 = 0x6;                              // 16MHz 9600
//      UCA1BR0 = 0xAE;                            // 16MHz 7200   UCA0BR0+UCA0BR1*256 = F/rate
//      UCA1BR1 = 0x8;                              // 16MHz 7200
    UCA1MCTL = UCBRS1;//UCBRS2 + UCBRS1 + UCBRS0;      // Modulation UCBRSx = 7 not Oversampling
    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UC1IE |= UCA1RXIE;                          // Enable USCI_A1 RX interrupt
}

/*************************************************
Function:       init_TA
Description:
     setting the fuction of TIMER
     source clk is 12khz(aclk)  
     timing is 1ms
Parameter:
     None
Return:
     None
**************************************************/

void init_Timer(void)
{
  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = 12;                                // 12khz 1ms                
  TACTL = TASSEL_1 + MC_1;                  // ACLK, UP
}

/****************
   UART1相关程序
****************/
/*************************************************
Function:       uart1_wrreg
Description:
     Write data to register of uart1
Parameter:
     addr         the first value to be written of uart1
     value        the second value to be written of uart1
Return:
     None
**************************************************/
void uart1_wrreg(uchar addr, uchar value)
{   
    USART1_SendData(addr);
   
    USART1_SendData(value);
    while(USART1_RX_STA !=1);
    USART1_RX_STA =0;
    
   
}
/*************************************************
Function:       uart1_rdreg
Description:
     Write data to register of uart1
Parameter:
     addr         the first value to be written of uart1
     value        the second value to be written of uart1
Return:
     None
**************************************************/
uint8_t uart1_rdreg(uchar addr)
{
   
    USART1_SendData(addr);
    while(USART1_RX_STA !=1);
    USART1_RX_STA =0;
    return  USART1_RX_BUF[0];
    
   
}


/****************
   SPI相关程序
****************/
/*************************************************
Function:       init_spi_0
Description:
     1.setup spi clk to 1mhz
     2.Data is captured on the first UCLK edge and changed on the
     following edge
Parameter:
     none
Return:
     None
**************************************************/

//void init_spi_0(void)  //usci_b0 spi  1MHZ //RC523 and SLM1523

void ext_spi_init()
{
    UCB0CTL0 |= UCMSB + UCMST + UCSYNC + BIT7;       // 3-pin, 8-bit SPI mstr, MSB 1st 
    UCB0CTL1 |= UCSSEL_2;                     // SMCLK
    UCB0BR0 = 16;
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

void slm_spi_init(void)
{
    UCB1CTL0 |= UCMSB + UCMST + UCSYNC+BIT7;       // 3-pin, 8-bit SPI mstr, MSB 1st 
    UCB1CTL1 |= UCSSEL_2;                     // SMCLK
    UCB1BR0 = 0x8; //0x4;4M
    UCB1BR1 = 0;
    
    UCB1CTL0 &=~ UCCKPH;
    UCB1CTL0 |= UCCKPL;
    UCB1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    SPI1CS_DE;
    SPI1CS_DE;
}

uchar ext_spi_rate(uchar speed)
{
    uchar status;
    switch(speed)
    {
    case 0x1:
        UCB0BR0 = 16;
        UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    case 0x2:
        UCB0BR0 =8;
        UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    case 0x4:
        UCB0BR0 =4;
        UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    default:
        status = STATUS_ERROR;
        break;
    }
    return status; 
}

uchar slm_spi_rate(uchar speed)
{
    uchar status;
    switch(speed)
    {
    case 0x1:
        UCB1BR0 = 16;
        UCB1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    case 0x2:
        UCB1BR0 =8;
        UCB1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    case 0x4:
        UCB1BR0 =4;
        UCB1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    case 0x08:
        UCB1BR0 = 2;
        UCB1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    case 0x10:
        UCB1BR0 = 1;
        UCB1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        status = STATUS_SUCCESS;
        break;
    default:
        status = STATUS_ERROR;
        break;
    }
    return status; 
}


void spi_mode(uchar mode,uchar speed)
{

    switch(mode)
    {
    case 0x00:
        UCB1CTL0 &=~ (UCCKPH + UCCKPL);
        break;
    case 0x01:
        UCB1CTL0 &=~ UCCKPH;
        UCB1CTL0 |= UCCKPL;
        break;
    case 0x02:
        UCB1CTL0 &=~ UCCKPL;
        UCB1CTL0 |= UCCKPH;
        break;
    case 0x03:
        UCB1CTL0 |= (UCCKPH + UCCKPL);
        break;
    default:
        break;
    }
    slm_spi_rate(speed);
}



/*************************************************
Function:       ext_spi_wrreg
Description:
     Write data to register of spi_0
Parameter:
     addr         the first value to be written of SPI
     value        the second value to be written of SPI
Return:
     None
**************************************************/

void ext_spi_wrreg(uchar addr, uchar value)
{
    uchar i;
    P3SEL = 0x3E;
    SPI0CS_EN;
    for(i=0;i<20;i++);
    
    UCB0TXBUF = addr;
    while (!(IFG2 & UCB0TXIFG));              // USCI_b0 TX buffer ready?
    UCB0TXBUF = value;
    while (!(IFG2 & UCB0TXIFG));              // USCI_b0 TX buffer ready?
    for(i=0;i<20;i++);
    SPI0CS_DE;
    for(i=0;i<100;i++);
    
}


/*************************************************
Function:       spi_wrreg
Description:
     Write data to register of spi_1
Parameter:
     addr         the first value to be written of SPI
     value        the second value to be written of SPI
Return:
     None
**************************************************/
void spi_wrreg(uchar addr, uchar value)
{
    uchar i;
    P5SEL = 0x0E;
    SPI1CS_EN;   
    for(i=0;i<20;i++);
    UCB1TXBUF = addr;
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    UCB1TXBUF = value;
    while (!(UC1IFG & UCB1TXIFG));               // USCI_b0 TX buffer ready?
    for(i=0;i<20;i++);
    SPI1CS_DE;
//    for(i=0;i<100;i++);
    
}

/*************************************************
Function:       ext_spi_wrbuf
Description:
     Write multi-data to register of spi_0
Parameter:
     addr         the first value to be written of SPI
     len          the lenth of data to be written  
     *data        the pointer of the data buffer
Return:
     None
**************************************************/
void ext_spi_wrbuf(uchar addr, uint len,uchar *data)
{
    uint i;
    P3SEL = 0x3E;
    SPI0CS_EN;
    for(i=0;i<20;i++);
    while (!(IFG2 & UCB0TXIFG));              // USCI_b0 TX buffer ready?
    UCB0TXBUF = addr;
    for(i=0; i<len;i++)
    {
        while (!(IFG2 & UCB0TXIFG));               // USCI_b0 TX buffer ready?
        UCB0TXBUF = data[i];
    }
    for(i=0;i<20;i++);
    SPI0CS_EN;
    for(i=0;i<20;i++);
}


/*************************************************
Function:       spi_wrbuf
Description:
     Write multi-data to register of spi_1
Parameter:
     addr         the first value to be written of SPI
     len          the lenth of data to be written  
     *data        the pointer of the data buffer
Return:
     None
**************************************************/
void spi_wrbuf(uchar addr, uint len,uchar *data)
{
    uint i;
    P5SEL = 0x0E;
    SPI1CS_EN;
    
    for(i=0;i<20;i++);
    
    UCB1TXBUF = addr;
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    for(i=0; i<len;i++)
    {
        UCB1TXBUF = data[i];
        while (!(UC1IFG & UCB1TXIFG));               // USCI_b0 TX buffer ready?
    }
    for(i=0;i<20;i++);
    SPI1CS_DE;
 //   for(i=0;i<100;i++);
}








/*************************************************
Function:       ext_spi_rdreg
Description:
     read data from register of spi_0
Parameter:
     addr         the first value to be written of SPI
Return:
     value      the first value to be read of spi
**************************************************/
//#ifdef SPI_t
uchar ext_spi_rdreg(uchar addr)
{
    uchar value,i;
    P3SEL = 0x3E;
    SPI0CS_EN;
    for(i=0;i<20;i++);
    UCB0TXBUF = addr;
    while (!(IFG2 & UCB0TXIFG));              // USCI_b0 TX buffer ready?
    UCB0TXBUF = 0x00;      
    while (!(IFG2 & UCB0TXIFG));              // USCI_b0 TX buffer ready?
    // Dummy write to start SPI
    while (!(IFG2 & UCB0RXIFG));              // USCI_b0 TX buffer ready?
    for(i=0;i<20;i++);
    value = UCB0RXBUF;
    SPI0CS_DE;
    for(i=0;i<100;i++);
    return value;

}

extern void delay_50us(uchar delay_time);

/*************************************************
Function:       spi_rdreg
Description:
     read data from register of spi_1
Parameter:
     addr         the first value to be written of SPI
Return:
     value      the first value to be read of spi
**************************************************/
uchar spi_rdreg(uchar addr)
{
    uchar value,i;
    P5SEL = 0x0E;
    SPI1CS_EN;
    for(i=0;i<20;i++);
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    UCB1TXBUF = addr;
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    UCB1TXBUF = 0;
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    // Dummy write to start SPI
    while (!(UC1IFG & UCB1RXIFG));              // USCI_b0 TX buffer ready?
    for(i=0;i<20;i++);
    value = UCB1RXBUF;
    SPI1CS_DE;
    for(i=0;i<20;i++);
    return value;
}

/*************************************************
Function:       spi_rdbuf
Description:
     Read multi-data from a register of spi_1
Parameter:
     addr         the first address to be read of SPI
     len          the lenth of data to be read  
     *data        the pointer of the data buffer
Return:
     None
**************************************************/
void spi_rdbuf(uchar addr, uint len,uchar *data)
{
    uint i,j;
    P5SEL = 0x0E;
    SPI1CS_EN;
    for(i=0;i<20;i++);
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    UCB1TXBUF = addr;
    for(j=0; j<len-1;j++)
    {
        while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
        UCB1TXBUF = addr;
        while (!(UC1IFG & UCB1RXIFG));              // USCI_b0 TX buffer ready?
        for(i=0;i<200;i++);
        data[j] = UCB1RXBUF ;
    }
    while (!(UC1IFG & UCB1TXIFG));              // USCI_b0 TX buffer ready?
    UCB1TXBUF = 0;
    while (!(UC1IFG & UCB1RXIFG));              // USCI_b0 TX buffer ready?
    for(i=0;i<200;i++);
    data[len-1] = UCB1RXBUF ;
    for(i=0;i<200;i++);
    SPI1CS_DE;
    for(i=0;i<200;i++);
}

void THM_Reset(void)
{
    uchar i;
	// 使芯片处于正常工作状态
	THM_STANDBY_L;    
	// 复位芯片
	THM_RST_L;
    for(i=0;i<100;i++)
    {}
	THM_RST_H;
    delay_1ms(50);
}

void init_mcu(void)
{
    init_crystal(); 
    init_port();
    init_uart();
    init_uart1();  
    ext_spi_init(); // ext 
    slm_spi_init(); // sl1523
    init_Timer();
}


//endfile

