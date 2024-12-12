#include "sys.h"
#include "rc522.h"
#include "rc522_2.h"
#include <string.h>
#include "delay.h"

#define   RC522_DELAY()  delay_us( 2 ) 

/*????*/

extern unsigned char RFID_NUM;		//阅读器编号：1/2


/* 函数名:RC522_1_Init / RC522_2_Init 
 * 描述  :初始化RC522_1 / RC522_2配置
 * 输入  :无
 * 返回  :无
 * 调用  :外部调用             */

void RC522_2_Init ( void )
{
	RFID_NUM=2;
	SPI2_Init(); 				  				//初始化SPI2
	RC522_2_Reset_Disable();	    //将RST置高,启动内部复位阶段;
	PcdReset_2 ();                  //复位RC522 
	PcdAntennaOff_2();              //关闭天线
	RC522_DELAY();                //delay 2us
	PcdAntennaOn_2();               //开启天线
	M500PcdConfigISOType_2 ( 'A' ); //设置工作模式
	PcdAntennaOff_2();
	//RFID_NUM=2;
}

/* 函数名:SPI1_Init / SPI2_Init
 * 描述  :初始化SPI1 / SPI2配置
 * 输入  :无
 * 返回  :无
 * 调用  :外部调用             */

void SPI2_Init (void)	
{
	SPI_InitTypeDef  SPI_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE );//PORTA时钟使能
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );
	
	// CS   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);					 //初始化PB12
	
    // SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // RST
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	//置高CS口
	RC522_2_CS_Disable();

    //其他SPI1配置
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;       //全双工;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                                //主机模式;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                            //传输数据为8bit;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                                   //时钟极性CPOL为空闲时低电平;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                                 //时钟采样点为时钟奇数沿(上升沿);
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                                    //NSS引脚由软件改变;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;          //预分频系数64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                           //MSB先行模式;
    SPI_InitStructure.SPI_CRCPolynomial = 7;                                     //CRC校验;
		
	//初始化SPI1
    SPI_Init(SPI2 , &SPI_InitStructure);
		
	//使能SPI1
	SPI_Cmd(SPI2 , ENABLE); 
 }


/* 函数名:PcdRese
 * 描述  :复位RC522_1和RC522_2
 * 输入  :无
 * 返回  :无
 * 调用  :外部调用             */
void PcdReset_2 ( void )
{
	
	RC522_2_Reset_Disable();
	delay_us ( 1 );
	RC522_2_Reset_Enable();
	delay_us ( 1 );
	RC522_2_Reset_Disable();
	delay_us ( 1 );

	
    WriteRawRC_2 ( CommandReg, 0x0f );

    while ( ReadRawRC_2 ( CommandReg ) & 0x10 );
	
    delay_us ( 1 );
    WriteRawRC_2 ( ModeReg, 0x3D );                //定义发送和接收常用模式 和Mifare卡通信,CRC初始值0x6363
    WriteRawRC_2 ( TReloadRegL, 30 );              //16位定时器低位   
    WriteRawRC_2 ( TReloadRegH, 0 );			     //16位定时器低位   
    WriteRawRC_2 ( TModeReg, 0x8D );				 //定义内部定时器的设置
    WriteRawRC_2 ( TPrescalerReg, 0x3E );			 //定义定时器分频系数
    WriteRawRC_2 ( TxAutoReg, 0x40 );				 //?调制发送信号为100%ASK		
}

/* 函数名:SPI_RC522_SendByte
 * 描述  :向RC522_1或RC522_2发送1 Byte 数据
 * 输入  :byte,要发送的数据
 * 返回  :RC522返回的数据
 * 调用  :内部调用    */
u8 SPI2_RC522_SendByte ( u8 byte )
{
	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);	
	SPI_I2S_SendData(SPI2, byte);  
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET); 
	return 	SPI_I2S_ReceiveData(SPI2);	
	
}


/* 函数名:ReadRawRC
 * 描述  :读RC522寄存器
 * 输入  :ucAddress,寄存器地址
 * 返回  :寄存器的当前值
 * 调用  :内部调用      */
u8 ReadRawRC_2 ( u8 ucAddress )
{
	u8 ucAddr, ucReturn;
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;      

	RC522_2_CS_Enable();
		
	SPI2_RC522_SendByte ( ucAddr );
	ucReturn = SPI2_RC522_ReadByte ();
	
	RC522_2_CS_Disable();

	return ucReturn;
}


/* 函数名:WriteRawRC
 * 描述  :写RC522寄存器
 * 输入  :ucAddress,寄存器地址、ucValue,写入寄存器的值
 * 返回  :无
 * 调用  :内部调用      */
void WriteRawRC_2 ( u8 ucAddress, u8 ucValue )
{  
	u8 ucAddr;
	ucAddr = ( ucAddress << 1 ) & 0x7E;   
	
	RC522_2_CS_Enable();
	
	SPI2_RC522_SendByte ( ucAddr );
	SPI2_RC522_SendByte ( ucValue );
	
	RC522_2_CS_Disable();


}

/* 函数名:M500PcdConfigISOType
 * 描述  :设置RC522的工作模式
 * 输入  :ucType，工作方式
 * 返回  :无
 * 调用  :外部调用      */
