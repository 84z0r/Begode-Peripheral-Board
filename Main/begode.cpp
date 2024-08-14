#include "begode.h"
#include "tools.h"
#include "usart.h"
#include "color.h"
#include "string.h"

const uint8_t frame00_Pattern[BEGODE_FRAME_SIZE] = { 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A };
const uint8_t frame04_Pattern[BEGODE_FRAME_SIZE] = { 0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x5A, 0x5A, 0x5A, 0x5A };
const uint8_t frame_Mask[BEGODE_FRAME_SIZE] = { 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };

Begode::Hardware::Hardware() : lastCNDTR(sizeof(this->circularBuffer)), ledTick(0U), lastLedTick(0U), dataSize(0U), ledDelay(0U), targetDuty(0U) {}

void Begode::Hardware::onSetup()
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }

    HAL_UART_Receive_DMA(&huart1, this->circularBuffer, sizeof(this->circularBuffer));
    WS28XX_Init(&this->LEDStrip, &htim2, 72, TIM_CHANNEL_2, NUM_LEDS_TOTAL);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

void Begode::Hardware::onLoop()
{
    if (this->dataUpdate())
    {
        this->processDisplay();
        this->processLight();
        this->processCooler();
    }

    this->ledTick = HAL_GetTick();
    this->ledDelay = this->ledTick - this->lastLedTick;
    if (this->ledDelay >= Settings::iMinLedDelay)
    {
        this->processLightPWM();
        this->processLEDs();
        this->lastLedTick = this->ledTick;
    }
}

bool Begode::Hardware::dataUpdate()
{
    uint32_t CNDTR = huart1.hdmarx->Instance->CNDTR;
    int iNewSizeTotal = this->lastCNDTR - CNDTR;
    if (iNewSizeTotal < 0) iNewSizeTotal += sizeof(this->circularBuffer);
    if (!iNewSizeTotal)
        return false;

    if (int(sizeof(this->rx_buf) - this->dataSize) < iNewSizeTotal)
        this->dataSize = 0U;

    if (CNDTR > this->lastCNDTR)
    {
        memcpy(this->rx_buf + this->dataSize, this->circularBuffer + (sizeof(this->circularBuffer) - this->lastCNDTR), this->lastCNDTR);
        this->dataSize += this->lastCNDTR;
        this->lastCNDTR = sizeof(this->circularBuffer);
    }

    uint16_t uSize = this->lastCNDTR - CNDTR;
    memcpy(this->rx_buf + this->dataSize, this->circularBuffer + (sizeof(this->circularBuffer) - this->lastCNDTR), uSize);
    this->dataSize += uSize;
    this->lastCNDTR = CNDTR;

    if (this->dataSize < BEGODE_FRAME_SIZE)
        return false;

    Frame_00* pFrame_00 = reinterpret_cast<Frame_00*>(Tools::FindPatternLast(this->rx_buf, this->dataSize, frame00_Pattern, sizeof(frame00_Pattern), frame_Mask));
    Frame_04* pFrame_04 = reinterpret_cast<Frame_04*>(Tools::FindPatternLast(this->rx_buf, this->dataSize, frame04_Pattern, sizeof(frame04_Pattern), frame_Mask));

    if (!pFrame_00 && !pFrame_04)
        return false;

    if (pFrame_00) this->wheelData.update(pFrame_00);
    if (pFrame_04) this->wheelData.update(pFrame_04);

    uint8_t* pFrameEnd = std::max(reinterpret_cast<uint8_t*>(pFrame_00), reinterpret_cast<uint8_t*>(pFrame_04)) + BEGODE_FRAME_SIZE;
    uint8_t* pDataEnd = this->rx_buf + dataSize;
    size_t sizeLeft = pDataEnd - pFrameEnd;
    if (sizeLeft) memcpy(this->rx_buf, pFrameEnd, sizeLeft);
    dataSize = sizeLeft;

    return true;
}

void Begode::Hardware::processDisplay()
{
    this->displayData.setTemp(this->wheelData.temperature);
    this->displayData.setTotal(this->wheelData.total_distance);
    this->displayData.setSpeed(static_cast<unsigned int>(std::round(this->wheelData.absSpeed)));
    this->displayData.setPedalsMode(this->wheelData.pedalsMode);
    this->displayData.setCharge(this->wheelData.battery);
    this->display.update(&this->displayData);
}

void Begode::Hardware::processLight()
{
    switch (this->wheelData.lightMode)
    {
#if AUTO_LIGHT || CUSTOM_FIRMWARE 
        case Light_Mode::AUTO:
        {
            if (this->wheelData.absSpeed > MOVING_SPEED_THRESHOLD)
                this->targetDuty = Settings::iLightMovingDuty;
            else
                this->targetDuty = (this->wheelData.lastMovingTime && ((this->wheelData.nowTime - this->wheelData.lastMovingTime) < LIGHT_OFF_DELAY)) ? Settings::iLightStillDuty : 0U;

            break;
        }
#else
        case Light_Mode::STROBE:
        {
            this->targetDuty = ((HAL_GetTick() % 500) < 250) ? 65535U : 0U;
            break;
        }
#endif
        case Light_Mode::ON:
        {
            this->targetDuty = (this->wheelData.absSpeed > MOVING_SPEED_THRESHOLD) ? Settings::iLightMovingDuty : Settings::iLightStillDuty;
            break;
        }
        case Light_Mode::OFF:
        {
            this->targetDuty = 0U;
            break;
        }
        default:
        break;
    }
}

