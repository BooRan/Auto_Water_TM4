/*
 * DS1307.h
 *
 *  Created on: May 25, 2014
 *      Author: Nick
 */

#ifndef DS1307_H_
#define DS1307_H_


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

#define DS1307_I2C_BASE			0x40023000//I2C3_BASE
#define DS1307_SLAVE_ADDRESS	0x68

typedef struct {
  unsigned short second; 		//00H
  unsigned short minute; 		//01H
  unsigned short hour; 			//02H
  unsigned short dayOfWeek; 	//03H
  unsigned short dayOfMonth; 	//04H
  unsigned short month; 		//05H
  unsigned short year; 			//06H
} DateTime;


extern DateTime DS1307_GetTime(void);
extern void DS1307_SetTime(void);
extern short bcdToDec(char val);
extern short decToBcd(char val);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* DS1307_H_ */
