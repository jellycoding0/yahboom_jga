#ifndef __ROBOT_FSM_H
#define __ROBOT_FSM_H

#include "stm32f1xx_hal.h"

// Define Robot FSM States
typedef enum {
  STATE_INIT = 0,
  STATE_READY,
  STATE_RUN,
  STATE_EMERGENCY,
  STATE_ERROR
} RobotState;

// Global variables declaration
extern RobotState current_state;
extern uint32_t last_packet_tick;
extern uint8_t imu_ok;

// Function prototypes
void Robot_FSM_Init(void);
void Robot_FSM_Update(void);
void Robot_FSM_FeedWatchdog(void);
void Robot_FSM_Handle_Command(char cmd, int speed);

#endif /* __ROBOT_FSM_H */
