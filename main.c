//*****************************************************************************
//	MAIN.C
//
//
//
//
//*****************************************************************************

//#define PART_TM4C123FH6PM
//#include "inc/tm4c123fh6pm.h"
#define PART_TM4C123GH6PM
#include "inc/tm4c123gh6pm.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/hibernate.h"
#include "driverlib/adc.h"

#include "drivers/I2C.c"
#include "drivers/typedefs.h"
#include "drivers/UART.c"
#include "drivers/DS1307.c"
#include "drivers/HIH6130.c"
#include "drivers/AMS.c"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void init_Clock(void);
void init_LED(void);
void init_Zones(void);
void init_IntTempSensor(void);
void init_genTimer1(void);
void checkIntTempSensor(void);
void init_Hibernation(void);
void init_BtnHandler(void);
void checkZoneStatus(void);
void processZones(void);
void clearAllZoneOverrides(void);
void setAllZoneOverrides(void);
void printCurrentStatus(void);
void delay(uint32_t milliSeconds);
void flashLED(uint8_t led);

#define SHUTDOWN_TEMP		100
#define TARGET_REL_HUM		50
#define NUMBER_OF_ZONES		4

int response = 0;
bool hasSeenButtonPress = false;
int buttonPressTimerTicks = 0;
int oneSecondCounter = 0;
DateTime overRideStartTime;
uint32_t clockTime;
DateTime dateTime;
Mode mode = RUN;
uint8_t statusLed =  LED_GREEN_PIN;

uint32_t ui32ADC0Value[4];
uint32_t ui32TempAvg;
uint32_t ui32TempValueC;
uint32_t ui32TempValueF;
uint32_t ui32MoistureAvg;

//				Status, OverrideStatus, Port, Pin, OnHour, OnMinute, OnLength;
Zone zone0 = { ZONE_OFF, OVERRIDE_INACTIVE, GPIO_PORTD_BASE, GPIO_PIN_3, 6, 40, 5};
Zone zone1 = { ZONE_OFF, OVERRIDE_INACTIVE, GPIO_PORTE_BASE, GPIO_PIN_1, 15, 25, 3};
Zone zone2 = { ZONE_OFF, OVERRIDE_INACTIVE, GPIO_PORTE_BASE, GPIO_PIN_2, 15, 26, 3};
Zone zone3 = { ZONE_OFF, OVERRIDE_INACTIVE, GPIO_PORTE_BASE, GPIO_PIN_3, 15, 27, 3};
//Zone zone0 = { .Status = ZONE_OFF, .OverrideStates = OVERRIDE_INACTIVE, .Port = ZONE_D_PORT, .Pin = GPIO_PIN_3, .OnHour = 19, .OnMinute = 12, .OnLength = 1};
//Zone zone1 = { .Status = ZONE_OFF, .OverrideStates = OVERRIDE_INACTIVE, .Port = ZONE_D_PORT, .Pin = GPIO_PIN_3, .OnHour = 19, .OnMinute = 12, .OnLength = 1};
//Zone zone2 = { .Status = ZONE_OFF, .OverrideStates = OVERRIDE_INACTIVE, .Port = ZONE_D_PORT, .Pin = GPIO_PIN_3, .OnHour = 19, .OnMinute = 7, .OnLength = 4};
//Zone zone3 = { .Status = ZONE_OFF, .OverrideStates = OVERRIDE_INACTIVE, .Port = ZONE_D_PORT, .Pin = GPIO_PIN_3, .OnHour = 19, .OnMinute = 10, .OnLength = 7};

Zone* Zones[NUMBER_OF_ZONES] = { &zone0, &zone1, &zone2, &zone3};

