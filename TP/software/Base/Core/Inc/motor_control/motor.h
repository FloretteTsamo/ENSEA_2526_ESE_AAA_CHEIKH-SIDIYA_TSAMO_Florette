/*
 * motor.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_MOTOR_CONTROL_MOTOR_H_
#define INC_MOTOR_CONTROL_MOTOR_H_



#endif /* INC_MOTOR_CONTROL_MOTOR_H_ */

#include "user_interface/shell.h"

int motor_set_ccr(h_shell_t* h_shell, int argc, char** argv);

int motor_start(h_shell_t* h_shell, int argc, char** argv);
int motor_stop(h_shell_t* h_shell, int argc, char** argv);
int motor_speed(h_shell_t* h_shell, int argc, char** argv);
