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

typedef struct {
    uint16_t dig_T1;   // temperature correction numbers
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;   // pressure correction numbers
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BMP280_Calib;

const BMP280_Calib *BMP280_GetCalib(void);

HAL_StatusTypeDef BMP280_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BMP280_ReadRaw(I2C_HandleTypeDef *hi2c, int32_t *adc_T, int32_t *adc_P);
int32_t BMP280_CompensateTemp(int32_t adc_T);
uint32_t BMP280_CompensatePress(int32_t adc_P);
#endif