/*****************************************************
 * 	Function: main
 *	Description: Runs initialization of all modules
 *				and main state loop
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
int main(void)
{
	init_Clock();				// Initialize clock
    init_LED();					// Initialize LEDs
    init_Zones();				// Initialize zones
    init_genTimer1();			// Initialize general timer 1
    init_BtnHandler();			// Initialize button interrupt handler
    init_Hibernation();			// Initialize hibernation module
    UART_SetupUART0();			// Initialize UART0
    I2C_SetupI2C3();			// Initialize I2C3
    init_IntTempSensor();		// Initialize internal temperature sensor
    AMS_InitSensor();			// Initialize analog moisture sensor

	// Enable master interrupts
	IntMasterEnable();

    //
    // Main loop
    //

	while(true)
	{
		switch(mode)
		{
			case RUN:
				statusLed = LED_GREEN_PIN;
				dateTime = DS1307_GetTime();			// Get the current time
				//HIH6130_UpdateData();					// Get HIH6130 Data
				checkZoneStatus();						// Check if status of each zone
				break;
			case OVERRIDE:
				statusLed = LED_RED_PIN;
				if(oneSecondCounter >= SECONDS_IN_24_HOURS)
				{
					// If 24 hours have passed, switch back to run mode
					clearAllZoneOverrides();
					oneSecondCounter = 0;
					mode = RUN;
				}
				else
				{
					setAllZoneOverrides();
				}
				break;
			case SYSTEM_SHUTDOWN:
				GPIOPinWrite(LED_REG, LED_RED_PIN, LED_ALL_OFF);				// Turn off all other leds
				GPIOPinWrite(LED_REG, LED_RED_PIN, LED_RED_PIN);				// Turn on red led to signal temperature shutdown
				int zoneNumber;
				for(zoneNumber=0; zoneNumber<NUMBER_OF_ZONES; zoneNumber++)		// Shutoff each zone
				{
					Zone* currentZone = Zones[zoneNumber];
					GPIOPinWrite(currentZone->Port, currentZone->Pin, currentZone->Pin);
				}
				IntMasterDisable();												// Disable all interrupts
				TimerEnable(BTN_OVERRIDE_TIM_BASE, TIMER_A);					// Turn off timer for button press
				TimerEnable(TIMER1_BASE, TIMER_A);								// Turn off general timer
				HibernateRequest();												// Go into hibernation until wake button is pressed
				break;
			default:

				break;
		}

		processZones();							// Process any changes to zones

	}


}

/*****************************************************
 * 	Function: init_Clock
 *	Description: Initializes system clock and enables
 *				clocking to all GPIO ports
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
void init_Clock(void)
{

	//Set up clock (Bypass PLL, run from external clock, main osc and 16 Mhz, div by 5)
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);  //setup clock
	clockTime = SysCtlClockGet();
	// Enable Ports
	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOB | SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOE | SYSCTL_RCGC2_GPIOF;

}

/*****************************************************
 * 	Function: init_LED
 *	Description: Enables LED ports as GPIO Output LOW
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
void init_LED(void)
{
		// Enable the GPIO port that is used for the on-board LEDs
		GPIOPinTypeGPIOOutput(LED_REG, LED_RED_PIN|LED_BLUE_PIN|LED_GREEN_PIN);
		// Turn all LEDs off
		GPIOPinWrite(LED_REG, LED_RED_PIN|LED_BLUE_PIN|LED_GREEN_PIN, LED_ALL_OFF);
}

/*****************************************************
 * 	Function: init_Zones
 *	Description: Initializes zones and output ports
 *	Input: NONE
 *	Output: Modifies zones[]
 *****************************************************/
void init_Zones(void)
{
	int zoneNumber;

	// Iterate through each of the zones
	for(zoneNumber=0; zoneNumber<NUMBER_OF_ZONES; zoneNumber++)
	{
		Zone* currentZone = Zones[zoneNumber];
		currentZone->Status = ZONE_OFF;
		currentZone->OverrideStatus = OVERRIDE_INACTIVE;

		// Enable the GPIO port that is used for the zone
		GPIOPinTypeGPIOOutput(currentZone->Port, currentZone->Pin);
		// Turn relay off = pin HIGH
		GPIOPinWrite(currentZone->Port, currentZone->Pin, currentZone->Pin);
	}
}

