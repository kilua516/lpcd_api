/**
 ****************************************************************
 * @file rc523.c
 *
 * @brief  rc523 driver.
 *
 * @author
 *
 *
 ****************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************
 */
//#include "define.h"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "../sys_msp430/sys_def.h"

#include "sl2523.h"

//#include "Mh523_Spi.h"
#include "iso14443_4.h"
#include "msp430x24x.h"

#define WATER_LEVEL 16 // 
#define FIFO_SIZE   64
#define FSD 256 //Frame Size for proximity coupling Device


#define READ_REG_CTRL   0x80
#define TP_FWT_302us    2048
#define TP_dFWT 192

#define MAX_RX_REQ_WAIT_MS  5000 // 命令等待超时时间100ms


//#define UART_Mode   1
//#define IIC_Mode    0

//#define NFC_DEBUG 1
//#define NOT_IRQ  1

transceive_buffer  mf_com_data;

extern ST_PCDINFO   gtPcdModuleInfo;
volatile u8 irq_flag_io = 0;


/**
 ****************************************************************
 * @brief write_reg()
 *
 * 写芯片的寄存�? *
 * @param:  addr 寄存器地址
 ****************************************************************
 */
void write_reg(u8 addr, u8 RegValue)
//void write_reg( uchar addr, uchar RegValue )
{
#if defined IIC_Mode
    write_word(0x0A,addr,RegValue);   //Mh523 设备地址0x51

#elif defined UART_Mode

    addr = addr & 0x3f;      //code the first byte
    uart1_wrreg(addr,RegValue);


#else
    addr = ( ( addr & 0x3f ) << 1 ) & 0x7F; //code the first byte
    spi_wrreg( addr, RegValue );
#endif
}


/**
 ****************************************************************
 * @brief read_reg()
 *
 * 读芯片的寄存�? *
 * @param: addr 寄存器地址
 * @return: c 寄存器的�? ****************************************************************
 */
uchar read_reg( uchar addr )
{
#if defined IIC_Mode
    return read_word(0x0A,addr);      //Mh523 设备地址0x51

#elif defined UART_Mode

    addr = (addr & 0x3f)|0x80;      //code the first byte
    uart1_rdreg(addr);

#else
    addr = ( ( addr & 0x3f ) << 1 ) | 0x80; //code the first byte
    return spi_rdreg( addr );
#endif
}


/**
 ****************************************************************
 * @brief set_bit_mask()
 *
 * 将寄存器的某些bit位�?
 *
 * @param: reg 寄存器地址
 * @param: mask 需要置位的bit�? ****************************************************************
 */
void set_bit_mask(u8 reg, u8 mask)
{
    char  tmp;

    tmp = read_reg(reg);
    write_reg(reg, tmp | mask);  // set bit mask
}

void write_buf( uchar addr, uchar* data, uint lenth )
{
    addr = ( ( addr & 0x3f ) << 1 ) & 0x7F; //code the first byte
    spi_wrbuf( addr, lenth, data );
}

/*************************************************
Function:       read_buf
Description:
     Read data FROM REGISTER OF  M1010
Parameter:
     *data        The pointer of the value to be READ
      lenth         the lenth of the data to be READ
Return:
     None
**************************************************/

void read_buf( uchar addr, uchar* data, uint lenth )
{
    addr = ( ( addr & 0x3f ) << 1 ) | 0x80; //code the first byte
    spi_rdbuf( addr, lenth, data );
}

/**
 ****************************************************************
 * @brief clear_bit_mask()
 *
 * 将寄存器的某些bit位清0
 *
 * @param: reg 寄存器地址
 * @param: mask 需要清0的bit�? ****************************************************************
 */
void clear_bit_mask(u8 reg,u8 mask)
{
    char  tmp;

    tmp = read_reg(reg);
    write_reg(reg, tmp & ~mask);  // clear bit mask
}



void pcd_reset()
{
#if(NFC_DEBUG)
    printf("pcd_reset\r\n");
#endif
    write_reg(CommandReg, PCD_RESETPHASE); //软复位数字芯�?}
}

