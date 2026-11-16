/*
 * app.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "app.h"

#include "user_interface/shell.h"
#include "motor_control/motor.h"
#include "acquisition/input_analog.h"

static char shell_uart2_received_char;

/**
 * @brief Ajoute au shell toutes les commandes de l'application.
 */
static void app_shell_commands_init(void)
{
	// Moteur
	shell_add(&hshell1, "start",   motor_start,   "Start motor");
	shell_add(&hshell1, "stop",    motor_stop,    "Stop motor");
	shell_add(&hshell1, "speed",   motor_speed,   "Set motor speed");
	shell_add(&hshell1, "SET_CCR", motor_set_ccr, "Set PWM duty (CCR)");

	// Acquisition
	shell_add(&hshell1, "CURRENT", input_analog_shell_current, "Courant phase U");
}

void init_device(void){
// Initialisation user interface
	// SHELL
	hshell1.drv.transmit = shell_uart2_transmit;
	hshell1.drv.receive = shell_uart2_receive;
	shell_init(&hshell1);
	app_shell_commands_init();
	if (HAL_UART_Receive_IT(&huart2, (uint8_t *)&shell_uart2_received_char, 1) != HAL_OK)
	{
		Error_Handler();
	}

	// LED
	led_init();

	// BUTTON
//	button_init();
//
// Initialisation motor control
	// MOTOR
//	motor_init();
	// ASSERV (PID)
//	asserv_init();
//
// Initialisation data acquistion
	// ANALOG INPUT
	if (input_analog_init() != 0)
	{
		Error_Handler();
	}
	// ENCODER INPUT
//	input_encoder_init();
}

uint8_t shell_uart2_transmit(const char *pData, uint16_t size)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)pData, size, HAL_MAX_DELAY);
	return size;
}

uint8_t shell_uart2_receive(char *pData, uint16_t size)
{
	*pData = shell_uart2_received_char;
	return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&shell_uart2_received_char, 1);
		shell_run(&hshell1);
	}
}

void loop(void){

}
