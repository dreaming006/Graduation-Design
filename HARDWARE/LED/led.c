#include "led.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK miniSTM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

	    
//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PA,PD端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //LED0-->PA.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.8
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //PA.8 输出高

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	    		 //LED1-->PD.2 端口配置, 推挽输出
 GPIO_Init(GPIOD, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_SetBits(GPIOD,GPIO_Pin_2); 						 //PD.2 输出高 	

}
 
//继电器控制 IO初始化 
void RELAY_IO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				//前进
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					
	GPIO_SetBits(GPIOC,GPIO_Pin_6);						 		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	    		//后退
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOC,GPIO_Pin_7); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	    		 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOC,GPIO_Pin_8);	
}
//IO口输出控制 IO初始化
void IO_OUT_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //前进
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	GPIO_SetBits(GPIOB,GPIO_Pin_9);						 		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	    		 //后退
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOB,GPIO_Pin_10); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	    		 
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 
	GPIO_SetBits(GPIOB,GPIO_Pin_11);	
}
//IO口输入捕获 IO初始化
void IO_IN_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;			//停止状态检测引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入，无输入时读取状态为低电平，输入高电平时读取状态为高电平
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					
							 		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	    	//后退状态检测引脚 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	    	//前进状态检测引脚	 
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	    	//霍尔传感器检测引脚	 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


uint8_t IOStatus_Catch(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
	u8 IOStatus;
	IOStatus=GPIO_ReadInputDataBit(GPIOx,GPIO_Pin);	
	return IOStatus;
}