void pcd_antenna_on()
{
    write_reg(TxControlReg, read_reg(TxControlReg) | 0x03); //Tx1RFEn=1 Tx2RFEn=1

}

void pcd_antenna_off()
{
    write_reg(TxControlReg, read_reg(TxControlReg) & (~0x03));

}

void pcd_sleep()
{
    write_reg(CommandReg, PCD_CMD_SLEEP);
}

void delay_1ms( uint delay_time ) //62.5ns
{
    uint i;
    uint j;
    for( j = 0; j < delay_time; j++ )
    {
        for( i = 0; i < 16000; i++ );
    }
}


/**
 ****************************************************************
 * @brief pcd_config()
 *
 * 配置芯片的A/B模式
 *
 * @param: u8 type
 * @return:
 * @retval:
 ****************************************************************
 */
char pcd_config(u8 type)
{


    if ('A' == type)
    {
        write_reg(ModeReg,0x3D);    // 11 // CRC seed:6363
        write_reg(TxModeReg, 0x00);//12 //Tx Framing A
        write_reg(RxModeReg, 0x00);//13 //Rx framing A
        write_reg(TxASKReg, 0x40);//15  //typeA
    }
    else if ('B' == type)
    {
//        P1OUT &= ~BIT6;
//        P1OUT &= ~BIT7;
        write_reg(Status2Reg, 0x00);    //��MFCrypto1On
        write_reg(ModeReg, 0x3F);   // CRC seed:FFFF
        write_reg(GsNReg, 0xF8);    //调制系数
        //write_reg(ModGsPReg, 0x0e); //调制指数
        write_reg(ModGsPReg, 0x24); //调制指数
        write_reg(AutoTestReg, 0x00);
        write_reg(TxASKReg, 0x00);  // typeB
        set_bit_mask(TypeBReg, 0xc3);   // 0x13
        write_reg(TxModeReg, 0x83); //Tx Framing B
        write_reg(RxModeReg, 0x83); //Rx framing B
        write_reg(BitFramingReg, 0x00); //TxLastBits=0
    }
    else
    {
        return USER_ERROR;
    }
    return MI_OK;
}

/**
 ****************************************************************
 * @brief pcd_com_transceive()
 *
 * 通过芯片和ISO14443卡通讯
 *
 * @param: pi->mf_command = 芯片命令�? * @param: pi->mf_length  = 发送的数据长度
 * @param: pi->mf_data[]  = 发送数�? * @return: status 值为MI_OK:成功
 * @retval: pi->mf_length  = 接收的数据BIT长度
 * @retval: pi->mf_data[]  = 接收数据
 ****************************************************************
 */
