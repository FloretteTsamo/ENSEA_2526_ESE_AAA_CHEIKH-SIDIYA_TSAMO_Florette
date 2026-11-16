/*
 * input_analog.c
 *
 * Mesure du courant de phase U (PA1 = ADC1_IN2) en polling.
 * Etape suivante : remplacer le polling par la chaine TIM1 -> ADC -> DMA.
 */

#include "acquisition/input_analog.h"
#include <stdio.h>
#include "adc.h"

/* ADC utilise pour la mesure du courant (un seul endroit a changer) */
static ADC_HandleTypeDef *const current_adc = &hadc1;

/**
 * @brief  Calibre l'ADC. A appeler une fois, avant toute conversion.
 * @retval 0 si OK, -1 en cas d'erreur
 */
int input_analog_init(void)
{
	if (HAL_ADCEx_Calibration_Start(current_adc, ADC_SINGLE_ENDED) != HAL_OK)
	{
		return -1;
	}
	return 0;
}

/**
 * @brief  Lance ADC_NB_SAMPLES conversions et renvoie la moyenne.
 * @param  raw : valeur brute moyenne (0 a 4095)
 * @retval 0 si OK, -1 en cas d'erreur
 */
int input_analog_read_raw(uint32_t *raw)
{
	uint32_t sum = 0;

	for (int i = 0; i < ADC_NB_SAMPLES; i++)
	{
		if (HAL_ADC_Start(current_adc) != HAL_OK)
		{
			return -1;
		}

		if (HAL_ADC_PollForConversion(current_adc, ADC_POLL_TIMEOUT_MS) != HAL_OK)
		{
			HAL_ADC_Stop(current_adc);
			return -1;
		}

		sum += HAL_ADC_GetValue(current_adc);
	}

	HAL_ADC_Stop(current_adc);
	*raw = sum / ADC_NB_SAMPLES;
	return 0;
}

/** @brief Valeur brute ADC -> tension (V) */
float input_analog_raw_to_voltage(uint32_t raw)
{
	return ((float)raw * ADC_VREF_V) / ADC_FULL_SCALE;
}

/** @brief Tension du capteur (V) -> courant (A) */
float input_analog_voltage_to_current(float voltage)
{
	return (voltage - CURRENT_SENSOR_OFFSET_V) / CURRENT_SENSOR_SENSITIVITY;
}

/**
 * @brief  Commande shell "CURRENT" : affiche la mesure.
 *         Affichage en mV et mA (entiers) : pas besoin du printf float.
 */
int input_analog_shell_current(h_shell_t *h_shell, int argc, char **argv)
{
	uint32_t raw;
	int size;

	if (input_analog_read_raw(&raw) != 0)
	{
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
				"ERREUR : lecture ADC\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}

	float voltage = input_analog_raw_to_voltage(raw);
	float current = input_analog_voltage_to_current(voltage);

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
			"ADC = %lu | V = %d mV | I = %d mA\r\n",
			(unsigned long)raw,
			(int)(voltage * MILLI),
			(int)(current * MILLI));
	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}
