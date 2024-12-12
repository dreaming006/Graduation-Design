#ifndef __RC522_2_H
#define __RC522_2_H	
#include "sys.h"
#include "stm32f10x.h"

/*******************************
*RC522_2����:
*1--SDA  <----->PB12
*2--SCK  <----->PB13
*3--MOSI <----->PB15
*4--MISO <----->PB14
*5--����
*6--GND <----->GND
*7--RST <----->PA12
*8--3.3V <----->3.3V
************************************/


#define SPI2_RC522_ReadByte()	      SPI2_RC522_SendByte(0)  //���޸�


/***********************RC522 �����궨��**********************/

#define          RC522_2_SCK_0()             GPIO_ResetBits( GPIOB, GPIO_Pin_13 )
#define          RC522_2_SCK_1()             GPIO_SetBits ( GPIOB, GPIO_Pin_13 )

#define          RC522_2_MOSI_0()            GPIO_ResetBits( GPIOB, GPIO_Pin_15 )
#define          RC522_2_MOSI_1()            GPIO_SetBits ( GPIOB, GPIO_Pin_15 )

#define          RC522_2_MISO_GET()          GPIO_ReadInputDataBit ( GPIOB, GPIO_Pin_14 )

#define          RC522_2_CS_Enable()         GPIO_ResetBits ( GPIOB, GPIO_Pin_12 )
#define          RC522_2_CS_Disable()        GPIO_SetBits ( GPIOB, GPIO_Pin_12 )

#define          RC522_2_Reset_Enable()      GPIO_ResetBits( GPIOA, GPIO_Pin_12 )
#define          RC522_2_Reset_Disable()     GPIO_SetBits ( GPIOA, GPIO_Pin_12 )


u8       SPI2_RC522_SendByte         ( u8 byte);
u8       ReadRawRC_2                  ( u8 ucAddress );               //��    �ܣ���RC522�Ĵ���
void     WriteRawRC_2                 ( u8 ucAddress, u8 ucValue );   //��    �ܣ�дRC522�Ĵ���
void     SPI2_Init                  ( void );                   
void     RC522_2_Init                 ( void );   //��    �ܣ�RC522��Ƶ��ģ���ʼ��
void     PcdReset_2                   ( void );                       //��    �ܣ���λRC522
void     M500PcdConfigISOType_2       ( u8 type );                    //��    �ܣ�����RC522�Ĺ�����ʽ 
char     PcdRequest_2                 ( u8 req_code, u8 * pTagType ); //��    �ܣ�Ѱ��
char     PcdAnticoll_2                ( u8 * pSnr);                   //��    �ܣ�����ײ

void     PcdAntennaOn_2               ( void );                 //��    �ܣ��������� 
void     PcdAntennaOff_2             ( void );                 //��    �ܣ��ر�����
void     SetBitMask_2                 ( u8 ucReg, u8 ucMask );  //��    �ܣ���RC522�Ĵ���λ
void     ClearBitMask_2               ( u8 ucReg, u8 ucMask );  //��    �ܣ���RC522�Ĵ���λ
char     PcdSelect_2                  ( u8 * pSnr );            //��    �ܣ�ѡ����Ƭ
char     PcdAuthState_2               ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr );   //��֤��Ƭ����     
char     PcdWrite_2                   ( u8 ucAddr, u8 * pData );//��    �ܣ�д���ݵ�M1��һ��
char     PcdRead_2                    ( u8 ucAddr, u8 * pData );//��    �ܣ���ȡM1��һ������
void     ShowID_2                     ( u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor);	 //��ʾ���Ŀ��ţ���ʮ����������ʾ
char     PcdHalt_2                    ( void );           //��    �ܣ����Ƭ��������״̬
void     CalulateCRC_2                ( u8 * pIndata, u8 ucLen, u8 * pOutData );//��    �ܣ���MF522����CRC16����

#endif
