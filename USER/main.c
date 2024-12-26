#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "rc522.h"
#include "rc522_1.h"
#include "rc522_2.h"
#include "flash.h"
#include "key.h"
#include "timer.h"
#include "string.h"
#include "core_cm3.h"
/*全局变量*/

u32 FLASH_SIZE=8*1024*1024;			//FLASH 大小为8M字节
unsigned char CT[2];    			//卡类型
unsigned char SN_1[4];   			//阅读器1读取的卡号
unsigned char SN_2[4];				//阅读器2读取的卡号
unsigned char status;				//读卡状态

unsigned char N[50][4]; 			//二维数组存放卡号和对应编号

unsigned int T3=0;					//定时器3计数满一次则计时0.01秒，用于求速度
unsigned int T2=0;					//定时器2计数满一次则计时0.01秒，运行时间，用于求运行距离
unsigned int T4=0;					//定时器4计数满一次则计时1秒，用于巡航模式时一次停留的定时
unsigned int T5=0;					//定时器5计数满一次则计时1秒，用于巡航模式时整个工作时间的定时
unsigned int T31=0;
unsigned int T21=0;
unsigned int T22=0;
unsigned int T41=0;
unsigned char Model=0;				//控制模式:1，预设模式:2，巡航模式：3
unsigned char RFID_NUM;				//阅读器工作序号，值为1时打开阅读器1，值为2时打开阅读器2
unsigned char Same_Bit=0;			//用于比较标签卡号时，计算相同位数
unsigned char Save_Status=0;		//W25Q64写入状态
unsigned char Read_Status=0;		//W25Q64读取状态
unsigned char GO_Status=0;			//前进状态：1
unsigned char BACK_Status=0;		//后退状态：1
unsigned char STOP_Status=1;		//停止状态：1
unsigned char Cruise_Status=0;
unsigned char IO_GO_Status=0;
unsigned char IO_BACK_Status=0;
unsigned char IO_STOP_Status=1;

unsigned int label=0;				//标签序号
unsigned char Now_Label[2];			//目前所在的标签号；
unsigned char Destination_Label;	//预设模式下的目的标签序号
unsigned char Origin_Label;
double RFID_distance=5.0;			//读卡器间距10cm
double v=0;							//真实速度
double x=0;							//当前与前一个标签的距离
double T;
double t2[50];
double t3[50];
double t4[50];
double x1[50];

unsigned int huoer_num,all_huoer_num;
unsigned char huoer_flag;
unsigned int h_num[50];

unsigned char SIZE;
unsigned char Addr_Broadcast[2]={0XFF,0XFF};//广播地址
unsigned char Addr_A[2]={0X02,0XCA};		//接受端LORA地址'd714
unsigned char Addr_Local[2]={0X0E,0X4E};	//开发板LORA地址'd3662
unsigned char Addr_Gateway[2]={0X02,0XCA};
unsigned char Lora_Channel = 0X17;			//广播通道’0x17 == 'd23 
//防止与楼下GPS程序的LORA冲突，可以设置为0x18 ==‘d24

/*函数申明*/
void Printing(unsigned char* Buffer,unsigned short len);	//将数组里的数据发送到串口(字符型)
unsigned char Request_Anticoll( unsigned char request_mode,
	       unsigned char* CT, unsigned char* SN, unsigned char rfid_num );		//寻卡、防冲撞、选卡

void Model_Control(void);      		//控制模式	
void Model_Automatically(void);		//预设模式	
		   
void Control_GO(void);	//控制模式：前进
void Control_BACK(void);//控制模式：后退
void Auto_GO(void);	//预设模式：前进
void Auto_BACK(void);//预设模式：前后退

void STOP(void);
  
void Save_RFID(void);
void Read_RFID(void);
void Read_IO(void);		   

void Printing(unsigned char* Buffer,unsigned short len)
{
	unsigned char i;
	for(i=0;i<len;i++)
	{
		printf("%d ",Buffer[i]); //串口显示SPI1通道的卡号   
	}
}				 

