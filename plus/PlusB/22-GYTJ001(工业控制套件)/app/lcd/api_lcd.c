/*********************************************************************************************
* 文件：api_lcd.c
* 作者：fuyou 2018.11.1
* 说明：lcd与传感器的修改部分
* 修改：
* 注释：
*********************************************************************************************/
#include "api_lcd.h"


extern char A0;
extern char D1;
extern void sensor_control(unsigned char cmd);

/*********************************************************************************************
* 名称：option_1_Handle()
* 功能：菜单项1的处理函数
* 参数：status:菜单项的选定状态
* 返回：无
* 修改：
* 注释：自定义菜单项1被选定后进行的操作
*********************************************************************************************/
void option_1_Handle(optionStatus_t status)
{
    menuExitHandle();
}

/*********************************************************************************************
* 名称：option_2_Handle()
* 功能：菜单项2的处理函数
* 参数：status:菜单项的选定状态
* 返回：无
* 修改：
* 注释：自定义菜单项2被选定后进行的操作
*********************************************************************************************/
void option_2_Handle(optionStatus_t status)
{
    if(status==SELECT)
    {
        D1 |= 0x02;
    }
    else if(status==UNSELECT)
    {
        D1 &= ~(0x02);
    }
    sensor_control(D1);
}

/*********************************************************************************************
* 名称：option_3_Handle()
* 功能：菜单项3的处理函数
* 参数：status:菜单项的选定状态
* 返回：无
* 修改：
* 注释：自定义菜单项3被选定后进行的操作
*********************************************************************************************/
void option_3_Handle(optionStatus_t status)
{
    if(status==SELECT)
    {
        D1 |= 0x01;
    }
    else if(status==UNSELECT)
    {
        D1 &= ~(0x01);
    }
    sensor_control(D1);
}

/*********************************************************************************************
* 名称：MenuOption_init()
* 功能：菜单项初始化
* 参数：无
* 返回：无
* 修改：
* 注释：自定义菜单项的内容和功能
*********************************************************************************************/
void MenuOption_init()
{
    //菜单项1的初始化
    optionDefaultName_set(1," 1-返回主界面");     //菜单项默认名称
    optionSelectName_set(1," 1-返回主界面");      //菜单项被选定后的名称
    optionHintInfo_set(1," ");                  //菜单项被选定后的提示信息
    optionCallFunc_set(1,option_1_Handle);      //菜单项被选定后的处理函数
    
    //菜单项2的初始化
    optionDefaultName_set(2," 2-打开电磁阀门");
    optionSelectName_set(2," 2-关闭电磁阀门");
    optionHintInfo_set(2,"电磁阀已打开");
    optionCallFunc_set(2,option_2_Handle);
    
    //菜单项3的初始化
    optionDefaultName_set(3," 3-启动直流电机");
    optionSelectName_set(3," 3-关闭直流电机");
    optionHintInfo_set(3,"电机已启动");
    optionCallFunc_set(3,option_3_Handle);
}

/*********************************************************************************************
* 名称：TableInfo_init()
* 功能：表格信息初始化
* 参数：无
* 返回：无
* 修改：
* 注释：自定义表格需要显示的信息
*********************************************************************************************/
void TableInfo_init()
{
    tableExplain_set(1,"网线接口");
    tableExplain_set(2,"-");
    tableExplain_set(3,"电磁阀门");
    tableExplain_set(4,"直流电机");
    tableExplain_set(5,"接近开关");
}

/*********************************************************************************************
* 名称：lcdSensor_init()
* 功能：lcd与传感器部分初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void lcdSensor_init()
{
    MenuOption_init();
    TableInfo_init();
    SensorName_set("电磁阀门","直流电机","接近开关");
}

/*********************************************************************************************
* 名称：syncSensorStatus()
* 功能：同步控制类传感器状态
* 参数：无
* 返回：无
* 修改：
* 注释：控制类传感器被上层设置后，需要运行此函数同步屏幕上的显示状态
*********************************************************************************************/
void syncSensorStatus()
{
    //这里根据D1判断继电器被控制的状态
    if(D1&0x02)
        optionState_set(2,SELECT);      //设置选项2为选择状态
    else
        optionState_set(2,UNSELECT);    //设置选项2为取消选择状态
    
    if(D1&0x01)
        optionState_set(3,SELECT);      //设置选项3为选择状态
    else
        optionState_set(3,UNSELECT);    //设置选项3为取消选择状态
}

/*********************************************************************************************
* 名称：lcdSensor_poll()
* 功能：lcd与传感器部分轮询
* 参数：无
* 返回：无
* 修改：
* 注释：轮询用于传感器数据的更新
*********************************************************************************************/
void lcdSensor_poll()
{
    char pbuf1[16]={0};
    char pbuf2[16]={0};
    char pbuf3[16]={0};
    
    //更新数据/状态
    if(D1&0x02)
        sprintf(pbuf1,"打开");
    else
        sprintf(pbuf1,"关闭");
    
    if(D1&0x01)
        sprintf(pbuf2,"打开");
    else
        sprintf(pbuf2,"关闭");
    
    sprintf(pbuf3,"%u",A0);
    
    //更新页面1的传感器数据
    SensorData_set(pbuf1,pbuf2,pbuf3);
    //更新页面2的表格数据
    tableData_set(1,"-");
    tableData_set(2,"-");
    tableData_set(3,pbuf1);
    tableData_set(4,pbuf2);
    tableData_set(5,pbuf3);
    //同步控制类传感器显示状态
    syncSensorStatus();
}

