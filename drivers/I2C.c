/*
 * I2C.c
 *
 *  Created on: Feb 22, 2014
 *      Author: Nick
 */


#define PART_TM4C123GH6PM
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "I2C.h"

/*****************************************************
 * 	Function: SetupI2C3
 *	Description: Setup process for I2C3
 *	Input: None
 *	Output: None
 *****************************************************/
static void I2C_SetupI2C3(void){

	clock = SysCtlClockGet();

	// Shared Pin Setting
    // You need to set the shared pins as input.
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    // Change the pad configuration to WPU
    GPIOPadConfigSet( GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // I2C Setting
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0); //  special I2CSCL treatment for M4F devices
    GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);

    GPIOPinConfigure(GPIO_PD0_I2C3SCL);
    GPIOPinConfigure(GPIO_PD1_I2C3SDA);

    SysCtlPeripheralEnable( SYSCTL_PERIPH_I2C3);

    I2CMasterInitExpClk( I2C3_BASE, clock, false);
    SysCtlDelay(10000);
}

/*****************************************************
 * 	Function: I2C_SendSlaveStart
 *	Description: Sends slave address and start bit
 *			followed by a stop bit
 *	Input: (uint32_t) ui32Base - I2C Base to use
 *		   (uint8_t)  ui8SlaveAddr - Slave address
 *	Output:	NONE
 *****************************************************/
static void I2C_SendSlaveStart(uint32_t ui32Base, uint8_t ui8SlaveAddr)
{
	// Set slave address
	I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, false);   // false = write, true = read
	// Send Start
	I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_START);
	// Wait for address to be sent
	I2C_WaitForDone(ui32Base);
	// Send stop
	I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	// Wait for bus to clear
	I2C_WaitForDone(ui32Base);
}

/*****************************************************
 * 	Function: I2C_ReadBytes
 *	Description: Reads n number of bytes
 *	Input: (uint32_t) ui32Base - I2C Base to use
 *		   (uint8_t)  ui8SlaveAddr - Slave address
 *		   (uint8_t)  ui8NumberOfBytes - Number of bytes to read
 *	Output:	NONE
 *****************************************************/
