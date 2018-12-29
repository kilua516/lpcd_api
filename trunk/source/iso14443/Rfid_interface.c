#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include"Rfid_interface.h"
#include "StatusWord.h"
#include "msp430x24x.h"
#include "../sys_msp430/sys_def.h"
#include "sl2523.h"
//#include "Lib.h"

tag_info  g_tag_info;
statistics_t  g_statistics;
u8  g_statistic_refreshed;

u32 g_cos_loop_times = 0;
u8  g_cos_loop = FALSE;
u8  g_loop_buf[60];//不支持太大的cos指令测试

typedef struct
{
    unsigned char       CLA;
    unsigned char       INS;
    unsigned char       P1;
    unsigned char       P2;
    unsigned char       Lc;
    unsigned char       DataIn[300];
    unsigned char       LeFlag;
    unsigned char       Le;
} APDU_SEND;

typedef struct
{
    unsigned int        LenOut;
    unsigned char       DataOut[300];
    unsigned char       SWA;
    unsigned char       SWB;
} APDU_RESP;



void gtag_init()
{
    memset(&g_tag_info, 0, sizeof(g_tag_info));
    memset(&g_statistics, 0, sizeof(g_statistics));
}

void Rfid_Init(void)
{
//  IomInOutSet(11,0);
//  IomSetVal(11,1);
    pcd_init();//初始化pcd寄存器
    pcd_antenna_off(); // 关闭场强
    gtag_init();//初始化rfid本地变量
#if FPGA
    P1OUT |= BIT6;
#endif
}

#if 1
// 用唤醒的方法进行训卡
char reqa_wakeup(void)
{
    char status;
    u8 sak;
    u8 buf[10];

    //g_statistics.reqa_cnt++;
    //g_statistic_refreshed=TRUE;


    pcd_default_info();
    delay_1ms(1);

    status =   pcd_request(WAKEUPA_CMD,g_tag_info.tag_type_bytes);
    //一次防冲突及选卡
    if (status == MI_OK)
    {

        printf("request successful,ATQA:0x%x, 0x%x \r\n",g_tag_info.tag_type_bytes[0],\
               g_tag_info.tag_type_bytes[1]);

        g_tag_info.uid_length = UID_4;
        //make_packet(COM_PKT_CMD_CARD_TYPE, g_tag_info.tag_type_bytes, sizeof(g_tag_info.tag_type_bytes));

        status = pcd_cascaded_anticoll(PICC_ANTICOLL1, 0, &g_tag_info.serial_num[0]);
        if (status == MI_OK)
        {
            printf("anticoll success:UID: 0x%x 0x%x 0x%x 0x%x\r\n",g_tag_info.serial_num[0],g_tag_info.serial_num[1],g_tag_info.serial_num[2],g_tag_info.serial_num[3]);
            status = pcd_cascaded_select(PICC_ANTICOLL1, &g_tag_info.serial_num[0], &sak);
            if(!status)
                printf("select successful: SAK: 0x%x\r\n", sak);
        }
    }
    //二次防冲突及选卡
    if(status == MI_OK && (sak & BIT2))
    {
        printf("SAK: 0x%x\r\n",sak);
        g_tag_info.uid_length = UID_7;
        status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
        if(status == MI_OK)
        {
            printf("secend anticoll successful");
            status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
            if(!status)
                printf("secend select successful: SAK: 0x%x\r\n", sak);
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
    }
    else
    {
        g_statistics.reqa_fail++;
#if(NFC_DEBUG)
        printf("reqa_fail\r\n");
#endif
        delay_1ms(8);
    }

    return status;
}

#endif

char pcd_anticoll_select(void)
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
    }
    else
    {
        g_statistics.reqa_fail++;
#if(NFC_DEBUG)
        printf("reqa_fail\r\n");
#endif
        // delay_1ms(8);
    }

    return status;

}


