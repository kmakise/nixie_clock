#include "stm32f10x.h"
#include "KeyBoard.h"



void KeyBoardInit(void)
{
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Pin = GPIO_Pin_5 |GPIO_Pin_6 |GPIO_Pin_7 |GPIO_Pin_8 |
									GPIO_Pin_8 |GPIO_Pin_9  |GPIO_Pin_13 |
									GPIO_Pin_14;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&gpio);
	
	gpio.GPIO_Mode = GPIO_Mode_IPD;
	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOB,&gpio);
}


uint8_t getKey_Ctrl(void)
{
	//key map          up         down       lift       right      cfm      
	uint32_t pin[5] = {GPIO_Pin_6,GPIO_Pin_8,GPIO_Pin_5,GPIO_Pin_9,GPIO_Pin_7};
	uint8_t i;
	for(i = 0;i < 5;i++)
	{
		if((GPIOB->IDR & pin[i]) == 0)
		{
			return i+1;
		}
	}
	return 0;
}

uint8_t getKey_NT(void)
{
	//key map         lift       right     
	uint32_t pin[2] = {GPIO_Pin_13,GPIO_Pin_14};
	uint8_t i;
	for(i = 0;i < 2;i++)
	{
		if((GPIOB->IDR & pin[i]) == 0)
		{
			return i+1; 
		}
	}
	return 0;
}
uint8_t getKey_EN(void)
{
	if((GPIOB->IDR & GPIO_Pin_12) == GPIO_Pin_12)
	{
		return 1; 
	}
	return 0;
}