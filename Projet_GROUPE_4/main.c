#include <msp430.h>

// unsigned int memo_capt=0;
// unsigned int capt;
// unsigned int diff_capt;
int compteur_0 = 0;
int compteur_1 = 0;
int compteur_angle = 0;
//typedef enum {DROITE, GAUCHE, face} sens ;
 
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
 #pragma vector = PORT2_VECTOR
__interrupt void ma_interrupt_opto_0(void)
{

  if((P2IFG & BIT0)==BIT0){
    compteur_0 = compteur_0 + 1;
    P2IFG &= ~(BIT0); 
    if(compteur_0==96){
      reculer();
    }
    compteur_angle++;
    //ajustement vitesse
    if(compteur_0==20){
      ajuster_vitesse();
    }
  }

  if((P2IFG & BIT3)==BIT3) {
    compteur_1 = compteur_1 + 1;
    compteur_angle++;
    P2IFG &= ~(BIT3);
  } 
  
    if((compteur_angle>=14)){
      avancer();
    }
}

void init_vitesse(){
  //roue1 - droite
  TA1CCR2 = 1000;

  //roue2 - gauche
  TA1CCR1 = TA1CCR2 * (0.964);
}
void config_signal(){

  TA1CTL = 0| (TASSEL_2 | ID_3); // source SMCLK, prédiv
  //TA1CTL |= TAIE; // autorisation interruption TAIE
  TA1CTL |= MC_1; // comptage en mode up  


  TA1CCTL2 |= OUTMOD_7; // activation du signal   1ere roue 
  TA1CCTL1 |= OUTMOD_7; // activation du signal  2e roue 
  TA1CCR0 = 2000;
}
void ajuster_vitesse(){
int min = 0;
int max = 0;
  if(compteur_0 > compteur_1){
    max = compteur_0;
    min = compteur_1;
  }else{
    max = compteur_1;
    min = compteur_0;
  }
  TA1CCR2 = 1000;
  TA1CCR2 = TA1CCR2*(max/min);
}

int main(void){
  WDTCTL = WDTPW +WDTHOLD; // desactivation Watch Dog Timer
  
  BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
  DCOCTL = CALDCO_1MHZ;
  //config
  /* P2.2 et P2.4 en mode timer
     P2.0, P2.1, P2.3, P2.5  en mode I/O
  */
  P2SEL &= (BIT0|BIT1|BIT3|BIT5); // mode I/O à 0
  P2SEL |= (BIT2|BIT4); // mode timer à 1
  P2SEL2 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5); // timer et I/O à 0

  //Directions : 0 entree et 1 sortie
  P2DIR &= ~(BIT0|BIT3); // entrees à 0
  P2DIR |= (BIT1|BIT2|BIT4|BIT5); // sorties à 1 

  // Gestion des Interruptions
  P2IE |= (BIT0|BIT3); 
  P2IES |=(BIT0|BIT3);

  //avancer();
  P2IFG &= ~(BIT0|BIT3);
  config_signal();
  tourner_droite();
  init_vitesse();
  __enable_interrupt();
  while(1);
}