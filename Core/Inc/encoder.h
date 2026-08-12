#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"

// Encoder channels
#define ENCODER_LEFT    1
#define ENCODER_RIGHT   2

/**
  * @brief  Initialize and start TIM2 and TIM4 in Encoder mode.
  * @retval None
  */
void Encoder_Init(void);

/**
  * @brief  Read the delta tick count since the last call and update cumulative ticks.
  * @param  encoder_idx: ENCODER_LEFT or ENCODER_RIGHT
  * @retval Delta ticks (positive for forward, negative for reverse)
  */
int16_t Encoder_Get_Delta(uint8_t encoder_idx);

/**
  * @brief  Get the cumulative ticks.
  * @param  encoder_idx: ENCODER_LEFT or ENCODER_RIGHT
  * @retval Cumulative ticks
  */
int32_t Encoder_Get_Total(uint8_t encoder_idx);

/**
  * @brief  Reset cumulative ticks to 0.
  * @param  encoder_idx: ENCODER_LEFT or ENCODER_RIGHT
  * @retval None
  */
void Encoder_Reset(uint8_t encoder_idx);

#endif /* __ENCODER_H__ */
