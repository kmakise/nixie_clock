#ifndef __NIXIEAPP_H
#define __NIXIEAPP_H


typedef enum
{
	WeekUp, 			//外部唤醒
	Sleep,				//休眠模式
	Normal, 			//持续工作模式
	Close,				//关闭显示模式
	SetMenu,			//设置菜单
	SetDate,			//设置日期
	SetTime,			//设置时间
	
}StatusTypedef;//全局状态机状态类型定义


void Nixie_Setup(void); //应用配置
void Nixie_Loop(void);	//应用轮询
void Nixie_Interrupt(void);//应用中断 每1ms一次

#endif /*__NIXIE_H*/