//寻卡、防冲撞
unsigned char Request_Anticoll(unsigned char request_mode,
	unsigned char* CT, unsigned char* SN, unsigned char rfid_num)
{
	//寻卡 寻天线内未进入休眠的卡
	if(rfid_num==1)
	{
		PcdAntennaOn_1();
		status = PcdRequest_1(request_mode,CT);/*寻卡*///顺便把卡的类型写到数组CT中 
		if(status==MI_OK)//尋卡成功
		{
			status=MI_ERR;
			status = PcdAnticoll_1(SN);/*防冲撞*///顺便把卡号写到SN数组中		
			if(status==MI_OK)
			{		 
				PcdAntennaOff_1();
				return status;//成功		
			}
		}
		status=MI_ERR;
//		PcdAntennaOff_1();
		return status;//失败
	}
	else
	{
		PcdAntennaOn_2();
		status = PcdRequest_2(request_mode,CT);/*寻卡*///顺便把卡的类型写到数组CT中
		if(status==MI_OK)//尋卡成功
		{
			status=MI_ERR;
			status = PcdAnticoll_2(SN);/*防冲撞*///顺便把卡号写到SN数组中		
			if(status==MI_OK)
			{		
				PcdAntennaOff_2();
				return status;//成功		
			}
		}
		status=MI_ERR;
//		PcdAntennaOff_2();
		return status;//失败
	}
}
//存储卡号进W25Q64
void Save_RFID(void)
{
	u8 t,len;
	u8 flag;
	u8 datatemp[4];
	u8 RX[2];
	u8 Save_Number=0;
	RFID_NUM=1;//打开阅读器1
	Save_Status=1;
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C9611\r\n");
	delay_ms(100);
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("AStart save RFID label to W25Q64\r\n");	
	while(Save_Status==1)
	{
		while(RFID_NUM==1)
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==6)
				{
					Save_Status=0;
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					//printf("ASave RFID label number:%d\r\n",Save_Number);
					delay_ms(100);
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave RFID label over\r\n");
					break;
				}
			}
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				status=MI_ERR;
				
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("AThe label identified by RFID-1 is ");
				for(t=0;t<4;t++)
				{
					printf("%d ",SN_1[t]);
				}	
				printf("\r\n");
				RFID_NUM=2;
			}		
		}
		while(RFID_NUM==2)
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==6)
				{
					Save_Status=0;
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					//printf("ASave RFID label number:%d\r\n",Save_Number);
					delay_ms(100);
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave RFID label over\r\n");
					break;
				}
			}
			if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				status=MI_ERR;
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("AThe label identified by RFID-2 is ");
				for(t=0;t<4;t++)
				{
					printf("%d ",SN_2[t]);
				}	
				printf("\r\n");
				for(t=0;t<4;t++)
				{
					if(SN_2[t]==SN_1[t])
						flag++;				
				}
				delay_ms(100);
				if(flag==4)
				{
					SIZE=sizeof(SN_1);
					SPI_Flash_Write((u8*)SN_1,(FLASH_SIZE-1000+Save_Number*SIZE),SIZE);//从倒数第1000个地址处开始,写入SIZE长度的数据
					SPI_Flash_Read(datatemp,(FLASH_SIZE-1000+Save_Number*SIZE),SIZE);		
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave RFID label is ");
					for(t=0;t<4;t++)
					{
						printf("%d ",datatemp[t]);
					}	
					printf("\r\n");		
					flag=0;
					Save_Number++;
					delay_ms(100);
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave_Number is %d",Save_Number);
				}				
				else
				{
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave Error:RFID-1 != RFID-2");
				}
				
				if(Save_Number>49)
				{
					Save_Status=0;
					RFID_NUM=0;
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave RFID label number:%d\r\n",Save_Number);
					delay_ms(100);
					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
					printf("ASave RFID label over\r\n");
				}
				RFID_NUM=1;
			}	
			
		}	
			
	}
}

