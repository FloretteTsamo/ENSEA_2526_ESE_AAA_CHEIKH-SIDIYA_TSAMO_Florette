
# ENSEA_2526_ESE_AAA_CHEIKH-SIDIYA_TSAMO_Florette — Commande MCC via hacheur (Nucleo-STM32G474RE)

## Équipe

- CHEIKH-SIDIYA 
- TSAMO Florette Miroslava

Encadrants : Alexis MARTIN , Nicolas PAPAZOGLOU

## Objectif du projet

À partir d'un hacheur complet et d'une carte Nucleo-STM32G474RE, ce TP consiste à :

1. Réaliser la commande des 4 transistors du hacheur en commande complémentaire décalée,
2. Faire l'acquisition des différents capteurs (courant, vitesse),
3. Réaliser l'asservissement en temps réel de la vitesse du moteur.

## Architecture matérielle

- Carte de puissance : hacheur 3 bras (U, V, W) à base de MOSFET IRF540N, pilotés via des drivers isolés HCPL-315J. Seuls les bras **U** et **V** sont utilisés dans ce TP pour commander la MCC (pont en H).
- Carte de commande : Nucleo-STM32G474RE.
- Correspondance bras / broches STM32 (timer TIM1) :

| Bras | Transistor haut | Transistor bas |
|---|---|---|
| U | PA8 — TIM1_CH1 | PB13 — TIM1_CH1N |
| V | PA9 — TIM1_CH2 | PB14 — TIM1_CH2N |
| W (non utilisé) | PA10 — TIM1_CH3 | PB15 — TIM1_CH3N |

## Organisation du code

```
Core/Src/
├── app.c                        initialisation générale (init_device) et boucle principale (loop)
├── acquisition/
│   ├── input_analog.c           mesure de courant/tension (ADC)
│   └── input_encoder.c          mesure de vitesse (codeur)
├── motor_control/
│   ├── motor.c                  génération PWM, commande du rapport cyclique
│   └── asserv.c                 asservissement PID
└── user_interface/
    ├── shell.c                  terminal de commande via UART
    ├── led.c                    gestion de la LED utilisateur
    └── button.c                 gestion du bouton utilisateur
```

## Avancement — Section 6 : Commande MCC basique

### 6.1 — Génération des 4 PWM

TIM1 est configuré en mode centré (center-aligned) pour générer les 2 couples de PWM complémentaires (U et V), avec temps mort, à partir de l'horloge APB2 à 170 MHz.

| Paramètre | Valeur | Justification |
|---|---|---|
| Fréquence PWM | 20 kHz | cahier des charges |
| Mode de comptage | Center-aligned | génère le déphasage ("décalage") entre bras U et V |
| Prescaler | 0 | — |
| Period (ARR) | 4249 | 170 MHz / (2 × 20 kHz) − 1 = 4249, soit une période de 50 µs |
| Résolution | ≈ 4250 pas (~12 bits) | supérieure au minimum de 10 bits demandé |
| Temps mort (DeadTime) | 34 (≈ 200 ns) | td(off) + tf de l'IRF540N ≈ 74 ns (datasheet, RG = 5,1 Ω) ; marge prise pour tenir compte de la résistance de grille réelle (30 Ω) et du temps de propagation du driver HCPL-315J — **valeur à faire valider par l'enseignant** |

Commande complémentaire décalée : le bras U reçoit le rapport cyclique de consigne, le bras V reçoit le complément (`ARR − CCR_U`), ce qui crée le déphasage entre les deux bras.

Rapport cyclique de test fixé à 60 % pour la vérification à l'oscilloscope, via `motor_init()` (`Core/Src/motor_control/motor.c`), qui démarre les 4 sorties PWM (`HAL_TIM_PWM_Start` / `HAL_TIMEx_PWMN_Start` sur CH1 et CH2) et applique la consigne.

**Vérification à l'oscilloscope :**

![Signaux PWM des bras U et V, rapport cyclique 60 %, temps mort visible](img/osc_signaux_U_V.png)

*Canal 1 (jaune) : bras U — Canal 2 (bleu) : bras V. Les deux signaux sont bien complémentaires et déphasés entre les deux bras, avec un temps mort visible à chaque commutation.*

![Mesure du temps mort et de l'amplitude des signaux de commande](img/osc_mesures_amplitude.png)

<!-- A COMPLETER : décrire ici les valeurs mesurées sur cette capture (temps mort mesuré, amplitude des signaux de commande des grilles) et les comparer à la valeur programmée de 200 ns -->

### 6.2 — Commande de vitesse via UART

Le shell (`Core/Src/user_interface/shell.c`) permet d'envoyer la commande suivante en UART (port série de la Nucleo) :

```
SET_CCR XXXX
```

Traitement effectué :
1. Détection de la commande `SET_CCR` par le shell,
2. Conversion des caractères `XXXX` en entier, avec validation que ce sont bien des chiffres,
3. Clamp de la valeur au maximum autorisé (`ARR` = 4249),
4. Application de la valeur au registre de comparaison des 4 transistors (bras U et son complément sur le bras V).

<!-- A COMPLETER : confirmer le nom exact de la commande/fonction telle qu'implémentée si différent, et donner un exemple de test (ex. commande envoyée + comportement observé) -->

### 6.3 — Premiers tests moteur

<!-- A COMPLETER :
- comportement observé à 50 % de rapport cyclique (vitesse nulle attendue)
- comportement observé à 70 % (problème constaté lors du passage brutal 50% -> 70%, ex. appel de courant, à-coup mécanique)
- description de la solution mise en place (rampe logicielle progressive du rapport cyclique) et du résultat obtenu
-->

## Problèmes rencontrés

- **Fichiers custom absents du build CMake.** Les fichiers `app.c`, `shell.c`, `led.c`, `motor.c` n'étaient pas listés dans le `CMakeLists.txt` racine du projet (`TP/software/Base/CMakeLists.txt`, section `target_sources`), ce qui provoquait une erreur de link (`undefined reference to init_device`). Il a fallu les ajouter manuellement, CubeMX ne gérant que ses propres fichiers générés (`main.c`, `gpio.c`, `tim.c`, etc.) et jamais les fichiers ajoutés par l'utilisateur.

## Prochaines étapes (section 7)

- Mesure du courant (ADC en polling puis via chaîne Timer/ADC/DMA synchronisée à la PWM),
- Mesure de la vitesse via le codeur et détermination de la constante de temps mécanique du moteur,
- Mise en place de l'asservissement de vitesse en temps réel (section 8 et suivantes).

