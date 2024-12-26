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
/*ȫ�ֱ���*/

u32 FLASH_SIZE=8*1024*1024;			//FLASH ��СΪ8M�ֽ�
unsigned char CT[2];    			//������
unsigned char SN_1[4];   			//�Ķ���1��ȡ�Ŀ���
unsigned char SN_2[4];				//�Ķ���2��ȡ�Ŀ���
unsigned char status;				//����״̬

unsigned char N[50][4]; 			//��ά�����ſ��źͶ�Ӧ���

unsigned int T3=0;					//��ʱ��3������һ�����ʱ0.01�룬�������ٶ�
unsigned int T2=0;					//��ʱ��2������һ�����ʱ0.01�룬����ʱ�䣬���������о���
unsigned int T4=0;					//��ʱ��4������һ�����ʱ1�룬����Ѳ��ģʽʱһ��ͣ���Ķ�ʱ
unsigned int T5=0;					//��ʱ��5������һ�����ʱ1�룬����Ѳ��ģʽʱ��������ʱ��Ķ�ʱ
unsigned int T31=0;
unsigned int T21=0;
unsigned int T22=0;
unsigned int T41=0;
unsigned char Model=0;				//����ģʽ:1��Ԥ��ģʽ:2��Ѳ��ģʽ��3
unsigned char RFID_NUM;				//�Ķ���������ţ�ֵΪ1ʱ���Ķ���1��ֵΪ2ʱ���Ķ���2
unsigned char Same_Bit=0;			//���ڱȽϱ�ǩ����ʱ��������ͬλ��
unsigned char Save_Status=0;		//W25Q64д��״̬
unsigned char Read_Status=0;		//W25Q64��ȡ״̬
unsigned char GO_Status=0;			//ǰ��״̬��1
unsigned char BACK_Status=0;		//����״̬��1
unsigned char STOP_Status=1;		//ֹͣ״̬��1
unsigned char Cruise_Status=0;
unsigned char IO_GO_Status=0;
unsigned char IO_BACK_Status=0;
unsigned char IO_STOP_Status=1;

unsigned int label=0;				//��ǩ���
unsigned char Now_Label[2];			//Ŀǰ���ڵı�ǩ�ţ�
unsigned char Destination_Label;	//Ԥ��ģʽ�µ�Ŀ�ı�ǩ���
unsigned char Origin_Label;
double RFID_distance=5.0;			//���������10cm
double v=0;							//��ʵ�ٶ�
double x=0;							//��ǰ��ǰһ����ǩ�ľ���
double T;
double t2[50];
double t3[50];
double t4[50];
double x1[50];

unsigned int huoer_num,all_huoer_num;
unsigned char huoer_flag;
unsigned int h_num[50];

unsigned char SIZE;
unsigned char Addr_Broadcast[2]={0XFF,0XFF};//�㲥��ַ
unsigned char Addr_A[2]={0X02,0XCA};		//���ܶ�LORA��ַ'd714
unsigned char Addr_Local[2]={0X0E,0X4E};	//������LORA��ַ'd3662
unsigned char Addr_Gateway[2]={0X02,0XCA};
unsigned char Lora_Channel = 0X17;			//�㲥ͨ����0x17 == 'd23 
//��ֹ��¥��GPS�����LORA��ͻ����������Ϊ0x18 ==��d24

/*��������*/
void Printing(unsigned char* Buffer,unsigned short len);	//������������ݷ��͵�����(�ַ���)
unsigned char Request_Anticoll( unsigned char request_mode,
	       unsigned char* CT, unsigned char* SN, unsigned char rfid_num );		//Ѱ��������ײ��ѡ��

void Model_Control(void);      		//����ģʽ	
void Model_Automatically(void);		//Ԥ��ģʽ	
		   
void Control_GO(void);	//����ģʽ��ǰ��
void Control_BACK(void);//����ģʽ������
void Auto_GO(void);	//Ԥ��ģʽ��ǰ��
void Auto_BACK(void);//Ԥ��ģʽ��ǰ����

void STOP(void);
  
void Save_RFID(void);
void Read_RFID(void);
void Read_IO(void);		   

void Printing(unsigned char* Buffer,unsigned short len)
{
	unsigned char i;
	for(i=0;i<len;i++)
	{
		printf("%d ",Buffer[i]); //������ʾSPI1ͨ���Ŀ���   
	}
}				 

//Ѱ��������ײ
unsigned char Request_Anticoll(unsigned char request_mode,
	unsigned char* CT, unsigned char* SN, unsigned char rfid_num)
{
	//Ѱ�� Ѱ������δ�������ߵĿ�
	if(rfid_num==1)
	{
		PcdAntennaOn_1();
		status = PcdRequest_1(request_mode,CT);/*Ѱ��*///˳��ѿ�������д������CT�� 
		if(status==MI_OK)//�����ɹ�
		{
			status=MI_ERR;
			status = PcdAnticoll_1(SN);/*����ײ*///˳��ѿ���д��SN������		
			if(status==MI_OK)
			{		 
				PcdAntennaOff_1();
				return status;//�ɹ�		
			}
		}
		status=MI_ERR;
//		PcdAntennaOff_1();
		return status;//ʧ��
	}
	else
	{
		PcdAntennaOn_2();
		status = PcdRequest_2(request_mode,CT);/*Ѱ��*///˳��ѿ�������д������CT��
		if(status==MI_OK)//�����ɹ�
		{
			status=MI_ERR;
			status = PcdAnticoll_2(SN);/*����ײ*///˳��ѿ���д��SN������		
			if(status==MI_OK)
			{		
				PcdAntennaOff_2();
				return status;//�ɹ�		
			}
		}
		status=MI_ERR;
//		PcdAntennaOff_2();
		return status;//ʧ��
	}
}
//�洢���Ž�W25Q64
void Save_RFID(void)
{
	u8 t,len;
	u8 flag;
	u8 datatemp[4];
	u8 RX[2];
	u8 Save_Number=0;
	RFID_NUM=1;//���Ķ���1
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
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
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
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
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
					SPI_Flash_Write((u8*)SN_1,(FLASH_SIZE-1000+Save_Number*SIZE),SIZE);//�ӵ�����1000����ַ����ʼ,д��SIZE���ȵ�����
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
	SPI_Flash_Read(Now_Label,10,2);//��ȡĿǰ����λ��
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
		//printf("��ȡ�������ǰ���ź�");
		Control_GO();
		
	}
	else if(IO_GO_Status==1&&IO_BACK_Status==0&&IO_STOP_Status==1)
	{
		//printf("��ȡ������ĺ����ź�");
		Control_BACK();
		
	}
	//printf("IO����Ϊ111ֹͣ");

}
//����ģʽ	
void Model_Control(void)
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	Model=1;
	SPI_Flash_Read(Now_Label,10,2);//��ȡĿǰ����λ��
	label=((Now_Label[0]-48)*10)+(Now_Label[1]-48);
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C9911%c%c\r\n",Now_Label[0],Now_Label[1]);
	//printf("Model Control\r\n");
	while(Model==1)
	{
		Read_IO();
		if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			for(t=0;t<len;t++)
			{
				RX[t]=USART_RX_BUF[t];
				RX[t]=RX[t]-48;		
			}
			USART_RX_STA=0;
			//printf("COM data received in model_control\r\n");
			if(RX[0]==3&&RX[1]==1&&STOP_Status==1)	//ǰ��
			{
				//printf("Model Control Go\r\n");
				GPIO_ResetBits(GPIOC,GPIO_Pin_6);	//�̵���
				GPIO_ResetBits(GPIOB,GPIO_Pin_9);	//IO��
				GPIO_ResetBits(GPIOA,GPIO_Pin_8);
				delay_ms(50);
				GPIO_SetBits(GPIOC,GPIO_Pin_6);
				GPIO_SetBits(GPIOB,GPIO_Pin_9);	
				GPIO_SetBits(GPIOA,GPIO_Pin_8);
				Control_GO();
				printf("Go finish\r\n");
			}
			else if(RX[0]==3&&RX[1]==0&&STOP_Status==1)	//����
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
			else if(RX[0]==9&&RX[1]==9&&STOP_Status==1)	//�˳�����ģʽ
			{
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("C9901%c%c\r\n",Now_Label[0],Now_Label[1]);//�˳����������ģʽ

				Model=0;
			}						
		}		
	}
}