void Read_RFID(void)
{
	u8 i;
	u8 len=0;
	u8 datatemp[4];
	Same_Bit=0;
	Read_Status=1;
//	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("AStart read RFID label in W25Q64\r\n");
//	delay_ms(200);
	SPI_Flash_Read(Now_Label,10,2);//读取目前所在位置
//	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//	printf("ANow_Label:%c%c\r\n",Now_Label[0],Now_Label[1]);
//	delay_ms(200);
	while(Read_Status==1)
	{
		SPI_Flash_Read(datatemp,(FLASH_SIZE-1000+len*4),4);

		for(i=0;i<4;i++)
		{
			if(datatemp[i]==255)
				Same_Bit++;
		}
		if(Same_Bit==4)
		{
			Read_Status=0;
			Same_Bit=0;
//			printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//			printf("A%d RFID label were read from W25Q64\r\n",len);
//			delay_ms(200);
//			printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//			printf("ARead RFID label over\r\n");
			break;
		}
		else
		{
			for(i=0;i<4;i++)
				N[len][i]=datatemp[i];
			len++;
		}
				
		if(len>49)
		{
			Read_Status=0;
//			printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//			printf("A%d RFID label were read from W25Q64\r\n",len);
//			delay_ms(200);
//			printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//			printf("ARead RFID label over\r\n");
		}			

	}
}
void Read_IO(void)
{
	IO_GO_Status=IOStatus_Catch(GPIOC,GPIO_Pin_2);
	IO_BACK_Status=IOStatus_Catch(GPIOC,GPIO_Pin_1);
	IO_STOP_Status=IOStatus_Catch(GPIOC,GPIO_Pin_0);
	if(IO_GO_Status==0&&IO_BACK_Status==1&&IO_STOP_Status==1)
	{
		//printf("读取到输入的前进信号");
		Control_GO();
		
	}
	else if(IO_GO_Status==1&&IO_BACK_Status==0&&IO_STOP_Status==1)
	{
		//printf("读取到输入的后退信号");
		Control_BACK();
		
	}
	//printf("IO输入为111停止");

}
//控制模式	
void Model_Control(void)
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	Model=1;
	SPI_Flash_Read(Now_Label,10,2);//读取目前所在位置
	label=((Now_Label[0]-48)*10)+(Now_Label[1]-48);
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C9911%c%c\r\n",Now_Label[0],Now_Label[1]);
	//printf("Model Control\r\n");
	while(Model==1)
	{
		Read_IO();
		if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			for(t=0;t<len;t++)
			{
				RX[t]=USART_RX_BUF[t];
				RX[t]=RX[t]-48;		
			}
			USART_RX_STA=0;
			//printf("COM data received in model_control\r\n");
			if(RX[0]==3&&RX[1]==1&&STOP_Status==1)	//前进
			{
				//printf("Model Control Go\r\n");
				GPIO_ResetBits(GPIOC,GPIO_Pin_6);	//继电器
				GPIO_ResetBits(GPIOB,GPIO_Pin_9);	//IO口
				GPIO_ResetBits(GPIOA,GPIO_Pin_8);
				delay_ms(50);
				GPIO_SetBits(GPIOC,GPIO_Pin_6);
				GPIO_SetBits(GPIOB,GPIO_Pin_9);	
				GPIO_SetBits(GPIOA,GPIO_Pin_8);
				Control_GO();
				printf("Go finish\r\n");
			}
			else if(RX[0]==3&&RX[1]==0&&STOP_Status==1)	//后退
			{
				GPIO_ResetBits(GPIOC,GPIO_Pin_7);
				GPIO_ResetBits(GPIOB,GPIO_Pin_10);
				GPIO_ResetBits(GPIOD,GPIO_Pin_2);
				delay_ms(50);
				GPIO_SetBits(GPIOC,GPIO_Pin_7);
				GPIO_SetBits(GPIOB,GPIO_Pin_10);
				GPIO_SetBits(GPIOD,GPIO_Pin_2);
				Control_BACK();
			}
			else if(RX[0]==0&&RX[1]==0)
			{
				
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("D9911%c%c\r\n",Now_Label[0],Now_Label[1]);
				
			}
			else if(RX[0]==9&&RX[1]==9&&STOP_Status==1)	//退出控制模式
			{
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("C9901%c%c\r\n",Now_Label[0],Now_Label[1]);//退出轨道机控制模式

				Model=0;
			}						
		}		
	}
}

