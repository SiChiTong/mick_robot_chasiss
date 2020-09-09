/******************************************************************************************************
  *
  * ��������W25Q16�洢��ʹ����غ���
  *       
  * ��ע��      SPIx (SPI1 SPI2 SPI3) spiģ���
  *             pBuffer               �����׵�ַ
  *             WriteAddr ��ReadAddr   24λ��ŵ�ַ
  *             NumByteToWrite         д�����ݵĸ���  
  * 2014-11-2
		
  W25Q16_FLASH_Init(SPI_TypeDef* SPIx);//�洢����ʼ������
	W25Q16_FLASH_BufferRead(SPI_TypeDef* SPIx,u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);//���ݶ�����
  W25Q16_FLASH_BufferWrite(SPI_TypeDef* SPIx,u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);//����д����
	
	u32 W25Q16_FLASH_ReadID(SPI_TypeDef* SPIx);//���洢��ID����	  ( W25Q16 ID��== 0xEF4015 )
	W25Q16_FLASH_ChipErase(SPI_TypeDef* SPIx);//�洢����Ƭ����
	
	W25Q16_Flash_WAKEUP(SPI_TypeDef* SPIx);//�ӵ���ģʽ�»���
  W25Q16_Flash_PowerDown(SPI_TypeDef* SPIx);//����ģʽ
	
	
	ע�� // W25X16: data input on the DIO pin is sampled on the rising edge of the CLK. (W25X16 ��ʱ�������ز����������ϵ�����)
       // Data on the DO and DIO pins are clocked out on the falling edge of CLK.(W25X16 �½����������)
			 
   ����SPI�����ù���ģʽʱ���й�   ��IO��ģ��SPiʱ���ʱ��ͬ��Ҫע���������
	
*******************************************************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h" 
#include "stm32f10x_spi.h" 


#include "W25Q16.h"



/*************************************************************************
*  �������ƣ�W25Q16_FLASH_SendByte
*  ����˵����W25Q16 ͨ��SPIд��һ���ֽ�  
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             
*  �������أ����� SPI�����ϵĻش�����
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP  ����д��Ҫ�Ƿ��������ֲ
*************************************************************************/
u8 W25Q16_FLASH_SendByte(SPI_TypeDef* SPIx,u8 byte)
{
   return My_STM32_SP_FLASH_SendByte(SPIx,byte);
}
/*************************************************************************
*  �������ƣ�W25Q16_FLASH_Init(SPI_TypeDef* SPIx)
*  ����˵����W25Q16 �洢����ʼ������
*              
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP
*************************************************************************/ 
void W25Q16_FLASH_Init(SPI_TypeDef* SPIx)
{
	   GPIO_InitTypeDef GPIO_InitStructure;
	/***************************************************/
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//��ʼ�� W25Q16 Ƭѡ����
		 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		 GPIO_Init(GPIOA, &GPIO_InitStructure);
		/***************************************************/
     My_STM32_SPI_Init(SPI1);//SPIģ���ʼ��

}
 
 
/*************************************************************************
*  �������ƣ�W25Q16_FLASH_WaitForWriteEnd
*  ����˵���� Polls the status of the Write In Progress (WIP) flag in the
*             FLASH's status  register  and  loop  until write  opertaion
*             has completed.
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP
*************************************************************************/
void W25Q16_FLASH_WaitForWriteEnd(SPI_TypeDef* SPIx)
{
  u8 FLASH_Status = 0;

  
  W25Q16_FLASH_CS_LOW();/* Select the FLASH: Chip Select low */
  W25Q16_FLASH_SendByte(SPIx, W25X_ReadStatusReg);/* Send "Read Status Register" instruction */
	
  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = W25Q16_FLASH_SendByte(SPIx, Dummy_Byte);	 
  }
  while ((FLASH_Status & Busy_Flag) == 1);  

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();
}



 /*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector. (4K-bytes)
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None ����һ����СΪ4KB�����ݿ� 
* W25Q16 ��֧�� 32KB 64KB ��Ĳ�д   ����ֱ��� 52h  D8h
*******************************************************************************/
void W25Q16_FLASH_SectorErase(SPI_TypeDef* SPIx,u32 SectorAddr)
{
  /* Send write enable instruction */
  W25Q16_FLASH_WriteEnable(SPIx);
  W25Q16_FLASH_WaitForWriteEnd(SPIx);
  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();
  /* Send Sector Erase instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_SectorErase);
  /* Send SectorAddr high nibble address byte */
  W25Q16_FLASH_SendByte(SPIx,(SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  W25Q16_FLASH_SendByte(SPIx,(SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  W25Q16_FLASH_SendByte(SPIx,SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();
  /* Wait the end of Flash writing */
  W25Q16_FLASH_WaitForWriteEnd(SPIx);
}

/*************************************************************************
*  �������ƣ�SPI_FLASH_ChipErase
*  ����˵����W25Q16 �������ݲ���
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP
*************************************************************************/
void W25Q16_FLASH_ChipErase(SPI_TypeDef* SPIx)
{
  /* Send write enable instruction */
  W25Q16_FLASH_WriteEnable(SPIx);

  /* Chip Erase */
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();
  /* Send Bulk Erase instruction  */
  W25Q16_FLASH_SendByte(SPIx,W25X_ChipErase);
  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  W25Q16_FLASH_WaitForWriteEnd(SPIx);
}

 
/*************************************************************************
*  �������ƣ�W25Q16_FLASH_PageWrite
*  ����˵����W25Q16 ҳ��дָ��
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             pBuffer �����׵�ַ
*             WriteAddr 24λ��ŵ�ַ
*             NumByteToWrite  д�����ݵĸ���  ��1 ~ 256�� 
*
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP  �ڲ�����
*************************************************************************/
void W25Q16_FLASH_PageWrite(SPI_TypeDef* SPIx, u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* Enable the write access to the FLASH */
 W25Q16_FLASH_WriteEnable(SPIx);
 
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();
  /* Send "Write to Memory " instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_PageProgram);
  /* Send WriteAddr high nibble address byte to write to */
  W25Q16_FLASH_SendByte(SPIx,(WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  W25Q16_FLASH_SendByte(SPIx,(WriteAddr & 0xFF00) >> 8);
  /* Send WriteAddr low nibble address byte to write to */
  W25Q16_FLASH_SendByte(SPIx,(WriteAddr & 0xFF));

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
     //printf("\n\r Err: SPI_FLASH_PageWrite too large!");
  }

  /* while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /* Send the current byte */
    W25Q16_FLASH_SendByte(SPIx,*pBuffer);
    /* Point on the next byte to be written */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  W25Q16_FLASH_WaitForWriteEnd(SPIx);
}

/*************************************************************************
*  �������ƣ�W25Q16_FLASH_BufferWrite(SPI_TypeDef* SPIx,u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
*  ����˵����W25Q16 д���ݺ���
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             pBuffer �����׵�ַ
*             WriteAddr 24λ�����ʼ��ַ 
*             NumByteToWrite  д�����ݵĸ���   
*
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP
*************************************************************************/
void W25Q16_FLASH_BufferWrite(SPI_TypeDef* SPIx,u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
		u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

		Addr = WriteAddr % SPI_FLASH_PageSize;
		count = SPI_FLASH_PageSize - Addr;
		NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
		NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

		if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
		{
				if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
				{
						W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, NumByteToWrite);
				}
				else /* NumByteToWrite > SPI_FLASH_PageSize */
				{
						while (NumOfPage--)
						{
							W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, SPI_FLASH_PageSize);
							WriteAddr +=  SPI_FLASH_PageSize;
							pBuffer += SPI_FLASH_PageSize;
						}
						if(NumOfSingle != 0)
						 W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, NumOfSingle);
				}
		}
		else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
		{
					if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
					{
							if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
							{
									temp = NumOfSingle - count;

									W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, count);
									WriteAddr +=  count;
									pBuffer += count;

									W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, temp);
							}
							else
							{
									W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, NumByteToWrite);
							}
				 }
				else /* NumByteToWrite > SPI_FLASH_PageSize */
				{
						NumByteToWrite -= count;
						NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
						NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

						W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, count);
						WriteAddr +=  count;
						pBuffer += count;

						while (NumOfPage--)
						{
							W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, SPI_FLASH_PageSize);
							WriteAddr +=  SPI_FLASH_PageSize;
							pBuffer += SPI_FLASH_PageSize;
						}

						if (NumOfSingle != 0)
						{
							W25Q16_FLASH_PageWrite(SPIx,pBuffer, WriteAddr, NumOfSingle);
						}
				}
		}
}
/*************************************************************************
*  �������ƣ� W25Q16_FLASH_BufferRead(SPI_TypeDef* SPIx,u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
*  ����˵���� W25Q16 �����ݺ���
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*             pBuffer �����׵�ַ
*             ReadAddr 24λ�����ʼ��ַ 
*             NumByteToRead  ��Ҫ�������ٸ�����
*
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP     W25Q16ÿ�ζ�ȡһ�����ݺ��ڲ���ַ������һ
*************************************************************************/
void W25Q16_FLASH_BufferRead(SPI_TypeDef* SPIx,u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_ReadData);
   
  /* Send ReadAddr high nibble address byte to read from */
  W25Q16_FLASH_SendByte(SPIx,(ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  W25Q16_FLASH_SendByte(SPIx,(ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  W25Q16_FLASH_SendByte(SPIx,ReadAddr & 0xFF);

  while (NumByteToRead--) /* while there is data to be read */
  {
			/* Read a byte from the FLASH */
			*pBuffer = W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);
			/* Point to the next location where the byte read will be saved */
			pBuffer++;  
  }

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();
}
/*************************************************************************
*  �������ƣ� W25Q16_FLASH_ReadID(SPI_TypeDef* SPIx)
*  ����˵���� W25Q16 ��ID ����
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*              
*  �������أ�����һ��24λID��
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP      
*************************************************************************/
u32 W25Q16_FLASH_ReadID(SPI_TypeDef* SPIx)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_JedecDeviceID);

  /* Read a byte from the FLASH */
  Temp0 = W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp1 = W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp2 = W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}