void Begode::Hardware::processLightPWM() const
{
    uint32_t CCR = TIM3->CCR2;
    int diff = this->targetDuty - CCR;
    if (diff)
    {
#if !AUTO_LIGHT && !CUSTOM_FIRMWARE
        if (this->wheelData.lightMode != Light_Mode::STROBE)
        {
#endif
            int iSpeed = ((diff > 0) ? LIGHT_FADE_SPEED : -LIGHT_FADE_SPEED) * this->ledDelay;
            if (std::abs(diff) > std::abs(iSpeed)) TIM3->CCR2 = CCR + iSpeed;
            else TIM3->CCR2 = this->targetDuty;
#if !AUTO_LIGHT && !CUSTOM_FIRMWARE
        }
        else TIM3->CCR2 = this->targetDuty;
#endif
    }
}

void Begode::Hardware::processCooler() const
{
    static bool bCooler, bPrevCooler = false;

    if (this->wheelData.absSpeed > MOVING_SPEED_THRESHOLD) bCooler = this->wheelData.temperature > TEMP_THRESHOLD_MOVE;
    else
    {
        if (this->wheelData.temperature > TEMP_THRESHOLD_STILL_START)
            bCooler = true;
        else if (this->wheelData.temperature < TEMP_THRESHOLD_STILL_STOP)
            bCooler = false;
    }

    if (bCooler != bPrevCooler)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, static_cast<GPIO_PinState>(bCooler));
        bPrevCooler = bCooler;
    }
}

void Begode::Hardware::processLEDs()
{
    switch (this->wheelData.ledMode)
    {
        case LED_Mode::RAINBOW: this->ledModeRainbow(); break;
        case LED_Mode::WHITE_RED: this->ledModeWhiteRed(); break;
        case LED_Mode::OFF: this->ledModeOff(); break;
        default: break;
    }

    this->ledBraking();
    this->ledStopLight();
    WS28XX_Update(&this->LEDStrip);
}

void Begode::Hardware::ledBraking()
{
    static uint32_t lastSwitch = 0;
    static bool bBrakeState = true;
    uint32_t delay = this->ledTick - lastSwitch;

    if (((this->wheelData.acceleration < BRAKE_LIGHT_ACCELERATION_THRESHOLD) && (this->wheelData.prevAbsSpeed > 2.f)) || (this->wheelData.bIsBraking && (this->wheelData.absSpeed < this->wheelData.prevAbsSpeed)))
    {
        this->wheelData.bIsBraking = true;

        for (uint16_t i = 0; i < Settings::iNumLEDsForBraking; ++i)
            WS28XX_SetPixel_RGB(&this->LEDStrip, i, (bBrakeState ? 255 : 0), 0, 0);

        if (delay >= BRAKE_BLINK_DELAY)
        {
            bBrakeState = !bBrakeState;
            lastSwitch = this->ledTick;
        }
    }
    else
    {
        this->wheelData.bIsBraking = false;
        bBrakeState = true;
    }
}

void Begode::Hardware::ledStopLight()
{
    if (!Settings::iNumStopLedsTotal || this->wheelData.bIsBraking)
        return;

    if (this->wheelData.absSpeed > 2.f)
    {
        for (uint16_t i = 0; i < Settings::iNumStopLedsTotal; ++i)
            WS28XX_SetPixel_RGB(&this->LEDStrip, i, 128, 0, 0);
    }
    else
    {
#if RAINBOW_STOP_LIGHT
        if (this->wheelData.ledMode == LED_Mode::RAINBOW)
            return;
#endif
        float brightness = std::abs(((this->ledTick % 4001) * 0.0005f) - 1.f);
        Color::RGBColor color(std::round(BATTERY_CHARGE_COLOR_R * brightness), std::round(BATTERY_CHARGE_COLOR_G * brightness), std::round(BATTERY_CHARGE_COLOR_B * brightness));

        if (this->wheelData.battery > 80)
        {
            for (uint16_t i = 0; i < NUM_STOP_LEDS_MAIN; ++i)
                WS28XX_SetPixel_RGB(&this->LEDStrip, i, color.red, color.green, color.blue);
        }
        else if (this->wheelData.battery > 60)
        {
            WS28XX_SetPixel_RGB(&this->LEDStrip, 0, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 1, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 2, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 3, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 4, color.red, color.green, color.blue);
        }
        else if (this->wheelData.battery > 40)
        {
            WS28XX_SetPixel_RGB(&this->LEDStrip, 0, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 1, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 2, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 3, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 4, 0, 0, 0);
        }
        else if (this->wheelData.battery > 20)
        {
            WS28XX_SetPixel_RGB(&this->LEDStrip, 0, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 1, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 2, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 3, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 4, 0, 0, 0);
        }
        else if (this->wheelData.battery > 0)
        {
            WS28XX_SetPixel_RGB(&this->LEDStrip, 0, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 1, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 2, color.red, color.green, color.blue);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 3, 0, 0, 0);
            WS28XX_SetPixel_RGB(&this->LEDStrip, 4, 0, 0, 0);
        }
        else
        {
            for (uint16_t i = 0; i < NUM_STOP_LEDS_MAIN; ++i)
                WS28XX_SetPixel_RGB(&this->LEDStrip, i, 0, 0, 0);
        }

        for (uint16_t i = NUM_STOP_LEDS_MAIN; i < Settings::iNumStopLedsTotal; ++i)
            WS28XX_SetPixel_RGB(&this->LEDStrip, i, 128, 0, 0);
    }
}