/*****************************************************
 * 	Function: init_IntTempSensor
 *	Description: Initializes internal temperature
 *			sensor and general timer 1
 *			Uses ADC0 Module and TIMER1
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
void init_IntTempSensor(void)
{
	// Enable clock to ADC0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	// Configure hardware over-sampling to take 64 samples per step
	// With 4 steps on 64 samples -> 256 samples
	ADCHardwareOversampleConfigure(ADC0_BASE, 64);

	// Configure sequence to trigger on processor trigger
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

	// Configure steps 0-3 to read internal temperature sensor
	// Configure step 3 to end conversion
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_TS);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

	// Enable sequence
	ADCSequenceEnable(ADC0_BASE, 0);
}

/*****************************************************
 * 	Function: init_genTimer1
 *	Description: Initialize genTimer1
 *	Input: None
 *	Output: NONE
 *****************************************************/
void init_genTimer1(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);	// Setup interrupt as 32 bit timer counting up
	TimerLoadSet(TIMER1_BASE, TIMER_A, clockTime);		// Load Timer
	IntEnable(INT_TIMER1A);								// Enable TIMER1A interrupt
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);	// Enable interrupt for timer
	TimerEnable(TIMER1_BASE, TIMER_A);					// Enable TIMER1A
}

/*****************************************************
 * 	Function: genTimer1Handler
 *	Description: Fires once per second with 32 bit timer
 *			Handles reading internal temperature sensor
 *			Prints current status to UART
 *			Toggles LED every 10th iteration
 *	Input: None
 *	Output: Increments oneSecondCounter
 *****************************************************/
void genTimer1Handler(void)
{
	// Clear timer Interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	checkIntTempSensor();					// Check internal temperature sensor
	ui32MoistureAvg = AMS_ReadSensor();		// Read the moisture sensor
	printCurrentStatus();					// Print status to UART

	if(oneSecondCounter%10 == 0)
	{
		// If we are in override flash the red led
		GPIOPinWrite(LED_REG, statusLed, statusLed);
	}
	else
	{
		// Turn off LEDs
		GPIOPinWrite(LED_REG, LED_GREEN_PIN | LED_RED_PIN, LED_ALL_OFF);
	}

	// Increment the one second counter
	oneSecondCounter++;
}

/*****************************************************
 * 	Function: checkIntTempSensor
 *	Description: Reads internal temperature sensor
 *	Input: NONE
 *	Output: ui32TempAvg, ui32TempValueC, ui32TempValueF
 *****************************************************/
void checkIntTempSensor(void)
{
	// Clear flag
	ADCIntClear(ADC0_BASE, 0);

	// Trigger processor
	ADCProcessorTrigger(ADC0_BASE, 0);

	// Wait for ADC status to be set
	while(!ADCIntStatus(ADC0_BASE, 0, false)){}

	// Get data and convert to useful values
	// Read all four steps of sequence into ui32ADC0Value
	ADCSequenceDataGet(ADC0_BASE, 0, ui32ADC0Value);
	ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
	ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
	ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

	// Shutdown if device is getting too hot
	if(ui32TempValueF >= SHUTDOWN_TEMP)
	{
		mode = SYSTEM_SHUTDOWN;
	}

}

/*****************************************************
 * 	Function: init_Hibernation
 *	Description: Initialize hibernation module
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
void init_Hibernation(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);				// Enable hibernation module
	HibernateEnableExpClk(SysCtlClockGet());						// Enable clocking to the hibernation module
	HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);					// Configure the clock source for hibernation module
	HibernateGPIORetentionEnable();									// Retain GPIO state during hibernation
//	HibernateRTCEnable();											// Enable the RTC
//	HibernateRTCSet(0);												// Clear the RTC time
	//HibernateRTCMatchSet(0, HibernateRTCGet() + 5);				// Set the match 0 register for 5 seconds from now
	HibernateWakeSet(HIBERNATE_WAKE_PIN);							// Configure to wake on button press or RTC match
								//  HIBERNATE_WAKE_PIN | HIBERNATE_WAKE_RTC

	// Clear any pending status.
	//
	int status = HibernateIntStatus(0);								// Clear any pending status
	HibernateIntClear(status);
}

/*****************************************************
 * 	Function: init_BtnHandler
 *	Description: Initializes button interrupt
 *			Initializes timer for button counter
 *	Input: NONE
 *	Output: NONE
 *****************************************************/
