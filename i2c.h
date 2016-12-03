#ifndef I2C_H_
#define I2C_H_

#include "stm32f10x.h"

#define  ClockSpeed            400000

#define I2C_DIRECTION_TX 0
#define I2C_DIRECTION_RX 1

#define OwnAddress1 0x28
#define OwnAddress2 0x30

#define I2C1_DR_Address              0x40005410

Status I2C_Write(I2C_TypeDef* I2Cx ,const uint8_t* buf ,uint32_t nbyte , uint8_t reg_address, uint8_t SlaveAddress);
Status I2C_Read(I2C_TypeDef* I2Cx ,uint8_t *buf ,uint32_t nbyte , uint8_t reg_address,uint8_t SlaveAddress);
void I2C_LowLevel_Init(I2C_TypeDef* I2Cx ,int ClockSpeed , int OwnAddress);


#endif //I2C_H_