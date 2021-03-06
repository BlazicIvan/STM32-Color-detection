#include "i2c.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x.h"



#define Timed(x) while (x)

void I2C1_Start()
{
	Timed(I2C_GetFlagStatus(I2C1 , I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1 , ENABLE);
	Timed (! I2C_CheckEvent(I2C1 , I2C_EVENT_MASTER_MODE_SELECT));
	//errReturn: return;
}

void I2C1_Write_Address(uint8_t address, uint8_t direction)
{
	I2C_Send7bitAddress(I2C1 , address<<1 , direction);
        if(direction == I2C_Direction_Transmitter)
          Timed (! I2C_CheckEvent(I2C1 ,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	else
          Timed (! I2C_GetFlagStatus(I2C1 ,I2C_FLAG_ADDR));
        //errReturn: return;
}

void I2C1_Write_Data(uint8_t data)
{
	I2C_SendData(I2C1 , data);
	Timed (! I2C_GetFlagStatus(I2C1 , I2C_FLAG_BTF));
	//errReturn: return;
}

void I2C1_Stop()
{
	I2C_GenerateSTOP(I2C1 , ENABLE);
	Timed(I2C_GetFlagStatus(I2C1 , I2C_FLAG_STOPF));
	//errReturn: return;
}

void I2C1_Restart()
{
	//I2C1_Start();
        //Timed(I2C_GetFlagStatus(I2C1 , I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1 , ENABLE);
	Timed (! I2C_CheckEvent(I2C1 , I2C_EVENT_MASTER_MODE_SELECT));
}

uint8_t I2C1_Read()
{
	/*
		Nema parametar NACK
	*/
	I2C_AcknowledgeConfig(I2C1 , DISABLE);
	
	/*
	// EV6_1 -- must be atomic -- Clear ADDR , generate STOP
	__disable_irq ();*/
	(void) I2C1 ->SR2;
	/*I2C_GenerateSTOP(I2Cx ,ENABLE);
	__enable_irq ();
	*/
	
	// Receive data EV7
	Timed (! I2C_GetFlagStatus(I2C1 , I2C_FLAG_RXNE));
	return I2C_ReceiveData(I2C1);
	
	//errReturn: return 0;
}
/*
int I2C_Write(I2C_TypeDef* I2Cx , uint8_t* buf ,uint32_t nbyte, uint8_t SlaveAddress) 
{
	I2Cx = I2C1;
	__IO uint32_t Timeout = 0;
	
	if (nbyte)
	{
		Timed(I2C_GetFlagStatus(I2Cx , I2C_FLAG_BUSY));
		
		// Intiate Start Sequence
		I2C_GenerateSTART(I2Cx , ENABLE);
		
		Timed (! I2C_CheckEvent(I2Cx , I2C_EVENT_MASTER_MODE_SELECT));
		
		// Send Address EV5
		I2C_Send7bitAddress(I2Cx , SlaveAddress ,I2C_Direction_Transmitter);
		Timed (! I2C_CheckEvent(I2Cx ,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
		
		
		// EV6 Write first byte EV8_1
		I2C_SendData(I2Cx , *buf ++);
		while (--nbyte) 
		{
			// wait on BTF
			Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_BTF));
			I2C_SendData(I2Cx , *buf ++);
		}
		
		Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_BTF));
		I2C_GenerateSTOP(I2Cx , ENABLE);
		Timed(I2C_GetFlagStatus(I2C1 , I2C_FLAG_STOPF));
	}
	return 0;
	
	errReturn: return -1;
}

int I2C_Read(I2C_TypeDef* I2Cx ,uint8_t *buf ,uint32_t nbyte,uint8_t SlaveAddress) 
{
	__IO uint32_t Timeout = 0;

	if (! nbyte)
	return 0;

	// Wait for idle I2C interface
	Timed(I2C_GetFlagStatus(I2Cx , I2C_FLAG_BUSY));
	
	// Enable Acknowledgment , clear POS flag
	I2C_AcknowledgeConfig(I2Cx , ENABLE);
	I2C_PECPositionConfig(I2Cx , I2C_NACKPosition_Current);
	
	// Intiate Start Sequence (wait for EV5)
	I2C_GenerateSTART(I2Cx , ENABLE);
	Timed (! I2C_CheckEvent(I2Cx , I2C_EVENT_MASTER_MODE_SELECT));
	
	// Send Address
	I2C_Send7bitAddress(I2Cx , SlaveAddress , I2C_Direction_Receiver);
	
	// EV6
	Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_ADDR));
	
	if (nbyte == 1) 
	{
		// Clear Ack bit
		I2C_AcknowledgeConfig(I2Cx , DISABLE);
		
		// EV6_1 -- must be atomic -- Clear ADDR , generate STOP
		__disable_irq ();
		(void) I2Cx ->SR2;
		I2C_GenerateSTOP(I2Cx ,ENABLE);
		__enable_irq ();
		
		// Receive data EV7
		Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_RXNE));
		*buf++ = I2C_ReceiveData(I2Cx);
	}
	else if (nbyte == 2) 
	{
		// Set POS flag
		I2C_PECPositionConfig(I2Cx , I2C_NACKPosition_Next);
		
		// EV6_1 -- must be atomic and in this order
		__disable_irq ();
		(void) I2Cx ->SR2; // Clear ADDR flag
		I2C_AcknowledgeConfig(I2Cx , DISABLE); // Clear Ack bit
		__enable_irq ();
		
		// EV7_3 -- Wait for BTF , program stop , read data twice
		Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_BTF));
		__disable_irq ();
		I2C_GenerateSTOP(I2Cx ,ENABLE);
		*buf++ = I2Cx ->DR;
		__enable_irq ();
		*buf++ = I2Cx ->DR;
	}
	else 
	{
		(void) I2Cx ->SR2; // Clear ADDR flag
		
		while (nbyte -- != 3)
		{
			// EV7 -- cannot guarantee 1 transfer completion time ,
			// wait for BTF instead of RXNE
			Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_BTF));
			*buf++ = I2C_ReceiveData(I2Cx);
		}
		
		Timed (! I2C_GetFlagStatus(I2Cx , I2C_FLAG_BTF));
		
		// EV7_2 -- Figure 1 has an error , doesn 't read N-2 !
		I2C_AcknowledgeConfig(I2Cx , DISABLE); // clear ack bit
		__disable_irq ();
		*buf++ = I2C_ReceiveData(I2Cx); // receive byte N-2
		I2C_GenerateSTOP(I2Cx ,ENABLE); // program stop
		__enable_irq ();
		*buf++ = I2C_ReceiveData(I2Cx); // receive byte N-1
		
		// wait for byte N
		Timed (! I2C_CheckEvent(I2Cx , I2C_EVENT_MASTER_BYTE_RECEIVED));
		*buf++ = I2C_ReceiveData(I2Cx);
		nbyte = 0;
	}
	
	// Wait for stop
	Timed(I2C_GetFlagStatus(I2Cx , I2C_FLAG_STOPF));
	
	return 0;
	
	errReturn: return -1;
}
*/

void I2C1_LowLevel_Init(int clockSpeed , int OwnAddress)
{
	I2C_TypeDef* I2Cx = I2C1;
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	
	// Enable GPIOB clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	
	// Configure I2C clock and GPIO
	GPIO_StructInit (& GPIO_InitStructure);

	/* I2C1 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 , ENABLE);
	
	/* I2C1 SDA and SCL configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB , &GPIO_InitStructure);
	/* I2C1 Reset */
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1 , ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1 , DISABLE);
	
	/* Configure I2Cx */
	I2C_StructInit (& I2C_InitStructure);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = OwnAddress;
	/* Iskljucen ack */
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = clockSpeed;
	I2C_Init(I2Cx , &I2C_InitStructure);
	I2C_Cmd(I2Cx , ENABLE);
}