void init_BtnHandler(void)
{
	// Unlock un-maskable pin
	HWREG(BTN_OVERRIDE_REG + GPIO_O_LOCK) = GPIO_LOCK_KEY;

	// Set up our interrupt for button presses
	IntMasterDisable();																				// Disable all interrupts
	GPIOIntDisable(BTN_OVERRIDE_REG, BTN_OVERRIDE);
	GPIOPinTypeGPIOInput(BTN_OVERRIDE_REG, BTN_OVERRIDE);
	GPIOPadConfigSet(BTN_OVERRIDE_REG, BTN_OVERRIDE, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);		// Set Pull-up
	GPIOIntTypeSet(BTN_OVERRIDE_REG, BTN_OVERRIDE, GPIO_BOTH_EDGES); 								// Set edge to trigger on
	GPIOIntClear(BTN_OVERRIDE_REG, BTN_OVERRIDE); 													// Clear the interrupt bit
	GPIOIntEnable(BTN_OVERRIDE_REG, BTN_OVERRIDE); 													// Enable the interrupt
	IntEnable(INT_GPIOE);

	// Lock un-maskable pin
	HWREG(BTN_OVERRIDE_REG + GPIO_O_LOCK) = 0;

	// Setup timer interrupt for button pressing
	// This timer will run up and when it is released we will check how long it was running
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(BTN_OVERRIDE_TIM_BASE, TIMER_CFG_PERIODIC);					// Setup interrupt as 32 bit timer counting up
	TimerLoadSet(BTN_OVERRIDE_TIM_BASE, BTN_OVERRIDE_TIM, clockTime/1000);		// Load Timer
	IntEnable(INT_TIMER0A);
	TimerIntEnable(BTN_OVERRIDE_TIM_BASE, TIMER_TIMA_TIMEOUT);

	// Turn the input pin to the button high
	GPIOPinTypeGPIOOutput(BTN_OVERRIDE_CONTROL_REG, BTN_OVERRIDE_CONTROL);
	GPIOPinWrite(BTN_OVERRIDE_CONTROL_REG, BTN_OVERRIDE_CONTROL, BTN_OVERRIDE_CONTROL);
}

/*****************************************************
 * 	Function: btn_IntHandler
 *	Description: Interrupt handler for button press
 *			Handles mode switching on release
 *			Hold for 2 seconds -> Override mode
 *	Input:
 *	Output:
 *****************************************************/
void btn_IntHandler(void)
{
	// Clear interrupt
	GPIOIntClear(BTN_OVERRIDE_REG, BTN_OVERRIDE);

	// Get current button status
	ButtonStatus buttonStatus = GPIOPinRead(BTN_OVERRIDE_REG, BTN_OVERRIDE);

	// If button is current pressed
	if(buttonStatus == BUTTON_PRESSED)
	{
		hasSeenButtonPress = true;									// Flag that we saw a button press
		TimerEnable(BTN_OVERRIDE_TIM_BASE, BTN_OVERRIDE_TIM);		// Start timer to count how long button is pressed
		GPIOPinWrite(LED_REG, LED_BLUE_PIN, LED_BLUE_PIN);			// Turn on LED to signal button press
	}
	// If button is not pressed and we have seen a button press
	else if (buttonStatus == BUTTON_RELEASED && hasSeenButtonPress)
	{
		hasSeenButtonPress = false;									// Clear button press flag
		TimerDisable(BTN_OVERRIDE_TIM_BASE, BTN_OVERRIDE_TIM);		// Disable timer counting button press
		GPIOPinWrite(LED_REG, LED_BLUE_PIN, LED_ALL_OFF);			// Turn off LED signaling button release

		uint32_t elapsedTime = buttonPressTimerTicks / 1000;		// Calculate time button was held
		buttonPressTimerTicks = 0;									// Clear counter

		// OVERRIDE - 2 second press
		if((elapsedTime) > 1 && (elapsedTime) < 3)
		{
			switch(mode)
			{
				case RUN:
					mode = OVERRIDE;				// If we are in the run state, set to override
					flashLED(LED_RED_PIN);
					oneSecondCounter = 0;			// Clear the 24 hour counter
					break;
				case OVERRIDE:
					mode = RUN;						// If we are in the override state, clear the override
					clearAllZoneOverrides();		// Clear the zone overrides
					flashLED(LED_GREEN_PIN);
					break;
				default:
					// If we are in another state ignore the button press
					break;
			}

		}

	}

}