//预设模式
void Model_Automatically(void)
{
	u8 t;
	u8 len;	 
	u8 RX[4];
	Model=2;
	SPI_Flash_Read(Now_Label,10,2);//读取目前所在位置
	label=((Now_Label[0]-48)*10)+(Now_Label[1]-48);
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C9811%c%c\r\n",Now_Label[0],Now_Label[1]);
	printf("Automatically start\r\n");	
	while(Model==2)
	{
		Read_IO();
		if(USART_RX_STA&0x8000)			//接收完成
		{					   
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			for(t=0;t<len;t++)
			{
				RX[t]=USART_RX_BUF[t];
				RX[t]=RX[t]-48;		
			}
			USART_RX_STA=0;
			if(RX[0]== 9&&RX[1]== 8&&STOP_Status==1)
			{
				Model=0;
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("C9801%c%c\r\n\r\n",Now_Label[0],Now_Label[1]);//退出轨道机预设模式
			}
			else if(RX[0]==0&&RX[1]==0)
			{
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("D9811%02d\r\n",label);
			}
			else if(RX[0]==9&&RX[1]==1)
			{
				printf("RESET\r\n");
				__set_FAULTMASK(1); 
				NVIC_SystemReset();
			} 	
			else
			{
				Destination_Label=RX[2]*10+RX[3];
				//printf("%d",Destination_Label);
				if(label > Destination_Label && label > 10 && label < 13)
				{
					Destination_Label = 10;
					
				}
				else if(label > Destination_Label && label > 7 && label < 10)
				{
					Destination_Label = 7;
				}
				if(RX[0]==3&&RX[1]==1&&STOP_Status==1)		//前进
				{
					GPIO_ResetBits(GPIOC,GPIO_Pin_6);	//继电器
					GPIO_ResetBits(GPIOB,GPIO_Pin_9);	//IO口
					GPIO_ResetBits(GPIOA,GPIO_Pin_8);
					delay_ms(50);
					GPIO_SetBits(GPIOC,GPIO_Pin_6);
					GPIO_SetBits(GPIOB,GPIO_Pin_9);	
					GPIO_SetBits(GPIOA,GPIO_Pin_8);
					Auto_GO();	
				}
				else if(RX[0]==3&&RX[1]==0&&STOP_Status==1)		//后退
				{	
					GPIO_ResetBits(GPIOC,GPIO_Pin_7);
					GPIO_ResetBits(GPIOB,GPIO_Pin_10);
					GPIO_ResetBits(GPIOD,GPIO_Pin_2);
					delay_ms(50);
					GPIO_SetBits(GPIOC,GPIO_Pin_7);
					GPIO_SetBits(GPIOB,GPIO_Pin_10);
					GPIO_SetBits(GPIOD,GPIO_Pin_2);
					Auto_BACK();
				}
			}
				
		}		
	}
	printf("Exit Model Automatically\r\n");
}

void STOP(void)
 {
	
	printf("STOP\r\n");
	GPIO_ResetBits(GPIOC,GPIO_Pin_8);
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);
	GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);//停止
	delay_ms(50);
	GPIO_SetBits(GPIOC,GPIO_Pin_8);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
	GPIO_SetBits(GPIOD,GPIO_Pin_2);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	TIM2_CLOSE();
	TIM3_CLOSE();
	TIM4_CLOSE();
	TIM5_CLOSE();
	TIM2->CNT = 0;
	TIM3->CNT = 0;
	TIM4->CNT = 0;
	TIM5->CNT = 0;
	T2=0;
	T3=0;
	T4=0;
	T5=0;
	huoer_num = 0;
	STOP_Status=1;
	BACK_Status=0;
	GO_Status=0;
	Cruise_Status=0;
	huoer_flag=0;
	v=0;

	if(x>=10)
	{
		printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
		printf("C9090%02d%.2f%.2f\r\n",label,x,v);
	}
	else
	{
		printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
		printf("C9090%02d0%.2f%.2f\r\n",label,x,v);
	}
