#pragma once    
#include <math.h>
#include "wheels.h"

//Model
#define WHEEL_MODEL MSX_MSP_RS

//Model Specific
#if WHEEL_MODEL == CUSTOM
#define NUM_LEDS_TOTAL 23
#define NUM_STOP_LEDS_MAIN 5
#define NUM_STOP_LEDS_SECONDARY 0
#define NUM_BRAKE_LEDS 9
#elif WHEEL_MODEL == COMMANDER //total LEDs 5+9+9=23
#define NUM_LEDS_TOTAL 23
#define NUM_STOP_LEDS_MAIN 5
#define NUM_STOP_LEDS_SECONDARY 0
#define NUM_BRAKE_LEDS 9 
#elif WHEEL_MODEL == MSX_MSP_RS //total LEDs 5+18+15+15=53; not sure about RS btw...
#define NUM_LEDS_TOTAL 53
#define NUM_STOP_LEDS_MAIN 5
#define NUM_STOP_LEDS_SECONDARY 18
#define NUM_BRAKE_LEDS 15
#elif WHEEL_MODEL == MSX_OLD //without leds around the stop light, so total LEDS: 5+15+15=35
#define NUM_LEDS_TOTAL 35
#define NUM_STOP_LEDS_MAIN 5
#define NUM_STOP_LEDS_SECONDARY 0
#define NUM_BRAKE_LEDS 15
#endif

//Custom Firmware
#define CUSTOM_FIRMWARE 0

//Front Light
#define AUTO_LIGHT 1
#define LIGHT_FADE_SPEED 50 //0 - 65535
#define LIGHT_OFF_DELAY 60000
#define LIGHT_STILL_DUTY 0.15f //0.0f - 1.0f
#define LIGHT_MOVING_DUTY 1.0f //0.0f - 1.0f

//LEDs
#define RAINBOW_STOP_LIGHT 1 //if not moving and rainbow mode is enabled
#define MIN_RAINBOW_SPEED 9
#define RAINBOW_SPEED_MULTIPLIER 1.5f
#define BRAKE_BLINK_DELAY 100
#define BRAKE_LIGHT_ACCELERATION_THRESHOLD -5.f

//Cooler
#define TEMP_THRESHOLD_MOVE 45.f
#define TEMP_THRESHOLD_STILL_START 49.5f
#define TEMP_THRESHOLD_STILL_STOP 47.f

//UART
#define BEGODE_FRAME_SIZE 24U

//Display
#define BIT_DELAY 10

//Other
#define MOVING_SPEED_THRESHOLD 6.f

#ifdef __cplusplus
namespace Settings
{
    inline constexpr size_t iMinBufSpace = BEGODE_FRAME_SIZE * (CUSTOM_FIRMWARE ? 4 : 2);
    inline constexpr uint16_t iLightStillDuty = std::round(LIGHT_STILL_DUTY * 65535);
    inline constexpr uint16_t iLightMovingDuty = std::round(LIGHT_MOVING_DUTY * 65535);
    inline constexpr uint16_t iRainbowAddHue = (65535 / (NUM_LEDS_TOTAL / 2));
    inline constexpr uint16_t iNumStopLedsTotal = NUM_STOP_LEDS_MAIN + NUM_STOP_LEDS_SECONDARY;
    inline constexpr uint16_t iNumLEDsForBraking = iNumStopLedsTotal + NUM_BRAKE_LEDS;
    inline constexpr uint16_t iRainbowStart = (RAINBOW_STOP_LIGHT ? 0 : Settings::iNumStopLedsTotal);
    inline constexpr uint16_t iBackPartLeds = iNumStopLedsTotal + ((NUM_LEDS_TOTAL - iNumStopLedsTotal) / 2);
}
#endif