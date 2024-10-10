/*
 * BME280.h
 *
 *  Created on: Oct 9, 2024
 *      Author: Bohdan
 */

#ifndef BME280_DRIVER_H
#define BME280_DRIVER_H

#include "stm32f4xx_hal.h"

/*
 * DEFINES
 */

#define BME280_I2C_ADDR (0x77 << 1) // Alternative address is 0x78
#define BME280_CHIP_ID 	0x60

#define BME280_PRESSURE_OFFSET 7735
/*
 * MEMORY MAP (p.27)
 */

#define BME280_ID			0xD0
#define BME280_RESET		0xE0	// If the value 0xB6 is written to the register, the device is reset using the complete power-on-reset procedure.
#define BME280_CTRL_HUM		0xF2	// The “ctrl_hum” register sets the humidity data acquisition options of the device. (Table 20, p.28).
#define BME280_STATUS		0xF3	// Contains two bits which indicate the status of the device.
#define BME280_CTRL_MEAS	0xF4	// Sets the pressure and temperature data acquisition options of the device.
#define BME280_CONFIG		0xF5	// The “config” register sets the rate, filter and interface options of the device.
#define BME280_PRESS_MSB	0xF7	// Contains the MSB part up[19:12] of the raw pressure measurement output data.
#define BME280_PRESS_LSB	0xF8	// Contains the LSB part up[11:4] of the raw pressure measurement output data.
#define BME280_PRESS_XLSB	0xF9	// Contains the XLSB part up[3:0] of the raw pressure measurement output data. Contents depend on temperature resolution.
#define BME280_TEMP_MSB		0xFA	// Contains the MSB part ut[19:12] of the raw temperature measurement output data.
#define BME280_TEMP_LSB		0xFB	// Contains the LSB part ut[11:4] of the raw temperature measurement output data.
#define BME280_TEMP_XLSB	0xFC	// Contains the XLSB part ut[3:0] of the raw temperature measurement output data. Contents depend on pressure resolution.
#define BME280_HUM_MSB		0xFD	// Contains the MSB part uh[15:8] of the raw humidity measurement output data.
#define BME280_HUM_LSB		0xFE	// Contains the LSB part uh[7:0] of the raw humidity measurement output data.

/*
 * OVERSAMPLING AND SENSOR MODES
 */

#define BME280_OS_1X		0b001
#define BME280_OS_2X		0b010
#define BME280_OS_4X		0b011
#define BME280_OS_8X		0b100
#define BME280_OS_16X		0b101

#define BME280_MODESLEEP	0b00
#define BME280_MODEFORCED	0b10
#define BME280_MODENORMAL	0b11

/*
 * COMPENSATION VALUES STRUCT
 */

typedef struct
{
	// Temperature
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;

	// Pressure
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;

	// Humidity
	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t dig_H6;

}BME280_CompValues;

/*
 * SENSOR STRUCT
 */

typedef struct{
	// I2C Handle
	I2C_HandleTypeDef *i2cHandle;

	// Compensation Values
	BME280_CompValues comp_values;

	uint32_t t_fine;

	// Sensor Readings
	float temp_c;
	float pressure_Pa;
	float humidity;

}BME280;


/*
 * INITIALIZATION
 */

uint8_t BME280_Initialize(BME280 *device, I2C_HandleTypeDef *i2cHandle);
HAL_StatusTypeDef BME280_GetCompensationValues(BME280 *device, I2C_HandleTypeDef *i2cHandle);

/*
 * SENSOR DATA
 */

HAL_StatusTypeDef BME280_ReadPressure(BME280 *device);
HAL_StatusTypeDef BME280_ReadTemperature(BME280 *device);
HAL_StatusTypeDef BME280_ReadHumidity(BME280 *device);

/*
 * LOW-LEVEL FUNCTIONS
 */

HAL_StatusTypeDef BME280_ReadRegister(BME280 *device, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef BME280_ReadRegisters(BME280 *device, uint8_t reg, uint8_t *data, uint8_t length);
HAL_StatusTypeDef BME280_WriteRegister(BME280 *device, uint8_t reg, uint8_t *data);

#endif /* BME280_DRIVER_H */
