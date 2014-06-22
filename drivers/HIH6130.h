/*
 * HIH6130.h
 *
 *  Created on: Jun 17, 2014
 *      Author: Nick
 */

#ifndef HIH6130_H_
#define HIH6130_H_


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

#define HIH6130_I2C_BASE			0x40023000//I2C3_BASE
#define HIH6130_SLAVE_ADDRESS		0x27

typedef enum
{
	NORMAL = 0,
	StaleData = 1,
	CommandMode = 2,
	DiagnosticMode = 3
} HIH6130_Status;

HIH6130_Status status;
uint8_t	humidity;
int8_t temperature;

extern HIH6130_Status HIH6130_UpdateData(void);
extern uint8_t HIH6130_GetStatus(void);
extern uint8_t HIH6130_GetHumidity(void);
extern int8_t HIH6130_GetTemperature(void);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* HIH6130_H_ */
