/******************************************************************************
	* @file    AmadeusMain.c
	* @author  ÕÅ¶«
	* @version V1.0.0
	* @date    2019-3-25
	* @brief   nixie tube clock main function
	******************************************************************************/
/*Iniclude -----------------------------------*/
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

#include "AmadeusMain.h"
#include "NixieApp.h"

void Sys_setup(void)
{
	//sys
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	delay_init();
	
	//hardware
	OLED_Init();
	OLED_Clear();
	
	NixieTubeInit();
	
	HV_ControlInit();
	HV_Control(DISABLE);
	
	KeyBoardInit();
	
	LEDTimerInit();
	LED_TIM3Init();
	LED_HartAttackInit();
	LED_Control(DISABLE);
	
	DS3231_GPIO_Config();
	Time_Handle();
	
	BeepInit();
	
	//app setup
	Nixie_Setup();
	
	//interrupt start
	TIM4_Init(999,71);
}

void Sys_loop(void)
{
	Nixie_Loop();
}

void Sys_Interrupt(void)
{
	Nixie_Interrupt();
}