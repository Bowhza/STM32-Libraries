/*
 * BME280.c
 *
 *  Created on: Oct 9, 2024
 *      Author: Bohdan
 */

#include "BME280.h"
#include <stdio.h>

/*
 * INITIALIZATION
 */

uint8_t BME280_Initialize(BME280 *device, I2C_HandleTypeDef *i2cHandle)
{
	// Set the device i2c handle
	device->i2cHandle = i2cHandle;

	// Reset the data stored
	device->pressure_Pa = 0.0f;
	device->temp_c = 0.0f;
	device->humidity = 0.0f;

	uint8_t errorCount = 0;
	uint8_t registerData;
	HAL_StatusTypeDef status;

	/*
	 * Check Device ID
	 */
	status = BME280_ReadRegister(device, BME280_ID, &registerData);
	errorCount += (status != HAL_OK);

	// Return if device communication could not be established.
	if(registerData != BME280_CHIP_ID)
	{
		return HAL_ERROR;
	}

	/*
	 * Configure the sensor
	 */

	// Run device reset procedure
	registerData = 0xB6;
	status = BME280_WriteRegister(device, BME280_RESET, &registerData);
	errorCount += (status != HAL_OK);

    // Wait for reset to complete
    HAL_Delay(100);

	// Configure 500ms Standby, 2 Filter Coefficient.
	registerData = 0x84;
	status = BME280_WriteRegister(device, BME280_CONFIG, &registerData);
	errorCount += (status != HAL_OK);

    // Read compensation values
    if (BME280_GetCompensationValues(device, i2cHandle) != HAL_OK)
    {
        return HAL_ERROR;
    }

	// Set oversampling for humidity
	registerData = BME280_OS_2X;
	status = BME280_WriteRegister(device, BME280_CTRL_HUM, &registerData);
	errorCount += (status != HAL_OK);

	// Put Device into Normal Mode, Set oversampling for temperature and pressure.
	registerData = (BME280_OS_1X << 5) | (BME280_OS_1X << 2) | BME280_MODENORMAL;
	status = BME280_WriteRegister(device, BME280_CTRL_MEAS, &registerData);
	errorCount += (status != HAL_OK);

	return errorCount;
}

/*
 * COMPENSATION VALUES
 */

HAL_StatusTypeDef BME280_GetCompensationValues(BME280 *device, I2C_HandleTypeDef *i2cHandle)
{
	uint8_t comp_vals[24];
	uint8_t hum_vals[9];

	// Compensation parameter register addresses can be found on page 24.
	if(BME280_ReadRegisters(device, 0x88, comp_vals, sizeof(comp_vals)) != HAL_OK)
	{
		return HAL_ERROR;
	}

	// Assign temperature and pressure compensation values
	device->comp_values.dig_T1 = (uint16_t)((comp_vals[1] << 8) | comp_vals[0]);
	device->comp_values.dig_T2 = (int16_t)((comp_vals[3] << 8) | comp_vals[2]);
	device->comp_values.dig_T3 = (int16_t)((comp_vals[5] << 8) | comp_vals[4]);
	device->comp_values.dig_P1 = (uint16_t)((comp_vals[7] << 8) | comp_vals[6]);
	device->comp_values.dig_P2 = (int16_t)((comp_vals[9] << 8) | comp_vals[8]);
	device->comp_values.dig_P3 = (int16_t)((comp_vals[11] << 8) | comp_vals[10]);
	device->comp_values.dig_P4 = (int16_t)((comp_vals[13] << 8) | comp_vals[12]);
	device->comp_values.dig_P5 = (int16_t)((comp_vals[15] << 8) | comp_vals[14]);
	device->comp_values.dig_P6 = (int16_t)((comp_vals[17] << 8) | comp_vals[16]);
	device->comp_values.dig_P7 = (int16_t)((comp_vals[19] << 8) | comp_vals[18]);
	device->comp_values.dig_P8 = (int16_t)((comp_vals[21] << 8) | comp_vals[20]);
	device->comp_values.dig_P9 = (int16_t)((comp_vals[23] << 8) | comp_vals[22]);

	if(BME280_ReadRegisters(device, 0xA1, hum_vals, sizeof(hum_vals)) != HAL_OK)
	{
		return HAL_ERROR;
	}

	// Assign humidity compensation values
	device->comp_values.dig_H1 = (uint8_t)(hum_vals[0]);
	device->comp_values.dig_H2 = (int16_t)(hum_vals[2] << 8) | hum_vals[1];
	device->comp_values.dig_H3 = (uint8_t)(hum_vals[3]);
	device->comp_values.dig_H4 = (int16_t)(hum_vals[5] << 8) | hum_vals[4];
	device->comp_values.dig_H5 = (int16_t)(hum_vals[7] << 8) | hum_vals[6];
	device->comp_values.dig_H6 = (int8_t)(hum_vals[8]);

	return HAL_OK;
}


