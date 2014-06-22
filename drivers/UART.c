/*
 * UART.c
 *
 *  Created on: Feb 23, 2014
 *      Author: Nick
 */

#define PART_TM4C123GH6PM
//#include "inc/tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "UART.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_uart.h"
#include "driverlib/uart.h"

/*****************************************************
 * 	Function: UART_SetupUART0
 *	Description:
 *	Input: None
 *	Output: None
 *****************************************************/
static void UART_SetupUART0(void)
{

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);

	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	char message[] = {'S', 'T', 'A', 'R', 'T'};
	UART_PrintMessage(message, 5, true);

}

/*****************************************************
 * 	Function: UART_PrintMessage
 *	Description:
 *	Input: None
 *	Output: None
 *****************************************************/
static void UART_PrintMessage(char input[], int count, bool endLine)
{
	int current = 0;
	while(current <= (count - 1))
	{
		UARTCharPut(UART0_BASE, input[current]);
		current++;
	}

	if(endLine)
	{
		UARTCharPut(UART0_BASE, '\r');
		UARTCharPut(UART0_BASE, '\n');
	}
}

/*****************************************************
 * 	Function: UART_PrintLong
 *	Description: Prints long value to UART
 *	Input: (long) input - Long value to print to UART
 *	Output: None
 *****************************************************/
static void UART_PrintLong(long input)
{
	char intToChar[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	char toBeOutput[10];
	int counter = 0;

	if(input == 0)
	{
		toBeOutput[counter] = intToChar[counter];
		counter++;
	}

	while(input)
	{
		int nextDigit = input % 10;
		toBeOutput[counter++] = intToChar[nextDigit];
		input = input / 10;
	}

	// Clear that extra increment
	counter--;

	do{
		UARTCharPut(UART0_BASE, toBeOutput[counter--]);
	}while(counter >= 0);


}



