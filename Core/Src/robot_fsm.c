#include "robot_fsm.h"
#include "motor.h"
#include "icm20948.h"
#include "main.h" // For LED and Buzzer pin defines
#include <stdio.h>

// Global variables definition
RobotState current_state = STATE_INIT;
uint32_t last_packet_tick = 0;

// Local timer for FSM LED blinks
static uint32_t last_led_tick = 0;

void Robot_FSM_Init(void)
{
    current_state = STATE_INIT;
    last_packet_tick = HAL_GetTick();
    last_led_tick = HAL_GetTick();
}

void Robot_FSM_FeedWatchdog(void)
{
    last_packet_tick = HAL_GetTick();
}

void Robot_FSM_Update(void)
{
    uint32_t current_time = HAL_GetTick();

    switch (current_state)
    {
        case STATE_READY:
            // Slow blink State LED & Switch LED every 500ms
            if (current_time - last_led_tick >= 500)
            {
                HAL_GPIO_TogglePin(State_LED_GPIO_Port, State_LED_Pin);
                HAL_GPIO_TogglePin(Switch_LED_GPIO_Port, Switch_LED_Pin);
                last_led_tick = current_time;
            }
            // Stop motors in ready state
            Motor_Stop();
            // Turn off buzzer
            HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
            break;

        case STATE_RUN:
            // Fast blink State LED & Switch LED every 100ms
            if (current_time - last_led_tick >= 100)
            {
                HAL_GPIO_TogglePin(State_LED_GPIO_Port, State_LED_Pin);
                HAL_GPIO_TogglePin(Switch_LED_GPIO_Port, Switch_LED_Pin);
                last_led_tick = current_time;
            }
            // Safety Watchdog: If no packet received for more than 1000ms, go to EMERGENCY
            if (current_time - last_packet_tick > 1000)
            {
                current_state = STATE_EMERGENCY;
                printf("Safety Trigger: Serial packet loss timeout! Entering EMERGENCY.\r\n");
            }
            // Turn off buzzer
            HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
            break;

        case STATE_EMERGENCY:
            // Extremely rapid blink State LED every 50ms
            if (current_time - last_led_tick >= 50)
            {
                HAL_GPIO_TogglePin(State_LED_GPIO_Port, State_LED_Pin);
                HAL_GPIO_TogglePin(Switch_LED_GPIO_Port, Switch_LED_Pin);
                last_led_tick = current_time;
            }
            // Stop motors immediately
            Motor_Stop();
            // Sound buzzer continuously to alert user
            HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
            break;

        case STATE_ERROR:
            // Stop motors
            Motor_Stop();
            HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
            break;

        default:
            break;
    }
}

void Robot_FSM_Handle_Command(char cmd, int speed)
{
    int16_t left_speed = 0;
    int16_t right_speed = 0;

    if (cmd == 'S') // Stop / Clear Emergency
    {
        current_state = STATE_READY;
        left_speed = 0;
        right_speed = 0;
        Robot_FSM_FeedWatchdog(); // Reset safety watchdog
    }
    else if (cmd == 'F' || cmd == 'B' || cmd == 'L' || cmd == 'R')
    {
        // Ignore motion commands in EMERGENCY state until cleared with 'S'
        if (current_state != STATE_EMERGENCY)
        {
            current_state = STATE_RUN;
            Robot_FSM_FeedWatchdog(); // Feed the watchdog

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
                right_speed = (speed * 130) / 500; // Spin Left
            }
            else if (cmd == 'R') // Spin Right
            {
                left_speed = speed;
                right_speed = -(speed * 130) / 500; // Spin Right
            }
        }
        else
        {
            // Do nothing, motors remain stopped
            left_speed = 0;
            right_speed = 0;
        }
    }

    Motor_Set_Speed(MOTOR_LEFT, left_speed);
    Motor_Set_Speed(MOTOR_RIGHT, right_speed);
}
