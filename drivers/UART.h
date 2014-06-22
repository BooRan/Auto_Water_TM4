/*
 * UART.h
 *
 *  Created on: Feb 23, 2014
 *      Author: Nick
 */

#ifndef UART_H_
#define UART_H_

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

extern void UART_SetupUART0(void);
extern void UART_PrintMessage(char input[], int count, bool endLine);
extern void UART_PrintLong(long input);

#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
