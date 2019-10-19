/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Zhoucj 2018.6.27
* ˵��������������
*       �ڵ�߱��������ԣ���ʱ�ϱ���������,ʵʱ�������������ֵ
*       ����A0��ʾ����ֵ������A1��ʾ��������
*       V0��ʾ����ֵ�����ϱ�ʱ����
*       D0(Bit0/Bit1)�ֱ��ʾA0/A1�����ϱ�ʹ��
*       D1��ʾ�̵�������״̬��OD1/CD1���п���
*       Ĭ��ֵ��A0=0,A1=0,V0=30,D0=3
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sapi.h"
#include "osal_nv.h"
#include "addrmgr.h"
#include "mt.h"
#include "hal_led.h"
#include "hal_adc.h"
#include "hal_uart.h"
#include "sensor.h"
#include "zxbee.h"
#include "zxbee-inf.h"
#include "Noise.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define RELAY1                  P0_6                            // ����̵�����������
#define RELAY2                  P0_7                            // ����̵�����������
#define ON                      0                               // �궨���״̬����ΪON
#define OFF                     1                               // �궨��ر�״̬����ΪOFF

#define Noise_alarm             10                              // �궨��������ֵ
/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static uint8 D0 = 3;                                            // Ĭ�ϴ������ϱ�����
static float A0 = 0.0;                                          // A0�洢����ֵ
static uint8 A1 = 0;                                            // A1���汨��ֵ
static uint16 V0 = 30;                                          // V0����Ϊ�ϱ�ʱ������Ĭ��Ϊ30s
/*********************************************************************************************
* ���ƣ�updateV0()
* ���ܣ�����V0��ֵ
* ������*val -- �����µı���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateV0(char *val)
{
  //���ַ�������val����ת��Ϊ���ͱ�����ֵ
  V0 = atoi(val);                                               // ��ȡ�����ϱ�ʱ�����ֵ
}
/*********************************************************************************************
* ���ƣ�updateA0()
* ���ܣ�����A0��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA0(void)
{
  // ��ȡ����ֵ��������A0
  Noise_update();
  A0 = get_Noise_val();
}
/*********************************************************************************************
* ���ƣ�updateA1()
* ���ܣ�����A1��ֵ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void updateA1(void)
{
  if(A0 > Noise_alarm)
    A1 = 1;
  else
    A1 = 0;
}
/*********************************************************************************************
* ���ƣ�sensorInit()
* ���ܣ�������Ӳ����ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorInit(void)
{
  // ��ʼ������������
  Noise_init();
  
  // ������ʱ���������������ϱ������¼���MY_REPORT_EVT
  osal_start_timerEx(sapi_TaskID, MY_REPORT_EVT, (uint16)((osal_rand()%10) * 1000));
  // ������ʱ������������������¼���MY_CHECK_EVT
  osal_start_timerEx(sapi_TaskID, MY_CHECK_EVT, 100);
}
/*********************************************************************************************
* ���ƣ�sensorLinkOn()
* ���ܣ��������ڵ������ɹ����ú���
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorLinkOn(void)
{
  sensorUpdate();
}
/*********************************************************************************************
* ���ƣ�sensorUpdate()
* ���ܣ����������ϱ�������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorUpdate(void)
{ 
  char pData[16];
  char *p = pData;
  
  ZXBeeBegin();                                                 // ��������֡��ʽ��ͷ
  
  // ����D0��λ״̬�ж���Ҫ�����ϱ�����ֵ
  if ((D0 & 0x01) == 0x01){                                     // ���¶��ϱ���������pData�����ݰ��������¶�����
    updateA0();
    sprintf(p, "%.1f", A0); 
    ZXBeeAdd("A0", p);
  }
  if ((D0 & 0x02) == 0x02){
    updateA1();
    sprintf(p, "%u", A1);
    ZXBeeAdd("A1", p);
  }
  
  p = ZXBeeEnd();                                               // ��������֡��ʽ��β
  if (p != NULL) {												
    ZXBeeInfSend(p, strlen(p));	                                // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
  }
}
/*********************************************************************************************
* ���ƣ�sensorCheck()
* ���ܣ����������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorCheck(void)
{
  char pData[16];
  char *p = pData;
  static uint8 last_A1 = 0;
  
  updateA0();
  updateA1();
  if(last_A1 != A1)
  { 
    ZXBeeBegin();                                               // ��������֡��ʽ��ͷ
    if ((D0 & 0x02) == 0x02){
      updateA1();
      sprintf(p, "%u", A1);
      ZXBeeAdd("A1", p);
    }
    p = ZXBeeEnd();                                             // ��������֡��ʽ��β
    if (p != NULL) {												
      ZXBeeInfSend(p, strlen(p));	                            // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
    }
    last_A1 = A1;
  }
}
/*********************************************************************************************
* ���ƣ�sensorControl()
* ���ܣ�����������
* ������cmd - ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorControl(uint8 cmd)
{
  // ����cmd����������Ӧ�Ŀ��Ƴ���
  if(cmd & 0x01){ 
    RELAY1 = ON;                                                // �����̵���1
  }
  else{
    RELAY1 = OFF;                                               // �رռ̵���1
  }
  if(cmd & 0x02){  
    RELAY2 = ON;                                                // �����̵���2
  }
  else{
    RELAY2 = OFF;                                               // �رռ̵���2        
  }
}
/*********************************************************************************************
* ���ƣ�ZXBeeUserProcess()
* ���ܣ������յ��Ŀ�������
* ������*ptag -- ������������
*       *pval -- �����������
* ���أ�ret -- pout�ַ�������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{ 
  int val;
  int ret = 0;	
  char pData[16];
  char *p = pData;
  
  // ���ַ�������pval����ת��Ϊ���ͱ�����ֵ
  val = atoi(pval);	
  // �����������
  if (0 == strcmp("CD0", ptag)){                                // ��D0��λ���в�����CD0��ʾλ�������
    D0 &= ~val;
  }
  if (0 == strcmp("OD0", ptag)){                                // ��D0��λ���в�����OD0��ʾλ��һ����
    D0 |= val;
  }
  if (0 == strcmp("D0", ptag)){                                 // ��ѯ�ϱ�ʹ�ܱ���
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", D0);
      ZXBeeAdd("D0", p);
    } 
  }
  if (0 == strcmp("A0", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA0();
      ret = sprintf(p, "%.1f", A0);     
      ZXBeeAdd("A0", p);
    } 
  }
  if (0 == strcmp("A1", ptag)){ 
    if (0 == strcmp("?", pval)){
      updateA1();
      ret = sprintf(p, "%u", A1);     
      ZXBeeAdd("A1", p);
    } 
  }
  if (0 == strcmp("V0", ptag)){
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", V0);                         	    // �ϱ�ʱ����
      ZXBeeAdd("V0", p);
    }else{
      updateV0(pval);
    }
  }
  return ret;
}
/*********************************************************************************************
* ���ƣ�MyEventProcess()
* ���ܣ��Զ����¼�����
* ������event -- �¼����
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void MyEventProcess( uint16 event )
{
  if (event & MY_REPORT_EVT) { 
    sensorUpdate();
    //������ʱ���������¼���MY_REPORT_EVT 
    osal_start_timerEx(sapi_TaskID, MY_REPORT_EVT, V0*1000);
  }  
  if (event & MY_CHECK_EVT) { 
    sensorCheck(); 
    // ������ʱ���������¼���MY_CHECK_EVT 
    osal_start_timerEx(sapi_TaskID, MY_CHECK_EVT, 1000);
  } 
}