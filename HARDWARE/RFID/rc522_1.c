#include "sys.h"
#include "rc522.h"
#include "rc522_1.h"
#include <string.h>
#include "delay.h"

#define   RC522_DELAY()  delay_us( 2 ) 

/*????*/

extern unsigned char RFID_NUM;		//�Ķ�����ţ�1/2


/* ������:RC522_1_Init / RC522_2_Init 
 * ����  :��ʼ��RC522_1 / RC522_2����
 * ����  :��
 * ����  :��
 * ����  :�ⲿ����             */
void RC522_1_Init ( void )
{
	RFID_NUM=1;
	SPI1_Init(); 				  				//��ʼ��SPI1
	RC522_1_Reset_Disable();	    //��RST�ø�,�����ڲ���λ�׶�;
	PcdReset_1 ();                  //��λRC522 
	PcdAntennaOff_1();              //�ر�����
	RC522_DELAY();                //delay 2us
	PcdAntennaOn_1();               //��������
	M500PcdConfigISOType_1 ( 'A' ); //���ù���ģʽ
	PcdAntennaOff_1();
}


/* ������:SPI1_Init / SPI2_Init
 * ����  :��ʼ��SPI1 / SPI2����
 * ����  :��
 * ����  :��
 * ����  :�ⲿ����             */
void SPI1_Init (void)	
{
	SPI_InitTypeDef  SPI_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );//PORTAʱ��ʹ��
	
	// CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);					 //��ʼ��PA4
	
    // SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // RST
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	//�ø�CS��
	RC522_1_CS_Disable();

    //����SPI1����
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;       //ȫ˫��;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                                //����ģʽ;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                            //��������Ϊ8bit;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                                   //ʱ�Ӽ���CPOLΪ����ʱ�͵�ƽ;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                                 //ʱ�Ӳ�����Ϊʱ��������(������);
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                                    //NSS����������ı�;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;          //Ԥ��Ƶϵ��64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                           //MSB����ģʽ;
    SPI_InitStructure.SPI_CRCPolynomial = 7;                                     //CRCУ��;
		
	//��ʼ��SPI1
    SPI_Init(SPI1 , &SPI_InitStructure);
		
	//ʹ��SPI1
	SPI_Cmd(SPI1 , ENABLE); 
 }


/* ������:PcdRese
 * ����  :��λRC522_1��RC522_2
 * ����  :��
 * ����  :��
 * ����  :�ⲿ����             */
void PcdReset_1 ( void )
{
	RC522_1_Reset_Disable();
	delay_us ( 1 );
	RC522_1_Reset_Enable();
	delay_us ( 1 );
	RC522_1_Reset_Disable();
	delay_us ( 1 );
	
    WriteRawRC_1 ( CommandReg, 0x0f );

    while ( ReadRawRC_1 ( CommandReg ) & 0x10 );
	
    delay_us ( 1 );
    WriteRawRC_1 ( ModeReg, 0x3D );                //���巢�ͺͽ��ճ���ģʽ ��Mifare��ͨ��,CRC��ʼֵ0x6363
    WriteRawRC_1 ( TReloadRegL, 30 );              //16λ��ʱ����λ   
    WriteRawRC_1 ( TReloadRegH, 0 );			     //16λ��ʱ����λ   
    WriteRawRC_1 ( TModeReg, 0x8D );				 //�����ڲ���ʱ��������
    WriteRawRC_1 ( TPrescalerReg, 0x3E );			 //���嶨ʱ����Ƶϵ��
    WriteRawRC_1 ( TxAutoReg, 0x40 );				 //?���Ʒ����ź�Ϊ100%ASK		
}

/* ������:SPI_RC522_SendByte
 * ����  :��RC522_1��RC522_2����1 Byte ����
 * ����  :byte,Ҫ���͵�����
 * ����  :RC522���ص�����
 * ����  :�ڲ�����    */
u8 SPI1_RC522_SendByte ( u8 byte )
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	
	SPI_I2S_SendData(SPI1, byte);  
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); 
	return 	SPI_I2S_ReceiveData(SPI1);		
}


