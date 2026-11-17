/*
 * input_analog.c
 *
 * Etape 2 : acquisition reguliere TIM1 -> ADC1 -> DMA.
 *
 * - TIM1 genere un TRGO (Update Event) une fois par periode PWM (RCR = 1).
 * - Le TRGO lance une sequence ADC1 de 2 conversions (U_Imes puis Bus_Imes).
 * - Le DMA copie les 2 resultats dans adc_dma_buffer (mode circulaire).
 * - A la fin de la sequence, HAL_ADC_ConvCpltCallback est appelee :
 *   on recopie les valeurs et on fait une impulsion sur ADC_SYNC (PC7)
 *   pour verifier a l'oscilloscope que la mesure est synchrone a la PWM.
 */

#include "acquisition/input_analog.h"
#include <stdio.h>
#include "adc.h"
#include "main.h"

/* ADC utilise pour la mesure des courants (un seul endroit a changer) */
static ADC_HandleTypeDef *const current_adc = &hadc1;

/* Buffer rempli par le DMA */
static uint32_t adc_dma_buffer[ADC_NB_CHANNELS];

/* Dernieres valeurs brutes (ecrites sous interruption) */
static volatile uint32_t u_current_raw;
static volatile uint32_t bus_current_raw;

/* Diagnostic : nombre de mesures recues et nombre d'erreurs ADC */
static volatile uint32_t adc_measure_count;
static volatile uint32_t adc_error_count;

/** @brief Valeur brute ADC -> courant (A) */
static float raw_to_current(uint32_t raw)
{
	float voltage = ((float)raw * ADC_VREF_V) / ADC_FULL_SCALE;
	return (voltage - CURRENT_SENSOR_OFFSET_V) / CURRENT_SENSOR_SENSITIVITY;
}

/**
 * @brief  Calibre l'ADC puis demarre la chaine ADC + DMA.
 *         Les conversions commencent des que le TIM1 tourne (PWM demarree).
 * @retval 0 si OK, -1 en cas d'erreur
 */
int input_analog_init(void)
{
	if (HAL_ADCEx_Calibration_Start(current_adc, ADC_SINGLE_ENDED) != HAL_OK)
	{
		return -1;
	}

	if (HAL_ADC_Start_DMA(current_adc, adc_dma_buffer, ADC_NB_CHANNELS) != HAL_OK)
	{
		return -1;
	}

	return 0;
}

/**
 * @brief  Appelee par la HAL quand le DMA a recu les ADC_NB_CHANNELS valeurs.
 *         Code court : on est dans une interruption a 20 kHz.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{
		HAL_GPIO_WritePin(ADC_SYNC_GPIO_Port, ADC_SYNC_Pin, GPIO_PIN_SET);

		u_current_raw   = adc_dma_buffer[ADC_RANK_U_CURRENT];
		bus_current_raw = adc_dma_buffer[ADC_RANK_BUS_CURRENT];
		adc_measure_count++;

		HAL_GPIO_WritePin(ADC_SYNC_GPIO_Port, ADC_SYNC_Pin, GPIO_PIN_RESET);
	}
}

/**
 * @brief  Appelee par la HAL en cas d'erreur ADC (par exemple un overrun).
 *         Apres un overrun, l'ADC n'envoie plus de requetes DMA :
 *         les valeurs restent bloquees. On relance donc la chaine ADC + DMA.
 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{
		adc_error_count++;
		HAL_ADC_Stop_DMA(current_adc);
		HAL_ADC_Start_DMA(current_adc, adc_dma_buffer, ADC_NB_CHANNELS);
	}
}

/** @brief Dernier courant de phase U mesure (A) = courant du moteur */
float input_analog_get_u_current(void)
{
	return raw_to_current(u_current_raw);
}

/** @brief Dernier courant de bus mesure (A) */
float input_analog_get_bus_current(void)
{
	return raw_to_current(bus_current_raw);
}

/**
 * @brief  Commande shell "CURRENT" : affiche les dernieres mesures.
 *         Affichage en mA (entiers) : pas besoin du printf float.
 */
int input_analog_shell_current(h_shell_t *h_shell, int argc, char **argv)
{
	int size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
			"Iu=%d Ibus=%d mA raw=%lu/%lu n=%lu e=%lu\r\n",
			(int)(input_analog_get_u_current() * MILLI),
			(int)(input_analog_get_bus_current() * MILLI),
			(unsigned long)u_current_raw,
			(unsigned long)bus_current_raw,
			(unsigned long)adc_measure_count,
			(unsigned long)adc_error_count);

	/* Securite : ne jamais envoyer plus que la taille du buffer */
	if (size >= SHELL_PRINT_BUFFER_SIZE)
	{
		size = SHELL_PRINT_BUFFER_SIZE - 1;
	}
	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}