/*************************************************************************
*  �������ƣ� W25Q16_FLASH_ReadID(SPI_TypeDef* SPIx)
*  ����˵���� W25Q16 ��ID ����
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*              
*  �������أ�����һ��8λID�� 
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP  When used only to obtain the Device ID while not in the power-down state   
*  ��W25Q16��ǰû�д��ڵ͹��ġ�������ģʽ�µ�ʱ��ž��ж�ID�Ĺ���
*************************************************************************/
u32 W25Q16_FLASH_ReadDeviceID(SPI_TypeDef* SPIx)
{
  u32 Temp = 0;

  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_DeviceID);
  W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);
  W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);
  W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);
  
  /* Read a byte from the FLASH */
  Temp = W25Q16_FLASH_SendByte(SPIx,Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();

  return Temp;
}
 
/*************************************************************************
*  �������ƣ� W25Q16_FLASH_WriteEnable(SPI_TypeDef* SPIx)
*  ����˵���� W25Q16 дʹ�ܺ���
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*              
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP      
*************************************************************************/
void W25Q16_FLASH_WriteEnable(SPI_TypeDef* SPIx)
{
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();

  /* Send "Write Enable" instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_WriteEnable);

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();
}



/*************************************************************************
*  �������ƣ� W25Q16_Flash_PowerDown(SPI_TypeDef* SPIx)
*  ����˵���� W25Q16 �������ģʽ
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*              
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP   
*************************************************************************/
void W25Q16_Flash_PowerDown(SPI_TypeDef* SPIx)   
{ 
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_PowerDown);

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();
}   
/*************************************************************************
*  �������ƣ� SPI_Flash_PowerDown(SPI_TypeDef* SPIx)
*  ����˵���� W25Q16 �ӵ���ģʽ�� ����
*  ����˵���� SPIx (SPI1 SPI2 SPI3)
*              
*  �������أ���
*  �޸�ʱ�䣺2014-11-2
*  ��    ע��CRP   W25Q16�洢�����ѵ�ǰ���� W25Q16���봦�ڵ���ģʽ       
*************************************************************************/
 
void W25Q16_Flash_WAKEUP(SPI_TypeDef* SPIx)   
{
  /* Select the FLASH: Chip Select low */
  W25Q16_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  W25Q16_FLASH_SendByte(SPIx,W25X_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  W25Q16_FLASH_CS_HIGH();                   //�ȴ�TRES1
}   
   
/*********************************************END OF FILE**********************/