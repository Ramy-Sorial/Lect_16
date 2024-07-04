/*
 * DS1307.h
 *
 *  Created on: Jul 4, 2024
 *      Author: Ramy Sorial
 */

#ifndef DEVICE_DRIVERS_DS1307_H_
#define DEVICE_DRIVERS_DS1307_H_

#include "stm32f1xx_hal.h"

typedef enum
{
	DS1307_OK,
	DS1307_NOK,
} DS1307_Stat_t;


typedef struct
{
	uint8_t DayOfMonth;
	uint8_t Month;
	uint16_t Year; //because the final result should be 20xx > 255
	uint8_t DayOfWeek;

	uint8_t Hours;
	uint8_t Minutes;

	uint8_t Seconds;

	uint8_t ClockHold :1;
	uint8_t Format_12H :1;
	uint8_t PM_AM :1;

	uint8_t TxBuffer[8];
	uint8_t RxBuffer[8];

	I2C_HandleTypeDef *bus;
} DS1307_t;

DS1307_Stat_t DS1307_Init(DS1307_t *clock, I2C_HandleTypeDef *I2C_Bus);

DS1307_Stat_t DS1307_GetClock(DS1307_t *clock);

DS1307_Stat_t DS1307_SetClock(DS1307_t *clock);

#endif /* DEVICE_DRIVERS_DS1307_H_ */
