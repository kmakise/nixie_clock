/******************************************************************************
	* @file    NixieApp.c
	* @author  张东
	* @version V1.0.0
	* @date    2019-3-25
	* @brief   nixie tube clock applicetion file
	******************************************************************************/

/*Iniclude --------------------------------------------------------------------*/
#include "stm32f10x.h"
//system
#include "sys.h"
#include "delay.h"
#include "tim.h"
//hardware
#include "oled.h"
#include "HighVoltage.h"
#include "NixieTube.h"
#include "KeyBoard.h"
#include "LEDTimer.h"
#include "DS3231.h"
#include "Beep.h"
//data space
#include "bmp.h"
#include "stdio.h"
#include "NixieApp.h"

/* Global Data Space ----------------------------------------------------------*/
StatusTypedef g_state = WeekUp;//全局状态 默认为唤醒状态
uint32_t g_WeekUpTick = 0;     //唤醒时间计算 单位：秒

/*Nixie and LED Display -------------------------------------------------------*/
void NixieTimeDisplay(void)
{
	static uint8_t str[7] = {0,0,0,0,0,0};
	str[0] = TimeValue.hour / 10;
	str[1] = TimeValue.hour % 10;
	str[2] = TimeValue.minute / 10;
	str[3] = TimeValue.minute % 10;
	str[4] = TimeValue.second / 10;
	str[5] = TimeValue.second % 10;
	NixieTubeDisplay(str);
}
void GetTimeValue(void)
{
	static uint16_t div = 0;
	div++;
	if(div > 300)//300分频
	{
		div = 0;
		Time_Handle(); //每秒向DS3231 获取三次时间信息
	}
}
//唤醒模式计时
void weekUpTick(void)
{
	static uint16_t div = 0;
	div++;
	if(div > 1000)//1000分频 = 1s
	{
		div = 0;
		g_WeekUpTick++;
	}
}
//分钟进位闪烁显示
void NixieAddFlash(void)
{
	static uint8_t flag = 0;
	if(TimeValue.second == 0)
	{
		if(flag == 0)
		{
			flag = 1;
			NixieTubeFlash();
		}
	}
	else
	{
		flag = 0;
	}
} 
//每5秒闪烁系统心跳
void LEDHartAttack(void)
{
	static uint8_t attack = 0;
	if(TimeValue.second % 5 == 0)
	{
		if(attack == 0)
		{
			attack = 1;
			LED_HartCmd(ENABLE);
			delay_ms(50);
			LED_HartCmd(DISABLE);
		}
	}
	else
	{
		attack = 0; 
	}
	
}

/*Oled display ----------------------------------------------------------------*/
void StatusView(void)
{
	//显示当前模式
	switch(g_state)
	{
		case WeekUp :OLED_ShowString(0,0,(uint8_t *)"Mode:WEEKUP ",11);break;
		case Sleep	:OLED_ShowString(0,0,(uint8_t *)"Mode:SLEEP  ",11);break;
		case Normal :OLED_ShowString(0,0,(uint8_t *)"Mode:NORMAL ",11);break;
		case Close	:OLED_ShowString(0,0,(uint8_t *)"Mode:CLOSE  ",11);break;
		case SetMenu:OLED_ShowString(0,0,(uint8_t *)"Menu:SETMENU",11);break;
		case SetDate:OLED_ShowString(0,0,(uint8_t *)"Menu:SETDATE",11);break;
		case SetTime:OLED_ShowString(0,0,(uint8_t *)"Menu:SETTIME",11);break;
		default:break;
	}
}
void InfoDisplay(void)
{
	uint8_t str[30];
	uint8_t temp[30];
	//显示时间
	sprintf((char *)str,"Time>:%02d:%02d:%02d",TimeValue.hour,TimeValue.minute,TimeValue.second);
	OLED_ShowString(0,2,(uint8_t *)str,13);
	//显示日期
	sprintf((char *)str,"Date>:20%02d/%02d/%02d",TimeValue.year,TimeValue.month,TimeValue.date);
	OLED_ShowString(0,4,(uint8_t *)str,13);
	//显示温度
	DS3231_Read_Temp(temp);
	temp[5] = '\0';
	sprintf((char *)str,"Temp>:%sC",temp);
	OLED_ShowString(0,6,(uint8_t *)str,13);
	//显示选项和光标
	sprintf((char *)str,"->Menu");
	OLED_ShowString(80,7,(uint8_t *)str,13);
}
/*Key event ---------------------------------------------------------*/

