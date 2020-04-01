#ifndef __LEDTIMER_H
#define __LEDTIMER_H

void LEDTimerInit(void);
void LED_TIM3Init(void);
void LED_HartAttackInit(void);


void LED_Control(uint8_t cmd);
void LED_HartCmd(uint8_t cmd);

#endif /*__LEDTIMER_H*/