//Ԥ��ģʽ
void Model_Automatically(void)
{
	u8 t;
	u8 len;	 
	u8 RX[4];
	Model=2;
	SPI_Flash_Read(Now_Label,10,2);//��ȡĿǰ����λ��
	label=((Now_Label[0]-48)*10)+(Now_Label[1]-48);
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C9811%c%c\r\n",Now_Label[0],Now_Label[1]);
	printf("Automatically start\r\n");	
	while(Model==2)
	{
		Read_IO();
		if(USART_RX_STA&0x8000)			//�������
		{					   
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
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
				printf("C9801%c%c\r\n\r\n",Now_Label[0],Now_Label[1]);//�˳������Ԥ��ģʽ
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
				if(RX[0]==3&&RX[1]==1&&STOP_Status==1)		//ǰ��
				{
					GPIO_ResetBits(GPIOC,GPIO_Pin_6);	//�̵���
					GPIO_ResetBits(GPIOB,GPIO_Pin_9);	//IO��
					GPIO_ResetBits(GPIOA,GPIO_Pin_8);
					delay_ms(50);
					GPIO_SetBits(GPIOC,GPIO_Pin_6);
					GPIO_SetBits(GPIOB,GPIO_Pin_9);	
					GPIO_SetBits(GPIOA,GPIO_Pin_8);
					Auto_GO();	
				}
				else if(RX[0]==3&&RX[1]==0&&STOP_Status==1)		//����
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
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);//ֹͣ
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
//	printf("C9090\r\n");//��������ֹͣ
}

void Control_GO(void)			//����ģʽ����ǰ��
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	u8 Same_Bit=0;//���ڱȽϱ�ǩ����ʱ��������ͬλ��
	GO_Status=1;
	STOP_Status=0;
	RFID_NUM=1;//���Ķ���1
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C1111%02d\r\n",label);
	printf("control go\r\n");
	while(GO_Status==1)
	{
		//printf("GO_status\r\n");
		while(RFID_NUM==1)//�Ķ���1���빤��״̬
		{
			//printf("rfid_1 work\r\n");
			if(USART_RX_STA&0x8000)
			{	
				//printf("get new rx");
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
				{
					STOP();	
					break;					
				}
				else if(RX[0]==0&&RX[1]==0)				//�����ϱ�
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
			
			//���͵�ǰ�����״̬����ǩ��Ŀ�����롢�ٶ�
//			if((T2!=0)&&(T2%15==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				T22 = T2;
//				T21 = TIM_GetCounter(TIM2);
//				x = v*((65000*T22+T21)/1000000.0);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);		
//			}

			//Ѱ��
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=2;//�ر��Ķ���1�����Ķ���2				
			}
		}

		while(RFID_NUM==2)//�Ķ���2���빤��״̬
		{
			printf("rfid_2 work\r\n");
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t]; 
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
			
			
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);	
//			}
			if(T3>100)
			{
				  
				TIM3_CLOSE();
				TIM3->CNT = 0;
				T3=0;//ʱ��̫�����������	
				RFID_NUM=1;	
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");
				break;				
			}
			//Ѱ��
			if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//�������ٶȵĶ�ʱ��3�ȹر�
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
					T3=0;//������ɺ����㣬�ȴ���һ�μ���
					for(len=0;len<50;len++)//�Ƚϱ�ǩ���ţ��õ���ǩ���
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//�ҵ���ͬ����ʱ�˳�ѭ��
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
				RFID_NUM=1;//�ر��Ķ���2�����Ķ���1	
				
			}						
		}
	}
	printf("control go end\r\n");	
}
void Control_BACK(void)		//����ģʽ��������
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	
	BACK_Status=1;
	STOP_Status=0;
	RFID_NUM=2;//���Ķ���2
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C1010%02d\r\n",label);
	printf("Control_BACK\r\n");
	while(BACK_Status==1)
	{
		while(RFID_NUM==2)//�Ķ���1���빤��״̬
		{
			
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
			//���͵�ǰ�����״̬����ǩ��Ŀ�����롢�ٶ�
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);		
//			}
		
			//Ѱ��
			 if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=1;//�ر��Ķ���2�����Ķ���1			
			}
		}

		while(RFID_NUM==1)//�Ķ���2���빤��״̬
		{
			
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);		
//			}
			if(T3>100)
			{
				TIM3_CLOSE();
				T3=0;//ʱ��̫����������㣬1���ӵ�6000	
				TIM3->CNT = 0;
				RFID_NUM=2;	
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");
				break;
			}
			//Ѱ��
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//�������ٶȵĶ�ʱ��3�ȹر�
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
					T3=0;//������ɺ����㣬�ȴ���һ�μ���
					for(len=0;len<50;len++)//�Ƚϱ�ǩ���ţ��õ���ǩ���
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//�ҵ���ͬ����ʱ�˳�ѭ��
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
				RFID_NUM=2;//�ر��Ķ���1�����Ķ���2			
			}
		}
	}
	printf("control back end\r\n");
}

