#include "motor_control/motor.h"
#include <stdlib.h>
#include <stdio.h>
#include "tim.h"
#include "user_interface/shell.h"

#define CCR_MAX 4249

int motor_set_ccr(h_shell_t* h_shell, int argc, char** argv)
{
	int size;

	if (argc < 2 || argv[1][0] < '0' || argv[1][0] > '9')
	{
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Usage : SET_CCR XXXX\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}

	uint32_t ccr = strtoul(argv[1], NULL, 10);

	// Vérification des limites
	if (ccr > CCR_MAX)
	{
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "ERREUR : CCR doit etre entre 0 et %d\r\n", CCR_MAX);
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, __HAL_TIM_GET_AUTORELOAD(&htim1)-ccr);

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "CCR = %lu\r\n", (unsigned long)ccr);
	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}

int motor_start(h_shell_t* h_shell, int argc, char** argv)
{
	uint32_t ccr = CCR_MAX / 2;

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
			__HAL_TIM_GET_AUTORELOAD(&htim1) - ccr);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

	int size = snprintf(h_shell->print_buffer,
			SHELL_PRINT_BUFFER_SIZE,
			"Moteur demarre\r\n");

	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}

int motor_stop(h_shell_t* h_shell, int argc, char** argv)
{
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

	int size = snprintf(h_shell->print_buffer,
			SHELL_PRINT_BUFFER_SIZE,
			"Moteur arrete\r\n");

	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}

int motor_speed(h_shell_t* h_shell, int argc, char** argv)
{
	if (argc < 2)
		return -1;

	uint32_t target = strtoul(argv[1], NULL, 10);

	if (target > CCR_MAX)
	{
		int size = snprintf(h_shell->print_buffer,
				SHELL_PRINT_BUFFER_SIZE,
				"ERREUR : CCR trop grand\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}

	uint32_t current = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1);

	while (current != target)
	{
		if (current < target)
			current += (target - current > 10) ? 10 : target - current;
		else
			current -= (current - target > 10) ? 10 : current - target;

		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, current);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
				__HAL_TIM_GET_AUTORELOAD(&htim1) - current);

		HAL_Delay(10);
	}

	int size = snprintf(h_shell->print_buffer,
			SHELL_PRINT_BUFFER_SIZE,
			"Vitesse demandee : %lu\r\n",
			(unsigned long)target);

	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}