char pcd_com_transceive(struct transceive_buffer *pi)
{
    u8  recebyte;
    char  status;
    u8  irq_en;
    u8  wait_for;
    u8  last_bits;
    u8  j;
    u8  val;
    u8  val2;
    u8  err;

    u8 irq_inv;
    u16  len_rest;
    u8  len;

    len = 0;
    len_rest = 0;
    err = 0;
    recebyte = 0;
    irq_en = 0;
    wait_for = 0;

//printf("TPrescalerReg : 0x%x\r\n", read_reg(TPrescalerReg));
//printf("TModeReg : 0x%x\r\n", read_reg(TModeReg));
//printf("TReloadRegL : 0x%x\r\n", read_reg(TReloadRegL));
//printf("TReloadRegH : 0x%x\r\n", read_reg(TReloadRegH));

    switch (pi->mf_command)
    {
        case PCD_IDLE:
            irq_en   = 0x00;
            wait_for = 0x00;
            break;
        case PCD_AUTHENT:
            irq_en = IdleIEn | TimerIEn;
            wait_for = IdleIRq;
            break;
        case PCD_RECEIVE:
            irq_en   = RxIEn | IdleIEn;
            wait_for = RxIRq;
            recebyte=1;
            break;
        case PCD_TRANSMIT:
            irq_en   = TxIEn | IdleIEn;
            wait_for = TxIRq;
            break;
        case PCD_TRANSCEIVE:
            irq_en = RxIEn | TimerIEn | TxIEn;
            //irq_en = RxIEn | IdleIEn | TxIEn;
            wait_for = RxIRq ;
            recebyte=1;
            break;
        default:
            pi->mf_command = MI_UNKNOWN_COMMAND;
            break;
    }

    if (pi->mf_command != MI_UNKNOWN_COMMAND
        && (((pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT) && pi->mf_length > 0)
            || (pi->mf_command != PCD_TRANSCEIVE && pi->mf_command != PCD_TRANSMIT))
       )
    {
        write_reg(CommandReg, PCD_IDLE);

        irq_inv = read_reg(ComIEnReg) & BIT7;
        write_reg(ComIEnReg, irq_inv |irq_en | BIT0);//使能Timer 定时器中�?        write_reg(ComIrqReg, 0x7F); //Clear INT
        write_reg(ComIrqReg, 0x7F); //Clear INT
        write_reg(DivIrqReg, 0x7F); //Clear INT
        //Flush Fifo
        set_bit_mask(FIFOLevelReg, BIT7);

        if (pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT || pi->mf_command == PCD_AUTHENT)
        {
#if (NFC_DEBUG)
            printf(" PCD_tx:");
#endif
            for (j = 0; j < pi->mf_length; j++)
            {

#if (NFC_DEBUG)
                printf("%02x ", (u16)pi->mf_data[j]);
#endif
            }
#if (NFC_DEBUG)
            printf("\r\n");
#endif

            len_rest = pi->mf_length;
            if (len_rest >= FIFO_SIZE)
            {
                len = FIFO_SIZE;
            }
            else
            {
                len = len_rest;
            }

            for (j = 0; j < len; j++)
            {
                write_reg(FIFODataReg, pi->mf_data[j]);
            }
            len_rest -= len;//Rest bytes
            if (len_rest != 0)
            {
                write_reg(ComIrqReg, BIT2); // clear LoAlertIRq
                set_bit_mask(ComIEnReg, BIT2);// enable LoAlertIRq
            }

            write_reg(CommandReg, pi->mf_command);
            if (pi->mf_command == PCD_TRANSCEIVE)
            {
                set_bit_mask(BitFramingReg,0x80);
            }

            while (len_rest != 0)
            {
#ifdef NOT_IRQ
                while(INT_PIN == 0);//Wait LoAlertIRq
     
#else
                while (irq_flag_io == 0);  //yht
                irq_flag_io = 0;         
#endif
                if (len_rest > (FIFO_SIZE - WATER_LEVEL))
                {
                    len = FIFO_SIZE - WATER_LEVEL;
                }
                else
                {
                    len = len_rest;
                }
                for (j = 0; j < len; j++)
                {
                    write_reg(FIFODataReg, pi->mf_data[pi->mf_length - len_rest + j]);
                }

                write_reg(ComIrqReg, BIT2);//在write fifo之后，再清除中断标记才可�?
                //printf("\n8 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (u8)INT_PIN);
                len_rest -= len;//Rest bytes

                //    printf("len_rest = %d\r\n",len_rest);
                if (len_rest == 0)
                {
                    clear_bit_mask(ComIEnReg, BIT2);// disable LoAlertIRq

                }
            }
            //Wait TxIRq
#ifdef NOT_IRQ
                while(INT_PIN == 0);//Wait LoAlertIRq
     
#else
                while (irq_flag_io == 0);  //yht
                irq_flag_io = 0;         
#endif
            val = read_reg(ComIrqReg);

            if (val & TxIRq)
            {
                //printf("## pcd_com: ComIrqReg =0x%x,ComIEnReg=0x%x\r\n", read_reg(ComIrqReg), read_reg(ComIEnReg));
                write_reg(ComIrqReg, TxIRq);
                val = 0x7F & read_reg(FIFOLevelReg);
            }
            //    val2 = read_reg(ComIrqReg);

            // printf(" INT:ien=%02x,cirq=%02x,err=%02x\r\n",read_reg(ComIEnReg), val,read_reg(ErrorReg));//XU
        }
        if (PCD_RECEIVE == pi->mf_command)
        {
            set_bit_mask(ControlReg, BIT6);// TStartNow
        }

        len_rest = 0; // bytes received
        write_reg(ComIrqReg, BIT3); // clear HoAlertIRq
        set_bit_mask(ComIEnReg, BIT3); // enable HoAlertIRq

        //等待命令执行完成
        // while(INT_PIN == 0);
// while (irq_flag_io == 0);  //yht
        //           irq_flag_io = 0;
//printf("##@ pcd_com: ComIrqReg =0x%x,err=0x%x\r\n", read_reg(ComIrqReg), read_reg(ErrorReg));
        while(1)
        {
#ifdef NOT_IRQ
                while(INT_PIN == 0);//Wait LoAlertIRq
     
#else
                while (irq_flag_io == 0);  //yht
                irq_flag_io = 0;         
#endif

            val = read_reg(ComIrqReg);
            // printf("irq : 0x%x\r\n", val);
            //  printf("ComIrqReg : 0x%x\r\n", read_reg(ComIrqReg));
            if ((val & BIT3) && !(val & BIT5))
            {
                if (len_rest + FIFO_SIZE - WATER_LEVEL > 255)
                {
#if (NFC_DEBUG)
                    printf("AF RX_LEN > 255B\r\n");
#endif
                    break;
                }
                for (j = 0; j <FIFO_SIZE - WATER_LEVEL; j++)
                {
                    pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
                }
                write_reg(ComIrqReg, BIT3);//在read fifo之后，再清除中断标记才可�?                len_rest += FIFO_SIZE - WATER_LEVEL;
                len_rest += FIFO_SIZE - WATER_LEVEL;
            }
            else
            {
                clear_bit_mask(ComIEnReg, BIT3);//disable HoAlertIRq
                break;
            }
        }


        val = read_reg(ComIrqReg);
//       printf(" INT:ien=%02x,cirq=%02x,err=%02x\r\n",read_reg(ComIEnReg), val,read_reg(ErrorReg));//XU
        //      printf("\n irq : 0x%x\r\n", val);
        //     printf(" INT:fflvl=%d,rxlst=%02x ,ien=%02x,cirq=%02x,err=%02x\r\n", (u16)read_reg(FIFOLevelReg),read_reg(ControlReg)&0x07,read_reg(ComIEnReg), val,read_reg(ErrorReg));//XU
#if (NFC_DEBUG)
        //printf(" INT:fflvl=%d,rxlst=%02bx ,ien=%02bx,cirq=%02bx\r\n", (u16)read_reg(FIFOLevelReg),read_reg(ControlReg)&0x07,read_reg(ComIEnReg), val);//XU
        printf(" INT:fflvl=%d,rxlst=%02x ,ien=%02x,cirq=%02x\r\n", (u16)read_reg(FIFOLevelReg),read_reg(ControlReg)&0x07,read_reg(ComIEnReg), val);//XU
        //printf(" INT:fflvl=%d", (u16)read_reg(FIFOLevelReg));//XU
        //printf(" ,rxlst=%02bx",read_reg(ControlReg)&0x07);//XU
        //printf(" ,ien=%02bx", read_reg(ComIEnReg));//XU
        //printf(" ,cirq=%02bx\r\n",val);//XU
#endif
        write_reg(ComIrqReg, val);// 清中�?
        if (val & BIT0)
        {
            //发生超时
            status = MI_NOTAGERR;
            //printf("error : 0x%x\r\n", read_reg(ErrorReg));
        }
        else
        {
            err = read_reg(ErrorReg);

            if( !(err & 0x7f) )  // if not real errirq, except wrrE
            {
                val &= ~ErrIRq;
            }
            err &= 0x7f;  // clear bit7 wrErr
            status = MI_COM_ERR;
            if ((val & wait_for) && (val & irq_en))
            {
                if (!(val & ErrIRq))
                {
                    //指令执行正确
                    status = MI_OK;

                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);

                        //if(0x80 & read_reg(RxModeReg))
                        //val -=2;  // yht crc count num

                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        {
                            //长度过长超出缓存
                            status = MI_COM_ERR;
#if (NFC_DEBUG)
                            printf("RX_LEN > 255B\r\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //防止spi读错�?val-1成为负�?                            {
                            {
                                pi->mf_length = (val-1)*8 + last_bits;
                            }
                            else
                            {
                                pi->mf_length = val*8;
                            }
                            pi->mf_length += len_rest*8;

#if (NFC_DEBUG)
                            printf(" RX:len=%02x,last_bits=%02x,,BitFramingReg=%02x,dat:", (u16)pi->mf_length,last_bits,read_reg(BitFramingReg));
#endif
                            if (val == 0)
                            {
                                val = 1;
                            }
                            for (j = 0; j < val; j++)
                            {
                                pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
                            }

//#if (NFC_DEBUG)
                            for (j = 0; j < pi->mf_length/8 + !!(pi->mf_length%8); j++)
                            {
                                //  if (j > 4)
                                //  {
                                //      printf("..");
                                //      break;
                                //  }
                                //  else
                                //  {
                                printf("%02X ", (u16)pi->mf_data[j]);
                                //  }
                                //printf("%02X ", (u16)pi->mf_data[j]);
                            }
                            //printf("l=%d", pi->mf_length/8 + !!(pi->mf_length%8));
                            printf("\r\n");
//#endif
                        }
                    }
                }
                else if ((err & CollErr) && (!(read_reg(CollReg) & BIT5)))
                {
                    //a bit-collision is detected
                    status = MI_COLLERR;
                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);
                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        {
                            //长度过长超出缓存
#if (NFC_DEBUG)
                            printf("COLL RX_LEN > 255B\r\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //防止spi读错�?val-1成为负�?                            {
                            {
                                pi->mf_length = (val-1)*8 + last_bits;
                            }
                            else
                            {
                                pi->mf_length = val*8;
                            }
                            pi->mf_length += len_rest*8;
#if (NFC_DEBUG)
                            printf(" RX: pi_cmd=%02x,last_bits=%02x,BitFramingReg=%02x,pi_len=%02x,pi_dat:", (u16)pi->mf_command,last_bits,read_reg(BitFramingReg),(u16)pi->mf_length);
#endif
                            if (val == 0)
                            {
                                val = 1;
                            }
                            for (j = 0; j < val; j++)
                            {
                                pi->mf_data[len_rest + j +1] = read_reg(FIFODataReg);
                            }
//#if (NFC_DEBUG)
                            for (j = 0; j < pi->mf_length/8 + !!(pi->mf_length%8); j++)
                            {
                                printf("%02X ", (u16)pi->mf_data[j+1]);
                            }
                            printf("\r\n");
//#endif
                        }
                    }
                    pi->mf_data[0] = (read_reg(CollReg) & CollPos);
                    if (pi->mf_data[0] == 0)
                    {
                        pi->mf_data[0] = 32;
                    }
#if(NFC_DEBUG)
                    printf("\n COLL_DET pos=%02x\r\n", (u16)pi->mf_data[0]);
                    printf("\n collreg=%02x\r\n", read_reg(CollReg));
#endif
                    pi->mf_data[0]--;// 与之前版本有点映射区别，为了不改变上层代码，这里直接减一�?
                }
                else if ((err & CollErr) && (read_reg(CollReg) & BIT5))
                {
                    //printf("COLL_DET,but CollPosNotValid=1\n");
                }
                //else if (err & (CrcErr | ParityErr | ProtocolErr))
                else if (err & (ProtocolErr))
                {
#if (NFC_DEBUG)
                    printf("protocol err=%b02x\r\n", err);
#endif
                    status = MI_FRAMINGERR;
                }
                else if ((err & (CrcErr | ParityErr)) && !(err &ProtocolErr) )
                {
                    //EMV  parity err EMV 307.2.3.4
                    val = 0x7F & read_reg(FIFOLevelReg);
                    last_bits = read_reg(ControlReg) & 0x07;
                    if (len_rest + val > MAX_TRX_BUF_SIZE)
                    {
                        //长度过长超出缓存
                        status = MI_COM_ERR;
#if (NFC_DEBUG)
                        printf("RX_LEN > 255B\r\n");
#endif
                    }
                    else
                    {
                        if (last_bits && val)
                        {
                            pi->mf_length = (val-1)*8 + last_bits;
                        }
                        else
                        {
                            pi->mf_length = val*8;
                        }
                        pi->mf_length += len_rest*8;
                    }
#if (NFC_DEBUG)
                    printf("crc-parity err=%b02x\r\n", err);
                    printf("l=%d\n", pi->mf_length );
#endif



                    status = MI_INTEGRITY_ERR;
                }
                else
                {
#if (NFC_DEBUG)
                    printf("unknown ErrorReg=%02bx\r\n", err);
#endif
                    status = MI_INTEGRITY_ERR;
                }
            }
            else
            {
                status = MI_COM_ERR;
#if (NFC_DEBUG)
                printf(" MI_COM_ERR\r\n");
#endif
            }
        }

        set_bit_mask(ControlReg, BIT7);// TStopNow =1,必要的；
        write_reg(ComIrqReg, 0x7F);// 清中�?
        write_reg(DivIrqReg, 0x7F);// 清中�?
        clear_bit_mask(ComIEnReg, 0x7F);//清中断使�?最高位是控制位
        clear_bit_mask(DivIEnReg, 0x7F);//清中断使�?最高位是控制位
        write_reg(CommandReg, PCD_IDLE);

    }
    else
    {
        status = USER_ERROR;
#if (NFC_DEBUG)
        printf("USER_ERROR\r\n");
#endif
    }