void Auto_GO(void)	//Ԥ��ģʽ����ǰ��
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	u8 Same_Bit=0;//���ڱȽϱ�ǩ����ʱ��������ͬλ��
	GO_Status=1;
	STOP_Status=0;
	RFID_NUM=1;//���Ķ���1
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C2222%02d\r\n",label);
	printf("Auto_GO\r\n");
	while(GO_Status==1)
	{
		while(RFID_NUM==1)//�Ķ���1���빤��״̬
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
			//���͵�ǰ�����״̬����ǩ��Ŀ�����롢�ٶ�
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);	 	
//			}
			//Ѱ��
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM2_CLOSE();
				TIM3_OPEN();
				TIM2->CNT = 0;
				T2 =0;
				TIM2_OPEN();
				status=MI_ERR;
				RFID_NUM=2;//�ر��Ķ���1�����Ķ���2				
			}
		}
		while(RFID_NUM==2)//�Ķ���2���빤��״̬
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1111%02d%.2f%.2f\r\n",label,x,v);		
//			}
			if(T3>100)//6.5s
			{
				TIM3_CLOSE();
				T3=0;//ʱ��̫�����������
				TIM3->CNT = 0;				
				RFID_NUM=1;
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");
				break;
			}
			//Ѱ��
			if(Request_Anticoll(PICC_REQALL,CT,SN_2,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//�������ٶȵĶ�ʱ��3�ȹر�
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
					T3=0;//������ɺ����㣬�ȴ���һ�μ���
					for(len=0;len<50;len++)//�Ƚϱ�ǩ���ţ��õ���ǩ���
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//�ҵ���ͬ����ʱ�˳�ѭ��
						{
							Same_Bit=0;
							break;
						}
						Same_Bit=0;
					}
					label=len+1;
					Now_Label[0]=label/10+48;		//��ǩ��תASCII��+48����������ת��Ϊ�ַ�������
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
				RFID_NUM=1;//�ر��Ķ���2�����Ķ���1			
			}					
		}
	}	
}
void Auto_BACK(void)	//Ԥ��ģʽ��������
{
	u8 t;
	u8 len;	 
	u8 RX[2];
	u8 Same_Bit=0;//���ڱȽϱ�ǩ����ʱ��������ͬλ��
	BACK_Status=1;
	STOP_Status=0;
	RFID_NUM=2;//���Ķ���2
	printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
	printf("C2020%02d\r\n",label);
	printf("Auto_BACK\r\n");
	while(BACK_Status==1)
	{
		while(RFID_NUM==2)//�Ķ���2���빤��״̬
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
			//���͵�ǰ�����״̬����ǩ��Ŀ�����롢�ٶ�
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T2/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);		
//			}
			//Ѱ��
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
		while(RFID_NUM==1)//�Ķ���1���빤��״̬
		{
			if(USART_RX_STA&0x8000)
			{					   
				len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
				for(t=0;t<len;t++)
				{
					RX[t]=USART_RX_BUF[t];
					RX[t]=RX[t]-48;		
				}
				USART_RX_STA=0;
				if(RX[0]==9&&RX[1]==0&&STOP_Status==0)	//ֹͣ
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
//			if((T2!=0)&&(T2%100==0))//T2=100ʱ����1s,ÿ�뷢��һ��
//			{
//				x=v*(T3/100);
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("C1010%02d%.2f%.2f\r\n",label,x,v);	 	
//			}
			if(T3>1000)
			{
				TIM3_CLOSE();
				T3=0;//ʱ��̫����������㣬10s��1000
				TIM3->CNT = 0;				
				RFID_NUM=2;
//				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
//				printf("AThe second time identification RFID Error\r\n");	
				break;					
			}
			//Ѱ��
			if(Request_Anticoll(PICC_REQALL,CT,SN_1,RFID_NUM)==MI_OK)
			{
				TIM3_CLOSE();//�������ٶȵĶ�ʱ��3�ȹر�
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
					T3=0;//������ɺ����㣬�ȴ���һ�μ���
					for(len=0;len<50;len++)//�Ƚϱ�ǩ���ţ��õ���ǩ���
					{
						for(t=0;t<4;t++)
						{
							if(N[len][t]==SN_1[t])
							{
								Same_Bit++;
							}
						}
						if(Same_Bit==4)//�ҵ���ͬ����ʱ�˳�ѭ��
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
				RFID_NUM=2;//�ر��Ķ���2�����Ķ���1			
			}					
		}
	}	

}

 int main(void)
 {	
	u8 t,id_number;
	u8 len=0;	 
	u8 RX[2]; 
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ� 
	RELAY_IO_Init();
	IO_OUT_Init();
	IO_IN_Init();
	KEY_Init();
	TIM3_Int_Init(10000,71);	//��ʱ��3��ʼ�� ,ÿ10ms������һ���ж�
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
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			//printf("len: %d\r\n",len);
			for(t=0;t<len;t++)
			{
				RX[t]=USART_RX_BUF[t];
				RX[t]=RX[t]-48;
				//printf("USART_RX_BUF[%d]: %d\r\n",t,USART_RX_BUF[t]);		
			}
			USART_RX_STA=0;
			if(RX[0]==9&&RX[1]==9)//����ģʽ
			{
				//printf("Get into Control_model\r\n");
				Model_Control();
			}			
			else if(RX[0]==9&&RX[1]==8)//Ԥ��ģʽ
			{

				Model_Automatically();				
			}			
			else if(RX[0]==9&&RX[1]==6)//¼���ǩ
			{
				Save_RFID();
			}
			else if(RX[0]==0&&RX[1]==0)	//�ϱ�Ŀǰ����λ��
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
			else if(RX[0]==9&&RX[1]==5)// ��ǩʶ��
			{
				printf("%c%c%c",Addr_A[0],Addr_A[1],Lora_Channel);
				printf("C9511\r\n");
				RFID_NUM=1;
				id_number = 0;
				while(RFID_NUM==1)
				{
					if(USART_RX_STA&0x8000)
					{					   
						len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
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
 