//	delay_ms(200);
//	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//	printf("C9090\r\n");//轨道运输机停止
}

void Control_GO(void)			//控制模式——前进
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	u8 Same_Bit=0;//用于比较标签卡号时，计算相同位数
	GO_Status=1;
	STOP_Status=0;
	RFID_NUM=1;//打开阅读器1
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C1111%02d\r\n",label);
	printf("control go\r\n");
	while(GO_Status==1)
	{
		//printf("GO_status\r\n");
		while(RFID_NUM==1)//阅读器1进入工作状态
		{
			//printf("rfid_1 work\r\n");
			if(USART_RX_STA&0x8000)
			{	
				//printf("get new rx");
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();	
					break;					
				}
				else if(RX[0]==0&&RX[1]==0)				//数据上报
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1111%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1111%02d0%.2f%.2f\r\n",label,x,v);
					}
				}
			}
			IO_GO_Status=IOStatus_Catch(GPIOC,GPIO_Pin_2);
			IO_BACK_Status=IOStatus_Catch(GPIOC,GPIO_Pin_1);
			IO_STOP_Status=IOStatus_Catch(GPIOC,GPIO_Pin_0);
			if(IO_GO_Status==0&&IO_BACK_Status==0&&IO_STOP_Status==0)
			{
				STOP();
				//printf("get pin to stop num1\r\n");
				break;
			}
			
			//发送当前轨道机状态、标签数目、距离、速度
//			if((T2!=0)&&(T2%15==0))//T2=100时，即1s,每秒发送一次
//			{
//				T22 = T2;
//				T21 = TIM_GetCounter(TIM2);
//				x = v*((65000*T22+T21)/1000000.0);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);		
//			}

			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=2;//关闭阅读器1，打开阅读器2				
			}
		}

		while(RFID_NUM==2)//阅读器2进入工作状态
		{
			printf("rfid_2 work\r\n");
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t]; 
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();
					break;
				}
				else if(RX[0]==0&&RX[1]==0)
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1111%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1111%02d0%.2f%.2f\r\n",label,x,v);
					}
				}	
			}
			
			IO_GO_Status=IOStatus_Catch(GPIOC,GPIO_Pin_2);
			IO_BACK_Status=IOStatus_Catch(GPIOC,GPIO_Pin_1);
			IO_STOP_Status=IOStatus_Catch(GPIOC,GPIO_Pin_0);
			if(IO_GO_Status==0&&IO_BACK_Status==0&&IO_STOP_Status==0)
			{
				STOP();
				printf("get pin to stop num2");
				break;
			}
			
			
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);	
//			}
			if(T3>100)
			{
				  
				TIM3_CLOSE();
				TIM3->CNT = 0;
				T3=0;//时间太长溢出，清零	
				RFID_NUM=1;	
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");
				break;				
			}
			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//用于求速度的定时器3先关闭
				for(t=0;t<4;t++)
				{
					if(SN_1[t]==SN_2[t])
					{
						Same_Bit++;
					}
				}
				if(Same_Bit==4)
				{
					Same_Bit=0;
					T31=TIM_GetCounter(TIM3);
					T = (65000*T3+T31)/1000.0;
					v = (10.0*RFID_distance)/T;
					TIM3->CNT = 0;
					T3=0;//计算完成后清零，等待下一次计算
					for(len=0;len<50;len++)//比较标签卡号，得到标签序号
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//找到相同卡号时退出循环
						{
							Same_Bit=0;
							break;
						}
						Same_Bit=0;
					}
					label=len+1;						
					Now_Label[0]=label/10+48;
					Now_Label[1]=label%10+48;
					SPI_Flash_Write((u8*)Now_Label,10,2);