#if (NFC_DEBUG)
    printf(" pcd_com: sta=%bd,err=%02bx\r\n", status, err);
#endif
    return status;
}


/////////////////////////////////////////////////////////////////////
//设置PCD定时�?//input:fwi=0~15
/////////////////////////////////////////////////////////////////////
void pcd_set_tmo(u8 fwi)
{
    write_reg(TPrescalerReg, (TP_FWT_302us) & 0xFF);
    write_reg(TModeReg, BIT7 | (((TP_FWT_302us)>>8) & 0xFF));

    write_reg(TReloadRegL, (1 << fwi)  & 0xFF);
    write_reg(TReloadRegH, ((1 << fwi)  & 0xFF00) >> 8);
}



void pcd_delay_sfgi(u8 sfgi)
{
    //SFGT = (SFGT+dSFGT) = [(256 x 16/fc) x 2^SFGI] + [384/fc x 2^SFGI]
    //dSFGT =  384 x 2^FWI / fc
    write_reg(TPrescalerReg, (TP_FWT_302us + TP_dFWT) & 0xFF);
    write_reg(TModeReg, BIT7 | (((TP_FWT_302us + TP_dFWT)>>8) & 0xFF));

    if (sfgi > 14 || sfgi < 1)
    {
        //FDTA,PCD,MIN = 6078 * 1 / fc
        sfgi = 1;
    }

    write_reg(TReloadRegL, (1 << sfgi) & 0xFF);
    write_reg(TReloadRegH, ((1 << sfgi) >> 8) & 0xFF);

//    write_reg(ComIrqReg, 0x7F);// Çå³ýÖÐ¶ÏWS
//    write_reg(ComIEnReg, BIT0);
    clear_bit_mask(TModeReg, BIT7); // clear TAuto
    set_bit_mask(ControlReg, BIT6); // set TStartNow

    while(!( read_reg(ComIrqReg)  & TimerIRq));

    write_reg(ComIrqReg, TimerIRq ); //clear tmr

}