//前面板按键触发事件
void KeyEvent_NT(void)
{
	//当前状态不为设置模式下启用
	if(g_state != SetMenu && g_state != SetDate && g_state != SetTime)
	{
		//按键事件
		switch(getKey_NT())
		{
			case 1://按键Normal被按下
			{
				//如果当前模式为Normal
				if(g_state == Normal)
				{
					g_state = WeekUp;
					BeepSpeakCmd(3,100);
				}
				else
				{
					g_state = Normal;
					BeepSpeakCmd(1,800);
				}
				while(getKey_NT() == 1);
				OLED_Clear();
				break;
			}
			case 2://按键Close被按下
			{
				//如果当前模式为Close
				if(g_state == Close)
				{
					g_state = WeekUp;
					BeepSpeakCmd(3,100);
				}
				else
				{
					g_state = Close;
					BeepSpeakCmd(1,800);
				}
				while(getKey_NT() == 2);
				OLED_Clear();
				break;
			}
		}
	}
}
//设置控制按键面板事件 状态响应 wsnc 目标状态 setMenu
void KeyEvent_Ctrl_WSNC2SetMenu(void)
{
	if(getKey_Ctrl() == 5)
	{
		g_state = SetMenu;
		BeepSpeakCmd(1,100);
		while(getKey_Ctrl() == 5);
		OLED_Clear();
	}
}

//环境侦测 用于重置weekup计时以及状态的跳转
void EnvironmentEvent(void)
{
	//4，294，967,296 秒 ~ 136年 毫秒 ~ 50天
	  
	//当前模式不为weekup时清除计时
	if(g_state != WeekUp)
	{
		g_WeekUpTick = 0;
	}
	//weekup模式时等待计时到超时时间点后跳转状态到sleep
	if(g_state == WeekUp)
	{
		//计时达到30分钟后跳转到sleep状态 
		if(g_WeekUpTick > 10)
		{
			g_state = Sleep;
		}
	}
	//sleep模式时等待环境变量激活后跳转到weekup
	if(getKey_EN() == 1)
	{
		g_state = WeekUp;
		g_WeekUpTick = 0; 
		OLED_ShowString(112,0,(uint8_t *)"-A",13);
	}
	else
	{
		OLED_ShowString(112,0,(uint8_t *)"-N",13);
	}
}


/*state function body -----------------------------------------------*/
//主菜单模式功能方法
void MenuStateFunc(void)
{
	static uint8_t cursor = 0;//光标序号 setdate：0 settime:1 back:2
		
	//显示界面选项和光标
	if(cursor == 0)OLED_ShowString(0,3,(uint8_t *)"->Set Date",13);
	else OLED_ShowString(0,3,(uint8_t *)"  Set Date",13);
	if(cursor == 1)OLED_ShowString(0,5,(uint8_t *)"->Set Time",13);
	else OLED_ShowString(0,5,(uint8_t *)"  Set Time",13);
	if(cursor == 2)OLED_ShowString(0,7,(uint8_t *)"->Back",13);
	else OLED_ShowString(0,7,(uint8_t *)"  Back",13);
	//响应按键切换 上左+ 下右- 确认跳转
	switch(getKey_Ctrl())
	{
		case 1://上
		case 3://左
		{
			cursor = (cursor > 0)?cursor-1:2;
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==1 || getKey_Ctrl()== 3);
			break;
		}
		case 2://下
		case 4://右
		{
			cursor = (cursor < 2)?cursor+1:0;
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==2 || getKey_Ctrl()== 4);
			break;
		}
		case 5://确认
		{
			switch(cursor)
			{
				case 0:g_state = SetDate;break;
				case 1:g_state = SetTime;break;
				case 2:g_state = WeekUp;break;
			}
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==5);
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
}