//					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//					printf("C1111%02d0.00%.2f\r\n",label,v);	
				}
				else
				{
					T3=0;
					TIM3->CNT = 0;
					Same_Bit=0;
//					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//					printf("AIdentification Error:RFID-1 != RFID-2");
				}
				status=MI_ERR;
				RFID_NUM=1;//关闭阅读器2，打开阅读器1	
				
			}						
		}
	}
	printf("control go end\r\n");	
}
void Control_BACK(void)		//控制模式——后退
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	
	BACK_Status=1;
	STOP_Status=0;
	RFID_NUM=2;//打开阅读器2
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C1010%02d\r\n",label);
	printf("Control_BACK\r\n");
	while(BACK_Status==1)
	{
		while(RFID_NUM==2)//阅读器1进入工作状态
		{
			
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();	
					break;		
				}
				else if(RX[0]==0&&RX[1]==0)
				{
					x=v*(T2/100);T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1010%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1010%02d0%.2f%.2f\r\n",label,x,v);
					}
				}		
			}
			IO_GO_Status=IOStatus_Catch(GPIOC,GPIO_Pin_2);
			IO_BACK_Status=IOStatus_Catch(GPIOC,GPIO_Pin_1);
			IO_STOP_Status=IOStatus_Catch(GPIOC,GPIO_Pin_0);
			if(IO_GO_Status==0&&IO_BACK_Status==0&&IO_STOP_Status==0)
			{
				STOP();
				break;
			}
			//发送当前轨道机状态、标签数目、距离、速度
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);		
//			}
		
			//寻卡
			 if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=1;//关闭阅读器2，打开阅读器1			
			}
		}

		while(RFID_NUM==1)//阅读器2进入工作状态
		{
			
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();
					break;	
				}	
				else if(RX[0]==0&&RX[1]==0)
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1010%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D1010%02d0%.2f%.2f\r\n",label,x,v);
					}
				}	
			}
			IO_GO_Status=IOStatus_Catch(GPIOC,GPIO_Pin_2);
			IO_BACK_Status=IOStatus_Catch(GPIOC,GPIO_Pin_1);
			IO_STOP_Status=IOStatus_Catch(GPIOC,GPIO_Pin_0);
			if(IO_GO_Status==0&&IO_BACK_Status==0&&IO_STOP_Status==0)
			{
				STOP();
				break;
			}
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);		
//			}
			if(T3>100)
			{
				TIM3_CLOSE();
				T3=0;//时间太长溢出，清零，1分钟到6000	
				TIM3->CNT = 0;
				RFID_NUM=2;	
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");
				break;
			}
			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//用于求速度的定时器3先关闭
				Same_Bit=0;
				for(t=0;t<4;t++)
				{
					if(SN_1[t]==SN_2[t])
					{
						Same_Bit++;
					}
				}
				if(Same_Bit==4)
				{
					Same_Bit=0;
					T31=TIM_GetCounter(TIM3);
					T = (65000*T3+T31)/1000.0;
					v = (10.0*RFID_distance)/T;
					TIM3->CNT = 0;
					T3=0;//计算完成后清零，等待下一次计算
					for(len=0;len<50;len++)//比较标签卡号，得到标签序号
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//找到相同卡号时退出循环
						{
							Same_Bit=0;
							break;
						}
						Same_Bit=0;
					}
					label=len+1;
					Now_Label[0]=label/10+48;
					Now_Label[1]=label%10+48;
					SPI_Flash_Write((u8*)Now_Label,10,2);
//					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//					printf("C1010%02d0.00%.2f\r\n",label,v);	
				}
				else
				{
					T3=0;
					Same_Bit=0;
					TIM3->CNT = 0;
//					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//					printf("AIdentification Error:RFID-1 != RFID-2");
				}
				status=MI_ERR;
				RFID_NUM=2;//关闭阅读器1，打开阅读器2			
			}
		}
	}
	printf("control back end\r\n");
}

