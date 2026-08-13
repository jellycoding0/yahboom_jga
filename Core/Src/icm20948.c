#include "icm20948.h"
#include "spi.h"
#include <stdio.h>

// External SPI handle from CubeMX
extern SPI_HandleTypeDef hspi2;

// Gyroscope bias offset calculated during calibration
static float gyro_z_bias = 0.0f;

// Basic single-byte SPI read/write helper
static uint8_t SPI2_ReadWriteByte(uint8_t tx_byte)
{
    uint8_t rx_byte = 0;
    HAL_SPI_TransmitReceive(&hspi2, &tx_byte, &rx_byte, 1, 5); // 5ms timeout
    return rx_byte;
}

// Select register bank (0 to 3)
void ICM20948_Select_Bank(uint8_t bank)
{
    ICM20948_CS_LOW();
    SPI2_ReadWriteByte(REG_BANK_SEL);
    SPI2_ReadWriteByte(bank << 4);
    ICM20948_CS_HIGH();
}

// Write to a register in the currently selected bank
void ICM20948_Write_Reg(uint8_t reg, uint8_t val)
{
    ICM20948_CS_LOW();
    SPI2_ReadWriteByte(reg & 0x7F); // Write has MSB = 0
    SPI2_ReadWriteByte(val);
    ICM20948_CS_HIGH();
}

// Read from a register in the currently selected bank
uint8_t ICM20948_Read_Reg(uint8_t reg)
{
    uint8_t val = 0;
    ICM20948_CS_LOW();
    SPI2_ReadWriteByte(reg | 0x80); // Read has MSB = 1
    val = SPI2_ReadWriteByte(0xFF);
    ICM20948_CS_HIGH();
    return val;
}

// Read multiple registers sequentially
void ICM20948_Read_Regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    ICM20948_CS_LOW();
    SPI2_ReadWriteByte(reg | 0x80); // Read has MSB = 1
    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = SPI2_ReadWriteByte(0xFF);
    }
    ICM20948_CS_HIGH();
}

// Initialize the ICM20948 IMU
uint8_t ICM20948_Init(void)
{
    uint8_t who_am_i = 0;
    
    // Ensure Chip Select is deselected initially
    ICM20948_CS_HIGH();
    HAL_Delay(10);
    
    // 1. Select Bank 0 and perform Software Reset to clear sleep/undefined states
    ICM20948_Select_Bank(0);
    ICM20948_Write_Reg(REG_PWR_MGMT_1, 0x80); // DEVICE_RESET = 1
    HAL_Delay(50); // Wait for reset to complete
    
    // 2. Wake up device, disable sleep mode, select Auto Clock Source
    ICM20948_Write_Reg(REG_PWR_MGMT_1, 0x01);
    HAL_Delay(20);
    
    // 3. Check WHO_AM_I register (Expected 0xEA)
    who_am_i = ICM20948_Read_Reg(REG_WHO_AM_I);
    printf("IMU WHO_AM_I: 0x%02X (Expected: 0xEA)\r\n", who_am_i);
    
    if (who_am_i != ICM20948_DEVICE_ID)
    {
        return 0; // Failure
    }
    
    // 4. Enable Accelerometer and Gyroscope (set PWR_MGMT_2 to 0x00)
    ICM20948_Write_Reg(REG_PWR_MGMT_2, 0x00);
    HAL_Delay(20);
    
    // 5. Configure Gyroscope in Bank 2
    ICM20948_Select_Bank(2);
    // Set Sample Rate Divider: GYRO_SMPLRT_DIV = 0 (1kHz update rate)
    ICM20948_Write_Reg(REG_GYRO_SMPLRT_DIV, 0x00);
    // Set Gyro Configuration: Full scale ±2000 dps, Gyro LPF Enabled
    // FS Select = 3 (bits 2:1 = 11 for ±2000 dps), LPF Enabled = 1 (bit 0 = 1)
    // Value: (3 << 1) | 1 = 0x07
    ICM20948_Write_Reg(REG_GYRO_CONFIG_1, 0x07);
    
    // 6. Select Bank 0 back
    ICM20948_Select_Bank(0);
    
    // Allow Gyro sensor internal PLL and MEMS structure to fully stabilize (Startup time: ~100ms)
    HAL_Delay(200);
    
    // 7. Perform Gyroscope Calibration (Compute zero-rate offset bias)
    int32_t bias_sum = 0;
    uint16_t sample_count = 200;
    
    for (uint16_t i = 0; i < sample_count; i++)
    {
        bias_sum += ICM20948_Read_GyroZ_Raw();
        HAL_Delay(5); // 5ms delay per sample for better noise averaging
    }
    gyro_z_bias = (float)bias_sum / (float)sample_count;
    
    return 1; // Success
}

// Read raw signed 16-bit Z-axis gyroscope value
int16_t ICM20948_Read_GyroZ_Raw(void)
{
    uint8_t buf[2];
    ICM20948_Read_Regs(REG_GYRO_ZOUT_H, buf, 2);
    return (int16_t)((buf[0] << 8) | buf[1]);
}

// Read calibrated Z-axis Gyro value in Degrees Per Second (dps)
float ICM20948_Get_GyroZ_dps(void)
{
    int16_t raw_z = ICM20948_Read_GyroZ_Raw();
    // Gyro Sensitivity for ±2000 dps range is 16.4 LSB/dps
    return ((float)raw_z - gyro_z_bias) / 16.4f;
}

// Read calibrated Z-axis Gyro value in Radians Per Second (rad/s)
float ICM20948_Get_GyroZ_rads(void)
{
    // Convert dps to rad/s (1 degree = pi/180 radians)
    return ICM20948_Get_GyroZ_dps() * 0.0174532925f;
}
