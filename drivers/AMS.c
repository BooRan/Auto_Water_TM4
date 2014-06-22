/*
 * AMS.c
 *
 *  Created on: Jun 17, 2014
 *      Author: Nick
 */


#define PART_TM4C123GH6PM
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "AMS.h"


/*****************************************************
 * 	Function: AMS_InitSensor
 *	Description: Initializes AMS on ADC1 sequence 0
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
static void AMS_InitSensor(void)
{
	// Configure input pin as ADC
	GPIOPinTypeADC(AMS_REG, AMS_PIN);

	// Configure control pin and GND pin
	GPIOPinTypeGPIOOutput(AMS_CONTROL_GND_REG, AMS_CONTROL_GND_PIN);
	GPIOPinWrite(AMS_CONTROL_GND_REG, AMS_CONTROL_GND_PIN, 0);
	GPIOPinTypeGPIOOutput(AMS_CONTROL_REG, AMS_CONTROL_PIN);
	GPIOPinWrite(AMS_CONTROL_REG, AMS_CONTROL_PIN, AMS_CONTROL_PIN);

	// Initialize clocking to ADC1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

	// Configure hardware over-sampling to take 64 samples per step
	// With 4 steps on 64 samples -> 256 samples
	ADCHardwareOversampleConfigure(ADC1_BASE, 64);

	// Configure sequence to trigger on processor trigger
	ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

	// Configure steps 0-3 to read channel 11
	// Configure step 3 to end conversion
	ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH11);
	ADCSequenceStepConfigure(ADC1_BASE, 0, 1, ADC_CTL_CH11);
	ADCSequenceStepConfigure(ADC1_BASE, 0, 2, ADC_CTL_CH11);
	ADCSequenceStepConfigure(ADC1_BASE, 0, 3, ADC_CTL_CH11|ADC_CTL_IE|ADC_CTL_END);

	// Enable sequence
	ADCSequenceEnable(ADC1_BASE, 0);
}

/*****************************************************
 * 	Function: AMS_ReadSensor
 *	Description: Reads average sensor value (256 samples)
 *	Input: NONE
 *	Output: (uint32_t) ui32AvgValue - Average sensor value
 *****************************************************/
static uint32_t AMS_ReadSensor(void)
{
	uint32_t ui32ADC1Value[4];
	uint32_t ui32AvgValue = 0;

	ADCIntClear(ADC1_BASE, 0);
	ADCProcessorTrigger(ADC1_BASE, 0);

	while(!ADCIntStatus(ADC1_BASE, 0, false))
	{
	}

	ADCSequenceDataGet(ADC1_BASE, 0, ui32ADC1Value);
	ui32AvgValue = (ui32ADC1Value[0] + ui32ADC1Value[1] + ui32ADC1Value[2] + ui32ADC1Value[3])/4;
	return ui32AvgValue;
}