/* ������:ReadRawRC
 * ����  :��RC522�Ĵ���
 * ����  :ucAddress,�Ĵ�����ַ
 * ����  :�Ĵ����ĵ�ǰֵ
 * ����  :�ڲ�����      */
u8 ReadRawRC_1 ( u8 ucAddress )
{
	u8 ucAddr, ucReturn;
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;      

	RC522_1_CS_Enable();	

	SPI1_RC522_SendByte ( ucAddr );
	ucReturn = SPI1_RC522_ReadByte ();
	
	RC522_1_CS_Disable();

	return ucReturn;
}


/* ������:WriteRawRC
 * ����  :дRC522�Ĵ���
 * ����  :ucAddress,�Ĵ�����ַ��ucValue,д��Ĵ�����ֵ
 * ����  :��
 * ����  :�ڲ�����      */
void WriteRawRC_1 ( u8 ucAddress, u8 ucValue )
{  
	u8 ucAddr;
	ucAddr = ( ucAddress << 1 ) & 0x7E;   
	
	RC522_1_CS_Enable();
	
	SPI1_RC522_SendByte ( ucAddr );
	SPI1_RC522_SendByte ( ucValue );

	RC522_1_CS_Disable();

}

/* ������:M500PcdConfigISOType
 * ����  :����RC522�Ĺ���ģʽ
 * ����  :ucType��������ʽ
 * ����  :��
 * ����  :�ⲿ����      */
void M500PcdConfigISOType_1 ( u8 ucType )
{
	if ( ucType == 'A')                     //ISO14443_A
	{
		ClearBitMask_1 ( Status2Reg, 0x08 );		

		WriteRawRC_1 ( ModeReg, 0x3D );//3F	
		WriteRawRC_1 ( RxSelReg, 0x86 );//84
		WriteRawRC_1 ( RFCfgReg, 0x7F );   //4F
		WriteRawRC_1 ( TReloadRegL, 30 );//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
		WriteRawRC_1 ( TReloadRegH, 0 );
		WriteRawRC_1 ( TModeReg, 0x8D );
		WriteRawRC_1 ( TPrescalerReg, 0x3E );
		delay_us   ( 2 );
		
		PcdAntennaOn_1 ();//������
	}
}

/*
 * ������:SetBitMask
 * ����  :��RC522�Ĵ�����λ
 * ����  :ucReg,�Ĵ�����ַ
 *        ucMask,��λֵ
 * ����  :��
 * ����  :�ڲ�����
 */
void SetBitMask_1 ( u8 ucReg, u8 ucMask )  
{
    u8 ucTemp;

    ucTemp = ReadRawRC_1 ( ucReg );
    WriteRawRC_1 ( ucReg, ucTemp | ucMask );         // set bit mask
}

/* ������:ClearBitMask
 * ����  :��RC522�Ĵ�����λ
 * ����  :ucReg,�Ĵ�����ַ
 *         ucMask,��λֵ
 * ����  :��
 * ����  :�ڲ�����           */
void ClearBitMask_1 ( u8 ucReg, u8 ucMask )  
{
    u8 ucTemp;
    ucTemp = ReadRawRC_1 ( ucReg );
	
    WriteRawRC_1 ( ucReg, ucTemp & ( ~ ucMask) );  // clear bit mask
	
}

/* ������:PcdAntennaOn
 * ����  :�������� 
 * ����  :��
 * ����  :��
 * ����  :�ڲ�����            */
void PcdAntennaOn_1 ( void )
{
    u8 uc;
    uc = ReadRawRC_1 ( TxControlReg );
	
    if ( ! ( uc & 0x03 ) )
			SetBitMask_1(TxControlReg, 0x03);

}

/* ������:PcdAntennaOff
 * ����  :�ر����� 
 * ����  :��
 * ����  :��
 * ����  :�ڲ�����            */
void PcdAntennaOff_1 ( void )
{
    ClearBitMask_1 ( TxControlReg, 0x03 );
}

/* ������:PcdComMF522
 * ����  :ͨ��RC522��ISO14443��ͨѶ
 * ����  :ucCommand,RC522������
 *         pInData,ͨ��RC522���͵���Ƭ������
 *         ucInLenByte,�������ݵ��ֽڳ���
 *         pOutData,���յ��Ŀ�Ƭ��������
 *         pOutLenBit,�������ݵ�λ����
 * ����  : ״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ڲ�����              */
