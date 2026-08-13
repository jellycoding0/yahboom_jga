/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"
#include "encoder.h"
#include "icm20948.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_data = 0;
uint8_t imu_ok = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  Motor_Init();
  Encoder_Init();
  printf("Initializing IMU...\r\n");
  if (ICM20948_Init())
  {
    imu_ok = 1;
    printf("IMU initialization success!\r\n");
  }
  else
  {
    imu_ok = 0;
    printf("IMU initialization failed!\r\n");
  }
  printf("System initialized. Teleop ready!\r\n");
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_led_tick = 0;
  uint32_t last_print_tick = 0;
  while (1)
  {
    uint32_t current_time = HAL_GetTick();

    /* Toggle LEDs every 500ms (Non-blocking) */
    if (current_time - last_led_tick >= 500)
    {
      HAL_GPIO_TogglePin(State_LED_GPIO_Port, State_LED_Pin);
      HAL_GPIO_TogglePin(Switch_LED_GPIO_Port, Switch_LED_Pin);
      last_led_tick = current_time;
    }

    /* Read and print encoder cumulative ticks every 200ms */
    if (current_time - last_print_tick >= 200)
    {
      // Reading delta updates internal total ticks
      Encoder_Get_Delta(ENCODER_LEFT);
      Encoder_Get_Delta(ENCODER_RIGHT);

      int32_t gyro_z_scaled = (int32_t)(ICM20948_Get_GyroZ_rads() * 10000.0f);
      printf("Encoder L: %ld | R: %ld | GyroZ: %ld | IMU: %d\r\n",
             Encoder_Get_Total(ENCODER_LEFT),
             Encoder_Get_Total(ENCODER_RIGHT),
             gyro_z_scaled,
             imu_ok);

      last_print_tick = current_time;
    }

    /* Buzzer control via KEY1 button (Active-Low) */
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
    {
      HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
    }
    else
    {
      HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

#define RX_BUFFER_SIZE  32
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // If newline or carriage return is received, parse the command
    if (rx_data == '\n' || rx_data == '\r')
    {
      if (rx_index > 0)
      {
        rx_buffer[rx_index] = '\0'; // Null-terminate string
        
        char cmd;
        int speed = 0;
        // Parse packet, e.g. "F 400" or "S"
        if (sscanf((char*)rx_buffer, "%c %d", &cmd, &speed) >= 1)
        {
          int16_t left_speed = 0;
          int16_t right_speed = 0;

          if (cmd == 'F') // Forward
          {
            left_speed = speed;
            right_speed = (speed * 130) / 500; // Scaled down (500rpm -> 130rpm)
          }
          else if (cmd == 'B') // Backward
          {
            left_speed = -speed;
            right_speed = -(speed * 130) / 500; // Scaled down
          }
          else if (cmd == 'L') // Spin Left
          {
            left_speed = -speed;
            right_speed = (speed * 130) / 500; // Spin Left (Left reverse, Right forward)
          }
          else if (cmd == 'R') // Spin Right
          {
            left_speed = speed;
            right_speed = -(speed * 130) / 500; // Spin Right (Left forward, Right reverse)
          }
          else if (cmd == 'S') // Stop
          {
            left_speed = 0;
            right_speed = 0;
          }

          Motor_Set_Speed(MOTOR_LEFT, left_speed);
          Motor_Set_Speed(MOTOR_RIGHT, right_speed);
        }
        rx_index = 0; // Reset index
      }
    }
    else
    {
      if (rx_index < RX_BUFFER_SIZE - 1)
      {
        rx_buffer[rx_index++] = rx_data;
      }
      else
      {
        rx_index = 0; // Buffer full, reset to prevent overflow
      }
    }
    
    // Re-enable UART Rx interrupt
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
