#include "motor.h"
#include "tim.h"
#include <stdlib.h>

/**
  * @brief  Initialize and start TIM8 PWM channels for motor control.
  */
void Motor_Init(void)
{
  // Start PWM on TIM8 Channels 1, 2, 3, and 4
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
  
  // Set initial speed to 0 (stopped)
  Motor_Stop();
}

/**
  * @brief  Set speed for a specific motor channel.
  * @param  motor_idx: MOTOR_LEFT or MOTOR_RIGHT
  * @param  speed: Target speed from -1000 (max reverse) to 1000 (max forward)
  */
void Motor_Set_Speed(uint8_t motor_idx, int16_t speed)
{
  // Constrain speed to min/max limits
  if (speed > MOTOR_SPEED_MAX)  speed = MOTOR_SPEED_MAX;
  if (speed < MOTOR_SPEED_MIN)  speed = MOTOR_SPEED_MIN;

  // Retrieve current timer Auto-Reload Register (ARR) value
  // This scales duty cycle dynamically even if the ARR is modified in CubeMX
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim8);
  uint32_t pulse = (abs(speed) * arr) / MOTOR_SPEED_MAX;

  if (motor_idx == MOTOR_LEFT)
  {
    if (speed >= 0)
    {
      // Forward: CH1 = Duty, CH2 = 0
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, pulse);
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
    }
    else
    {
      // Reverse: CH1 = 0, CH2 = Duty
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, pulse);
    }
  }
  else if (motor_idx == MOTOR_RIGHT)
  {
    if (speed >= 0)
    {
      // Forward: CH3 = Duty, CH4 = 0
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, pulse);
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, 0);
    }
    else
    {
      // Reverse: CH3 = 0, CH4 = Duty
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, pulse);
    }
  }
}

/**
  * @brief  Stop all motors immediately.
  */
void Motor_Stop(void)
{
  __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
  __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
  __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, 0);
}
