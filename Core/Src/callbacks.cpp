#include "callbacks.h"
#include "begode.h"

Begode::Hardware& Hardware = Begode::Hardware::Get();

void Setup()
{
    Hardware.onSetup();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size)
{
    Hardware.onRxEventCallback(Size);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef* htim)
{
    Hardware.processLightPWM();
    Hardware.processLEDs();
}