void Auto_GO(void)	//预设模式——前进
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	u8 Same_Bit=0;//用于比较标签卡号时，计算相同位数
	GO_Status=1;
	STOP_Status=0;
	RFID_NUM=1;//打开阅读器1
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C2222%02d\r\n",label);
	printf("Auto_GO\r\n");
	while(GO_Status==1)
	{
		while(RFID_NUM==1)//阅读器1进入工作状态
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();
					break;					
				}
				else if(RX[0]==0&&RX[1]==0)
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2222%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2222%02d0%.2f%.2f\r\n",label,x,v);
					}
				}	
			}
			//发送当前轨道机状态、标签数目、距离、速度
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);	 	
//			}
			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=2;//关闭阅读器1，打开阅读器2				
			}
		}
		while(RFID_NUM==2)//阅读器2进入工作状态
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();
					break;					
				}
				else if(RX[0]==0&&RX[1]==0)
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2222%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2222%02d0%.2f%.2f\r\n",label,x,v);
					}
				}
			}	
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);		
//			}
			if(T3>100)//6.5s
			{
				TIM3_CLOSE();
				T3=0;//时间太长溢出，清零
				TIM3->CNT = 0;				
				RFID_NUM=1;
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");
				break;
			}
			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//用于求速度的定时器3先关闭
				for(t=0;t<4;t++)
				{
					if(SN_1[t]==SN_2[t])
					{
						Same_Bit++;
					}
				}
				if(Same_Bit==4)
				{
					Same_Bit=0;
					T31=TIM_GetCounter(TIM3);
					T = (65000*T3+T31)/1000.0;
					v = (10.0*RFID_distance)/T;
					TIM3->CNT = 0;
					T3=0;//计算完成后清零，等待下一次计算
					for(len=0;len<50;len++)//比较标签卡号，得到标签序号
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//找到相同卡号时退出循环
						{
							Same_Bit=0;
							break;
						}
						Same_Bit=0;
					}
					label=len+1;
					Now_Label[0]=label/10+48;		//标签号转ASCII码+48，整形数字转换为字符型数字
					Now_Label[1]=label%10+48;
					SPI_Flash_Write((u8*)Now_Label,10,2);
					if(label==Destination_Label)
					{
						STOP();
						break;
					}	
				}
				else
				{
					T3=0;
					Same_Bit=0;
					TIM3->CNT = 0;
//					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//					printf("AIdentification Error:RFID-1 != RFID-2");
				}
				status=MI_ERR;
				RFID_NUM=1;//关闭阅读器2，打开阅读器1			
			}					
		}
	}	
}
void Auto_BACK(void)	//预设模式——后退
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	u8 Same_Bit=0;//用于比较标签卡号时，计算相同位数
	BACK_Status=1;
	STOP_Status=0;
	RFID_NUM=2;//打开阅读器2
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C2020%02d\r\n",label);
	printf("Auto_BACK\r\n");
	while(BACK_Status==1)
	{
		while(RFID_NUM==2)//阅读器2进入工作状态
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();
					break;	
				}
				else if(RX[0]==0&&RX[1]==0)
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2020%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2020%02d0%.2f%.2f\r\n",label,x,v);
					}
				}	
			}
			//发送当前轨道机状态、标签数目、距离、速度
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);		
//			}
			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=1;	
			}
		}
		while(RFID_NUM==1)//阅读器1进入工作状态
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//停止
				{
					STOP();
					break;					
				}
				else if(RX[0]==0&&RX[1]==0)
				{
					T22 = T2;
					T21 = TIM_GetCounter(TIM2);
					x = v*((65000*T22+T21)/1000000.0);
					if(x>=10)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2020%02d%.2f%.2f\r\n",label,x,v);
					}
					else
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						printf("D2020%02d0%.2f%.2f\r\n",label,x,v);
					}
				}
			}	