char PcdComMF522_1 ( u8 ucCommand, u8 * pInData, u8 ucInLenByte, u8 * pOutData, u32 * pOutLenBit )		
{
    char cStatus = MI_ERR;
    u8 ucIrqEn   = 0x00;
    u8 ucWaitFor = 0x00;
    u8 ucLastBits;
    u8 ucN;
    u32 ul;

    switch ( ucCommand )
    {
       case PCD_AUTHENT:		//Mifare��֤
          ucIrqEn   = 0x12;		//��������ж�����ErrIEn  ��������ж�IdleIEn
          ucWaitFor = 0x10;		//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
          break;
			 
       case PCD_TRANSCEIVE:		//���շ��� ���ͽ���
          ucIrqEn   = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
          ucWaitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ������жϱ�־λ
          break;
			 
       default:
         break;
			 
    }
   
    WriteRawRC_1 ( ComIEnReg, ucIrqEn | 0x80 );		//IRqInv��λ�ܽ�IRQ��Status1Reg��IRqλ��ֵ�෴ 
    ClearBitMask_1 ( ComIrqReg, 0x80 );			//Set1��λ����ʱ,CommIRqReg������λ����
    WriteRawRC_1 ( CommandReg, PCD_IDLE );		//д��������
    SetBitMask_1 ( FIFOLevelReg, 0x80 );			//��λFlushBuffer����ڲ�FIFO�Ķ���дָ���Լ�ErrReg��BufferOvfl?��־λ�����
    
    for ( ul = 0; ul < ucInLenByte; ul ++ )
		WriteRawRC_1 ( FIFODataReg, pInData [ ul ] );    		//д���ݽ�FIFOdata
			
    WriteRawRC_1 ( CommandReg, ucCommand );					//д����
   
    
    if ( ucCommand == PCD_TRANSCEIVE )
			SetBitMask_1(BitFramingReg,0x80);  				//StartSend��λ�������ݷ��� ��λ���շ�����ʹ��ʱ����Ч
    
    ul = 1000;//����ʱ��Ƶ�ʵ���,����M1�����ȴ�ʱ��25ms���ɵ���
		
    do 														//��֤��Ѱ���ȴ�ʱ��	
    {
         ucN = ReadRawRC_1 ( ComIrqReg );							//��ѯ�¼��ж�
         ul --;
    } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );		//�˳�����i=0,��ʱ���ж�,��д��������
		
    ClearBitMask_1 ( BitFramingReg, 0x80 );					//��������StartSendλ
		
    if ( ul != 0 )
    {
		if ( ! (( ReadRawRC_1 ( ErrorReg ) & 0x1B )) )			//�������־�Ĵ���BufferOfI CollErr ParityErr ProtocolErr
		{
			cStatus = MI_OK;
			
			if ( ucN & ucIrqEn & 0x01 )					//�Ƿ�����ʱ���ж�
				cStatus = MI_NOTAGERR;   
				
			if ( ucCommand == PCD_TRANSCEIVE )
			{
				ucN = ReadRawRC_1 ( FIFOLevelReg );			//��FIFO�б�����ֽ���
				
				ucLastBits = ReadRawRC_1 ( ControlReg ) & 0x07;	//�����յ����ֽڵ���Чλ��
				
				if ( ucLastBits )
					* pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	//N���ֽ�����ȥ1(���һ���ֽ�)+���һλ��λ�� ��ȡ����������λ��
				else
					* pOutLenBit = ucN * 8;   					//�����յ����ֽ������ֽ���Ч
				
				if ( ucN == 0 )	
                    ucN = 1;    
				
				if ( ucN > MAXRLEN )
					ucN = MAXRLEN;   
				
				for ( ul = 0; ul < ucN; ul ++ )
				  pOutData [ ul ] = ReadRawRC_1 ( FIFODataReg );   
			}		
        }
			else
				cStatus = MI_ERR;   
    }
   
   SetBitMask_1 ( ControlReg, 0x80 );           // stop timer now
   WriteRawRC_1 ( CommandReg, PCD_IDLE ); 
	
   return cStatus;
}


