#include "stm32f10x.h"
 
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h" 
 
 
 

#include "bsp_uart.h"
#include "NRF_24L01.h"




static unsigned char TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};	//���ص�ַ
static unsigned char RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0x01};	//���յ�ַ

unsigned char NRF24l01_TX_BUFF[TX_PLOAD_WIDTH]={0};	 // ��ʼ����������ջ�����
unsigned char NRF24l01_RX_BUFF[RX_PLOAD_WIDTH]={0};	

 
void NRF24l01_GPIO_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStruct;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOG,ENABLE);
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   
		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5   | GPIO_Pin_7 ;                                      
		GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15 |GPIO_Pin_8;                                       
		GPIO_Init(GPIOG, &GPIO_InitStruct);
 

		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6 ;                                      
		GPIO_Init(GPIOA, &GPIO_InitStruct);

}





/**************��ʱ����*******************************/
void NRF24L01_Delayus(unsigned int n)
{
	n=n*10;
	while(n-- >0 );
}
/****************************************************************************************
*NRF24L01��ʼ��
***************************************************************************************/
void init_NRF24L01(void)
{
	 UART_send_string(USART1,"NRF24L01��ʼ��ʼ��\n");//�ַ�������
	NRF24l01_GPIO_Init();
	NRF24L01_Delayus(5000);
 	CE_Out_L;    // chip enable  �����ݴ���
 	CSN_Out_H;  // Spi  disable   CSNΪ�ͺ�SPI�ӿڵȴ�ִ������ 
	SCK_Out_H;
  //IRQ=1;
	
	SPI_Write_Buf(NRF_24l01_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    // д���ص�ַ�����͵�ַ	
	SPI_Write_Buf(NRF_24l01_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // д���ն�0ͨ����ַ	
	SPI_RW_Reg(NRF_24l01_WRITE_REG + EN_AA, 0x00);      //  Ƶ��0��ACKӦ�� 	
	SPI_RW_Reg(NRF_24l01_WRITE_REG + EN_RXADDR, 0x01);  //  �������յ�ַֻ��Ƶ��0
	SPI_RW_Reg(NRF_24l01_WRITE_REG + SETUP_AW, 0x03);      //  ���ý��ա����͵ĵ�ַ����Ϊ5�ֽ�	
	//SPI_RW_Reg(NRF_24l01_WRITE_REG + SETUP_RETR, 0x00);      // ��ֹ�ط�
	SPI_RW_Reg(NRF_24l01_WRITE_REG + RF_CH,40);        //   �����ŵ����� Ƶ��
	SPI_RW_Reg(NRF_24l01_WRITE_REG + RF_SETUP, 0x0f);   		//���÷�������Ϊ2MHZ�����书��Ϊ���ֵ0dB	
	SPI_RW_Reg(NRF_24l01_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //���ý������ݳ���	
  SPI_RW_Reg(NRF_24l01_WRITE_REG + CONFIG, 0x0e);              // CRCʹ�ܣ�16λCRCУ�飬�ϵ磬����ģʽ ���������жϡ������ж�
	CE_Out_H;
	 UART_send_string(USART1,"NRF24L01��ʼ�����\n");//�ַ�������
}
/***********************************************************************
*������uint SPI_RW(uint uchar)
*���ܣ�NRF24L01��SPIдʱ��
******************************************************************/
unsigned char SPI_RW(unsigned char dat)
{
		unsigned char i;  
/**********Description:**************************************
  Writes one byte to nRF24L01, and return the byte read from nRF24L01 during write, according to SPI protocol
  ���������8λ�Ĵ���װ���Ǵ����͵�����10101010�������ط��͡��½��ؽ��ա���λ�ȷ��͡�
  ��ô��һ������������ʱ�� ���ݽ�����sdo=1���Ĵ����е�10101010����һλ�����油��������һλδ֪��x������0101010x��
  �½��ص�����ʱ��sdi�ϵĵ�ƽ�����浽�Ĵ�����ȥ����ô��ʱ�Ĵ���=0101010sdi��
  ������ 8��ʱ�������Ժ������Ĵ��������ݻ��ཻ��һ�Ρ������������һ��spiʱ��
	************************************************************/
   	for(i=0;i<8;i++) // output 8-bit data
   	{			
        if(dat & 0x80)
				   MOSI_Out_H;
			  else
					  MOSI_Out_L;
			  
				dat = (dat << 1);           // shift next bit into MSB..
				SCK_Out_H;                      // Set SCK high..
			//��Ƭ��Ϊ���� 24l01Ϊ�ӻ�    ����MISO�Ӵӻ�MISO	//CLK������Ϊ������������     clk�½���Ϊ�ӻ���������
				if(MISO)
				dat ++;       		  // capture current MISO bit ��MISO�ϲ�׽��ǰλ			
				SCK_Out_L;            		  // ..then set SCK low again	
		}
		return(dat);           		  // return read data
}
//**************************************************************************/
//*���ܣ�NRF24L01��д�Ĵ�������
//****************************************************************/
unsigned char SPI_RW_Reg(unsigned char reg, unsigned char value)
{
	unsigned char status;	
	CSN_Out_L;                   // CSN low, init SPI transaction
	status = SPI_RW(reg);      // select register
	SPI_RW(value);             // ..and write value to it..
	CSN_Out_H;                   // CSN high again	
	return(status);            // return nRF24L01 status uchar
}
//*************************************************************************
//*������ unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char uchars)
//*����: ����д���ݣ�Ϊ�Ĵ�����ַ��pBuf��Ϊ��д�����ݵ�ַ��uchars��д�����ݵĸ���
//************************************************************/
unsigned char SPI_Write_Buf(unsigned char reg, unsigned char *pBuf, unsigned char length)
{	
	unsigned char status,i;
	CSN_Out_L;            //CSN�õ� ��ʼ��������    
	status = SPI_RW(reg);  
 //inerDelay_us(10);	
	for(i=0; i<length; i++) //
		SPI_RW(*pBuf++);
	CSN_Out_H;           //�ر�SPI
	return(status);    // 
	
}
//******************************************************************************
//*������uchar SPI_Read(uchar reg)
//*���ܣ�NRF24L01��SPIʱ��
//********************************************************************/
unsigned char SPI_Read(unsigned char reg)
{
	unsigned char reg_val;	
	CSN_Out_L;                // CSN low, initialize SPI communication...
	SPI_RW(reg);            // Select register to read from..
	reg_val= SPI_RW(0);    // ..then read registervalue
	CSN_Out_H;                // CSN high, terminate SPI communication
	return(reg_val);        // return register value
}
//***************************************************************/
//*������uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
//*����: ���ڶ����ݣ�reg��Ϊ�Ĵ�����ַ��pBuf��Ϊ���������ݵ�ַ��uchars���������ݵĸ���
//********************************************************************/
unsigned char SPI_Read_Buf(unsigned char reg, unsigned char *pBuf, unsigned char uchars)
{
	unsigned char status,i;	
	CSN_Out_L;                    		// Set CSN low, init SPI tranaction
	status = SPI_RW(reg);       		// Select register to write to and read status uchar	
	for(i=0;i<uchars;i++)
			pBuf[i] = SPI_RW(0);    // 	
	CSN_Out_H;                           	
	return(status);                    // return nRF24L01 status uchar
}

/**************************************************
*����������Ϊ����ģʽ
*void SetRX_Mode(void)
*���ܣ����ݽ������� 
**************************************************/
void SwitchToRxMode(void)
{
//	unsigned char value;
	CE_Out_L;	
	SPI_RW_Reg(NRF_24l01_FLUSH_RX,0);//flush Rx
	//value=SPI_RW(STATUS);	 // read register STATUS's value
	SPI_RW_Reg(NRF_24l01_WRITE_REG+STATUS,0xff); // clear RX_DR or TX_DS or MAX_RT interrupt flag	
//	value=SPI_RW(CONFIG);	// read register CONFIG's value
	//value=value|0x01;//set bit 1
	SPI_RW_Reg(NRF_24l01_WRITE_REG + CONFIG, 0x0f); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
 
	CE_Out_H;	
	NRF24L01_Delayus(130);
} 

/**************************************************
 Function: SwitchToTxMode();
 Description:
	switch to Tx mode
**************************************************/
 
void SwitchToTxMode(void)
{
	// unsigned char value;
	  CE_Out_L;
	 SPI_RW_Reg(NRF_24l01_FLUSH_TX,0); //flush Tx
	 SPI_RW_Reg(NRF_24l01_WRITE_REG+STATUS,0xff); // clear RX_DR or TX_DS or MAX_RT interrupt flag
	// value=SPI_RW(CONFIG);	// read register CONFIG's value
	 //value=value&0xfe;     //reset bit 0
   SPI_RW_Reg(NRF_24l01_WRITE_REG + CONFIG, 0x0e); //switch to Tx mode	
   CE_Out_H;    //  read-in data
 	 NRF24L01_Delayus(130);
	
}
/************************************************************************
*������void Transmit_Tx_bufferdata(void)
*���ܣ����ݷ���
**************************************************************************/
void Transmit_Tx_bufferdata(unsigned char *pBuf) 
{
	   CE_Out_L;
	  //SPI_RW_Reg(WRITE_REG + CONFIG, 0x06);      // CRCʹ�ܣ�16λCRCУ�飬�ϵ� 	
  	SPI_Write_Buf(NRF_24l01_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // д�뷢�͵�ַ
  	//SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ	
  	SPI_Write_Buf(NRF_24l01_WR_TX_PLOAD, pBuf, TX_PLOAD_WIDTH);                  // д���ݰ���TX FIFO 	
	  CE_Out_H;;//������ź�
	  NRF24L01_Delayus(8300);
}

/************************************************************************
*������unsigned char NRF24l01_Interrupt_Handle(void)
*
*���ܣ��жϴ�������
*
*���ز���  ���ܵ��жϷ���1     �����жϷ���2   ���򷵻� 0
*
*��ע��CRP  20150506
*
**************************************************************************/

unsigned char NRF24l01_Interrupt_Handle(void)//���ӷ���ֵ��Ҫ�Ƿ����жϺ�������
{


   unsigned char sta=0,nrf_flag=0;
	 sta = SPI_Read(STATUS);	  // ��״̬�Ĵ���
	  if(sta&0x40)  //�����ж�
		{
			 SPI_Read_Buf(NRF_24l01_RD_RX_PLOAD,NRF24l01_RX_BUFF,RX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer	
	 	   //for(i=0; i < RX_PLOAD_WIDTH;i++)
				//{

				//	UART_send_char(USART1,NRF24l01_RX_BUFF[i]); //USART1~UART5   u32tempdat��0~4294967296
				//	UART_send_char(USART1,' ');

			// }
				//UART_send_char(USART1,'\n'); 
				//UART_send_char(USART1,'\n'); 
			 nrf_flag=0x01;
		}
		else if(sta&0x20)  //�����ж�
		{
			  UART_send_char(USART1,'F'); 
				UART_send_char(USART1,'\n'); 		 
       SPI_RW_Reg(NRF_24l01_FLUSH_TX,0xff); 			
			  nrf_flag=0x02;
		}
		//else if(sta & 0x10)  //����ط����
		//{
		//	led2=~led2;
		//	flag=3;	
		//}
		else ;
	 SPI_RW_Reg(NRF_24l01_WRITE_REG + STATUS, 0xff); //��������жϱ�־λ 
		
		return nrf_flag;
		
}