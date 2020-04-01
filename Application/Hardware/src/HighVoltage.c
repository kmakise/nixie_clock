#include "stm32f10x.h"



void HV_Control(uint8_t cmd)
{
		if(cmd)
		{
			GPIO_SetBits(GPIOB,GPIO_Pin_1);
		}
		else
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_1);
		}
}


void HV_ControlInit(void)
{
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_1;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&gpio);
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
}