/*****************************************************
 * 	Function: btn_TimHandler
 *	Description: Interupt handler for button timer
 *	Input: NONE
 *	Output: Increments buttonPressTimerTicks
 *****************************************************/
void btn_TimHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);			// Clear the timer interrupt
    buttonPressTimerTicks++;
}

/*****************************************************
 * 	Function: checkZoneStatus
 *	Description: Checks zone control times against
 *			current time for each zone
 *	Input: NONE
 *	Output: Modifies currentZone->Status
 *****************************************************/
void checkZoneStatus(void)
{
	// For each of the zones
	int zoneNumber;
	for(zoneNumber=0; zoneNumber<NUMBER_OF_ZONES; zoneNumber++)
	{
		Zone* currentZone = Zones[zoneNumber];

		// Get values for this zone
		uint32_t onTimeMinutes = (currentZone->OnMinute + currentZone->OnHour*60);
		uint32_t onLengthMinutes = currentZone->OnLength;
		uint32_t currentTimeMinutes = dateTime.minute + dateTime.hour*60;

		// Check if zone should be on
		if((currentTimeMinutes >= onTimeMinutes) &&
			(currentTimeMinutes < (onTimeMinutes + onLengthMinutes)))	//* TARGET_REL_HUM/HIH6130_GetHumidity()
		{
			currentZone->Status = ZONE_ON;
		}
		// Otherwise it should be off
		else
		{
			currentZone->Status = ZONE_OFF;
		}
	}


}

/*****************************************************
 * 	Function: processZones
 *	Description: Processes each zone's status
 *			and modifies GPIO
 *	Input: NONE
 *	Output: Modifies GPIO
 *****************************************************/
void processZones(void)
{
	// For each of the zones
	int zoneNumber;
	for(zoneNumber=0; zoneNumber<NUMBER_OF_ZONES; zoneNumber++)
	{
		Zone* currentZone = Zones[zoneNumber];

		// Check if zone should be on and is not currently in override and system is not currently in shutdown
		if(currentZone->Status == ZONE_ON && currentZone->OverrideStatus != OVERRIDE_ACTIVE)
		{
			// Turn zone on = write pin LOW
			//zones[zonePointer][IND_ZONE_REG] |= zones[zonePointer][IND_ZONE_PINNUM];
			GPIOPinWrite(currentZone->Port, currentZone->Pin, 0);
		}
		// Otherwise it should be off
		else
		{
			// Turn zone off = write pin HIGH
			//zones[zonePointer][IND_ZONE_REG] &= ~zones[zonePointer][IND_ZONE_PINNUM];
			GPIOPinWrite(currentZone->Port, currentZone->Pin, currentZone->Pin);
		}

	}
}

/*****************************************************
 * 	Function: clearAllZoneOverrides
 *	Description: Clears Override Status for every zone
 *	Input: NONE
 *	Output: Modifies Zones[]->OverrideStatus
 *****************************************************/