/* ������:PcdRequest
 * ����  :Ѱ��
 * ����  :ucReq_code,Ѱ����ʽ
 *                     = 0x52,Ѱ��Ӧ�������з���14443A��׼�Ŀ�
 *                     = 0x26,Ѱδ��������״̬�Ŀ�
 *         pTagType,��Ƭ���ʹ���
 *                   = 0x4400,Mifare_UltraLight
 *                   = 0x0400,Mifare_One(S50)
 *                   = 0x0200,Mifare_One(S70)
 *                   = 0x0800,Mifare_Pro(X))
 *                   = 0x4403,Mifare_DESFire
 * ����  : ״ֵ̬
 *         = MI_OK,�ɹ�
 * ?����  :�ⲿ����          */
char PcdRequest_1 ( u8 ucReq_code, u8 * pTagType )
{
    char cStatus;  
    u8 ucComMF522Buf [ MAXRLEN ]; 
    u32 ulLen;

    ClearBitMask_1 ( Status2Reg, 0x08 );	//����ָʾMIFARECyptol��Ԫ��ͨ�Լ����п�������ͨ�ű����ܵ����
    WriteRawRC_1 ( BitFramingReg, 0x07 );	//���͵����һ���ֽڵ���λ
    SetBitMask_1 ( TxControlReg, 0x03 );	//TX1,TX2�ܽŵ�����źŴ��ݾ����͵��Ƶ�13.56�������ز��ź�

    ucComMF522Buf [ 0 ] = ucReq_code;		//���� ��Ƭ������

    cStatus = PcdComMF522_1 ( PCD_TRANSCEIVE,	ucComMF522Buf, 1, ucComMF522Buf, & ulLen );	//?Ѱ��  

    if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//Ѱ���ɹ����ؿ�������
    {    
       * pTagType = ucComMF522Buf [ 0 ];
       * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
    }
     
    else
     cStatus = MI_ERR;

    return cStatus;
}

/* ������:PcdAnticoll
 * ����  :����ײ
 * ����  :pSnr,��Ƭ���к�,4�ֽ�
 * ����  :״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ⲿ����           */
char PcdAnticoll_1 ( u8 * pSnr )
{
    char cStatus;
    u8 uc, ucSnr_check = 0;
    u8 ucComMF522Buf [ MAXRLEN ]; 
	u32 ulLen;

    ClearBitMask_1 ( Status2Reg, 0x08 );		//��MFCryptol Onλ ֻ�гɹ�ִ��MFAuthent�����,��λ������λ
    WriteRawRC_1 ( BitFramingReg, 0x00);		//����Ĵ��� ֹͣ�շ�
    ClearBitMask_1 ( CollReg, 0x80 );			//��ValuesAfterColl���н��յ�λ�ڳ�ͻ�����
   
    ucComMF522Buf [ 0 ] = 0x93;	//?��Ƭ����ͻ����
    ucComMF522Buf [ 1 ] = 0x20;
   
    cStatus = PcdComMF522_1 ( PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, & ulLen);//�뿨Ƭͨ��
	
    if ( cStatus == MI_OK)		//ͨ�ųɹ�
    {
		for ( uc = 0; uc < 4; uc ++ )
        {
            * ( pSnr + uc )  = ucComMF522Buf [ uc ];			//����UID
            ucSnr_check ^= ucComMF522Buf [ uc ];
        }
			
        if ( ucSnr_check != ucComMF522Buf [ uc ] )
        		cStatus = MI_ERR;    
				 
    }
    
    SetBitMask_1 ( CollReg, 0x80 );

    return cStatus;
}

/* ������:PcdSelect
 * ����  :ѡ����Ƭ
 * ����  :pSnr,��Ƭ���к�,4�ֽ�
 * ����  :״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ⲿ����         */
char PcdSelect_1 ( u8 * pSnr )
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
        
    CalulateCRC_1 ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );
    ClearBitMask_1 ( Status2Reg, 0x08 );
    ucN = PcdComMF522_1 ( PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, & ulLen );
    
    if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
      ucN = MI_OK;  
    else
      ucN = MI_ERR;    

    return ucN; 
}

