#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//ͨ�ö�ʱ�� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
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