void pcd_lpcd_config_start(u8 delta, u32 t_inactivity_ms, u8 skip_times,u8  t_detect_us)
{
    u8  WUPeriod;
    u8  SwingsCnt;
#if (NFC_DEBUG)
    printf("pcd_lpcd_config_start\n");
#endif
    WUPeriod = t_inactivity_ms * 32.768 / 256  + 0.5;
    SwingsCnt = t_detect_us * 27.12 / 2 / 16 + 0.5;

    write_reg(0x01,0x0F); //先复位寄存器再进行lpcd

    write_reg(0x14, 0x8B);  // Tx2CW = 1 ，continue载波发射打开
    write_reg(0x37, 0x00);//恢复版本�?    write_reg(0x37, 0x5e);  // 打开私有寄存器保护开�?    write_reg(0x3c, 0x30 | delta);  //设置Delta[3:0]的�? 开�?2k
    write_reg(0x3d, WUPeriod);  //设置休眠时间
    write_reg(0x3e, 0x80 | ((skip_times & 0x07) << 4) | (SwingsCnt & 0x0F));    //开启LPCD_en设置,跳过探测次数，探测时�?    write_reg(0x37, 0x00);  // 关闭私有寄存器保护开�?    write_reg(0x03, 0x20);  //打开卡探测中断使�?    write_reg(0x01, 0x10);  //PCD soft powerdown

    //具体应用相关，本示例工程配置为高电平为有中断
    clear_bit_mask(0x02, BIT7);
}

