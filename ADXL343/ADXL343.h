/*
 * ADXL343.h
 *
 *  Created on: Oct 7, 2024
 *      Author: Bohdan
 */

#ifndef ADXL343_DRIVER_H
#define ADXL343_DRIVER_H

#include "stm32f4xx_hal.h" // Needed for I2C

/*
 * DEFINES
 */

#define ADXL343_I2C_ADDR  (0x53 << 1) /* Alternative I2C Address is 0x1D (p.16)*/
#define ADXL343_DEVICE_ID 0xE5

/*
 * MEMORY MAP (p.22)
 */

#define ADXL343_DEVID 			0x00	// Device ID Register
#define ADXL343_THRESH_TAP 		0x1D	// Tap Threshold
#define ADXL343_OFSX 			0x1E	// X-axis offset
#define ADXL343_OFSY			0x1F	// Y-axis offset
#define ADXL343_OFSZ			0x20	// Z-axis offset
#define ADXL343_DUR				0x21	// Tap Duration
#define ADXL343_LATENT			0x22	// Tap latency
#define ADXL343_WINDOW			0x23	// Tap Window
#define ADXL343_THRESH_ACT  	0x24	// Activity threshold
#define ADXL343_THRESH_INACT 	0x25	// Inactivity threshold
#define ADXL343_TIME_INACT		0x26	// Inactivity time
#define ADXL343_ACT_INACT_CTL	0x27	// Axis enable control for activity and inactivity detection
#define ADXL343_THRESH_FF		0x28	// Free-fall threshold
#define ADXL343_TIME_FF			0x29	// Free-fall time
#define ADXL343_TAP_AXES		0x2A	// Axis control for single tap/double tap
#define ADXL343_ACT_TAP_STATUS	0x2B	// Source of single tap/double tap
#define ADXL343_BW_RATE			0x2C	// Data rate and power mode control
#define ADXL343_POWER_CTL		0x2D	// Power-saving features control
#define ADXL343_INT_ENABLE		0x2E	// Interrupt enable control
#define ADXL343_INT_MAP			0x2F	// Interrupt mapping control
#define ADXL343_INT_SOURCE		0x30	// Source of interrupts
#define ADXL343_DATA_FORMAT		0x31	// Data format control
#define ADXL343_DATAX0			0x32	// X-Axis Data 0
#define ADXL343_DATAX1			0x33	// X-Axis Data 1
#define ADXL343_DATAY0			0x34	// Y-Axis Data 0
#define ADXL343_DATAY1			0x35	// Y-Axis Data 1
#define ADXL343_DATAZ0			0x36	// Z-Axis Data 0
#define ADXL343_DATAZ1			0x37	// Z-Axis Data 1
#define ADXL343_FIFO_CTL		0x38	// FIFO control
#define ADXL343_FIFO_STATUS		0x39	// FIFO status

/*
 * SENSOR STRUCT
 */

typedef struct {
	// I2C Handle (I2C1 or I2C2)
	I2C_HandleTypeDef *i2cHandle;

	// Accelerometer Data (X, Y, Z)
	float acc_data[3];
} ADXL343;

/*
 * INITIALIZATION
 */

uint8_t ADXL343_Initialize(ADXL343 *device, I2C_HandleTypeDef *i2cHandle);

/*
 * SENSOR DATA
 */

HAL_StatusTypeDef ADXL343_ReadAcceleration(ADXL343 *device);

/*
 * LOW-LEVEL FUNCTIONS
 */

HAL_StatusTypeDef ADXL343_ReadRegister(ADXL343 *device, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef ADXL343_ReadRegisters(ADXL343 *device, uint8_t reg, uint8_t *data, uint8_t length);
HAL_StatusTypeDef ADXL343_WriteRegister(ADXL343 *device, uint8_t reg, uint8_t *data);

#endif /* ADXL343_DRIVER_H */
