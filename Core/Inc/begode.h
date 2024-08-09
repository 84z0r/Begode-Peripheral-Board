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
        char _pad_36;
        uint8_t ledMode;
        uint8_t alert;
        uint8_t lightMode;
        char _pad_40[2];
        uint8_t frameType_04;
        char _pad_43[5];
    };

    enum class LED_Mode : uint8_t
    {
        RAINBOW,
        OFF,
        NUM_MODES
    };

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
        float hardwarePWM;
        float total_distance;
        unsigned long nowTime;
        unsigned long prevTime;
        unsigned long timeDelta;
        unsigned long lastMovingTime;
        uint8_t battery;
        uint8_t pedalsMode;
        LED_Mode ledMode;
        Light_Mode lightMode;

        void update(const Frame_00* pFrame_00);
        void update(const Frame_04* pFrame_04);
    };

    class Hardware : public Singleton<Hardware>
    {
    public:
        Hardware();
        WheelData wheelData;

        void onSetup();
        void onRxEventCallback(uint16_t Size);
        bool dataUpdate(uint16_t Size);
        void processDisplay();
        void processLight();
        void processLightPWM() const;
        void processCooler() const;
        void processLEDs();
        void ledModeOff();
        void ledModeRainbow();
        void ledBraking();

    private:
        uint8_t rx_buf[1024];
        Display display;
        DisplayData displayData;
        WS28XX_HandleTypeDef LEDStrip;
        size_t dataSize;
        uint16_t targetDuty;
    };

    uint8_t calc_battery(unsigned short voltage);
}