void clearAllZoneOverrides(void)
{
	int zoneNumber;
	// Iterate through all zones
	for(zoneNumber=0; zoneNumber<NUMBER_OF_ZONES; zoneNumber++)
	{
		// Set zone override status to inactive
		Zones[zoneNumber]->OverrideStatus = OVERRIDE_INACTIVE;
	}
}

/*****************************************************
 * 	Function: setAllZoneOverrides
 *	Description: Sets Override Status for every zone
 *	Input: NONE
 *	Output: Modifies Zones[]->OverrideStatus
 *****************************************************/
void setAllZoneOverrides(void)
{
	int zoneNumber;
	// Iterate through zones
	for(zoneNumber=0; zoneNumber<NUMBER_OF_ZONES; zoneNumber++)
	{
		// Set zone override status to active
		Zones[zoneNumber]->OverrideStatus = OVERRIDE_ACTIVE;
	}
}

/*****************************************************
 * 	Function: printCurrentStatus
 *	Description: Prints current system status to UART
 *	Input: NONE
 *	Output: Prints to UART
 *****************************************************/
void printCurrentStatus(void)
{
//	UARTCharPut(UART0_BASE, '[');
//	UARTCharPut(UART0_BASE, '2');
//	UARTCharPut(UART0_BASE, 'J');
//	UARTCharPut(UART0_BASE, '[');
//	UARTCharPut(UART0_BASE, 'H');

	//UART_PrintMessage(message, 6, false);
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, 'I');
	UARTCharPut(UART0_BASE, 'M');
	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
	UART_PrintLong((long)dateTime.hour);
	//UART_PrintMessage(semiColon, 1, false);
	UARTCharPut(UART0_BASE, ':');
	UART_PrintLong((long)dateTime.minute);
	//UART_PrintMessage(semiColon, 1, false);
	UARTCharPut(UART0_BASE, ':');
	UART_PrintLong((long)dateTime.second);
	//UART_PrintMessage(semiColon, 0, true);
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

	UARTCharPut(UART0_BASE, 'I');
	UARTCharPut(UART0_BASE, 'N');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, 'M');
	UARTCharPut(UART0_BASE, 'P');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
	UART_PrintLong((long)ui32TempValueF);
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

//	UARTCharPut(UART0_BASE, 'H');
//	UARTCharPut(UART0_BASE, 'I');
//	UARTCharPut(UART0_BASE, 'H');
//	UARTCharPut(UART0_BASE, ' ');
//	UARTCharPut(UART0_BASE, 'S');
//	UARTCharPut(UART0_BASE, 'T');
//	UARTCharPut(UART0_BASE, 'A');
//	UARTCharPut(UART0_BASE, 'T');
//	UARTCharPut(UART0_BASE, 'U');
//	UARTCharPut(UART0_BASE, 'S');
//	UARTCharPut(UART0_BASE, ':');
//	UARTCharPut(UART0_BASE, ' ');
//	UART_PrintLong((long)HIH6130_GetStatus());
//	UARTCharPut(UART0_BASE, ' ');
//	UARTCharPut(UART0_BASE, '\r');
//	UARTCharPut(UART0_BASE, '\n');

	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, 'X');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, 'M');
	UARTCharPut(UART0_BASE, 'P');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
	UART_PrintLong((long)HIH6130_GetTemperature());
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

	UARTCharPut(UART0_BASE, 'R');
	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, 'L');
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, 'H');
	UARTCharPut(UART0_BASE, 'U');
	UARTCharPut(UART0_BASE, 'M');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
	UART_PrintLong((long)HIH6130_GetHumidity());
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

	UARTCharPut(UART0_BASE, 'M');
	UARTCharPut(UART0_BASE, 'O');
	UARTCharPut(UART0_BASE, 'I');
	UARTCharPut(UART0_BASE, 'S');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, 'U');
	UARTCharPut(UART0_BASE, 'R');
	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
	UART_PrintLong((long)ui32MoistureAvg);
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

