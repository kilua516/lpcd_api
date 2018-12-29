/**************************************************************************************************
//--------------�ļ���Ϣ---------------------------------------------------------------------------
// �� �� ��:    	StatusWord.h
// ��    ��:		Ver1.0
// �� �� ��:		peterzheng
// ��������:		2012.12.14
// �ļ�����:    		����R-APDU��״̬��
//=================================================================================================
//-----------------�޸ļ�¼------------------------------------------------------------------------
// �޸�����:
// ��ǰ�汾:		Ver1.0
// �� �� ��:
// �޸�����:
// ע    ��:
//-------------------------------------------------------------------------------------------------
**************************************************************************************************/
#ifndef _STATUSWORD_H_
#define _STATUSWORD_H_

#define SW_OK						0x9000//��������
#define SW_OK_APP					0x9099//��������
#define SW_DATA_LEFT				0x6100//��������+61XX ��ʣ�����ݳ���
#define SW_DATA_LEN					0x6C00//����6CXX ��ʣ�����ݳ���
#define SW_INVALID_FILE				0x6283//Ӧ������ ��ѡ���ļ���Ч
#define SW_PIN_CNT					0x63C0//����0x63CX,У�����,ʣ��X�γ��Դ���
#define SW_Blue_FAILD			    0x6481//����ͨ��ʧ��
#define SW_OP_FLASH_FAILD			0x6581//����FLASHʧ��
#define SW_LCLE_ERROR				0x6700//���������,����LC���ȴ���,�����������ݽ��ܽ������
#define SW_FILETYPE_ERROR			0x6981//�ļ����Ͳ�ƥ��
#define SW_AC_ERROR					0x6982//��ȫ״̬������
#define SW_KEY_LOCKED				0x6983//��Կ�Ѿ�����ס
#define SW_NEED_RANDOM				0x6984//δȡ�������
#define SW_USE_CONDITION_ERROR		0x6985//ʹ�������������Լ�Ӧ����ʱ����
#define SW_MAC_ERROR				0x6988//��ȫ�����������ȷ
#define SW_CRYPT_ERROR				0x6989//�ӽ��ܴ����쳣
#define SW_DATA_ERROR				0x6A80//�������������ȷ
#define SW_UNSUPPORT				0x6A81//���ܲ�֧��
#define SW_NO_FILE					0x6A82//û���ҵ��ļ�
#define SW_NO_REECORD				0x6A83//û���ҵ���¼
#define SW_NO_SPACE					0x6A84//û���㹻�Ŀռ�
#define SW_PARAM_ERROR				0x6A86//P1��P2��������ȷ
#define SW_INS_ERROR				0x6D00//����ȷ��INS
#define SW_CLA_ERROR				0x6E00//����ȷ��CLA
#define SW_UNDEFINED				0x6F00//δ����Ĵ���(����RSA�����һЩ�����Լ�ȡ��Ӧ��FLAG����)
#define SW_APP_LOCKED				0x9303//Ӧ����������
#define SW_NO_KEY					0x9403//��Կ������֧��
#define SW_APP_LOCKED				0x9303//Ӧ����������
//�Զ���SW
#define SW_CANT_DEAL				0x6900//���ܴ���
#define SW_STATE_ERROR				0x6901//��ǰ״̬����,ָ���ǵ�ǰ״̬������Ԥ��Ҫ��
#define SW_DATA_LACK				0x6902//�ؼ�����ȱʧ
#define SW_EMPTY_SAL_ERROR			0x6903//�ն˺�ѡ�б�Ϊ��
#define SW_RCD_ERROR				0x6904//ģ���ʽ����
#define SW_GPO_RSP_ERROR			0x6905//GPO��Ӧ�����쳣
#define SW_GAC_RSP_ERROR			0x6906//GAC��Ӧ�����쳣
#define SW_RDCACHE_ERROR			0x6907//���ݻ������
#define SW_DATA_AREA_ERROR			0x6908//���ݳ����˱����ֵ��
#define SW_NO_SERVER				0x6909//���������
#define SW_TRANS_ERROR				0x690A//����ģ�鴫�����
#define SW_ONLINE_ERROR				0x690B//����ʧ��
#define SW_FCI_ERROR				0x690C//FCI��ʽ����
#define SW_TL_ERROR					0x690D//ģ���ʽ����
#define SW_LOG_NULL				    0x690E//�������Ϣ
#define SW_GAC_AAC			        0x6910

//qxj�Զ���
#define SW_BLUE_INIT_ERROR          0x6D01	 	//���˻�����
#define SW_INIT_CRC_ERROR           0x6D02        //CRCУ�����
#define SW_INIT_LEN_ERROR           0x6D03       //91ָ��ȴ���

#define SW_INIT_FINISHED            0x6D04 //�豸�Ѿ����˻���
#define SW_INIT_FLASH_ERROR         0x6D05 //дflashʧ��
#define SW_RAND_ERROR               0x6D0B

#define SW_TEST_IC_ERROR            0x6D06 //����IC������
#define SW_TEST_MC_ERROR            0x6D07 //��������������
#define SW_TEST_TIMEOUT         0x6D08 //��ʱ
#define  SW_TEST_M25_M25_FAILED         0x6D09
#define  SW_TEST_M25_M25_OTHER_FAILED         0x6D0A


//new define
#define SW_PUBKEY_NULL				    0x6A88//����ЧRSA��Կ


#endif

