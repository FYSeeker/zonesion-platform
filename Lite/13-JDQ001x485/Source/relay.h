/*********************************************************************************************
* �ļ���relay.h
* ���ߣ�
* ˵����relayͷ�ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#ifndef _RELAY_h_
#define _RELAY_h_

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "hal_types.h"
#include <ioCC2530.h>

void relay_init(void);
void relay_on(unsigned char cmd);
void relay_off(unsigned char cmd);
unsigned char get_relay_status(void);

#endif