//			if((T2!=0)&&(T2%100==0))//T2=100时，即1s,每秒发送一次
//			{
//				x=v*(T3/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);	 	
//			}
			if(T3>1000)
			{
				TIM3_CLOSE();
				T3=0;//时间太长溢出，清零，10s到1000
				TIM3->CNT = 0;				
				RFID_NUM=2;
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");	
				break;					
			}
			//寻卡
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//用于求速度的定时器3先关闭
				for(t=0;t<4;t++)
				{
					if(SN_1[t]==SN_2[t])
					{
						Same_Bit++;
					}
				}
				if(Same_Bit==4)
				{
					Same_Bit=0;
					T31=TIM_GetCounter(TIM3);
					T = (65000*T3+T31)/1000.0;
					v = (10.0*RFID_distance)/T;
					TIM3->CNT = 0;
					T3=0;//计算完成后清零，等待下一次计算
					for(len=0;len<50;len++)//比较标签卡号，得到标签序号
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//找到相同卡号时退出循环
						{
							Same_Bit=0;
							break;
						}
						Same_Bit=0;
					}
					label=len+1;
					Now_Label[0]=label/10+48;
					Now_Label[1]=label%10+48;
					SPI_Flash_Write((u8*)Now_Label,10,2);
					if(label==Destination_Label)
					{
						STOP();
						break;
					}		
				}
				else
				{
					T3=0;
					Same_Bit=0;
					TIM3->CNT = 0;
//					printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//					printf("AIdentification Error:RFID-1 != RFID-2");
				}
				status=MI_ERR;
				RFID_NUM=2;//关闭阅读器2，打开阅读器1			
			}					
		}
	}	

}

 int main(void)
 {	
	u8 t,id_number;
	u8 len=0;	 
	u8 RX[2]; 
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(115200);	 //串口初始化为115200
	LED_Init();		  	 //初始化与LED连接的硬件接口 
	RELAY_IO_Init();
	IO_OUT_Init();
	IO_IN_Init();
	KEY_Init();
	TIM3_Int_Init(10000,71);	//定时器3初始化 ,每10ms，进入一次中断
	TIM2_Int_Init(10000,71);
	TIM4_Int_Init(10000,7199);//
	TIM5_Int_Init(10000,7199);
	TIM2_CLOSE();
	TIM3_CLOSE();
	TIM4_CLOSE(); 
	TIM5_CLOSE();
	RC522_1_Init();
	RC522_2_Init(); 
	SPI_Flash_Init();	 
	Read_RFID();
	Now_Label[0]=7/10+48;
	Now_Label[1]=7%10+48;
	SPI_Flash_Write((u8*)Now_Label,10,2);
	while(1)
	{	
		Read_IO();
		
		if(USART_RX_STA&0x8000)
		{
			//printf("IO stop,COM data in\r\n");
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			//printf("len: %d\r\n",len);
			for(t=0;t<len;t++)
			{
				RX[t]=USART_RX_BUF[t];
				RX[t]=RX[t]-48;
				//printf("USART_RX_BUF[%d]: %d\r\n",t,USART_RX_BUF[t]);		
			}
			USART_RX_STA=0;
			if(RX[0]==9&&RX[1]==9)//控制模式
			{
				//printf("Get into Control_model\r\n");
				Model_Control();
			}			
			else if(RX[0]==9&&RX[1]==8)//预设模式
			{

				Model_Automatically();				
			}			
			else if(RX[0]==9&&RX[1]==6)//录入标签
			{
				Save_RFID();
			}
			else if(RX[0]==0&&RX[1]==0)	//上报目前所在位置
			{
				//x=v*(T2/100);
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("D0000%c%c\r\n",Now_Label[0],Now_Label[1]);
			}
			else if(RX[0]==9&&RX[1]==1)
			{
				__set_FAULTMASK(1); 
				NVIC_SystemReset();
			}
			else if(RX[0]==9&&RX[1]==5)// 标签识别
			{
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("C9511\r\n");
				RFID_NUM=1;
				id_number = 0;
				while(RFID_NUM==1)
				{
					if(USART_RX_STA&0x8000)
					{					   
						len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
						for(t=0;t<len;t++)
						{
							RX[t]=USART_RX_BUF[t];
							RX[t]=RX[t]-48;		
						}
						USART_RX_STA=0;
						if(RX[0]==9&&RX[1]==5)
						{
							
							RFID_NUM=0;
							printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
							printf("%d ",id_number);
							printf("\r\n");
							id_number=0;
							break;
						}
					}
					if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
					{
						printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
						GPIO_ResetBits(GPIOA,GPIO_Pin_8);
						for(t=0;t<4;t++)
						{
							printf("%d ",SN_1[t]);
						}
						printf("\r\n");
						id_number=id_number+1;
						status=MI_ERR;						
					}									
//					delay_ms(100);
					GPIO_SetBits(GPIOA,GPIO_Pin_8);					
				}	
			}
		}
	}
}
 