/* ������:CalulateCRC
 * ����  :��RC522����CRC16
 * ����  :pIndata,����CRC16������
 *         ucLen,����CRC16�������ֽڳ���
 *         pOutData,��ż�������ŵ��׵�ַ
 * ����  :��
 * ����  :�ڲ�����              */
void CalulateCRC_1 ( u8 * pIndata, u8 ucLen, u8 * pOutData )
{
    u8 uc, ucN;

    ClearBitMask_1(DivIrqReg,0x04);
    WriteRawRC_1(CommandReg,PCD_IDLE);
    SetBitMask_1(FIFOLevelReg,0x80);
    
    for ( uc = 0; uc < ucLen; uc ++)
        WriteRawRC_1 ( FIFODataReg, * ( pIndata + uc ) );   

    WriteRawRC_1 ( CommandReg, PCD_CALCCRC );
    uc = 0xFF;

    do {
        ucN = ReadRawRC_1 ( DivIrqReg );
        uc --;} 
    while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );
        
    pOutData [ 0 ] = ReadRawRC_1 ( CRCResultRegL );
    pOutData [ 1 ] = ReadRawRC_1 ( CRCResultRegM );
    
}

/* ������:PcdAuthState
 * ����  :��֤��Ƭ����
 * ����  :ucAuth_mode,������֤ģʽ
 *                     = 0x60,��֤A��Կ
 *                     = 0x61,��֤B��Կ
 *         u8 ucAddr,���ַ
 *         pKey,����
 *         pSnr,��Ƭ���к�,4�ֽ�
 * ����  : ״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ⲿ����          */
char PcdAuthState_1 ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr )
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

    cStatus = PcdComMF522_1 ( PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, & ulLen );
    
    if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC_1 ( Status2Reg ) & 0x08 ) ) ){
            cStatus = MI_ERR; 
    }
		
    return cStatus;    
}

/* ������:PcdWrite
 * ����  :д���ݵ�M1��һ��
 * ����  :u8 ucAddr,���ַ
 *         pData,д�������,16�ֽ�
 * ����  :״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ⲿ����           */
char PcdWrite_1 ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ];
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_WRITE;
    ucComMF522Buf [ 1 ] = ucAddr;
    
    CalulateCRC_1 ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 
    cStatus = PcdComMF522_1 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;   
        
    if ( cStatus == MI_OK )
    {
      memcpy(ucComMF522Buf, pData, 16);
      for ( uc = 0; uc < 16; uc ++ )
              ucComMF522Buf [ uc ] = * ( pData + uc );  
            
      CalulateCRC_1 ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

      cStatus = PcdComMF522_1 ( PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, & ulLen );
            
            if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
        cStatus = MI_ERR;   
            
    } 

    return cStatus;
    
}

/* ������:PcdRead
 * ����  :��ȡM1��һ������
 * ����  :u8 ucAddr,���ַ
 *         pData,����������,16�ֽ�
 * ����  :״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ⲿ����           */
char PcdRead_1 ( u8 ucAddr, u8 * pData )
{
    char cStatus;
    u8 uc, ucComMF522Buf [ MAXRLEN ]; 
    u32 ulLen;

    ucComMF522Buf [ 0 ] = PICC_READ;
    ucComMF522Buf [ 1 ] = ucAddr;
    
    CalulateCRC_1 ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
   
    cStatus = PcdComMF522_1 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );
    
    if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
    {
            for ( uc = 0; uc < 16; uc ++ )
        * ( pData + uc ) = ucComMF522Buf [ uc ];   
    }
        
    else
      cStatus = MI_ERR;   
    
    return cStatus;

}

/* ������:PcdHalt
 * ����  :���Ƭ��������״̬
 * ����  :��
 * ����  :״ֵ̬
 *         = MI_OK,�ɹ�
 * ����  :�ⲿ����        */
char PcdHalt_1( void )
{
    u8 ucComMF522Buf [ MAXRLEN ]; 
    u32  ulLen;

    ucComMF522Buf [ 0 ] = PICC_HALT;
    ucComMF522Buf [ 1 ] = 0;

    CalulateCRC_1 ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
    PcdComMF522_1 ( PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, & ulLen );

    return MI_OK;   
}

