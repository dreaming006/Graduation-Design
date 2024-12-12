#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//通用定时器 驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/03
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  


void TIM3_Int_Init(u16 arr,u16 psc); 
void TIM3_OPEN(void);
void TIM3_CLOSE(void);

void TIM2_Int_Init(u16 arr,u16 psc); 
void TIM2_OPEN(void);
void TIM2_CLOSE(void);

void TIM4_Int_Init(u16 arr,u16 psc); 
void TIM4_OPEN(void);
void TIM4_CLOSE(void);

void TIM5_Int_Init(u16 arr,u16 psc); 
void TIM5_OPEN(void);
void TIM5_CLOSE(void);
#endif

