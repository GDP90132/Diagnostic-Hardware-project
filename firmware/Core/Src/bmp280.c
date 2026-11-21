/*
 * bmp280.c
 *
 *  Created on: Nov 20, 2026
 *      Author: gill
 */

#include "bmp280.h"

static uint8_t calib[BMP280_CALIB_LEN];

static BMP280_Calib cal;   // the 12 decoded numbers, stored once

// join 2 bytes (low byte first) into one unsigned number.
static uint16_t u16_le(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
// same as above line but treat the result as signed.
static int16_t  s16_le(const uint8_t *p) { return (int16_t)u16_le(p); }

const BMP280_Calib *BMP280_GetCalib(void) { return &cal; }

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
    if (st != HAL_OK) return st;          // changed: stop here if the read failed

    cal.dig_T1 = u16_le(&calib[0]);       // new: decode the 12 numbers
    cal.dig_T2 = s16_le(&calib[2]);
    cal.dig_T3 = s16_le(&calib[4]);
    cal.dig_P1 = u16_le(&calib[6]);
    cal.dig_P2 = s16_le(&calib[8]);
    cal.dig_P3 = s16_le(&calib[10]);
    cal.dig_P4 = s16_le(&calib[12]);
    cal.dig_P5 = s16_le(&calib[14]);
    cal.dig_P6 = s16_le(&calib[16]);
    cal.dig_P7 = s16_le(&calib[18]);
    cal.dig_P8 = s16_le(&calib[20]);
    cal.dig_P9 = s16_le(&calib[22]);
    return HAL_OK;
}

HAL_StatusTypeDef BMP280_ReadRaw(I2C_HandleTypeDef *hi2c, int32_t *adc_T, int32_t *adc_P)
{
    uint8_t ctrl = 0x25;     // temp x1, pressure x1, forced mode
    uint8_t d[6];
    HAL_StatusTypeDef st;

    st = HAL_I2C_Mem_Write(hi2c, BMP280_ADDR, 0xF4, I2C_MEMADD_SIZE_8BIT, &ctrl, 1, 100);
    if (st != HAL_OK) return st;

    HAL_Delay(10);           // give the chip time to finish measuring

    st = HAL_I2C_Mem_Read(hi2c, BMP280_ADDR, 0xF7, I2C_MEMADD_SIZE_8BIT, d, 6, 100);
    if (st != HAL_OK) return st;

    *adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    *adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);
    return HAL_OK;
}

static int32_t t_fine;

int32_t BMP280_CompensateTemp(int32_t adc_T)
{
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1))) * ((int32_t)cal.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)cal.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)cal.dig_T1))) >> 12) *
            ((int32_t)cal.dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;   // temperature in 0.01 °C
}

uint32_t BMP280_CompensatePress(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)cal.dig_P6;
    var2 = var2 + ((var1 * (int64_t)cal.dig_P5) << 17);
    var2 = var2 + (((int64_t)cal.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)cal.dig_P3) >> 8) + ((var1 * (int64_t)cal.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)cal.dig_P1) >> 33;
    if (var1 == 0) return 0;          // avoid dividing by zero
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)cal.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)cal.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)cal.dig_P7) << 4);
    return (uint32_t)p;               // in Pa, scaled by 256
}
