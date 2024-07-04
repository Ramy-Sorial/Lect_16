/*
 * DS1307.c
 *
 *  Created on: Jul 4, 2024
 *      Author: Ramy Sorial
 */

#include "DS1307.h"

#define DS1307_SLA 0b1101000

static uint8_t I2C_Write(DS1307_t *clock, uint8_t SLA, uint8_t *Data, uint16_t len)
{
	HAL_StatusTypeDef H = HAL_I2C_Master_Transmit(clock->bus, SLA << 1, Data, len, 100);
	//return __HAL_I2C_GET_FLAG(clock->bus,I2C_FLAG_AF) ? 0 : 1;
	return H == HAL_OK ? 1 : 0;
}

static uint8_t I2C_Read(DS1307_t *clock, uint8_t SLA, uint8_t *Data, uint16_t len)
{
	HAL_StatusTypeDef H = HAL_I2C_Master_Receive(clock->bus, SLA << 1, Data, len, 100);
	//return __HAL_I2C_GET_FLAG(clock->bus,I2C_FLAG_AF) ? 0 : 1;
	return H == HAL_OK ? 1 : 0;
}

static uint8_t Dec2BCD(uint8_t Dec)
{
	return ((Dec / 10) << 4) | (Dec % 10);
}

static uint8_t BCD2Dec(uint8_t BCD)
{
	return (BCD & 0xf) + ((BCD >> 4) * 10);

}

DS1307_Stat_t DS1307_Init(DS1307_t *clock, I2C_HandleTypeDef *I2C_Bus)
{
	clock->bus = I2C_Bus;
	return DS1307_OK;
}

DS1307_Stat_t DS1307_GetClock(DS1307_t *clock)
{
	clock->TxBuffer[0] = 0;
	uint8_t OK = 1;
	OK &= I2C_Write(clock, DS1307_SLA, clock->TxBuffer, 1);
	if (!OK)
	{
		return DS1307_NOK;
	}
	OK &= I2C_Read(clock, DS1307_SLA, clock->RxBuffer, 7);

	if (OK)
	{
		clock->ClockHold = clock->RxBuffer[0] >> 7;
		clock->Seconds = BCD2Dec(clock->RxBuffer[0] & 0b1111111);
		clock->Minutes = BCD2Dec(clock->RxBuffer[1]);

		//get the 12/24 format bit
		clock->Format_12H = clock->RxBuffer[2] & 0b01000000;

		//12 hour format is selected
		if (clock->Format_12H)
		{
			//get the 5-bit hour part
			clock->Hours = BCD2Dec(clock->RxBuffer[2] & 0b11111);
			//get the PM_AM indicator
			clock->PM_AM = (clock->RxBuffer[2] & 0b100000) ? 1 : 0;
		}
		//24 hour format selected
		else
		{
			//get the 6-bit hour part
			clock->Hours = BCD2Dec(clock->RxBuffer[2] & 0b111111);
			//clock is PM starting from >12 o'clock
			clock->PM_AM = clock->Hours > 11 ? 1 : 0;
		}

		clock->DayOfWeek = BCD2Dec(clock->RxBuffer[3]);
		clock->DayOfMonth = BCD2Dec(clock->RxBuffer[4]);
		clock->Month = BCD2Dec(clock->RxBuffer[5]);
		clock->Year = BCD2Dec(clock->RxBuffer[6]) + 2000;
	}
	return OK ? DS1307_OK : DS1307_NOK;
}

DS1307_Stat_t DS1307_SetClock(DS1307_t *clock)
{
	clock->TxBuffer[0] = 0;

	clock->TxBuffer[1] = Dec2BCD(clock->Seconds) | (clock->ClockHold << 7);
	clock->TxBuffer[2] = Dec2BCD(clock->Minutes);
	clock->TxBuffer[3] = Dec2BCD(clock->Hours) | (clock->PM_AM << 5) | (clock->Format_12H << 6);
	clock->TxBuffer[4] = Dec2BCD(clock->DayOfWeek);
	clock->TxBuffer[5] = Dec2BCD(clock->DayOfWeek);
	clock->TxBuffer[6] = Dec2BCD(clock->Month);
	clock->TxBuffer[7] = Dec2BCD(clock->Year - 2000);

	return I2C_Write(clock, DS1307_SLA, clock->TxBuffer, 8) == 1 ? DS1307_OK : DS1307_NOK;
}
