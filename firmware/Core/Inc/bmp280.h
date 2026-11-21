/*
 * bmp280.h
 *
 *  Created on: Nov 20, 2026
 *      Author: gill
 */
#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

#define BMP280_ADDR      (0x77 << 1)
#define BMP280_REG_ID    0xD0
#define BMP280_CHIP_ID   0x58
#define BMP280_REG_CALIB 0x88
#define BMP280_CALIB_LEN 24

HAL_StatusTypeDef BMP280_Init(I2C_HandleTypeDef *hi2c);

#endif

