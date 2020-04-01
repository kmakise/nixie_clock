#include "stm32f10x.h"
#include "LEDTimer.h"


void LEDTimerInit(void)
{  
  GPIO_InitTypeDef         GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef        TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);// 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB , ENABLE);   //使能GPIO外设时钟使能
                                                                                                                                                                
																																							  //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1 ; 											  //TIM_CH12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 														  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	TIM_TimeBaseStructure.TIM_Period = 7200; 																		  //设置在下一个更新事件装入活动的自动重装载寄存器周期的值    
	TIM_TimeBaseStructure.TIM_Prescaler =0; 																		  //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision =0; 																	//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  									//TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 															//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
																																								//TIM2通通道一
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 														//选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState= TIM_OutputState_Enable; 									//比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 300; 																					//设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; 											//输出极性:TIM输出比较极性低
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);  																		//根据TIM_OCInitStruct中指定的参数初始化外设TIMx
																																								//TIM2通道二
  TIM_OCInitStructure.TIM_OutputState =TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 150;
  TIM_OC2Init(TIM2, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_CtrlPWMOutputs(TIM2,ENABLE);        																			//MOE 主输出使能        
  TIM_ARRPreloadConfig(TIM2, ENABLE); 																					//使能TIMx在ARR上的预装载寄存器
  TIM_Cmd(TIM2, ENABLE);  																											//使能TIM2
	
	TIM_SetCompare1(TIM2,0); 
	TIM_SetCompare2(TIM2,0); 
}
void LED_TIM3Init(void)  
{  

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //①时钟 TIM3 使能
	//定时器 TIM3 初始化
	TIM_TimeBaseStructure.TIM_Period = 999; //设置自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = 71; //设置时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割
	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM 向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  //②初始化 TIM3
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );  //③允许更新中断
	//中断优先级 NVIC 设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);   //④初始化 NVIC 寄存器
	TIM_Cmd(TIM3, ENABLE);  //⑤使能 TIM3
}  

void LED_HartAttackInit(void)
{
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_15;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_13;
	
	GPIO_Init(GPIOC,&gpio);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_15);
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
}

uint8_t g_LED_DISPLAY_FLAG = 0;


void LEDFlash(void)
{
	static uint16_t pluse = 0;
	static uint16_t div = 0;
	static uint8_t  flag = 0;
	div++;
	if(div > 20)
	{
		if(flag == 0)
		{
			pluse++;
			if(pluse > 500)
			{
				flag = 1;
			}
		}
		else
		{
			pluse--;
			if(pluse < 2)
			{
				flag = 0;
			}
		}
		if(g_LED_DISPLAY_FLAG)
		{
			TIM_SetCompare1(TIM2,pluse); 
		}
		else
		{
			TIM_SetCompare1(TIM2,0); 
		}		
	}	
}

void LEDBreathing(void)
{
	static uint16_t pluse = 0;
	static uint16_t div = 0;
	static uint8_t  flag = 0;
	div++;
	if(div > 100)
	{
		if(flag == 0)
		{
			pluse++;
			if(pluse > 4800)
			{
				flag = 1;
			}
		}
		else
		{
			pluse--;
			if(pluse < 2)
			{
				flag = 0;
			}
		}
		if(g_LED_DISPLAY_FLAG)
		{
			TIM_SetCompare2(TIM2,pluse); 
		}
		else
		{
			TIM_SetCompare2(TIM2,0); 
		}
	}
}

void TIM3_IRQHandler(void) 
{    
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update );
	
	LEDFlash();
	LEDBreathing();
} 

void LED_Control(uint8_t cmd)
{
	if(cmd == ENABLE)
	{
		g_LED_DISPLAY_FLAG = 1;
	}
	else if(cmd == DISABLE)
	{
		g_LED_DISPLAY_FLAG = 0;
	}
}

void LED_HartCmd(uint8_t cmd)
{
	GPIOC->BSRR = GPIO_Pin_13 << 16 * cmd;
	GPIOB->BSRR = GPIO_Pin_15 << 16 * cmd;
}



