//设定日期模式
void DateStateFunc(void)
{
	static uint8_t cursor = 0;
	static uint8_t flag_now = 0;
	static Time_Typedef timeVal;
	uint8_t str[30];
	//当前未载入数据
	if(flag_now == 0)
	{
		flag_now = 1;//使能已载入数据标志位
		memcpy(&timeVal,&TimeValue,sizeof(Time_Typedef));
	}
	//显示日期
	sprintf(str,"  20%02d/%02d/%02d",timeVal.year,timeVal.month,timeVal.date);
	OLED_ShowString(0,3,(uint8_t *)str,12);
	//显示光标
	switch(cursor)
	{
		case 0:OLED_ShowString(0,4,(uint8_t *)"    ^        ",12);break;
		case 1:OLED_ShowString(0,4,(uint8_t *)"     ^       ",12);break;
		case 2:OLED_ShowString(0,4,(uint8_t *)"        ^    ",12);break;
		case 3:OLED_ShowString(0,4,(uint8_t *)"          ^  ",12);break;
		case 4:OLED_ShowString(0,4,(uint8_t *)"           ^ ",12);break;
		default:OLED_ShowString(0,4,(uint8_t *)"             ",12);break;
	}
	if(cursor>4)
	{
		if(cursor == 5)OLED_ShowString(0,7,(uint8_t *)"->Save     Back",12);
		else OLED_ShowString(0,7,(uint8_t *)"  Save   ->Back",12);
	}
	else
	{
		OLED_ShowString(0,7,(uint8_t *)"  Save     Back",12);
	}
	//响应按键切换 上左+ 下右- 确认跳转
	switch(getKey_Ctrl())
	{ 
		case 1://上
		{
			switch(cursor)
			{
				case 0:timeVal.year = ((timeVal.year/10) == 9)?timeVal.year%10:timeVal.year+10;break;
				case 1:timeVal.year = ((timeVal.year%10) == 9)?(timeVal.year/10)*10:timeVal.year+1;break;
				case 2:timeVal.month = (timeVal.month < 12)? timeVal.month + 1:1;break;
				case 3:timeVal.date = ((timeVal.date/10) == 3)?timeVal.date%10:timeVal.date+10;break;
				case 4:timeVal.date = ((timeVal.date%10) == 9)?(timeVal.date/10)*10:timeVal.date+1;break;
			}
			BeepSpeakCmd(1,100);
			delay_ms(200);
			break;
		}
		case 2://下
		{
			switch(cursor)
			{
				case 0:timeVal.year = ((timeVal.year/10) == 0)?timeVal.year + 90:timeVal.year-10;break;
				case 1:timeVal.year = ((timeVal.year%10) == 0)?(timeVal.year/10)*10+9:timeVal.year-1;break;
				case 2:timeVal.month = (timeVal.month > 1)? timeVal.month - 1:12;break;
				case 3:timeVal.date = ((timeVal.date/10) == 0)?timeVal.date + 30:timeVal.date-10;break;
				case 4:timeVal.date = ((timeVal.date%10) == 0)?(timeVal.date/10)*10+9:timeVal.date-1;break;
			}
			BeepSpeakCmd(1,100);
			delay_ms(200);
			break;
		}
		case 3://左
		{
			cursor = (cursor > 0)?cursor-1:6;
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==1 || getKey_Ctrl()== 3);
			break;
		}
		case 4://右
		{
			cursor = (cursor < 6)?cursor+1:0;
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==2 || getKey_Ctrl()== 4);
			break;
		}
		case 5://确认
		{
			switch(cursor)
			{
				case 5://save
				{
					Time_Handle();
					TimeValue.year = timeVal.year;
					TimeValue.month = timeVal.month;
					TimeValue.date = timeVal.date;
					DS3231_ReadWrite_Time(0);
					g_state = WeekUp;
					flag_now = 0;
					break;
				}
				case 6:g_state = SetMenu;flag_now = 0;break;//back
			}
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==5);
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
	//日期检查
	if(timeVal.date > 31)
	{
		timeVal.date = 31;
	}
	if(timeVal.month == 2 && timeVal.date > 29)
	{
		timeVal.date = 29;
	}
	if(timeVal.month == 4||timeVal.month == 6||timeVal.month == 9||timeVal.month == 11)
	{
		if(timeVal.date > 30)
		{
			timeVal.date = 30;
		}
	}
}

