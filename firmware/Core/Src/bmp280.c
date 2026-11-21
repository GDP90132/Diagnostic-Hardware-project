/*
 * bmp280.c
 *
 *  Created on: Nov 20, 2026
 *      Author: gill
 */

#include "bmp280.h"

static uint8_t calib[BMP280_CALIB_LEN];

HAL_StatusTypeDef BMP280_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t id = 0;
    HAL_StatusTypeDef st;

    st = HAL_I2C_Mem_Read(hi2c, BMP280_ADDR, BMP280_REG_ID,
                          I2C_MEMADD_SIZE_8BIT, &id, 1, 100);
    if (st != HAL_OK) return st;
    if (id != BMP280_CHIP_ID) return HAL_ERROR;

    st = HAL_I2C_Mem_Read(hi2c, BMP280_ADDR, BMP280_REG_CALIB,
                          I2C_MEMADD_SIZE_8BIT, calib, BMP280_CALIB_LEN, 100);
    return st;
}

