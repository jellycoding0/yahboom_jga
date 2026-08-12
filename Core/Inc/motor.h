#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"

// Motor channels
#define MOTOR_LEFT    1
#define MOTOR_RIGHT   2

// Motor speed range limits
#define MOTOR_SPEED_MAX     1000
#define MOTOR_SPEED_MIN    -1000

/**
  * @brief  Initialize and start TIM8 PWM channels for motor control.
  * @retval None
  */
void Motor_Init(void);

/**
  * @brief  Set speed for a specific motor channel.
  * @param  motor_idx: MOTOR_LEFT or MOTOR_RIGHT
  * @param  speed: Target speed from -1000 (max reverse) to 1000 (max forward)
  * @retval None
  */
void Motor_Set_Speed(uint8_t motor_idx, int16_t speed);

/**
  * @brief  Stop all motors immediately.
  * @retval None
  */
void Motor_Stop(void);

#endif /* __MOTOR_H__ */