static void I2C_ReadBytes(uint32_t ui32Base, uint8_t ui8SlaveAddr, uint32_t *ui8ResponseArray, uint8_t ui8NumberOfBytes)
{
	// Set slave address
	I2CMasterSlaveAddrSet( ui32Base, ui8SlaveAddr, true);   // false = write, true = read
	// Set register to read from
	//I2CMasterDataPut( ui32Base, ui8RegisterAddr);
	// Send message
	I2CMasterControl( ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_START);
	// Wait for data to be sent
	uint32_t i2c_err = I2C_WaitForDone( ui32Base);

	int counter = 0;
	for(counter=0; counter <= ui8NumberOfBytes; counter++)
	{
		// Read
		if( (i2c_err == I2C_MASTER_ERR_NONE))
		{
			// If no errors occured get data
			ui8ResponseArray[counter] = I2CMasterDataGet(ui32Base);
		}
		else
		{
			// If an error occured, clear our result
			ui8ResponseArray[counter] = 0xff;
		}

		// Check if this is our last read
		if(counter < (ui8NumberOfBytes - 1))
		{
			// Send Ack
			I2CMasterControl( ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		}
		else
		{
			// Send stop
			I2CMasterControl( ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
		}

		// Wait
		i2c_err = I2C_WaitForDone( ui32Base);
	}
}

/*****************************************************
 * 	Function: I2C_ReadBytesFromAddress
 *	Description: Reads n number of bytes from address
 *	Input: (uint32_t) ui32Base - I2C Base to use
 *		   (uint8_t)  ui8SlaveAddr - Slave address
 *		   (uint8_t)  ui8NumberOfBytes - Number of bytes to read
 *		   (uint8_t)  ui8RegisterAddr - Address to start read at
 *	Output:	NONE
 *****************************************************/
static void I2C_ReadBytesFromAddress(uint32_t ui32Base, uint8_t ui8SlaveAddr, uint8_t ui8RegisterAddr, uint32_t *ui8ResponseArray, uint8_t ui8NumberOfBytes)
{
	// Set slave address, false = write
	I2CMasterSlaveAddrSet( ui32Base, ui8SlaveAddr, false);
	// Set register to read from
	I2CMasterDataPut( ui32Base, ui8RegisterAddr);
	// Send message
	I2CMasterControl( ui32Base, I2C_MASTER_CMD_SINGLE_SEND);
	// Wait for data to be sent
	I2C_WaitForDone( ui32Base);

	// Set slave address, true = read
	I2CMasterSlaveAddrSet( ui32Base, ui8SlaveAddr, true);
	// Set register to read from
	//I2CMasterDataPut( ui32Base, ui8RegisterAddr);
	// Send message
	I2CMasterControl( ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_START);
	// Wait for data to be sent
	uint32_t i2c_err = I2C_WaitForDone( ui32Base);

	int counter = 0;
	for(counter=0; counter <= ui8NumberOfBytes; counter++)
	{
		// Read
		if( (i2c_err == I2C_MASTER_ERR_NONE))
		{
			// If no errors occured get data
			ui8ResponseArray[counter] = I2CMasterDataGet(ui32Base);
		}
		else
		{
			// If an error occured, clear our result
			ui8ResponseArray[counter] = 0xff;
		}

		// Check if this is our last read
		if(counter < (ui8NumberOfBytes - 1))
		{
			// Send Ack
			I2CMasterControl( ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		}
		else
		{
			// Send stop
			I2CMasterControl( ui32Base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
		}

		// Wait
		i2c_err = I2C_WaitForDone( ui32Base);
	}
}

/*****************************************************
 * 	Function: I2C_ReadSingleByte
 *	Description: Reads single byte from address
 *	Input: (uint32_t) ui32Base - I2C Base to use
 *		   (uint8_t)  ui8SlaveAddr - Slave address
 *		   (uint8_t)  ui8RegisterAddr - Address to read
 *	Output:	(uint8_t) response - I2C byte response
 *****************************************************/
// uint32_t ui32Base = I2C3_BASE
// uint8_t ui8RegisterAddr =
static uint8_t I2C_ReadSingleByte(uint32_t ui32Base, uint8_t ui8SlaveAddr, uint8_t ui8RegisterAddr)
{
	uint8_t data = 0;

	// Set slave address
	I2CMasterSlaveAddrSet( ui32Base, ui8SlaveAddr, false);   // false = write, true = read
	// Set register to read from
	I2CMasterDataPut( ui32Base, ui8RegisterAddr);
	// Send message
	I2CMasterControl( ui32Base, I2C_MASTER_CMD_SINGLE_SEND);
	// Wait for data to be sent
	I2C_WaitForDone( ui32Base);

	// Set read mode
	I2CMasterSlaveAddrSet( ui32Base, ui8SlaveAddr, true);   // false = write, true = read

	// Single Read
	I2CMasterControl( ui32Base, I2C_MASTER_CMD_SINGLE_RECEIVE);

	uint32_t i2c_err = I2C_WaitForDone( ui32Base);

	// Wait for data to be sent/read
	if( (i2c_err == I2C_MASTER_ERR_NONE))
	{
		// If no errors occured get data
		data = I2CMasterDataGet(ui32Base);
	}
	else
	{
		// If an error occured, clear our result
		data = 0xff;
	}

	return data;
}

/*****************************************************
 * 	Function: I2C_WriteByte
 *	Description: Writes single byte to address
 *	Input: (uint32_t) ui32Base - I2C Base to use
 *		   (uint8_t)  ui8SlaveAddr - Slave address
 *		   (uint8_t)  ui8RegisterAddr - Address to write
 *		   (uint8_t)  ui8Data - Data to write
 *	Output:	None
 *****************************************************/
static void I2C_WriteByte(uint32_t ui32Base, uint8_t ui8SlaveAddr, uint8_t ui8RegisterAddr, uint8_t ui8Data)
{
	// Set slave address
	I2CMasterSlaveAddrSet(ui32Base, ui8SlaveAddr, false);   // false = write, true = read
	// Set register to write to
	I2CMasterDataPut(ui32Base, ui8RegisterAddr);
	// Send message
	I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_SEND_START);
	// Wait for address to be sent
	I2C_WaitForDone(ui32Base);
	// Set data to write
	I2CMasterDataPut(ui32Base, ui8Data);
	// Single write
	I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_SEND_CONT);
	// Wait for data to be sent
	I2C_WaitForDone(ui32Base);
	// Set finish
	I2CMasterControl(ui32Base, I2C_MASTER_CMD_BURST_SEND_FINISH);
	// Wait for bus to clear
	I2C_WaitForDone(ui32Base);
}

/*****************************************************
 * 	Function: WaitI2CDone
 *	Description: Waits for I2C Master bus to clear
 *	Input: (uint32_t) ui32Base - I2C Base to use
 *	Output: (uint32_t) I2C Master error status
 *	I2C_MASTER_ERR_NONE, I2C_MASTER_ERR_ADDR_ACK,
 *	I2C_MASTER_ERR_DATA_ACK, or I2C_MASTER_ERR_ARB_LOST
 *****************************************************/
static uint32_t I2C_WaitForDone(uint32_t ui32Base){
    // Wait until done transmitting
    while( I2CMasterBusy(ui32Base));
    // Return I2C error code
    return I2CMasterErr( ui32Base);
}

/*****************************************************
 * 	Function: I2C_Delay
 *	Description: Delays milliseconds
 *	Input: (uint32_t) milliSeconds - Time in ms to delay
 *	Output: NONE
 *****************************************************/
static void I2C_Delay(uint32_t milliSeconds)
{
	uint32_t counter;
	for(counter=0; counter<(clock/1000)*(milliSeconds); )
	{
		// Do nothing
		counter++;
	}
}
