#ifndef __RC522_1_H
#define __RC522_1_H	
#include "sys.h"
#include "stm32f10x.h"

/*******************************
*RC522_1连线:
*1--SDA  <----->PA4
*2--SCK  <----->PA5
*3--MOSI <----->PA7
*4--MISO <----->PA6
*5--悬空
*6--GND <----->GND
*7--RST <----->PA11
*8--3.3V <----->3.3V
************************************/


#define SPI1_RC522_ReadByte()	      SPI1_RC522_SendByte(0)  

/***********************RC522 函数宏定义**********************/
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
u8       ReadRawRC_1                  ( u8 ucAddress );               //功    能：读RC522寄存器
void     WriteRawRC_1                 ( u8 ucAddress, u8 ucValue );   //功    能：写RC522寄存器
void     SPI1_Init                  ( void );
void     RC522_1_Init               ( void );                     //功    能：RC522射频卡模块初始化  
void     PcdReset_1                  ( void );                       //功    能：复位RC522
void     M500PcdConfigISOType_1       ( u8 type );                    //功    能：设置RC522的工作方式 
char     PcdRequest_1                 ( u8 req_code, u8 * pTagType ); //功    能：寻卡
char     PcdAnticoll_1                ( u8 * pSnr);                   //功    能：防冲撞

void     PcdAntennaOn_1               ( void );                 //功    能：开启天线 
void     PcdAntennaOff_1              ( void );                 //功    能：关闭天线
void     SetBitMask_1                 ( u8 ucReg, u8 ucMask );  //功    能：置RC522寄存器位
void     ClearBitMask_1               ( u8 ucReg, u8 ucMask );  //功    能：清RC522寄存器位
char     PcdSelect_1                  ( u8 * pSnr );            //功    能：选定卡片
char     PcdAuthState_1              ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr );   //验证卡片密码     
char     PcdWrite_1                   ( u8 ucAddr, u8 * pData );//功    能：写数据到M1卡一块
char     PcdRead_1                    ( u8 ucAddr, u8 * pData );//功    能：读取M1卡一块数据
void     ShowID_1                     ( u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor);	 //显示卡的卡号，以十六机进制显示
char     PcdHalt_1                    ( void );           //功    能：命令卡片进入休眠状态
void     CalulateCRC_1                ( u8 * pIndata, u8 ucLen, u8 * pOutData );//功    能：用MF522计算CRC16函数

#endif
