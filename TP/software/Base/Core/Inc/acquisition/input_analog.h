/*
 * input_analog.h
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#ifndef INC_INPUT_ANALOG_H_
#define INC_INPUT_ANALOG_H_



#endif /* INC_INPUT_ANALOG_H_ */

/*
 * input_analog.h
 *
 * Mesure des courants du hacheur avec l'ADC (etape 1 : polling).
 */

#ifndef ACQUISITION_INPUT_ANALOG_H_
#define ACQUISITION_INPUT_ANALOG_H_

#include <stdint.h>
#include "user_interface/shell.h"

/* ---- Caracteristiques de l'ADC (12 bits, Vref = 3,3 V) ---- */
#define ADC_FULL_SCALE          4095.0f   // valeur max sur 12 bits
#define ADC_VREF_V              3.3f      // tension de reference (V)
#define ADC_POLL_TIMEOUT_MS     10        // attente max d'une conversion
#define ADC_NB_SAMPLES          16        // moyenne pour reduire le bruit

/* ---- Fonction de transfert du capteur de courant ----
 * I = (V_adc - OFFSET) / SENSITIVITY
 * !!! VALEURS A VERIFIER DANS LA DATASHEET DU CAPTEUR (voir schema KiCad) !!!
 */
#define CURRENT_SENSOR_OFFSET_V     1.65f   // tension de sortie a 0 A (V)
#define CURRENT_SENSOR_SENSITIVITY  0.050f  // sensibilite (V/A)

#define MILLI                       1000.0f

int   input_analog_init(void);
int   input_analog_read_raw(uint32_t *raw);
float input_analog_raw_to_voltage(uint32_t raw);
float input_analog_voltage_to_current(float voltage);

/* Commande shell : "CURRENT" */
int   input_analog_shell_current(h_shell_t *h_shell, int argc, char **argv);

#endif /* ACQUISITION_INPUT_ANALOG_H_ */