char reqb_wakeup(uchar baud_speed)
{
    char  status;
    u8  i;
    u8  cnt;
    u8  ATQB[16];
    u8  req_code;
    u8 dsi_dri; // param [1]
    S_ATTRIB attrib_val;
    //g_statistics.reqb_cnt++;
    //g_statistic_refreshed=TRUE;

    req_code = 8;//pcmd[1];

    cnt = 1;//应用中 可以使用轮询N次
    while(cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if(status == (char)MI_COLLERR)    // 有冲突，超过一张卡
        {
            if((status = pcd_request_b(req_code, 0, 2, ATQB)) != MI_OK)
            {
                for (i = 1; i < 4; i++)
                {
                    if((status = pcd_slot_marker(i, ATQB)) == MI_OK)
                    {
                        break;
                    }
                }
                if (status == MI_OK)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else if (status == MI_OK)
        {
            printf("rq-B ,n=%d\r\n", cnt);
            break;
        }
    }

    if (status == MI_OK)
    {
        printf("rq-B OK\r\n");
        //typeB 106????
        switch(baud_speed)
        {
            case 0x1:
                //dsi_dri =0x0;
                attrib_val.param[1]=0x0;
                break;
            case 0x2:
                //   dsi_dri=0x5;
                attrib_val.param[1]=0x5;
                break;
            case 0x3:
                attrib_val.param[1]=0xa;
                break;
            case 0x04:
                attrib_val.param[1]=0xf;
                break;
            default:
                attrib_val.param[1]=0x0;
                break;
        }

        status = pcd_attri_b(&ATQB[1], attrib_val.param[1], ATQB[10]&0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50;//恢复默认值
            //make_packet(COM_PKT_CMD_REQB, ATQB, 12);
            //      printf("ATA-B OK!!\n");

        }
    }

    if(status == MI_OK)
    {
    }
    else
    {
        //g_statistics.reqb_fail++;
#if(NFC_DEBUG)
        printf("reqb_fail\n");
#endif
    }
    return status;
}



int Rfid_exchange(u8 *senddata,u16 tx_len,u8 *recdata,u16 *rx_len)
{
    return(ISO14443_4_HalfDuplexExchange(&gtPcdModuleInfo, senddata, tx_len, recdata, rx_len));
    //return M_ExchangeByteStream( senddata, tx_len,recdata, rx_len );


}

u16 iISO14443TransMit(u8* pbInData, u16 dwInLen, u8* pbOutData, u16* pwOutLen)
{
    u16 wRet;

    if(0 == Rfid_exchange(pbInData,dwInLen, pbOutData, pwOutLen))
    {
#if 0
        printf("iISO14443TransMit recvlen:0x%x  \r\n",*pwOutLen );
        for(i=0; i<*pwOutLen; i++)
        {
            printf("0x%x ", pbOutData[i]);
        }
        printf("iISO14443TransMit end !!!!!!!\r\n: " );
        printf( "out pbOutData = 0x%x\r\n",pbOutData );
#endif
        wRet = *(pbOutData+*pwOutLen-2)<<8|*(pbOutData+*pwOutLen-1);//Convert(pbOutData+*pwOutLen-2);
        *pwOutLen -= 2;
    }
    else
    {

        wRet = SW_TRANS_ERROR;
        *pwOutLen = 0;
    }
    return wRet;
}




int Rfid_isoexchange(u8 cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
    int ucRet = MI_OK;
    u8 ucInData[300];
    u8 ucOutData[300];
    u16  uiSendLen = 0;
    u16   uiRecLen = 0;



    if((ApduSend == NULL) || (ApduRecv == NULL) || cid > 14 || ApduSend->Lc > 255)
    {
        return MI_ERR_PARAM;
    }
    memset(ucInData, 0x00, sizeof(ucInData));
    memset(ucOutData, 0x00, sizeof(ucOutData));

    memcpy(ucInData, (u8*)&(ApduSend->CLA), 4);
    uiSendLen = 4;

    if((ApduSend->Lc == 0) && (ApduSend->Le == 0))
    {
        ucInData[uiSendLen] = 0x00;
        uiSendLen++;
    }

    if(ApduSend->Lc)
    {
        ucInData[uiSendLen] = ApduSend->Lc;
        uiSendLen++;
        memcpy(ucInData+uiSendLen, ApduSend->DataIn, ApduSend->Lc);
        uiSendLen = uiSendLen + ApduSend->Lc;
    }

    if(ApduSend->Le)
    {
        if(ApduSend->Le == 256)
        {
            ucInData[uiSendLen] = 0x00;
        }
        else
        {
            ucInData[uiSendLen] = ApduSend->Le;
        }
        uiSendLen++;
    }

    ucRet = Rfid_exchange(ucInData,uiSendLen,ucOutData,&uiRecLen);

    if(ucRet != MI_OK)
    {
        return ucRet;
    }

    if (uiRecLen < 2)
    {
        ApduRecv->SWA = 0;
        ApduRecv->SWB = 0;
        return MI_ERR_TRANSMIT; // RET_RF_ERR_TRANSMIT
    }


    ApduRecv->LenOut = uiRecLen - 2;
    if((ApduSend->Le < ApduRecv->LenOut)&&(ApduSend->Le > 0))
        ApduRecv->LenOut = ApduSend->Le;
    memcpy(ApduRecv->DataOut, ucOutData, ApduRecv->LenOut);
    ApduRecv->SWA = ucOutData[uiRecLen - 2];
    ApduRecv->SWB = ucOutData[uiRecLen - 1];

    return MI_OK;
}



void Rfid_Open(void)
{
    // pcd_hlta();
    //Soft_Powerdown(0);
    delay_1ms(8);
    pcd_antenna_on();


}


void Rfid_Close(void)
{
    // pcd_hlta();
    pcd_antenna_off();
    pcd_sleep();
}


extern unsigned char sendBuffer[20];
extern unsigned char recvBuffer[20];
char Detect_ContactlessCard(void)
{
    u8 ats[15];
    char status = 1;

    if(reqa_wakeup() == MI_OK)
    {
        if(pcd_rats_a(0,ats) == MI_OK)
        {
            status = 0;
        }
    }

    return status;
}


int test_a(u8 rate)
{
    u8 card_det = 0;
    u16 i,j,testcnd;
    u16 okcnt;
    u8 cur_rate;
    uchar ret,I_block,val;
    u16 uiRecLen;
    uchar len;
    // uchar rate;
    u8 ats[30],ucRet;
    APDU_SEND apdu_s;
    APDU_RESP apdu_r;
    transceive_buffer  *pi;
    pi = &mf_com_data;
    pcd_config('A');
    pcd_antenna_on();
    delay_1ms(5);
    
    if(reqa_wakeup() == MI_OK)
    {
        card_det = 1;

        if(pcd_rats_a(0,ats) == MI_OK)
        {
            printf("Rats_success:recv len = %d\r\n",    mf_com_data.mf_length/8);
            printf("TL:0x%x T0:0x%x TA(1):0x%x TB(1):0x%x TC(1):0x%x \r\n",ats[0],ats[1],ats[2],ats[3],ats[4]);

            pcd_set_tmo(4);
            //rate = 3;
            if( pcd_pps_rate(pi, ats, 0, rate) ==MI_OK)
                printf("pcd_pps_rate_successful  \r\n");
            // rate ++;
            //  if(rate == 4)rate = 1;
            pcd_set_rate(rate);
            okcnt = 0;
            testcnd = 10;
            while(testcnd)
            {
                testcnd--;
                len =5;

                sendBuffer[0] = 0x00;
                sendBuffer[1] = 0x84;
                sendBuffer[2] = 0x00;
                sendBuffer[3] = 0x00;
                sendBuffer[4] = 0x04;

                if(Rfid_exchange(sendBuffer,len,recvBuffer,&uiRecLen) == MI_OK)
                {
                    okcnt ++;
                    //   printf("block_exchange successful \r\n");
                    continue;
                }
                else
                {
                    printf("block_exchange fail %d times \r\n",testcnd);
                    break;
                }

            }

            if(rate == 1)
                cur_rate = 106;
            else if(rate == 2)
                cur_rate = 212;
            else if(rate == 3)
                cur_rate = 424;
            else if(rate == 4)
                cur_rate = 848;

            printf("## TEST 10 times total in rata %d, pass %d times ##\r\n",rate,okcnt);
            iso14443_4_deselect(0);
            delay_1ms(10);
        }

        else
        {
            // printf("Rats_fail:");

        }
//
    }
    else
    {
        // printf("Rats_fail:");
    }
    return card_det;
}


char test_auth(void)
{

    u8 ats[15];
    char i,j;
    uchar ret;

    u8 val;
    char status = 1;

    pcd_config('A');
    pcd_antenna_on();
    delay_1ms(5);

    if(reqa_wakeup() == MI_OK)
    {

        if(pcd_auth_state(PICC_AUTHENT1A,0,g_tag_info.serial_num,"\xff\xff\xff\xff\xff\xff") != MI_OK)
        {
            printf("pcd_auth_state_Failed\r\n");
            return 1;
        }
        else
            printf("pcd_auth_state_succed\r\n");
        //clear_bit_mask(Status2Reg,BIT3) ;//MFCrypto1On
        for(j=0; j<4; j++)
        {
            switch(j)
            {
                case 0:
                    val = 0x0;
                    break;
                case 1:
                    val = 0x5a;
                    break;
                case 2:
                    val = 0xa5;
                    break;
                case 3:
                    val = 0xff;
                    break;

            }
            for(i=0; i<100; i++)
            {
                sendBuffer[i] = i;//val;
            }

            ret = pcd_write(1,sendBuffer);
            if(ret != 0x00)
            {
                printf("write_block  fail erron:0x%x\r\n",ret);
                // break;
                //      continue;
            }

            ret = pcd_read(1,recvBuffer);
            if(ret != 0x00)
            {
                printf("read_block  fail erron:0x%x\r\n",ret);
                return 1;
            }

            for(i=0; i<16; i++)
            {
                if( sendBuffer[i ] != recvBuffer[i])
                    while(1);
            }

        }



    }
    iso14443_4_deselect(0);
    return status;

}


unsigned char block_1[22] = {0x02, 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
void test_b(uchar rate)
{
    u16 i,j,testcnd;
    uchar ret,I_block,val;
    u16 uiRecLen;
    u16 okcnt;
    u8 cur_rate;
    u16 txlen;
    //uchar rate;
    u8 ats[30],ucRet;
    APDU_SEND apdu_s;
    APDU_RESP apdu_r;
    transceive_buffer  *pi;
    pi = &mf_com_data;
    u8 sof,eof;
    sof = 0;
    eof = 0;
    //rate = 0;
    txlen = 0;
    pcd_config('B');
    pcd_antenna_on();
    delay_1ms(3);
    while(1)
    {

        delay_1ms(1000);
        if(reqb_wakeup(rate) == MI_OK)
        {
            printf("reqb_wakeup_success:recv len = %d\r\n",    mf_com_data.mf_length/8);


            if(sof)
            {
                //      set_bit_mask(M_FCONB3,1,0x01);
                //     set_bit_mask(M_FCONB3,1,0x02);
            }
            //EOF低电平最大验证设置为12.5ETU,MP300返回分别为12；12.5；12.6 最后一个有问题
            if(eof)
            {
                //     set_bit_mask(M_FCONB3,1,0xc0);
            }

            // rate ++;
            //  if(rate == 4)rate = 1;
#ifdef FPGA
            if (rate == 1)
            {
                P1OUT &=~BIT7;
            }
            else
            {
                P1OUT |= BIT7;
            }
#endif

            pcd_set_rate(rate);

            okcnt = 0;
            testcnd = 10;
            while(testcnd)
            {
                testcnd--;
                txlen =5;

                sendBuffer[0] = 0x00;
                sendBuffer[1] = 0x84;
                sendBuffer[2] = 0x00;
                sendBuffer[3] = 0x00;
                sendBuffer[4] = 0x04;

                //   delay_1ms(5000);
                if(Rfid_exchange(sendBuffer,txlen,recvBuffer,&uiRecLen) == MI_OK)
                {
                    okcnt ++;
                    //   printf("block_exchange successful \r\n");
                    continue;
                }
                else
                {
                    printf("block_exchange fail status 0x%x  \r\n",ret);
                    break;
                }

            }
            iso14443_4_deselect(0);
            if(rate == 1)
                cur_rate = 106;
            else if(rate == 2)
                cur_rate = 212;
            else if(rate == 3)
                cur_rate = 424;
            else if(rate == 4)
                cur_rate = 848;

            printf("## TEST 10 times total in rata %d, pass %d times ##\r\n",rate,okcnt);
        }
        else
        {
            // printf("type B active fail:");
        }
    }

}





