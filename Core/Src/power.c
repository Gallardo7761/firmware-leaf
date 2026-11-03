/**
 * @file    power.c
 * @author  Jose
 * @date    23/9/26
 * @brief   Energy saving handling
 * @project firmware-leaf
 */

#include "power.h"

#include "stm32f1xx_hal.h"

extern RTC_HandleTypeDef hrtc;
extern void SystemClock_Config(void);

bool Power_Enter_Stop_Mode(uint32_t seconds, volatile bool *abort_flag)
{
    RTC_TimeTypeDef now = {0};
    RTC_AlarmTypeDef alarm = {0};

    // deactivate the alarm (also clears the Alarm Flag)
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    // set the alarm 'seconds' seconds from now, wrapping at midnight
    HAL_RTC_GetTime(&hrtc, &now, RTC_FORMAT_BIN);
    uint32_t wake = (now.Hours * 3600U + now.Minutes * 60U + now.Seconds + seconds) % 86400U;
    alarm.AlarmTime.Hours = wake / 3600U;
    alarm.AlarmTime.Minutes = (wake % 3600U) / 60U;
    alarm.AlarmTime.Seconds = wake % 60U;
    alarm.Alarm = RTC_ALARM_A;
    // also enables EXTI line 17, the one that wakes the chip from STOP
    HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);

    __disable_irq();
    // the ISR may have set the flag after the caller checked it
    if (abort_flag != NULL && *abort_flag) {
        __enable_irq();
        HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
        return false;
    }

    // disable TIM1 update interrupt
    HAL_SuspendTick();

    // enter STOP mode with low power regulator
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    // -----------------------------------
    // here the node should be sleeping...
    // -----------------------------------

    // restore main oscilator on wake up
    SystemClock_Config();

    // resume TIM1 update interrupt
    HAL_ResumeTick();

    // pending EXTI/RTC ISR runs here, when the clocks have been restored again
    __enable_irq();

    // deactivate the alarm to avoid random interrupts
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

    return true;
}
