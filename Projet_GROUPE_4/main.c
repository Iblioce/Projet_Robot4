#include <msp430.h>


int compteur_0 = 0;
int compteur_1 = 0;
int compteur_angle = 0;

typedef enum { AVANCER, ARRET, NB_ETATS} ETAT;
typedef enum { CAPT_ON, CAPT_OFF, NB_EVENTS} EVENT;

typedef void (*Action)(void);

int detecter_obstacle(){
  ADC_init();
  ADC_Demarrer_conversion(3);
  int res = ADC_Lire_resultat();
  Aff_valeur(convert_Hex_Dec(res));
  __delay_cycles(20000);
  Aff_Efface();
  return res;
}

int get_capt(){
  if (detecter_obstacle()>500){
    return 1;
  }else{
    return 0;
  }
}

 void arreter(){
  TA1CCR2 = 0;
  TA1CCR1 = 0;
}

void init_vitesse(){
  //roue2 - gauche
  TA1CCR1 = 1000 ;
  //roue1 - droite
  TA1CCR2 = TA1CCR1 * 0.971;

  
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

EVENT get_event(){
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
    [CAPT_OFF] = {AVANCER, init_vitesse},
    [CAPT_ON] = {ARRET, arreter}
  },
  [ARRET] = {
    [CAPT_OFF] = {AVANCER, init_vitesse},
    [CAPT_ON] = {ARRET, arreter}
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

  TA1CCTL2 |= OUTMOD_7; // activation du signal   1ere roue
  TA1CCTL1 |= OUTMOD_7; // activation du signal  2e roue
  TA1CCR0 = 2000;
  init_vitesse();
}



void ajuster_vitesse(){
  if(compteur_0 > compteur_1){
    TA1CCR1 = 1000;
    TA1CCR2 = TA1CCR1*(compteur_0/compteur_1);
  }else{
    TA1CCR2 = 1000;
    TA1CCR1 = TA1CCR2*(compteur_1/compteur_0);
  }
  
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

  P1SEL &= ~(BIT0|BIT6);
  P1SEL2 &= ~(BIT0|BIT6);

  //Directions : 0 entree et 1 sortie
  P2DIR &= ~(BIT0|BIT3); // entrees à 0
  P2DIR |= (BIT1|BIT2|BIT4|BIT5); // sorties à 1
  P1DIR |= (BIT0|BIT6);

  P1OUT &= ~ (BIT0|BIT6);

  // Gestion des Interruptions
  P2IE |= (BIT0|BIT3);
  P2IES |=(BIT0|BIT3);

  P2IFG &= ~(BIT0|BIT3);

  avancer(); 
  config_signal();

  // Table de transition
  ETAT etat = AVANCER;
  EVENT event;
  Transition trs;
  Aff_Init();
  
  //__enable_interrupt();
  while(1){
    if(compteur_0 >= 101){
      arreter();
    }else {
      event = get_event();
      trs = tab_transition[etat][event];
      trs.action();
      etat=trs.etat_suivant;
    }
    
    //ajuster_vitesse();
  }

   
}