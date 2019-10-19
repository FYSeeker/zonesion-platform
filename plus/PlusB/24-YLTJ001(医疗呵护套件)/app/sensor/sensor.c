/*********************************************************************************************
* 文件：sensor.c
* 作者：
* 说明：通用传感器控制接口程序
* 修改：
* 注释：
*********************************************************************************************/
#include "sensor.h"

unsigned char D0 = 0xff;                                        // 主动上报使能，默认只允许A0主动上报
unsigned char D1 = 0;                                           // 默认关闭控制类传感器
unsigned int V0 = 30;                                           // 主动上报时间间隔，单位秒,0不主动上报
short A0;                                                       // 心率
float A1;                                                       // 红外体温
uint16_t A2, A3;                                                   // 收缩压，舒张压
float A5 =0, A6=0, A7=0;                                        // 板载温度、湿度、电压

/*********************************************************************************************
* 名称：sensor_init()
* 功能：
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void sensor_init(void)
{
    relay_init();                                               //初始化控制类传感器（继电器）
    mlx90615_init();                                            //初始化红外体温传感器
    heartrate_io_Init();                                        //初始化心率传感器管脚
    blood_init();                                               //初始化血压传感器
}

/*********************************************************************************************
* 名称：sensor_type()
* 功能：
* 参数：
* 返回：返回传感器类型，3字节字符串表示
* 修改：
* 注释：
*********************************************************************************************/
char *sensor_type(void)
{
    return SENSOR_TYPE;                                           //返回传感器类型
}

/*********************************************************************************************
* 名称：sensor_control()
* 功能：传感器控制
* 参数：cmd - 控制命令
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensor_control(unsigned char cmd)
{
    if(cmd & 0x01)
    {
        relay_on(1);
    }
    else
    {
        relay_off(1);
    }
    if(cmd & 0x02)
    {
        relay_on(2);
    }
    else
    {
        relay_off(2);
    }
}

/*********************************************************************************************
* 名称：updateA0
* 功能：更新A0的值
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void updateA0(void)
{
    A0 = heartrate_get();
}

/*********************************************************************************************
* 名称：updateA1
* 功能：更新A1的值
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void updateA1(void)
{
    A1 = mlx90615_t();
}

/*********************************************************************************************
* 名称：updateA1
* 功能：更新A2的值
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void updateA2(void)
{
    char buf[16]={0};
    uint16_t sbp,dbp;  
    
    sbp = SBP_get();
    dbp = DBP_get();
    
    if((A2!=sbp)||(A3!=dbp))
    {
        A2 = sbp;                                               //收缩压
        A3 = dbp;                                               //舒张压
        zxbeeBegin();
        sprintf(buf, "%d", A2);                                 //将A2的参数以%f的格式赋予buf
        zxbeeAdd("A2", buf);                                    //调用zxbeeAdd,将A2参数写入发送队列
        sprintf(buf, "%d", A3);                                 //将A3的参数以%f的格式赋予buf
        zxbeeAdd("A3", buf);                                    //调用zxbeeAdd,将A3参数写入发送队列
        rfUartSendData(zxbeeEnd());
    }
}

/*********************************************************************************************
* 名称：updateA5
* 功能：更新A5的值
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void updateA5(void)
{
    A5 = Htu21dTemperature_Get();                               // 更新A5 板载温度
}

/*********************************************************************************************
* 名称：updateA6
* 功能：更新A6的值
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void updateA6(void)
{
    A6 = Htu21dHumidity_Get();                                  // 更新A6 板载湿度
}

/*********************************************************************************************
* 名称：updateA7
* 功能：更新A7的值
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void updateA7(void)
{
    A7 = BatteryVotage_Get();                                   // 更新A7 电池电压
}

/*********************************************************************************************
* 名称：sensor_poll()
* 功能：轮询传感器，并主动上报传感器数据
* 参数：t: 调用次数
* 返回：
* 修改：
* 注释：此函数每秒钟调用1次，t为调用次数
*********************************************************************************************/
void sensor_poll(unsigned int t)
{
    char buf[16] = {0};
    
    updateA0();
    updateA1();
    updateA2();
    updateA5();
    updateA6();
    updateA7();
    
    if (V0 != 0)
    {
        if (t % V0 == 0)
        {
            zxbeeBegin();
            if (D0 & 0x01)
            {
                sprintf(buf, "%d", A0);
                zxbeeAdd("A0", buf);
            }
            if (D0 & 0x02)
            {
                sprintf(buf, "%.1f", A1);
                zxbeeAdd("A1", buf);
            }
            if (D0 & 0x04)
            {
                sprintf(buf, "%d", A2);
                zxbeeAdd("A2", buf);
            }
            if (D0 & 0x08)
            {
                sprintf(buf, "%d", A3);
                zxbeeAdd("A3", buf);
            }
            if (D0 & 0x20)
            {
                sprintf(buf, "%.1f", A5);
                zxbeeAdd("A5", buf);
            }
            if (D0 & 0x40)
            {
                sprintf(buf, "%.1f", A6);
                zxbeeAdd("A6", buf);
            }
            if (D0 & 0x80)
            {
                sprintf(buf, "%.1f", A7);
                zxbeeAdd("A7", buf);
            }
            char *p = zxbeeEnd();
            if (p != NULL)
            {
                rfUartSendData(p);                              //发送无线数据
            }
        }
    }
}