//设定时间模式
void TimeStateFunc(void)
{
	static uint8_t cursor = 0;
	static uint8_t flag_now = 0;
	static Time_Typedef timeVal;
	uint8_t str[30];
	//当前未载入数据
	if(flag_now == 0)
	{
		flag_now = 1;//使能已载入数据标志位
		memcpy(&timeVal,&TimeValue,sizeof(Time_Typedef));
	}
	//显示日期
	sprintf(str,"    %02d:%02d:%02d",timeVal.hour,timeVal.minute,timeVal.second);
	OLED_ShowString(0,3,(uint8_t *)str,12);
	//显示光标
	switch(cursor)
	{
		case 0:OLED_ShowString(0,4,(uint8_t *)"    ^        ",12);break;
		case 1:OLED_ShowString(0,4,(uint8_t *)"     ^       ",12);break;
		case 2:OLED_ShowString(0,4,(uint8_t *)"       ^     ",12);break;
		case 3:OLED_ShowString(0,4,(uint8_t *)"        ^    ",12);break;
		case 4:OLED_ShowString(0,4,(uint8_t *)"          ^  ",12);break;
		case 5:OLED_ShowString(0,4,(uint8_t *)"           ^ ",12);break;
		default:OLED_ShowString(0,4,(uint8_t *)"             ",12);break;
	}
	if(cursor>5)
	{
		if(cursor == 6)OLED_ShowString(0,7,(uint8_t *)"->Save     Back",12);
		else OLED_ShowString(0,7,(uint8_t *)"  Save   ->Back",12);
	}
	else
	{
		OLED_ShowString(0,7,(uint8_t *)"  Save     Back",12);
	}
	//响应按键切换 上左+ 下右- 确认跳转
	switch(getKey_Ctrl())
	{ 
		case 1://上
		{
			switch(cursor)
			{
				case 0:timeVal.hour = ((timeVal.hour/10) == 2)?timeVal.hour%10:timeVal.hour+10;break;
				case 1:timeVal.hour = ((timeVal.hour%10) == 9)?(timeVal.hour/10)*10:timeVal.hour+1;break;
				case 2:timeVal.minute = ((timeVal.minute/10) == 5)?timeVal.minute%10:timeVal.minute+10;break;
				case 3:timeVal.minute = ((timeVal.minute%10) == 9)?(timeVal.minute/10)*10:timeVal.minute+1;break;break;
				case 4:timeVal.second = ((timeVal.second/10) == 5)?timeVal.second%10:timeVal.second+10;break;break;
				case 5:timeVal.second = ((timeVal.second%10) == 9)?(timeVal.second/10)*10:timeVal.second+1;break;
			}
			BeepSpeakCmd(1,100);
			delay_ms(200);
			break;
		}
		case 2://下
		{
			switch(cursor)
			{
				case 0:timeVal.hour = ((timeVal.hour/10) == 0)?timeVal.hour + 20:timeVal.hour-10;break;
				case 1:timeVal.hour = ((timeVal.hour%10) == 0)?(timeVal.hour/10)*10+9:timeVal.hour-1;break;
				case 2:timeVal.minute = ((timeVal.minute/10) == 0)?timeVal.minute + 50:timeVal.minute-10;break;
				case 3:timeVal.minute = ((timeVal.minute%10) == 0)?(timeVal.minute/10)*10+9:timeVal.minute-1;break;
				case 4:timeVal.second = ((timeVal.second/10) == 0)?timeVal.second + 50:timeVal.second-10;break;
				case 5:timeVal.second = ((timeVal.second%10) == 0)?(timeVal.second/10)*10+9:timeVal.second-1;break;
			}
			BeepSpeakCmd(1,100);
			delay_ms(200);
			break;
		}
		case 3://左
		{
			cursor = (cursor > 0)?cursor-1:7;
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==1 || getKey_Ctrl()== 3);
			break;
		}
		case 4://右
		{
			cursor = (cursor < 7)?cursor+1:0;
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==2 || getKey_Ctrl()== 4);
			break;
		}
		case 5://确认
		{
			switch(cursor)
			{
				case 6://save
				{
					Time_Handle();
					TimeValue.hour = timeVal.hour;
					TimeValue.minute = timeVal.minute;
					TimeValue.second = timeVal.second;
					DS3231_ReadWrite_Time(0);
					g_state = WeekUp;
					flag_now = 0;
					break;
				}
				case 7:g_state = SetMenu;flag_now = 0;break;//back
			}
			BeepSpeakCmd(1,100);
			while(getKey_Ctrl()==5);
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
	//时间检查
	if(timeVal.hour > 23)
	{
		timeVal.hour = 23;
	}
	if(timeVal.minute > 59)
	{
		timeVal.minute = 59;
	}
	if(timeVal.second > 59)
	{
		timeVal.second  = 59;
	}
}

