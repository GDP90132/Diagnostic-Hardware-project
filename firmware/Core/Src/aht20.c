/*
 * aht20.c
 *
 *  Created on: Nov 21, 2026
 *      Author: gill
 */


#include "aht20.h"

static uint8_t crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;                       // starting value from the datasheet
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];                       // mix in the next byte
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x31);   // top bit set: shift and XOR
            else            crc = (uint8_t)(crc << 1);            // top bit clear: just shift
        }
    }
    return crc;
}

HAL_StatusTypeDef AHT20_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t status = 0;
    HAL_Delay(100);                        // wait after power-up
    HAL_StatusTypeDef st = HAL_I2C_Master_Receive(hi2c, AHT20_ADDR, &status, 1, 100);
    if (st != HAL_OK) return st;
    if ((status & 0x18) != 0x18) return HAL_ERROR;   // not calibrated
    HAL_Delay(10);
    return HAL_OK;
}

HAL_StatusTypeDef AHT20_Read(I2C_HandleTypeDef *hi2c, int32_t *temp_x100, int32_t *hum_x100)
{
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    uint8_t d[7];
    HAL_StatusTypeDef st;

    st = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDR, cmd, 3, 100);
    if (st != HAL_OK) return st;

    HAL_Delay(80);                         // wait for the measurement

    st = HAL_I2C_Master_Receive(hi2c, AHT20_ADDR, d, 7, 100);
    if (st != HAL_OK) return st;

    if (d[0] & 0x80) return HAL_BUSY;      // still measuring
    if (crc8(d, 6) != d[6]) return HAL_ERROR;   // data corrupted

    uint32_t raw_h = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
    uint32_t raw_t = ((uint32_t)(d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];

    *hum_x100  = (int32_t)(((uint64_t)raw_h * 10000) >> 20);          // 0.01 %RH
    *temp_x100 = (int32_t)(((uint64_t)raw_t * 20000) >> 20) - 5000;   // 0.01 degC
    return HAL_OK;
}

