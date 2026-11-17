/*
 * input_analog.h
 *
 * Mesure des courants du hacheur (etape 2) :
 * chaine TIM1 (TRGO) -> ADC1 (2 canaux) -> DMA circulaire.
 * Une mesure par periode PWM, synchronisee sur le TIM1.
 */

#ifndef ACQUISITION_INPUT_ANALOG_H_
#define ACQUISITION_INPUT_ANALOG_H_

#include <stdint.h>
#include "user_interface/shell.h"

/* ---- Caracteristiques de l'ADC (12 bits, Vref = 3,3 V) ---- */
#define ADC_FULL_SCALE              4095.0f   // valeur max sur 12 bits
#define ADC_VREF_V                  3.3f      // tension de reference (V)

/* ---- Organisation du buffer DMA (= ordre des Ranks dans CubeMX) ---- */
#define ADC_NB_CHANNELS             2
#define ADC_RANK_U_CURRENT          0         // Rank 1 : PA1 = ADC1_IN2 (U_Imes)
#define ADC_RANK_BUS_CURRENT        1         // Rank 2 : PC2 = ADC1_IN8 (Bus_Imes)

/* ---- Fonction de transfert du capteur de courant ----
 * I = (V_adc - OFFSET) / SENSITIVITY
 * !!! VALEURS A VERIFIER DANS LA DATASHEET DU CAPTEUR (voir schema KiCad) !!!
 */
#define CURRENT_SENSOR_OFFSET_V     1.65f     // tension de sortie a 0 A (V)
#define CURRENT_SENSOR_SENSITIVITY  0.050f    // sensibilite (V/A)

#define MILLI                       1000.0f

int   input_analog_init(void);
float input_analog_get_u_current(void);
float input_analog_get_bus_current(void);

/* Commande shell : "CURRENT" */
int   input_analog_shell_current(h_shell_t *h_shell, int argc, char **argv);

#endif /* ACQUISITION_INPUT_ANALOG_H_ */
