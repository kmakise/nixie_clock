
#include "stm32f10x.h"
#include "NixieTube.h"
#include "delay.h"
#include "stdio.h"

//PA3 - NT DATA
//PA4 - NT CLK
//PA5 - NT OUT

#define NT_DATA(a) GPIOA->BSRR = GPIO_Pin_3 << 16 * !a
#define NT_CLK(a)  GPIOA->BSRR = GPIO_Pin_4 << 16 * !a
#define NT_OUT(a)  GPIOA->BSRR = GPIO_Pin_5 << 16 * !a


void NixieTubeInit(void)
{
	GPIO_InitTypeDef gpio;
	uint8_t num[6] = {0,0,0,0,0,0,};
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&gpio);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5);
	
	NixieTubeDisplay(num);
}

void NixieTubeDrive(uint8_t * num)
{
	uint16_t i;
	for(i = 0;i < 64;i++)
	{
		//data level out
		NT_DATA(num[i]);
		//clock out
		NT_CLK(1);
		NT_CLK(0);
	}
	//out put
	NT_OUT(1);
	NT_OUT(0);
}

const uint8_t TUBE[6][11] = 
{ 
// 0   1  2  3  4  5  6  7  8  9 NA
	{12, 3, 4, 5, 6,15, 8, 9,10,11,0},//1
	{13,22,21,20,19,18,17,16,23,14,0},//2
	{31,32,39,30,29,28,27,26,25,24,0},//3
	{33,42,41,40,47,38,37,36,35,34,0},//4
	{52,43,44,45,46,55,48,49,50,51,0},//5
	{54,53,63,56,57,58,59,60,61,62,0},//6
};

uint8_t NixieTubeRunning = 0;

void NixieTubeDisplay(uint8_t * num)
{
	uint8_t buf[64] = { 
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
	};
	uint8_t i = 0;
	if(NixieTubeRunning)
	{
		for(i = 0;i < 6;i++)
		{
			buf[TUBE[i][num[i]]] = 1;
		}
	}
	NixieTubeDrive(buf);
}
void NixieTubeControl(uint8_t cmd)
{
	if(cmd == ENABLE)
	{
		NixieTubeRunning = 1;
	}
	else if(cmd == DISABLE)
	{
		NixieTubeRunning = 0;
	}
}

void NixieTubeTest(void)
{
	//1. <-0 2.0-9 3.NA
	uint8_t num[6] = {10,10,10,10,10,10};
	uint8_t i = 0;
	for(i = 0;i < 6;i++)
	{
		num[i] = 0;
		NixieTubeDisplay(num);
		delay_ms(70);
	}
	for(i = 0;i < 10;i++)
	{
		num[0] = num[1] = num[2] = num[3] = num[4] = num[5] = i;
		NixieTubeDisplay(num);
		delay_ms(70);
	}
	for(i = 0;i < 6;i++)
	{
		num[i] = 10;
		NixieTubeDisplay(num);
		delay_ms(70);
	}
}



void NixieTubeFlash(void)
{
	uint8_t num[6];
	uint8_t i,j;
	for(i = 0;i < 10;i++)
	{
		for(j = 0;j < 6;j++)
		{
			num[j] = rand()%10; 
		}
		NixieTubeDisplay(num);
		delay_ms(15);
	}
}



