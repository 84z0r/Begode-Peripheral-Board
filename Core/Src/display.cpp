#include "display.h"
#include "settings.h"
#include <stdio.h>

#define DRAW_MODE_INCREMENTAL   0x40
#define DRAW_MODE_FIXED         0x44
#define DRAW_START_ADDRESS      0xC0
#define DRAW_BRIGHTNESS         0x88
#define SYSTICK_LOAD (SystemCoreClock/1000000U)

const uint8_t digToSegment[13] =
{
    0b10000001, //-
    0b10000000, //.
    0b10000000, ///
    0b11111110, //0
    0b10110000, //1
    0b11101101, //2
    0b11111001, //3
    0b10110011, //4
    0b11011011, //5
    0b11011111, //6
    0b11110000, //7
    0b11111111, //8
    0b11111011  //9
};

Begode::Display::Display() : m_pinCLK(GPIO_PIN_2), m_pinDIO(GPIO_PIN_3), m_pinCLK_grp(GPIOA), m_pinDIO_grp(GPIOA), m_brightness(7)
{
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, GPIO_PIN_SET);
}

void Begode::Display::setBrightness(uint8_t value)
{
    m_brightness = value % 8;
}

void Begode::Display::start()
{
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_SET);
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, GPIO_PIN_SET);
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, GPIO_PIN_RESET);
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_RESET);
}

void Begode::Display::stop()
{
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, GPIO_PIN_RESET);
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_SET);
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, GPIO_PIN_SET);
}

void Begode::Display::bitDelay()
{
    uint32_t m = DWT->CYCCNT;
    if (BIT_DELAY)
    {
        uint32_t e = m + (BIT_DELAY * SYSTICK_LOAD);
        if (m > e)
        {  //overflow
            while (DWT->CYCCNT > e)
            {
                asm volatile("nop");
            }
        }
        while (DWT->CYCCNT < e)
        {
            asm volatile("nop");
        }
    }
}

void Begode::Display::waitAck()
{
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, GPIO_PIN_RESET);
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_SET);
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_RESET);
}

void Begode::Display::writeBit(bool bit)
{
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(m_pinDIO_grp, m_pinDIO, static_cast<GPIO_PinState>(bit));
    this->bitDelay();
    HAL_GPIO_WritePin(m_pinCLK_grp, m_pinCLK, GPIO_PIN_SET);
    this->bitDelay();
}

void Begode::Display::writeByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        this->writeBit(data & 0x01);
        data >>= 1;
    }

    this->waitAck();
}

void Begode::Display::writeBytes(const DisplayData* display_data)
{
    const uint8_t* data = reinterpret_cast<const uint8_t*>(display_data);
    for (uint8_t i = 0; i < sizeof(DisplayData); ++i)
    {
        uint8_t part = data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            this->writeBit(part & 0x01);
            part >>= 1;
        }
    }

    this->waitAck();
}

void Begode::Display::update(const DisplayData* data)
{
    this->start();
    this->writeByte(DRAW_MODE_INCREMENTAL);
    this->stop();
    this->start();
    this->writeByte(DRAW_START_ADDRESS);
    this->writeBytes(data);
    this->stop();
    this->start();
    this->writeByte(DRAW_BRIGHTNESS | this->m_brightness);
    this->stop();
}

Begode::DisplayData::DisplayData() :
    seg_temp_2(0b11111110),
    seg_temp_1(0b11111110),
    seg_pedals_mode(0b11111110),
    seg_temp_0(0b11111110),
    seg_total_5(0b11111110),
    seg_total_4(0b11111110),
    seg_total_3(0b11111110),
    seg_total_2(0b11111110),
    seg_total_1(0b11111110),
    seg_total_0(0b11111110),
    seg_speed_0(0b11111110),
    seg_speed_1(0b11111110),
    seg_battery_pic(0b11111111),
    seg_charge(0b11111111)
{}

void Begode::DisplayData::setTemp(float temp)
{
    int iTemp = std::round(temp * 10.f);
    char s[16] = { 0 };
    int written = snprintf(s, sizeof(s), "%03d", iTemp);

    if (written < 3)
        return;

    this->seg_temp_0 = digToSegment[s[written - 3] - '-'];
    this->seg_temp_1 = digToSegment[s[written - 2] - '-'];
    this->seg_temp_2 = digToSegment[s[written - 1] - '-'];
}

void Begode::DisplayData::setTotal(float total)
{
    int iTotal = std::round(total * 10.f);
    char s[16] = { 0 };
    int written = snprintf(s, sizeof(s), "%06d", iTotal);

    if (written < 6)
        return;

    this->seg_total_0 = digToSegment[s[written - 6] - '-'];
    this->seg_total_1 = digToSegment[s[written - 5] - '-'];
    this->seg_total_2 = digToSegment[s[written - 4] - '-'];
    this->seg_total_3 = digToSegment[s[written - 3] - '-'];
    this->seg_total_4 = digToSegment[s[written - 2] - '-'];
    this->seg_total_5 = digToSegment[s[written - 1] - '-'];
}

void Begode::DisplayData::setSpeed(unsigned int speed)
{
    char s[16] = { 0 };
    int written = snprintf(s, sizeof(s), "%02d", speed);

    if (written < 2)
        return;

    this->seg_speed_0 = digToSegment[s[written - 2] - '-'];
    this->seg_speed_1 = digToSegment[s[written - 1] - '-'];
}

void Begode::DisplayData::setCharge(uint8_t charge)
{
    if (charge > 80) this->seg_charge = 0b11111111;
    else if (charge > 60) this->seg_charge = 0b10111111;
    else if (charge > 40) this->seg_charge = 0b10011111;
    else if (charge > 20) this->seg_charge = 0b10001111;
    else if (charge > 0) this->seg_charge = 0b10000111;
    else this->seg_charge = 0b10000011;
}

void Begode::DisplayData::setBatteryPic(bool bEnable)
{
    this->seg_battery_pic = bEnable ? 0b11111111 : 0b10000000;
}

void Begode::DisplayData::setPedalsMode(uint8_t mode)
{
    this->seg_pedals_mode = digToSegment[mode % 10];
}