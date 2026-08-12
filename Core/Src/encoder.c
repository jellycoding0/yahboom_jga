#include "encoder.h"
#include "tim.h"

// Static variables to keep track of the encoder counts
static uint16_t last_count_left = 0;
static uint16_t last_count_right = 0;

static int32_t total_ticks_left = 0;
static int32_t total_ticks_right = 0;

/**
  * @brief  Initialize and start TIM2 and TIM4 in Encoder mode.
  */
void Encoder_Init(void)
{
  // Start encoder interface for both TIM2 (Left) and TIM4 (Right)
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

  // Initialize previous count values
  last_count_left = __HAL_TIM_GET_COUNTER(&htim2);
  last_count_right = __HAL_TIM_GET_COUNTER(&htim4);

  // Reset total ticks
  total_ticks_left = 0;
  total_ticks_right = 0;
}

/**
  * @brief  Read the delta tick count since the last call and update cumulative ticks.
  * @param  encoder_idx: ENCODER_LEFT or ENCODER_RIGHT
  * @retval Delta ticks (positive for forward, negative for reverse)
  */
int16_t Encoder_Get_Delta(uint8_t encoder_idx)
{
  int16_t delta = 0;

  if (encoder_idx == ENCODER_LEFT)
  {
    uint16_t current_count = __HAL_TIM_GET_COUNTER(&htim2);
    // Explicit cast to signed 16-bit handles 16-bit timer overflow/underflow automatically
    // Negated to match forward/reverse direction of the right encoder
    delta = -(int16_t)(current_count - last_count_left);
    last_count_left = current_count;
    total_ticks_left += delta;
  }
  else if (encoder_idx == ENCODER_RIGHT)
  {
    uint16_t current_count = __HAL_TIM_GET_COUNTER(&htim4);
    // Explicit cast to signed 16-bit handles 16-bit timer overflow/underflow automatically
    delta = (int16_t)(current_count - last_count_right);
    last_count_right = current_count;
    total_ticks_right += delta;
  }

  return delta;
}

/**
  * @brief  Get the cumulative ticks.
  * @param  encoder_idx: ENCODER_LEFT or ENCODER_RIGHT
  * @retval Cumulative ticks
  */
int32_t Encoder_Get_Total(uint8_t encoder_idx)
{
  if (encoder_idx == ENCODER_LEFT)
  {
    return total_ticks_left;
  }
  else if (encoder_idx == ENCODER_RIGHT)
  {
    return total_ticks_right;
  }
  return 0;
}

/**
  * @brief  Reset cumulative ticks to 0.
  * @param  encoder_idx: ENCODER_LEFT or ENCODER_RIGHT
  */
void Encoder_Reset(uint8_t encoder_idx)
{
  if (encoder_idx == ENCODER_LEFT)
  {
    total_ticks_left = 0;
  }
  else if (encoder_idx == ENCODER_RIGHT)
  {
    total_ticks_right = 0;
  }
}