/*Setup , loop  Interrupt funcation ---------------------------------*/

void Nixie_Setup(void)
{
	//oled display the "start" page and "for" page
	OLED_Clear();
	OLED_DrawBMP(0,0,128,8,(uint8_t *)BMP_Start);
	delay_ms(1000);
	OLED_Clear();
	OLED_DrawBMP(0,0,128,8,(uint8_t *)BMP_For);
	
	//Enable device
	HV_Control(ENABLE);
	LED_Control(ENABLE);
	NixieTubeControl(ENABLE);
	
	//NixieTube Test
	NixieTubeTest();
	
	//oled clear
	OLED_Clear();
}

void Nixie_Loop(void)
{
	//global state machine
	switch(g_state)
	{
		case WeekUp://外部唤醒模式 当检测到外部触发失效后 跳转到 休眠模式
		{
			EnvironmentEvent();
			HV_Control(ENABLE);					//保持高压模块工作状态
			LED_Control(ENABLE);				//保持LED控制工作状态
			NixieTubeControl(ENABLE);		//保持辉光管工作状态
			InfoDisplay();							//系统信息显示
			KeyEvent_NT();							//模式跳转
			KeyEvent_Ctrl_WSNC2SetMenu();
			break;
		}
		case Sleep://休眠模式 侦测外部环境触发信息
		{
			EnvironmentEvent();
			HV_Control(DISABLE);				//保持高压模块停止状态
			LED_Control(DISABLE);			  //保持LED控制停止状态
			NixieTubeControl(DISABLE);	//保持辉光管停止状态
			InfoDisplay();							//系统信息显示
			KeyEvent_NT();							//模式跳转
			KeyEvent_Ctrl_WSNC2SetMenu();
			break;
		}
		case Normal://正常模式 持续工作 由按键进入或者跳出状态到weekup
		{
			HV_Control(ENABLE);					//保持高压模块工作状态
			LED_Control(ENABLE);				//保持LED控制工作状态
			NixieTubeControl(ENABLE);		//保持辉光管工作状态
			InfoDisplay();							//系统信息显示
			KeyEvent_NT();							//模式跳转
			KeyEvent_Ctrl_WSNC2SetMenu();
			break;
		}
		case Close://关闭模式 关闭显示 由按键进入或者跳出状态到weekup
		{
			HV_Control(DISABLE);				//保持高压模块停止状态
			LED_Control(DISABLE);			  //保持LED控制停止状态
			NixieTubeControl(DISABLE);	//保持辉光管停止状态
			InfoDisplay();							//系统信息显示
			KeyEvent_NT();							//模式跳转
			KeyEvent_Ctrl_WSNC2SetMenu();
			break;
		}
		case SetMenu://主菜单选择模式 选择日期设置和时间设置
		{
			MenuStateFunc();
			break;
		}
		case SetDate://年月日设定模式 修改  返回 保存
		{
			DateStateFunc();
			break;
		}
		case SetTime://时间设定模式 修改 返回 保存
		{
			TimeStateFunc();
			break;
		}
		default:g_state = WeekUp;break;
	}
	
	StatusView();				//显示当前状态
	NixieAddFlash();    //分钟进位闪烁显示
	NixieTimeDisplay(); //辉光钟时间显示
	LEDHartAttack();		//心跳显示
}

void Nixie_Interrupt(void)
{
	GetTimeValue();//时间获取
	weekUpTick();//唤醒模式计时
}