void Begode::Hardware::ledModeRainbow()
{
    static uint16_t LEDStarter = 0U;

    for (uint16_t hue = LEDStarter, i = Settings::iRainbowStart; i < NUM_LEDS_TOTAL; i++, hue += Settings::iRainbowAddHue)
    {
        Color::RGBColor color = Color::HSVColor(hue, 255, 255).toRGB();
        WS28XX_SetPixel_RGB(&this->LEDStrip, i, color.red, color.green, color.blue);
    }

    int addStarter = std::round(this->wheelData.speed * RAINBOW_SPEED_MULTIPLIER) * this->ledDelay;
    if (std::abs(addStarter) < MIN_RAINBOW_SPEED)
        addStarter = (this->wheelData.speed >= 0.f) ? MIN_RAINBOW_SPEED : -MIN_RAINBOW_SPEED;

    LEDStarter += addStarter;
}

void Begode::Hardware::ledModeWhiteRed()
{
    for (uint16_t i = Settings::iNumStopLedsTotal; i < NUM_BACK_HALF_LEDS; ++i)
        WS28XX_SetPixel_RGB(&this->LEDStrip, i, 255, 0, 0);

    for (uint16_t i = NUM_BACK_HALF_LEDS; i < NUM_LEDS_TOTAL; ++i)
        WS28XX_SetPixel_RGB(&this->LEDStrip, i, 255, 255, 255);
}

void Begode::Hardware::ledModeOff()
{
    for (uint16_t i = Settings::iNumStopLedsTotal; i < NUM_LEDS_TOTAL; ++i)
        WS28XX_SetPixel_RGB(&this->LEDStrip, i, 0, 0, 0);
}

void Begode::WheelData::update(const Frame_00* pFrame_00)
{
    this->prevTime = this->nowTime;
    this->nowTime = HAL_GetTick();
    this->timeDelta = this->nowTime - this->prevTime;
    this->prevAbsSpeed = this->absSpeed;
#if !CUSTOM_FIRMWARE
    this->speed = 0.f - (Tools::byteSwap(pFrame_00->speed) * 0.036f);
    this->temperature = Tools::byteSwap(pFrame_00->temperature) / 340.0f + 36.53f; //mpu6050
#else
    this->speed = Tools::byteSwap(pFrame_00->speed) * 0.036f;
    this->temperature = Tools::byteSwap(pFrame_00->temperature) / 333.87f + 21.f; //mpu6500
    //this->hardwarePWM = Tools::byteSwap(pFrame_00->hardwarePWM) * 0.1f;
#endif
    this->absSpeed = std::abs(this->speed);
    if (this->absSpeed > MOVING_SPEED_THRESHOLD) this->lastMovingTime = this->nowTime;
    this->acceleration = (this->absSpeed - this->prevAbsSpeed) / (this->timeDelta * 0.001f);
    this->battery = calc_battery(Tools::byteSwap(pFrame_00->voltage));
}

void Begode::WheelData::update(const Frame_04* pFrame_04)
{
    this->total_distance = Tools::byteSwap(pFrame_04->totalDistance) * 0.001f;
    this->pedalsMode = ((Tools::byteSwap(pFrame_04->settings) >> 13) % 3) + 1;
    this->ledMode = static_cast<LED_Mode>(pFrame_04->ledMode % static_cast<uint8_t>(LED_Mode::NUM_MODES));
    this->lightMode = static_cast<Light_Mode>(pFrame_04->lightMode % static_cast<uint8_t>(Light_Mode::NUM_MODES));
}

uint8_t Begode::calc_battery(unsigned short voltage)
{
    if (voltage > 6680)
        return 100;
    else if (voltage > 5440)
        return static_cast<uint8_t>((voltage - 5380) / 13);
    else if (voltage > 5290)
        return static_cast<uint8_t>(std::round((voltage - 5290) / 32.5f));

    return 0;
}