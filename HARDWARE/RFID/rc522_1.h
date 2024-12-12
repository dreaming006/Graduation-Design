#ifndef __RC522_1_H
#define __RC522_1_H	
#include "sys.h"
#include "stm32f10x.h"

/*******************************
*RC522_1����:
*1--SDA  <----->PA4
*2--SCK  <----->PA5
*3--MOSI <----->PA7
*4--MISO <----->PA6
*5--����
*6--GND <----->GND
*7--RST <----->PA11
*8--3.3V <----->3.3V
************************************/


#define SPI1_RC522_ReadByte()	      SPI1_RC522_SendByte(0)  

/***********************RC522 �����궨��**********************/
#define          RC522_1_CS_Enable()         GPIO_ResetBits ( GPIOA, GPIO_Pin_4 )
#define          RC522_1_CS_Disable()        GPIO_SetBits ( GPIOA, GPIO_Pin_4 )

#define          RC522_1_Reset_Enable()      GPIO_ResetBits( GPIOA, GPIO_Pin_11 )
#define          RC522_1_Reset_Disable()     GPIO_SetBits ( GPIOA, GPIO_Pin_11 )

#define          RC522_1_SCK_0()             GPIO_ResetBits( GPIOA, GPIO_Pin_5 )
#define          RC522_1_SCK_1()             GPIO_SetBits ( GPIOA, GPIO_Pin_5 )

#define          RC522_1_MOSI_0()            GPIO_ResetBits( GPIOA, GPIO_Pin_7 )
#define          RC522_1_MOSI_1()            GPIO_SetBits ( GPIOA, GPIO_Pin_7 )

#define          RC522_1_MISO_GET()          GPIO_ReadInputDataBit ( GPIOA, GPIO_Pin_6 )


u8       SPI1_RC522_SendByte         ( u8 byte);
u8       ReadRawRC_1                  ( u8 ucAddress );               //��    �ܣ���RC522�Ĵ���
void     WriteRawRC_1                 ( u8 ucAddress, u8 ucValue );   //��    �ܣ�дRC522�Ĵ���
void     SPI1_Init                  ( void );
void     RC522_1_Init               ( void );                     //��    �ܣ�RC522��Ƶ��ģ���ʼ��  
void     PcdReset_1                  ( void );                       //��    �ܣ���λRC522
void     M500PcdConfigISOType_1       ( u8 type );                    //��    �ܣ�����RC522�Ĺ�����ʽ 
char     PcdRequest_1                 ( u8 req_code, u8 * pTagType ); //��    �ܣ�Ѱ��
char     PcdAnticoll_1                ( u8 * pSnr);                   //��    �ܣ�����ײ

void     PcdAntennaOn_1               ( void );                 //��    �ܣ��������� 
void     PcdAntennaOff_1              ( void );                 //��    �ܣ��ر�����
void     SetBitMask_1                 ( u8 ucReg, u8 ucMask );  //��    �ܣ���RC522�Ĵ���λ
void     ClearBitMask_1               ( u8 ucReg, u8 ucMask );  //��    �ܣ���RC522�Ĵ���λ
char     PcdSelect_1                  ( u8 * pSnr );            //��    �ܣ�ѡ����Ƭ
char     PcdAuthState_1              ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr );   //��֤��Ƭ����     
char     PcdWrite_1                   ( u8 ucAddr, u8 * pData );//��    �ܣ�д���ݵ�M1��һ��
char     PcdRead_1                    ( u8 ucAddr, u8 * pData );//��    �ܣ���ȡM1��һ������
void     ShowID_1                     ( u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor);	 //��ʾ���Ŀ��ţ���ʮ����������ʾ
char     PcdHalt_1                    ( void );           //��    �ܣ����Ƭ��������״̬
void     CalulateCRC_1                ( u8 * pIndata, u8 ucLen, u8 * pOutData );//��    �ܣ���MF522����CRC16����

#endif
