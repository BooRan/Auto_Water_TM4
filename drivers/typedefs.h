/*
 * typedefs.h
 *
 *  Created on: Oct 21, 2013
 *      Author: Nick
 */

#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_


// SYSTEM CLOCK DEFINES
#define EXT_CRYSTAL_FREQ    16000000
#define SYSTEM_DIVISOR		4
#define SYSTEM_CLOCK		50000000				//EXT_CRYSTAL_FREQ/SYSTEM_DIVISOR
#define TICK_PER_SEC		SYSTEM_CLOCK
#define TICKS_IN_x_SECS(x)	(TICK_PER_SEC*x)
#define TICKS_IN_x_mSECS(x)	(TICK_PER_SEC/1000)*x
#define TICKS_IN_x_uSECS(x)	(TICK_PER_SEC/1000000)*x
#define SECONDS_IN_24_HOURS	86400

// LED DEFINES
#define LED_ALL_OFF			  0
// Defines below are the pins for the launchpad
#define LED_RED_PIN		  	  GPIO_PIN_1
#define LED_BLUE_PIN		  GPIO_PIN_2
#define LED_GREEN_PIN		  GPIO_PIN_3
#define LED_ALL_PINS		  LED_RED_PIN|LED_BLUE_PIN|LED_GREEN_PIN
#define LED_REG				  GPIO_PORTF_BASE


// Defines below are the pins for the launchpad
#define RED_LED_PIN_MASK      (0x02)
#define BLUE_LED_PIN_MASK     (0x04)
#define GREEN_LED_PIN_MASK    (0x08)
#define ALL_LEDS_MASK         (RED_LED_PIN_MASK | BLUE_LED_PIN_MASK | GREEN_LED_PIN_MASK)
#define LED_PORT_DATA_REG     (GPIO_PORTF_DATA_R)

#define RED_LED_ON				LED_PORT_DATA_REG |= RED_LED_PIN_MASK;
#define RED_LED_OFF				LED_PORT_DATA_REG &= ~RED_LED_PIN_MASK;
#define RED_LED_TOGGLE			LED_PORT_DATA_REG ^= RED_LED_PIN_MASK;
#define BLUE_LED_ON				LED_PORT_DATA_REG |= LED_BLUE_PIN;
#define BLUE_LED_OFF			LED_PORT_DATA_REG &= ~LED_BLUE_PIN;
#define BLUE_LED_TOGGLE			LED_PORT_DATA_REG ^= LED_BLUE_PIN;
#define GREEN_LED_ON			LED_PORT_DATA_REG |= LED_GREEN_PIN;
#define GREEN_LED_OFF			LED_PORT_DATA_REG &= ~LED_GREEN_PIN;
#define GREEN_LED_TOGGLE		LED_PORT_DATA_REG ^= LED_GREEN_PIN;

// BUTTON DEFINES
#define BTN_1_PIN					GPIO_PIN_4
#define BTN_2_PIN					GPIO_PIN_0
#define BTN_REG						GPIO_PORTF_BASE
#define BTN_OVERRIDE				GPIO_PIN_0
#define BTN_OVERRIDE_CONTROL		GPIO_PIN_2
#define BTN_OVERRIDE_REG			GPIO_PORTE_BASE
#define BTN_OVERRIDE_CONTROL_REG	GPIO_PORTB_BASE

// BUTTON TIMER DEFINES
#define BTN_OVERRIDE_TIM_BASE	TIMER0_BASE
#define BTN_OVERRIDE_TIM		TIMER_A

// I2C DEFINES
//#define SLAVE_ADDRESS 0x00
//#define SLAVE_DATA_ADDRESS 0x23
//#define INIT_REG 0x33
//#define INIT_MSB 0x01
//#define INIT_LSB 0xC0

typedef enum
{
	ZONE_ON = 0,
	ZONE_OFF = 1
} ZoneStatus;

typedef enum
{
	OVERRIDE_INACTIVE = 0,
	OVERRIDE_ACTIVE = 1
} OverrideStatus;

typedef enum
{
	RUN,
	OVERRIDE,
	SYSTEM_SHUTDOWN
} Mode;

typedef enum
{
	BUTTON_PRESSED = 0,
	BUTTON_RELEASED
} ButtonStatus;


typedef struct {
	ZoneStatus 		Status;
	OverrideStatus	OverrideStatus;
	uint32_t		Port;
	uint32_t 		Pin;
	uint32_t		OnHour;
	uint32_t		OnMinute;
	uint32_t		OnLength;
} Zone;

#define TRUE                  (1)
#define FALSE                 (0)
#define SET                   (1)
#define CLEAR                 (0)
#define ZERO                  (0)
#define ONE                   (1)
#define NULL                  ((void*)0)
#define OFF                   (0)
#define ON                    (1)

#endif /* TYPEDEFS_H_ */