void M500PcdConfigISOType_2 ( u8 ucType )
{
	if ( ucType == 'A')                     //ISO14443_A
	{
		ClearBitMask_2 ( Status2Reg, 0x08 );		

		WriteRawRC_2 ( ModeReg, 0x3D );//3F	
		WriteRawRC_2 ( RxSelReg, 0x86 );//84
		WriteRawRC_2 ( RFCfgReg, 0x7F );   //4F
		WriteRawRC_2 ( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		WriteRawRC_2 ( TReloadRegH, 0 );
		WriteRawRC_2 ( TModeReg, 0x8D );
		WriteRawRC_2 ( TPrescalerReg, 0x3E );
		delay_us   ( 2 );
		
		PcdAntennaOn_2 ();//开天线
	}
}

/*
 * 函数名:SetBitMask
 * 描述  :对RC522寄存器置位
 * 输入  :ucReg,寄存器地址
 *        ucMask,置位值
 * 返回  :无
 * 调用  :内部调用
 */
void SetBitMask_2 ( u8 ucReg, u8 ucMask )  
{
    u8 ucTemp;

    ucTemp = ReadRawRC_2 ( ucReg );
    WriteRawRC_2 ( ucReg, ucTemp | ucMask );         // set bit mask
}

/* 函数名:ClearBitMask
 * 描述  :对RC522寄存器清位
 * 输入  :ucReg,寄存器地址
 *         ucMask,置位值
 * 返回  :无
 * 调用  :内部调用           */
void ClearBitMask_2 ( u8 ucReg, u8 ucMask )  
{
    u8 ucTemp;
    ucTemp = ReadRawRC_2 ( ucReg );
	
    WriteRawRC_2 ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask
	
}

/* 函数名:PcdAntennaOn
 * 描述  :开启天线 
 * 输入  :无
 * 返回  :无
 * 调用  :内部调用            */
void PcdAntennaOn_2 ( void )
{
    u8 uc;
    uc = ReadRawRC_2 ( TxControlReg );
	
    if ( ! ( uc & 0x03 ) )
			SetBitMask_2(TxControlReg, 0x03);

}

/* 函数名:PcdAntennaOff
 * 描述  :关闭天线 
 * 输入  :无
 * 返回  :无
 * 调用  :内部调用            */
void PcdAntennaOff_2 ( void )
{
    ClearBitMask_2 ( TxControlReg, 0x03 );
}

/* 函数名:PcdComMF522
 * 描述  :通过RC522和ISO14443卡通讯
 * 输入  :ucCommand,RC522命令字
 *         pInData,通过RC522发送到卡片的数据
 *         ucInLenByte,发送数据的字节长度
 *         pOutData,接收到的卡片返回数据
 *         pOutLenBit,返回数据的位长度
 * 返回  : 状态值
 *         = MI_OK,成功
 * 调用  :内部调用              */
char PcdComMF522_2 ( u8 ucCommand, u8 * pInData, u8 ucInLenByte, u8 * pOutData, u32 * pOutLenBit )		
{
    char cStatus = MI_ERR;
    u8 ucIrqEn   = 0x00;
    u8 ucWaitFor = 0x00;
    u8 ucLastBits;
    u8 ucN;
    u32 ul;

    switch ( ucCommand )
    {
       case PCD_AUTHENT:		//Mifare认证
          ucIrqEn   = 0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
          ucWaitFor = 0x10;		//认证寻卡等待时候 查询空闲中断标志位
          break;
			 
       case PCD_TRANSCEIVE:		//接收发送 发送接收
          ucIrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
          ucWaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与空闲中断标志位
          break;
			 
       default:
         break;
			 
    }
   
    WriteRawRC_2 ( ComIEnReg, ucIrqEn | 0x80 );		//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反 
    ClearBitMask_2 ( ComIrqReg, 0x80 );			//Set1该位清零时,CommIRqReg的屏蔽位清零
    WriteRawRC_2 ( CommandReg, PCD_IDLE );		//写空闲命令
    SetBitMask_2 ( FIFOLevelReg, 0x80 );			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl?标志位被清楚
    
    for ( ul = 0; ul < ucInLenByte; ul ++ )
		WriteRawRC_2 ( FIFODataReg, pInData [ ul ] );    		//写数据进FIFOdata
			
    WriteRawRC_2 ( CommandReg, ucCommand );					//写命令
   
    
    if ( ucCommand == PCD_TRANSCEIVE )
			SetBitMask_2(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效
    
    ul = 1000;//根据时钟频率调整,操作M1卡最大等待时间25ms（可调）
		
    do 														//认证与寻卡等待时间	
    {
         ucN = ReadRawRC_2 ( ComIrqReg );							//查询事件中断
         ul --;
    } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//退出条件i=0,定时器中断,与写空闲命令
		
    ClearBitMask_2 ( BitFramingReg, 0x80 );					//清理允许StartSend位
		
    if ( ul != 0 )
    {
		if ( ! (( ReadRawRC_2 ( ErrorReg ) & 0x1B )) )			//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
		{
			cStatus = MI_OK;
			
			if ( ucN & ucIrqEn & 0x01 )					//是否发生定时器中断
				cStatus = MI_NOTAGERR;   
				
			if ( ucCommand == PCD_TRANSCEIVE )
			{
				ucN = ReadRawRC_2 ( FIFOLevelReg );			//读FIFO中保存的字节数
				
				ucLastBits = ReadRawRC_2 ( ControlReg ) & 0x07;	//最后接收到的字节的有效位数
				
				if ( ucLastBits )
					* pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N个字节数减去1(最后一个字节)+最后一位的位数 读取到的数据总位数
				else
					* pOutLenBit = ucN * 8;   					//最后接收到的字节整个字节有效
				
				if ( ucN == 0 )	
                    ucN = 1;    
				
				if ( ucN > MAXRLEN )
					ucN = MAXRLEN;   
				
				for ( ul = 0; ul < ucN; ul ++ )
				  pOutData [ ul ] = ReadRawRC_2 ( FIFODataReg );   
			}		
        }
			else
				cStatus = MI_ERR;   
    }
   
   SetBitMask_2 ( ControlReg, 0x80 );           // stop timer now
   WriteRawRC_2 ( CommandReg, PCD_IDLE ); 
	
   return cStatus;
}


/* 函数名:PcdRequest
 * 描述  :寻卡
 * 输入  :ucReq_code,寻卡方式
 *                     = 0x52,寻感应区内所有符合14443A标准的卡
 *                     = 0x26,寻未进入休眠状态的卡
 *         pTagType,卡片类型代码
 *                   = 0x4400,Mifare_UltraLight
 *                   = 0x0400,Mifare_One(S50)
 *                   = 0x0200,Mifare_One(S70)
 *                   = 0x0800,Mifare_Pro(X))
 *                   = 0x4403,Mifare_DESFire
 * 返回  : 状态值
 *         = MI_OK,成功
 * ?调用  :外部调用          */
char PcdRequest_2 ( u8 ucReq_code, u8 * pTagType )
{
    char cStatus;  
    u8 ucComMF522Buf [ MAXRLEN ]; 
    u32 ulLen;

    ClearBitMask_2 ( Status2Reg, 0x08 );	//清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
    WriteRawRC_2 ( BitFramingReg, 0x07 );	//发送的最后一个字节的七位
    SetBitMask_2 ( TxControlReg, 0x03 );	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

    ucComMF522Buf [ 0 ] = ucReq_code;		//存入 卡片命令字

    cStatus = PcdComMF522_2 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//?寻卡  

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//寻卡成功返回卡的类型
    {    
       * pTagType = ucComMF522Buf [ 0 ];
       * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
    }
     
    else
     cStatus = MI_ERR;

    return cStatus;
}

/* 函数名:PcdAnticoll
 * 描述  :防冲撞
 * 输入  :pSnr,卡片序列号,4字节
 * 返回  :状态值
 *         = MI_OK,成功
 * 调用  :外部调用           */
char PcdAnticoll_2 ( u8 * pSnr )
{
    char cStatus;
    u8 uc, ucSnr_check = 0;
    u8 ucComMF522Buf [ MAXRLEN ]; 
	u32 ulLen;

    ClearBitMask_2 ( Status2Reg, 0x08 );		//清MFCryptol On位 只有成功执行MFAuthent命令后,该位才能置位
    WriteRawRC_2 ( BitFramingReg, 0x00);		//清理寄存器 停止收发
    ClearBitMask_2 ( CollReg, 0x80 );			//清ValuesAfterColl所有接收的位在冲突后被清除
   
    ucComMF522Buf [ 0 ] = 0x93;	//?卡片防冲突命令
    ucComMF522Buf [ 1 ] = 0x20;
   
    cStatus = PcdComMF522_2 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//与卡片通信
	
    if ( cStatus == MI_OK)		//通信成功
    {
		for ( uc = 0; uc < 4; uc ++ )
        {
            * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//读出UID
            ucSnr_check ^= ucComMF522Buf [ uc ];
        }
			
        if ( ucSnr_check != ucComMF522Buf [ uc ] )
        		cStatus = MI_ERR;    
				 
    }
    
    SetBitMask_2 ( CollReg, 0x80 );

    return cStatus;
}

/* 函数名:PcdSelect
 * 描述  :选定卡片
 * 输入  :pSnr,卡片序列号,4字节
 * 返回  :状态值
 *         = MI_OK,成功
 * 调用  :外部调用         */
char PcdSelect_2 ( u8 * pSnr )
{
    char ucN;
    u8 uc;
    u8 ucComMF522Buf [ MAXRLEN ]; 
    u32  ulLen;

    ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
    ucComMF522Buf [ 1 ] = 0x70;
    ucComMF522Buf [ 6 ] = 0;
    
    for ( uc = 0; uc < 4; uc ++ )
    {
        ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
        ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
    }
        
    CalulateCRC_2 ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );
    ClearBitMask_2 ( Status2Reg, 0x08 );
    ucN = PcdComMF522_2 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );
    
    if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
      ucN = MI_OK;  
    else
      ucN = MI_ERR;    

    return ucN; 
}