/*
    lpcd功能开始函�?*/
void pcd_lpcd_start()
{
#if (NFC_DEBUG)
    printf("pcd_lpcd_start\r\n");
#endif

    write_reg(0x01,0x0F); //先复位寄存器再进行lpcd

    write_reg(0x37, 0x00);//恢复版本�?    if (read_reg(0x37) == 0x10)
    {
        write_reg(0x01, 0x00);  // idle
    }
    write_reg(0x14, 0x8B);  // Tx2CW = 1 ，continue载波发射打开

    write_reg(0x37, 0x5e);  // 打开私有寄存器保护开�?
    //write_reg(0x3c, 0x30);    //设置Delta[3:0]的�? 开�?2k //0 不能使用
    //write_reg(0x3c, 0x31);    //设置Delta[3:0]的�? 开�?2k
    //write_reg(0x3c, 0x32);    //设置Delta[3:0]的�? 开�?2k
    //write_reg(0x3c, 0x33);    //设置Delta[3:0]的�? 开�?2k
    //write_reg(0x3c, 0x34);    //设置Delta[3:0]的�? 开�?2k
    //write_reg(0x3c, 0x35);    //设置Delta[3:0]的�? 开�?2k XU
    write_reg(0x3c, 0x37);  //设置Delta[3:0]的�? 开�?2k XU
    //write_reg(0x3c, 0x3A);    //设置Delta[3:0]的�? 开�?2k XU
    //write_reg(0x3c, 0x3F);    //设置Delta[3:0]的�? 开�?2k XU

    write_reg(0x3d, 0x0d);  //设置休眠时间
    write_reg(0x3e, 0x95);  //设置连续探测次数，开启LPCD_en
    write_reg(0x37, 0x00);  // 关闭私有寄存器保护开�?    write_reg(0x03, 0x20);  //打开卡探测中断使�?    write_reg(0x01, 0x10);  //PCD soft powerdown

    //具体应用相关，配置为高电平为有中�?    clear_bit_mask(0x02, BIT7);

}

