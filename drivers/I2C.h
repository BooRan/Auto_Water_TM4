/*
 * I2C.h
 *
 *  Created on: Feb 22, 2014
 *      Author: Nick
 */

#ifndef I2C_H_
#define I2C_H_


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

uint32_t clock;

extern void I2C_SetupI2C3(void);
extern void I2C_SendSlaveStart(uint32_t, uint8_t);
extern void I2C_ReadBytes(uint32_t, uint8_t, uint32_t *, uint8_t);
extern void I2C_ReadBytesFromAddress(uint32_t, uint8_t, uint8_t, uint32_t *, uint8_t);
extern uint8_t I2C_ReadSingleByte(uint32_t, uint8_t, uint8_t);
extern void I2C_WriteByte(uint32_t, uint8_t, uint8_t, uint8_t);
extern uint32_t I2C_WaitForDone(uint32_t);
extern void I2C_Delay(uint32_t);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */
