#ifndef __ICM20948_DRV_H__
#define __ICM20948_DRV_H__

#include "main.h"

// SPI2 Chip Select (CS) Control for PB12
#define ICM20948_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define ICM20948_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

// Register Bank Select
#define REG_BANK_SEL        0x7F

// Bank 0 Registers
#define REG_WHO_AM_I        0x00
#define REG_PWR_MGMT_1      0x06
#define REG_PWR_MGMT_2      0x07
#define REG_GYRO_ZOUT_H     0x37
#define REG_GYRO_ZOUT_L     0x38

// Bank 2 Registers
#define REG_GYRO_SMPLRT_DIV 0x00
#define REG_GYRO_CONFIG_1   0x01

// Expected WHO_AM_I device ID
#define ICM20948_DEVICE_ID  0xEA

// Function prototypes
uint8_t ICM20948_Init(void);
void ICM20948_Select_Bank(uint8_t bank);
void ICM20948_Write_Reg(uint8_t reg, uint8_t val);
uint8_t ICM20948_Read_Reg(uint8_t reg);
void ICM20948_Read_Regs(uint8_t reg, uint8_t *buf, uint8_t len);

int16_t ICM20948_Read_GyroZ_Raw(void);
float ICM20948_Get_GyroZ_dps(void);
float ICM20948_Get_GyroZ_rads(void);

#endif /* __ICM20948_DRV_H__ */