void pcd_lpcd_end()
{
#if (NFC_DEBUG)
    printf("pcd_lpcd_end\r\n");
#endif
    write_reg(0x01,0x0F); //先复位寄存器再进行lpcd
}

u8 pcd_lpcd_check()
{
    if (INT_PIN && (read_reg(DivIrqReg) & BIT5)) //TagDetIrq
    {
        write_reg(DivIrqReg, BIT5); //清除卡检测到中断
        pcd_lpcd_end();
        return TRUE;
    }
    return FALSE;
}

#if 0
void page45_lock()
{
    write_reg(VersionReg, 0);
}

//打开芯片的page4私有寄存器的写保�?void page4_unlock()
{
    write_reg(VersionReg, 0x5E);
}
//打开芯片的page5私有寄存器的写保�?void page5_unlock();
{
    write_reg(VersionReg, 0xAE);
}
#endif

void pcd_set_rate(u8 rate)
{
    u8 val,rxwait;
    switch(rate)
    {
        case 1:
            clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
            clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
            write_reg(ModWidthReg, 0x26);//Miller Pulse Length

            //     write_reg(RxSelReg, 0x88);

            break;

        case 2:
            clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
            set_bit_mask(TxModeReg, BIT4);
            clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
            set_bit_mask(RxModeReg, BIT4);
            write_reg(ModWidthReg, 0x12);//Miller Pulse Length
            //rxwait相对�?06基本速率需增加相应倍数
            val = read_reg(RxSelReg);
            rxwait = ((val & 0x3F)*2);
            if (rxwait > 0x3F)
            {
                rxwait = 0x3F;
            }
            write_reg(RxSelReg,(rxwait | (val & 0xC0)));

            break;

        case 3:
            clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
            set_bit_mask(TxModeReg, BIT5);
            clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
            set_bit_mask(RxModeReg, BIT5);
            write_reg(ModWidthReg, 0x0A);//Miller Pulse Length
            //rxwait相对�?06基本速率需增加相应倍数
            val = read_reg(RxSelReg);
            rxwait = ((val & 0x3F)*4);
            if (rxwait > 0x3F)
            {
                rxwait = 0x3F;
            }
            write_reg(RxSelReg,(rxwait | (val & 0xC0)));

            break;

        case 4:
            clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
            set_bit_mask(TxModeReg,BIT4|BIT5);
            clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
            set_bit_mask(RxModeReg,BIT4| BIT5);
            write_reg(ModWidthReg, 0x05);//Miller Pulse Length
            //rxwait相对�?06基本速率需增加相应倍数
            val = read_reg(RxSelReg);
            rxwait = ((val & 0x3F)*8);
            if (rxwait > 0x3F)
            {
                rxwait = 0x3F;
            }
            write_reg(RxSelReg,(rxwait | (val & 0xC0)));

            break;


        default:
            clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
            clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
            write_reg(ModWidthReg, 0x26);//Miller Pulse Length

            break;
    }
}

