#include "led.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK mini�SSTM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

	    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //LED0-->PA.8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.8
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //PA.8 �����

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	    		 //LED1-->PD.2 �˿�����, �������
 GPIO_Init(GPIOD, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 GPIO_SetBits(GPIOD,GPIO_Pin_2); 						 //PD.2 ����� 	

}
 
//�̵������� IO��ʼ�� 
void RELAY_IO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				//ǰ��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//������� 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					
	GPIO_SetBits(GPIOC,GPIO_Pin_6);						 		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	    		//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOC,GPIO_Pin_7); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	    		 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOC,GPIO_Pin_8);	
}
//IO��������� IO��ʼ��
void IO_OUT_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //ǰ��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	GPIO_SetBits(GPIOB,GPIO_Pin_9);						 		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	    		 //����
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOB,GPIO_Pin_10); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	    		 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOB,GPIO_Pin_11);	
}
//IO�����벶�� IO��ʼ��
void IO_IN_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;			//ֹͣ״̬�������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //�������룬������ʱ��ȡ״̬Ϊ�͵�ƽ������ߵ�ƽʱ��ȡ״̬Ϊ�ߵ�ƽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					
							 		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	    	//����״̬������� 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	    	//ǰ��״̬�������	 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	    	//�����������������	 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


uint8_t IOStatus_Catch(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
	u8 IOStatus;
	IOStatus=GPIO_ReadInputDataBit(GPIOx,GPIO_Pin);	
	return IOStatus;
}
