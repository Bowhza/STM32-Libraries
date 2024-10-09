/*
 * ADXL343.c
 *
 *  Created on: Oct 7, 2024
 *      Author: Bohdan
 */

#include "ADXL343.h"

/*
 * INITIALIZATION
 */

uint8_t ADXL343_Initialize(ADXL343 *device, I2C_HandleTypeDef *i2cHandle)
{
	// Set the device i2c handle
	device->i2cHandle = i2cHandle;

	// Reset the data stored
	device->acc_data[0] = 0.0f;
	device->acc_data[1] = 0.0f;
	device->acc_data[2] = 0.0f;

	uint8_t errorCount = 0;
	uint8_t registerData;
	HAL_StatusTypeDef status;

	/*
	 * Check Device ID
	 */
	status = ADXL343_ReadRegister(device, ADXL343_DEVID, &registerData);
	errorCount += (status != HAL_OK);

	// Return if device communication could not be established.
	if(registerData != ADXL343_DEVICE_ID)
	{
		return HAL_ERROR;
	}

	// Takes ADXL343 out of standby mode, 8Hz Sampling Rate
	registerData = 0x08;
	status = ADXL343_WriteRegister(device, ADXL343_POWER_CTL, &registerData);
	errorCount += (status != HAL_OK);

	return errorCount;
}

/*
 * SENSOR DATA
 */

HAL_StatusTypeDef ADXL343_ReadAcceleration(ADXL343 *device)
{
	uint8_t registerData[6];

	// Read the registers for X, Y, Z starting at DATAX0
	HAL_StatusTypeDef status = ADXL343_ReadRegisters(device, ADXL343_DATAX0, registerData, 6);

	int16_t acceleration_raw_signed[3];

	// Combine the readings for the full 16-bit value for X, Y, Z
	acceleration_raw_signed[0] = ((int16_t) registerData[1] << 8 | (int16_t) registerData[0]);
	acceleration_raw_signed[1] = ((int16_t) registerData[3] << 8 | (int16_t) registerData[2]);
	acceleration_raw_signed[2] = ((int16_t) registerData[5] << 8 | (int16_t) registerData[4]);

	// Calculate the X, Y and Z components (3.9mg per LSB)
	device->acc_data[0] = 0.0039f * acceleration_raw_signed[0];
	device->acc_data[1] = 0.0039f * acceleration_raw_signed[1];
	device->acc_data[2] = 0.0039f * acceleration_raw_signed[2];

	return status;
}

/*
 * LOW-LEVEL FUNCTIONS
 */

HAL_StatusTypeDef ADXL343_ReadRegister(ADXL343 *device, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(device->i2cHandle, ADXL343_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef ADXL343_ReadRegisters(ADXL343 *device, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(device->i2cHandle, ADXL343_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef ADXL343_WriteRegister(ADXL343 *device, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(device->i2cHandle, ADXL343_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}


