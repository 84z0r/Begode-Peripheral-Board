#pragma once
#include <inttypes.h>
#include "main.h"

namespace Begode
{
    class __attribute__((__packed__)) DisplayData
    {
    private:
        uint8_t seg_temp_2;
        uint8_t seg_temp_1;
        uint8_t seg_pedals_mode;
        uint8_t seg_temp_0;
        uint8_t seg_total_5;
        uint8_t seg_total_4;
        uint8_t seg_total_3;
        uint8_t seg_total_2;
        uint8_t seg_total_1;
        uint8_t seg_total_0;
        uint8_t seg_speed_0;
        uint8_t seg_speed_1;
        uint8_t seg_battery_pic;
        uint8_t seg_charge;
    public:
        DisplayData();
        void setTemp(float temp);
        void setTotal(float total);
        void setPedalsMode(uint8_t mode);
        void setSpeed(unsigned int speed);
        void setCharge(uint8_t charge);
        void setBatteryPic(bool bEnable);
    };

    class Display
    {
    public:
        Display();
        void onSetup();
        void update(const DisplayData* data);
        void setBrightness(uint8_t value); //0 - 7
    private:
        void bitDelay();
        void start();
        void stop();
        void writeByte(uint8_t data);
        void writeBytes(const DisplayData* display_data);
        void writeBit(bool i);
        void waitAck();
        uint16_t m_pinCLK;
        uint16_t m_pinDIO;
        GPIO_TypeDef* m_pinCLK_grp;
        GPIO_TypeDef* m_pinDIO_grp;
        uint8_t m_brightness;
    };
}