/* 函数名:CalulateCRC
 * 描述  :用RC522计算CRC16
 * 输入  :pIndata,计算CRC16的数组
 *         ucLen,计算CRC16的数组字节长度
 *         pOutData,存放计算结果存放的首地址
 * 返回  :无
 * 调用  :内部调用              */
void CalulateCRC_2 ( u8 * pIndata, u8 ucLen, u8 * pOutData )
{
    u8 uc, ucN;

    ClearBitMask_2(DivIrqReg,0x04);
    WriteRawRC_2(CommandReg,PCD_IDLE);
    SetBitMask_2(FIFOLevelReg,0x80);
    
    for ( uc = 0; uc < ucLen; uc ++)
        WriteRawRC_2 ( FIFODataReg, * ( pIndata + uc ) );   

    WriteRawRC_2 ( CommandReg, PCD_CALCCRC );
    uc = 0xFF;

    do {
        ucN = ReadRawRC_2 ( DivIrqReg );
        uc --;} 
    while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );
        
    pOutData [ 0 ] = ReadRawRC_2 ( CRCResultRegL );
    pOutData [ 1 ] = ReadRawRC_2 ( CRCResultRegM );
    
}

/* 函数名:PcdAuthState
 * 描述  :验证卡片密码
 * 输入  :ucAuth_mode,密码验证模式
 *                     = 0x60,验证A密钥
 *                     = 0x61,验证B密钥
 *         u8 ucAddr,块地址
 *         pKey,密码
 *         pSnr,卡片序列号,4字节
 * 返回  : 状态值
 *         = MI_OK,成功
 * 调用  :外部调用          */
