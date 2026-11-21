/*
 * aht20.h
 *
 *  Created on: Nov 21, 2026
 *      Author: gill
 */

#ifndef INC_AHT20_H_
#define INC_AHT20_H_

#include "main.h"

#define AHT20_ADDR (0x38 << 1)

HAL_StatusTypeDef AHT20_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef AHT20_Read(I2C_HandleTypeDef *hi2c, int32_t *temp_x100, int32_t *hum_x100);

#endif
