#include "stm32f10x.h"
#include "delay.h"

void BeepInit(void)
{
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&gpio);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
}

void BeepSpeakCmd(uint8_t num,uint16_t time)
{
	uint8_t i;
	for(i = 0;i < num;i++)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
		delay_ms(time);
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
		delay_ms(time);
	}
}
