/*
 * AMS.h
 *
 *  Created on: Jun 17, 2014
 *      Author: Nick
 */

#ifndef AMS_H_
#define AMS_H_


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


#define AMS_PIN						GPIO_PIN_5
#define AMS_REG						GPIO_PORTB_BASE
#define AMS_CONTROL_PIN				GPIO_PIN_0
#define AMS_CONTROL_REG				GPIO_PORTB_BASE
#define AMS_CONTROL_GND_PIN			GPIO_PIN_1
#define AMS_CONTROL_GND_REG			GPIO_PORTB_BASE

extern void AMS_InitSensor(void);
extern uint32_t AMS_ReadSensor(void);


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* AMS_H_ */