/*********************************************************************************************
* 名称：sensor_check()
* 功能：周期性检查函数，可设定轮询时间
* 参数：无
* 返回：设置轮询的时间，单位ms,范围:1-65535
* 修改：
* 注释：此函数用于需要快速做出相应的传感器
*********************************************************************************************/
unsigned short sensor_check()
{
    return 100;                                                 //返回值决定下次调用时间，此处为100ms
}

/*********************************************************************************************
* 名称：z_process_command_call()
* 功能：处理上层应用发过来的指令
* 参数：ptag: 指令标识 D0，D1， A0 ...
*       pval: 指令值， “？”表示读取，
*       obuf: 指令处理结果存放地址
* 返回：>0指令处理结果返回数据长度，0：没有返回数据，<0：不支持指令。
* 修改：
* 注释：
*********************************************************************************************/
int z_process_command_call(char* ptag, char* pval, char* obuf)
{
    int ret = -1;
    if (memcmp(ptag, "D0", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "D0=%d", D0);
        }
    }
    if (memcmp(ptag, "CD0", 3) == 0)
    {
        int v = atoi(pval);
        if (v > 0)
        {
            D0 &= ~v;
        }
    }
    if (memcmp(ptag, "OD0", 3) == 0)
    {
        int v = atoi(pval);
        if (v > 0)
        {
            D0 |= v;
        }
    }
    if (memcmp(ptag, "D1", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "D1=%d", D1);
        }
    }
    if (memcmp(ptag, "CD1", 3) == 0)  				            //若检测到CD1指令
    {
        int v = atoi(pval);                                     //获取CD1数据
        D1 &= ~v;                                               //更新D1数据
        sensor_control(D1);                                     //更新电磁阀直流电机状态
    }
    if (memcmp(ptag, "OD1", 3) == 0)  				            //若检测到OD1指令
    {
        int v = atoi(pval);                                     //获取OD1数据
        D1 |= v;                                                //更新D1数据
        sensor_control(D1);                                     //更新电磁阀直流电机状态
    }
    if (memcmp(ptag, "V0", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "V0=%d", V0);
        }
        else
        {
            V0 = atoi(pval);
        }
    }
    if (memcmp(ptag, "A0", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A0=%d", A0);
        }
    }
    if (memcmp(ptag, "A1", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A1=%.1f", A1);
        }
    }
    if (memcmp(ptag, "A2", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A2=%d", A2);
        }
    }
    if (memcmp(ptag, "A3", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A3=%d", A3);
        }
    }
    if (memcmp(ptag, "A4", 2) == 0)
    {
        if (pval[0] == '?')
        {
            //
        }
    }
    if (memcmp(ptag, "A5", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A5=%.1f", A5);
        }
    }
    if (memcmp(ptag, "A6", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A6=%.1f", A6);
        }
    }
    if (memcmp(ptag, "A7", 2) == 0)
    {
        if (pval[0] == '?')
        {
            ret = sprintf(obuf, "A7=%.1f", A7);
        }
    }
    return ret;
}