char PcdAuthState_2 ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = ucAuth_mode;
    ucComMF522Buf [ 1 ] = ucAddr;
    
    for ( uc = 0; uc < 6; uc ++ )
        ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );   
    
    for ( uc = 0; uc < 6; uc ++ )
        ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );   

    cStatus = PcdComMF522_2 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );
    
    if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC_2 ( Status2Reg ) & 0x08 ) ) ){
            cStatus = MI_ERR; 
    }
		
    return cStatus;    
}

/* 函数名:PcdWrite
 * 描述  :写数据到M1卡一块
 * 输入  :u8 ucAddr,块地址
 *         pData,写入的数据,16字节
 * 返回  :状态值
 *         = MI_OK,成功
 * 调用  :外部调用           */
char PcdWrite_2 ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_WRITE;
    ucComMF522Buf [ 1 ] = ucAddr;
    
    CalulateCRC_2 ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 
    cStatus = PcdComMF522_2 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;   
        
    if ( cStatus == MI_OK )
    {
      memcpy(ucComMF522Buf, pData, 16);
      for ( uc = 0; uc < 16; uc ++ )
              ucComMF522Buf [ uc ] = * ( pData + uc );  
            
      CalulateCRC_2 ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

      cStatus = PcdComMF522_2 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
            
            if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;   
            
    } 

    return cStatus;
    
}

/* 函数名:PcdRead
 * 描述  :读取M1卡一块数据
 * 输入  :u8 ucAddr,块地址
 *         pData,读出的数据,16字节
 * 返回  :状态值
 *         = MI_OK,成功
 * 调用  :外部调用           */
char PcdRead_2 ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ]; 
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_READ;
    ucComMF522Buf [ 1 ] = ucAddr;
    
    CalulateCRC_2 ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
   
    cStatus = PcdComMF522_2 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );
    
    if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
    {
            for ( uc = 0; uc < 16; uc ++ )
        * ( pData + uc ) = ucComMF522Buf [ uc ];   
    }
        
    else
      cStatus = MI_ERR;   
    
    return cStatus;

}

/* 函数名:PcdHalt
 * 描述  :命令卡片进入休眠状态
 * 输入  :无
 * 返回  :状态值
 *         = MI_OK,成功
 * 调用  :外部调用        */
char PcdHalt_2( void )
{
    u8 ucComMF522Buf [ MAXRLEN ]; 
    u32  ulLen;

    ucComMF522Buf [ 0 ] = PICC_HALT;
    ucComMF522Buf [ 1 ] = 0;

    CalulateCRC_2 ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
    PcdComMF522_2 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    return MI_OK;   
}

