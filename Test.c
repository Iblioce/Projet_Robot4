#include <msp430.h>
#include "Afficheur/Afficheur.h"
#include "ADC/ADC.h"

// Variables modifiées sous interruption : déclarées volatile
volatile unsigned int compteur_0 = 0;
volatile unsigned int compteur_1 = 0;
volatile unsigned int compteur_angle = 0;

// Nombre total de tics odométriques pour parcourir 1,5m 
#define TICKS_1M5 1500 

typedef enum { AVANCER, TOURNER, ARRET, NB_ETATS } ETAT;
typedef enum { CAPT_ON, CAPT_OFF, FIN_PARCOURS, NB_EVENTS } EVENT;

typedef void (*Action)(void);

void luminosite(void) {
    ADC_init();
    Init_USI_SPI();

    ADC_Demarrer_conversion(2);
    int res = ADC_Lire_resultat();
    
    // Correction de la division entière
    long res_pourcent = ((long)res * 100) / 1023;

    // Si luminosité faible (<= 50%), on allume les LEDs P1.0 et P1.6
    if (res_pourcent <= 50) {
        P1OUT |= (BIT6 | BIT0);
    } else {
        P1OUT &= ~(BIT6 | BIT0);
    }
}

int detecter_obstacle(void) {
    ADC_init();
    ADC_Demarrer_conversion(3);
    int res = ADC_Lire_resultat();
    Aff_valeur(convert_Hex_Dec(res));
    __delay_cycles(20000);
    Aff_Efface();
    return res;
}

int get_capt(void) {
    int res = detecter_obstacle();
    // GP2D120 : tension ~2.25V à 5cm (ADC ~700) et ~0.8V à 15cm (ADC ~250)
    if (res >= 250 && res <= 700) {
        return 1;
    } else {
        return 0;
    }
}

void arreter(void) {
    TA1CCR1 = 0;
    TA1CCR2 = 0;
}

void avancer(void) {
    P2OUT &= ~(BIT1);
    P2OUT |= BIT5;
}

void reculer(void) {
    P2OUT &= ~(BIT5);
    P2OUT |= BIT1;
}

void tourner_droite(void) {
    P2OUT &= ~(BIT1);
    P2OUT &= ~(BIT5);
    compteur_angle = 0;
}

void tourner_gauche(void) {
    P2OUT |= BIT1;
    P2OUT |= BIT5;
    compteur_angle = 0;
}

EVENT get_event(void) {
    if (compteur_0 >= TICKS_1M5) {
        return FIN_PARCOURS;
    }
    if (get_capt() == 1) {
        return CAPT_ON;
    }
    return CAPT_OFF;
}

typedef struct {
    ETAT etat_suivant;
    Action action;
} Transition;

// Table de transitions mise à jour avec l'état ARRET
Transition tab_transition[NB_ETATS][NB_EVENTS] = {
    [AVANCER] = {
        [CAPT_OFF]     = {AVANCER, avancer},
        [CAPT_ON]      = {TOURNER, tourner_gauche},
        [FIN_PARCOURS] = {ARRET,   arreter}
    },
    [TOURNER] = {
        [CAPT_OFF]     = {AVANCER, avancer},
        [CAPT_ON]      = {TOURNER, tourner_gauche},
        [FIN_PARCOURS] = {ARRET,   arreter}
    },
    [ARRET] = {
        [CAPT_OFF]     = {ARRET,   arreter},
        [CAPT_ON]      = {ARRET,   arreter},
        [FIN_PARCOURS] = {ARRET,   arreter}
    }
};

#pragma vector = PORT2_VECTOR
__interrupt void ma_interrupt_opto_0(void)
{
    if (P2IFG & BIT0) {
        compteur_0++;
        P2IFG &= ~(BIT0);
    }
    if (P2IFG & BIT3) {
        compteur_1++;
        P2IFG &= ~(BIT3);
    }
}

void config_signal(void) {
    TA1CTL = TASSEL_2 | ID_1; // SMCLK, ID /2
    TA1CTL |= TAIE;
    TA1CTL |= MC_1;           // Up-mode

    // Roue 1 - gauche (P2.4 / TA1.2)
    TA1CCTL2 |= OUTMOD_7;
    // Roue 2 - droite (P2.2 / TA1.1)
    TA1CCTL1 |= OUTMOD_7;

    TA1CCR0 = 2000;
    TA1CCR1 = 1000;
    TA1CCR2 = 1000;
}

void ajuster_vitesse(void) {
    unsigned int c0, c1;

    // Copie atomique des compteurs
    __disable_interrupt();
    c0 = compteur_0;
    c1 = compteur_1;
    compteur_0 = 0;
    compteur_1 = 0;
    __enable_interrupt();

    if (c1 == 0) return;

    // Correction du PWM basée sur le ratio en virgule fixe (base 1000)
    if (c0 > c1) {
        unsigned long nouveau_ccr1 = ((unsigned long)1000 * c1) / c0;
        TA1CCR1 = (unsigned int)nouveau_ccr1;
        TA1CCR2 = 1000;
    } else if (c0 < c1) {
        unsigned long nouveau_ccr2 = ((unsigned long)1000 * c0) / c1;
        TA1CCR1 = 1000;
        TA1CCR2 = (unsigned int)nouveau_ccr2;
    } else {
        TA1CCR1 = 1000;
        TA1CCR2 = 1000;
    }
}

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P2SEL &= ~(BIT0 | BIT1 | BIT3 | BIT5);
    P2SEL |= (BIT2 | BIT4);
    P2SEL2 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5);

    // Config LEDs (P1.0 et P1.6)[cite: 1]
    P1SEL &= ~(BIT0 | BIT6);
    P1SEL2 &= ~(BIT0 | BIT6);
    P1DIR |= (BIT0 | BIT6);

    // Directions P2
    P2DIR &= ~(BIT0 | BIT3);
    P2DIR |= (BIT1 | BIT2 | BIT4 | BIT5);

    // Interruptions optocoupleurs (P2.0 et P2.3)
    P2IE |= (BIT0 | BIT3);
    P2IES |= (BIT0 | BIT3);
    P2IFG &= ~(BIT0 | BIT3);

    P2OUT &= ~(BIT5);
    P2OUT |= BIT1;
    P1OUT &= ~(BIT0 | BIT6);

    config_signal();

    ETAT etat = AVANCER;
    avancer();
    EVENT event;
    Transition trs;

    Aff_Init();
    __enable_interrupt();

    while (1) {
        luminosite();

        event = get_event();
        trs = tab_transition[etat][event];
        trs.action();
        etat = trs.etat_suivant;

        // Ajustement périodique de trajectoire
        if (compteur_0 >= 20) {
            ajuster_vitesse();
        }
    }
}
