/*
 * HIH6130.c
 *
 *  Created on: Jun 17, 2014
 *      Author: Nick
 */


#define PART_TM4C123GH6PM
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
//#include "I2C.c"
#include "HIH6130.h"

/*****************************************************
 * 	Function: HIH6130_UpdateData
 *	Description: Reads data from HIH6130
 *	Input: NONE
 *	Output: (Status) status - Current status
 *****************************************************/
static HIH6130_Status HIH6130_UpdateData(void)
{

	uint32_t response[4];

	// Send measure command to HIH6131
	I2C_SendSlaveStart(HIH6130_I2C_BASE, HIH6130_SLAVE_ADDRESS);

	// Give a small delay for the measurement to take place
	I2C_Delay(1);

	// Read four bytes from HIH6130
	I2C_ReadBytes(HIH6130_I2C_BASE, HIH6130_SLAVE_ADDRESS, response, 4);

	// status = uppper two bits of first byte
	status = (HIH6130_Status)(response[0] >> 6) & 0x03;

	if(status == NORMAL)
	{
		// humidity = lower six bits of first byte and second byte
		humidity = ((((response[0] & 0x3F) << 8) | (response[1]))) / 0xA3;

		// temperature =  third byte and upper six bytes of fourth byte
		// 0x28 = 0 counts
		temperature = (((((response[2] << 8) | (response[3])) >> 2) / 0x63) - 0x28) * 1.8 + 32;
	}

	return status;
}

/*****************************************************
 * 	Function: HIH6130_GetStatus
 *	Description: Returns HIH6130 status
 *	Input:	NONE
 *	Output:	(uint8_t) status - Current status
 *****************************************************/
static uint8_t HIH6130_GetStatus(void)
{
	return status;
}

/*****************************************************
 * 	Function: HIH6130_GetHumidity
 *	Description: Returns HIH6130 humidity reading
 *	Input: NONE
 *	Output: (uint8_t) humidity - Current humidity
 *****************************************************/
static uint8_t HIH6130_GetHumidity(void)
{
	return humidity;
}

/*****************************************************
 * 	Function: HIH6130_GetTemperature
 *	Description: Returns HIH6130 temperature reading
 *	Input: NONE
 *	Output: (in8_t) temperature - Current temperature
 *****************************************************/
static int8_t HIH6130_GetTemperature(void)
{
	return temperature;
}