//	UARTCharPut(UART0_BASE, 'C');
//	UARTCharPut(UART0_BASE, 'Y');
//	UARTCharPut(UART0_BASE, 'C');
//	UARTCharPut(UART0_BASE, 'L');
//	UARTCharPut(UART0_BASE, 'E');
//	UARTCharPut(UART0_BASE, 'S');
//	UARTCharPut(UART0_BASE, ':');
//	UARTCharPut(UART0_BASE, ' ');
//	UART_PrintLong((long)numberOfSwitches);
//	UARTCharPut(UART0_BASE, ' ');
//	UARTCharPut(UART0_BASE, '\r');
//	UARTCharPut(UART0_BASE, '\n');


	int zoneNumber;
	for(zoneNumber=0; zoneNumber < NUMBER_OF_ZONES; zoneNumber++)
	{
		Zone* currentZone = Zones[zoneNumber];

		//UART_PrintMessage(zone, 5, false);
		UARTCharPut(UART0_BASE, 'Z');
		UARTCharPut(UART0_BASE, 'O');
		UARTCharPut(UART0_BASE, 'N');
		UARTCharPut(UART0_BASE, 'E');
		UARTCharPut(UART0_BASE, ' ');
		UART_PrintLong(zoneNumber);

		//UART_PrintMessage(semiColon, 2, false);
		UARTCharPut(UART0_BASE, ':');
		UARTCharPut(UART0_BASE, ' ');

		if(currentZone->OverrideStatus == OVERRIDE_ACTIVE)
		{
			UARTCharPut(UART0_BASE, 'O');
			UARTCharPut(UART0_BASE, 'V');
			UARTCharPut(UART0_BASE, 'E');
			UARTCharPut(UART0_BASE, 'R');
			UARTCharPut(UART0_BASE, 'I');
			UARTCharPut(UART0_BASE, 'D');
			UARTCharPut(UART0_BASE, 'E');
		}
		else if(currentZone->Status == ZONE_ON)//ZoneStatus.ZONE_ON)
		{
			//UART_PrintMessage(zoneActive, 6, true);
			UARTCharPut(UART0_BASE, 'A');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'I');
			UARTCharPut(UART0_BASE, 'V');
			UARTCharPut(UART0_BASE, 'E');
		}
		else
		{
			//UART_PrintMessage(zoneNotActive, 10, true);
			UARTCharPut(UART0_BASE, 'N');
			UARTCharPut(UART0_BASE, 'O');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, ' ');
			UARTCharPut(UART0_BASE, 'A');
			UARTCharPut(UART0_BASE, 'C');
			UARTCharPut(UART0_BASE, 'T');
			UARTCharPut(UART0_BASE, 'I');
			UARTCharPut(UART0_BASE, 'V');
			UARTCharPut(UART0_BASE, 'E');
		}

		UARTCharPut(UART0_BASE, '\r');
		UARTCharPut(UART0_BASE, '\n');
	}

	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

}

/*****************************************************
 * 	Function: delay
 *	Description: Delays milliseconds
 *	Input: (uint32_t) milliSeconds - Time in ms to delay
 *	Output: NONE
 *****************************************************/
void delay(uint32_t milliSeconds)
{
	uint32_t counter;
	for(counter=0; counter<TICKS_IN_x_mSECS(milliSeconds); )
	{
		// Do nothing
		counter++;
	}
}

/*****************************************************
 * 	Function: flashLED
 *	Description: Flashes specified LED ON, OFF, ON, OFF
 *			Requires 30 ms delay time to complete
 *	Input: (uint8_t) led - GPIO pin of led to flash
 *	Output: Modifies GPIO
 *****************************************************/
void flashLED(uint8_t led)
{
	GPIOPinWrite(LED_REG, led, led);	// Flash red LED to signal override mode activated
	delay(10);
	GPIOPinWrite(LED_REG, led, LED_ALL_OFF);
	delay(10);
	GPIOPinWrite(LED_REG, led, led);
	delay(10);
	GPIOPinWrite(LED_REG, led, LED_ALL_OFF);
}