#if 0
void calculate_crc(u8 *pin, u8 len, u8 *pout)
{
    u8  i, n;

    clear_bit_mask(DivIrqReg, 0x04);
    write_reg(CommandReg, PCD_IDLE);
    set_bit_mask(FIFOLevelReg, 0x80);

    for (i = 0; i < len; i++)
    {
        write_reg(FIFODataReg, *(pin + i));
    }
    write_reg(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = read_reg(DivIrqReg);
        i--;
    }
    while((i!=0) && !(n&0x04));

#if (NFC_DEBUG)
    printf("crc:i=%02bx,n=%02bx\n", i, n);
#endif
    pout[0] = read_reg(CRCResultRegL);
    pout[1] = read_reg(CRCResultRegM);
    clear_bit_mask(DivIrqReg, 0x04);
}
#endif





/**
 ****************************************************************
 * @brief pcd_init()
 *
 * 初始化芯片基础寄存�? *
 * @param:
 * @return:
 * @retval:
 ****************************************************************
 */

void pcd_init()
{

    pcd_reset();
    delay_1ms(10);
    clear_bit_mask(Status2Reg, BIT3);
    write_reg(DivIEnReg, 0x80);   // irqpin  cmos output
//    clear_bit_mask(ComIEnReg, BIT7); // 高电�? 会触发中�?    write_reg(WaterLevelReg, 0x10);  //
    write_reg(RxSelReg, 0x88);//RxWait
    //write_reg(CWGsPReg, 0x3f);  //
    write_reg(CWGsPReg, 0x17);  //
    write_reg(RFCfgReg, 0x38); //
    write_reg(ControlReg, 0x10);  //default val

    write_reg(0x3f, 0x01); //
    //write_reg(0x45, 0x54); //
    
   // write_reg(0x46, 0x0c);          //�˲��Ĵ���1
   // write_reg(0x49, 0xff);          //�˲��Ĵ���2
    //write_reg(0x65, 0xc0);          //
    
    write_reg(0x68,0x3);
    write_reg(0x3f,0);

}



