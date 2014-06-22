/*
 * DS1307.c
 *
 *  Created on: May 25, 2014
 *      Author: Nick
 */


#define PART_TM4C123GH6PM
#include <stdint.h>
#include <stdbool.h>
//#include "I2C.c"
#include "DS1307.h"

/*****************************************************
 * 	Function: GetTime
 *	Description: Gets current time from DS1307
 *	Input: NONE
 *	Output: (DateTime) dt - current DateTime object
 *****************************************************/
static DateTime DS1307_GetTime(void)
{
	DateTime dt;
	uint32_t response[7];
	I2C_ReadBytesFromAddress(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x00, response, 7);
	dt.second 		= bcdToDec(response[0x00]);
	dt.minute 		= bcdToDec(response[0x01]);
	dt.hour	  		= bcdToDec(response[0x02]);
	dt.dayOfWeek	= bcdToDec(response[0x03]);
	dt.dayOfMonth	= bcdToDec(response[0x04]);
	dt.month		= bcdToDec(response[0x05]);
	dt.year			= bcdToDec(response[0x06]);

	return dt;
}

/*****************************************************
 * 	Function: SetTime
 *	Description: Sets DS1307 to specified time
 *	Input: NONE
 *	Output: Outputs I2C communication to DS1307
 *****************************************************/
static void DS1307_SetTime(void)
{

  short second =      45; //0-59
  short minute =      40; //0-59
  short hour =        22; //0-23
  short weekDay =     5; //1-7
  short monthDay =    10; //1-31
  short month =       4; //1-12
  short year  =       14; //0-99

  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x00, decToBcd(second));
  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x01, decToBcd(minute));
  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x02, decToBcd(hour));
  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x03, decToBcd(weekDay));
  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x04, decToBcd(monthDay));
  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x05, decToBcd(month));
  I2C_WriteByte(DS1307_I2C_BASE, DS1307_SLAVE_ADDRESS, 0x06, decToBcd(year));
}

/*****************************************************
 * 	Function: bcdToDec
 *	Description: Converts binary coded decimal value
 *			to decimal value
 *	Input: (char) val - binary coded decimal value to
 *						be converted
 *	Output: (short) value converted to decimal
 *****************************************************/
static short bcdToDec(char val)
{
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

/*****************************************************
 * 	Function: decToBcd
 *	Description: Converts decimal value to binary
 *				coded decimal value
 *	Input: (char) val - decimal value to be converted
 *	Output: (short) value converted to binary coded decimal
 *****************************************************/
static short decToBcd(char val)
{
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