/*
 * SENSOR DATA
 */


HAL_StatusTypeDef BME280_ReadTemperature(BME280 *device)
{
	uint8_t raw_temp_reading[3];

	HAL_StatusTypeDef status = BME280_ReadRegisters(device, BME280_TEMP_MSB, raw_temp_reading, 3);

	uint32_t adc_T = (raw_temp_reading[0] << 12) | (raw_temp_reading[1] << 4) | (raw_temp_reading[2] >> 4);

	//Calculation on Page 25.
	int32_t var1, var2, T;
	var1 = (((adc_T >> 3) - (device->comp_values.dig_T1 << 1)) * (device->comp_values.dig_T2)) >> 11;
	var2 = ((((adc_T >> 4) - (device->comp_values.dig_T1)) * ((adc_T >> 4) - device->comp_values.dig_T2)) >> 12) * (device->comp_values.dig_T3) >> 14;
	device->t_fine = var1 + var2;
	T = (device->t_fine * 5 + 128) >> 8;
	// Divide by 100.0 to get temp in C (Resolution is 0.01 DegC)
	device->temp_c = T / 100.0f;

	return status;
}

HAL_StatusTypeDef BME280_ReadPressure(BME280 *device)
{
	uint8_t raw_pressure_reading[3];

	HAL_StatusTypeDef status = BME280_ReadRegisters(device, BME280_PRESS_MSB, raw_pressure_reading, 3);

	uint32_t adc_P = (raw_pressure_reading[0] << 12) | (raw_pressure_reading[1] << 4) | (raw_pressure_reading[2] >> 4);

	//Calculation on Page 25.
    int64_t var1, var2, p;

    var1 = ((int64_t)device->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)device->comp_values.dig_P6;
    var2 = var2 + ((var1 * (int64_t)device->comp_values.dig_P5) << 17);
    var2 = var2 + (((int64_t)device->comp_values.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)device->comp_values.dig_P3) >> 8) + ((var1 * (int64_t)device->comp_values.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)device->comp_values.dig_P1)) >> 33;

    if (var1 == 0) {
        return HAL_ERROR;
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)device->comp_values.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)device->comp_values.dig_P8) * p) >> 19;
    p = (((p + var1 + var2) >> 8) + (((int64_t)device->comp_values.dig_P7) << 4));

    // Divide p by 256 to get pressure in Pascal units
    device->pressure_Pa = (p / 256.0f);

	return status;
}

HAL_StatusTypeDef BME280_ReadHumidity(BME280 *device)
{
	uint8_t raw_humidity_reading[2];

	HAL_StatusTypeDef status = BME280_ReadRegisters(device, BME280_HUM_MSB, raw_humidity_reading, 2);

	uint16_t adc_H = (raw_humidity_reading[1] << 8) | raw_humidity_reading[0];

	// Calculation on page 25-26
	int32_t v_x1_u32r;

	v_x1_u32r = (device->t_fine - 76800);
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)device->comp_values.dig_H4) << 20) - (((int32_t)device->comp_values.dig_H5) * v_x1_u32r)) + ((int32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)device->comp_values.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)device->comp_values.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)device->comp_values.dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)device->comp_values.dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	// Divide value by 1024 to get % relative humidity.
	device->humidity = ((uint32_t)(v_x1_u32r >> 12)) / 1024.0f;

	return status;
}

/*
 * LOW-LEVEL FUNCTIONS
 */

HAL_StatusTypeDef BME280_ReadRegister(BME280 *device, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(device->i2cHandle, BME280_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BME280_ReadRegisters(BME280 *device, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(device->i2cHandle, BME280_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BME280_WriteRegister(BME280 *device, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(device->i2cHandle, BME280_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}


