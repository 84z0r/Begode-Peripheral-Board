#pragma once
#include "singleton.h"
#include "display.h"
#include "settings.h"
#include "ws28xx.h"

namespace Begode
{
    struct __attribute__((__packed__)) Frame_00
    {
        char _pad_0[2];
        unsigned short voltage;
        short speed;
        char _pad_6[2];
        unsigned short distance;
        short phaseCurrent;
        short temperature;
        short hardwarePWM;
        char _pad_16[2];
        uint8_t frameType_00;
        char _pad_19[5];
    };

    struct __attribute__((__packed__)) Frame_04
    {
        char _pad_0[2];
        unsigned int totalDistance;
        unsigned short settings;
        unsigned short powerOffTime;
        unsigned short tiltBackSpeed;
        char _pad_12;
        uint8_t ledMode;
        uint8_t alert;
        uint8_t lightMode;
        char _pad_16[2];
        uint8_t frameType_04;
        char _pad_19[5];
    };

#if CUSTOM_FIRMWARE
    struct __attribute__((__packed__)) Frame_01
    {
        char _pad_0[6];
        int8_t pedalsMode;
        char _pad_7[11];
        uint8_t frameType_01;
        char _pad_19[5];
    };
#endif

    enum class LED_Mode : uint8_t
    {
        RAINBOW_FULL,
        RAINBOW,
        WHITE_RED,
        OFF,
        NUM_MODES
    };

#if CUSTOM_FIRMWARE
    inline LED_Mode operator++(LED_Mode& mode)
    {
        uint8_t lm = static_cast<uint8_t>(mode);
        mode = static_cast<LED_Mode>(++lm % static_cast<uint8_t>(LED_Mode::NUM_MODES));
        return mode;
    }

    struct __attribute__((__packed__)) EEPROM
    {
        uint8_t ledMode;
    };
#endif

    enum class Light_Mode : uint8_t
    {
#if AUTO_LIGHT || CUSTOM_FIRMWARE
        AUTO,
        ON,
        OFF,
#else
        OFF,
        ON,
        STROBE,
#endif
        NUM_MODES
    };

    class WheelData
    {
    public:
        float speed;
        float absSpeed;
        float prevAbsSpeed;
        float acceleration;
        float temperature;
        //float hardwarePWM;
        float total_distance;
        unsigned long nowTime;
        unsigned long prevTime;
        unsigned long timeDelta;
        unsigned long lastMovingTime;
        bool bIsBraking;
        uint8_t battery;
        uint8_t pedalsMode;
        LED_Mode ledMode;
        Light_Mode lightMode;
#if CUSTOM_FIRMWARE
        EEPROM eeprom;
        uint8_t prevPedalsMode;
        void update(const Frame_01* pFrame_01);
#endif
        void update(const Frame_00* pFrame_00);
        void update(const Frame_04* pFrame_04);

    };

    class Hardware : public Singleton<Hardware>
    {
    public:
        Hardware();
        WheelData wheelData;

        void onSetup();
        void onLoop();

    private:
        bool dataUpdate();
        void processDisplay();
        void processLight();
        void processCooler() const;
        void processLightPWM() const;
        void processLEDs();
        void ledStopLight();
        void ledModeOff();
        void ledModeRainbow(bool bRainbowStop);
        void ledModeWhiteRed();
        void ledBraking();

        uint8_t rx_buf[UART_DATA_PROCESS_BUFFER_SIZE];
        uint8_t circularBuffer[UART_CIRCULAR_BUFFER_SIZE];
        Display display;
        DisplayData displayData;
        WS28XX_HandleTypeDef LEDStrip;
        uint32_t lastCNDTR;
        uint32_t ledTick;
        uint32_t lastLedTick;
        uint16_t dataSize;
        uint16_t ledDelay;
        uint16_t targetDuty;
    };

    uint8_t calc_battery(unsigned short voltage);
}