#include <msp430.h>


int compteur_0 = 0;
int compteur_1 = 0;

typedef enum { AVANCER, TOURNER, NB_ETATS} ETAT;
typedef enum { CAPT_ON, CAPT_OFF, NB_EVENTS} EVENT;

typedef void (*Action)(void);

void get_capt(){
}

 void arreter(){
  TA1CCR2 = 0;
  TA1CCR1 = 0;
}

void avancer(){
  P2OUT &= ~(BIT1);
  P2OUT |= BIT5;
}

void reculer(){
  P2OUT &= ~(BIT5);
  P2OUT |= BIT1;
}

void tourner_droite(){
  P2OUT &= ~(BIT1);
  P2OUT &= ~BIT5;
  compteur_angle = 0;
  //TA1CCR2 = 0;
}

void tourner_gauche(){
  P2OUT |= BIT1;
  P2OUT |= BIT5;
  compteur_angle = 0;
}

void get_event(){
  EVENT event;
  if(get_capt()==1){
    event=CAPT_ON;
  }
  else{
    event=CAPT_OFF;
  }

  return event;
}

typedef struct {
 ETAT etat_suivant; 
 Action action;  

 }Transition;

Transition tab_transition [NB_ETATS][NB_EVENTS]= {
  [AVANCER] = {
    [CAPT_ON] = {AVANCER, avancer},
    [CAPT_OFF] = {TOURNER, tourner_gauche}
  },
  [TOURNER] = {
    [CAPT_ON] = {AVANCER, avancer},
    [CAPT_OFF] = {TOURNER, tourner_gauche}
  }
};

 #pragma vector = PORT2_VECTOR
__interrupt void ma_interrupt_opto_0(void)
{

  if((P2IFG & BIT0)){
    compteur_0 = compteur_0 + 1;
    P2IFG &= ~(BIT0);
  }

  if((P2IFG & BIT3) ) {
    compteur_1 = compteur_1 + 1;
    P2IFG &= ~(BIT3);
  }

}
void config_signal(){

  
  TA1CTL = 0| (TASSEL_2 | ID_1); // source SMCLK, prédiv
  TA1CTL |= TAIE; // autorisation interruption TAIE
  TA1CTL |= MC_1; // comptage en mode up  

  //roue1 - gauche
  TA1CCTL2 |= OUTMOD_7; // activation du signal   1ere roue
  TA1CCTL1 |= OUTMOD_7; // activation du signal  2e roue
  TA1CCR0 = 2000;
  TA1CCR2 = 1419;

  //roue2 - droite
  TA1CCR1 =1400;


}

void init_vitesse(){
  //roue1 - droite
  TA1CCR2 = 1000;

  //roue2 - gauche
  TA1CCR1 = TA1CCR2 * (0.964);
}

void ajuster_vitesse(){
int min = 0;
int max = 0;
  if(compteur_0 > compteur_1){
    max = compteur_0;
    min = compteur_1;
    TA1CCR1 = 1000;
    TA1CCR2 = TA1CCR1*(max/min);
  }else{
    max = compteur_1;
    min = compteur_0;
    TA1CCR2 = 1000;
    TA1CCR1 = TA1CCR2*(max/min);
  }
  
}

int detecter_obstacle(){
  Aff_Efface();
  ADC_init();
  ADC_Demarrer_conversion(1);
  Aff_valeur(convert_Hex_Dec(ADC_Lire_resultat()));
  __delay_cycles(10000);
}

int main(void){
  WDTCTL = WDTPW +WDTHOLD; // desactivation Watch Dog Timer
  //config
  /* P2.2 et P2.4 en mode timer
     P2.0, P2.1, P2.3, P2.5  en mode I/O
  */

  BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
  DCOCTL = CALDCO_1MHZ;

  P2SEL &= (BIT0|BIT1|BIT3|BIT5); // mode I/O à 0
  P2SEL |= (BIT2|BIT4); // mode timer à 1
  P2SEL2 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5); // timer et I/O à 0

  //Directions : 0 entree et 1 sortie
  P2DIR &= ~(BIT0|BIT3); // entrees à 0
  P2DIR |= (BIT1|BIT2|BIT4|BIT5); // sorties à 1

  // Gestion des Interruptions
  P2IE |= (BIT0|BIT3);
  P2IES |=(BIT0|BIT3);

  P2OUT &= ~(BIT5);
  P2OUT |= BIT1;

  P2IFG &= ~(BIT0|BIT3);

  config_signal();

  // Table de transition
  ETAT etat = AVANCER;
  EVENT event;
  Transition trs;

  while(1){
    event = get_event();
    trs = tab_transition[etat][event];
    trs.action();
    etat=trs.etat_suivant;
  }

  // __enable_interrupt();
}
