#pragma once
#include <cmath>
#include "NimaLTD.I-CUBE-WS28XX_conf.h"

//Custom Firmware
#define CUSTOM_FIRMWARE 0

//Front Light
#define AUTO_LIGHT 1

//UART
#define BEGODE_FRAME_SIZE 24U

//Display
#define BIT_DELAY 10

//LEDs
#define NUM_LEDS_TOTAL WS28XX_PIXEL_MAX
#define NUM_BRAKE_LEDS 9
#define MIN_RAINBOW_SPEED 9

//Back LEDs
#define NUM_BACK_LEDS 5
#define BRAKE_BLINK_DELAY 100
#define BRAKE_LIGHT_ACCELERATION_THRESHOLD -5.f

//Cooler
#define TEMP_THRESHOLD_MOVE 45.f
#define TEMP_THRESHOLD_STILL_START 49.5f
#define TEMP_THRESHOLD_STILL_STOP 47.f

//Front Light
#define LIGHT_FADE_SPEED 50 //0 - 65535
#define LIGHT_OFF_DELAY 60000
#define LIGHT_STILL_DUTY 0.15f //0.0f - 1.0f
#define LIGHT_MOVING_DUTY 1.0f //0.0f - 1.0f

//Other
#define MOVING_SPEED_THRESHOLD 6.f

namespace Settings
{
    inline constexpr size_t iMinBufSpace = BEGODE_FRAME_SIZE * (CUSTOM_FIRMWARE ? 4 : 2);
    inline constexpr uint16_t iLightStillDuty = std::round(LIGHT_STILL_DUTY * 65535);
    inline constexpr uint16_t iLightMovingDuty = std::round(LIGHT_MOVING_DUTY * 65535);
    inline constexpr uint16_t iRainbowAddHue = (65535 / (NUM_LEDS_TOTAL / 2));
    inline constexpr uint8_t iNumLEDsForBraking = NUM_BACK_LEDS + NUM_BRAKE_LEDS;
}