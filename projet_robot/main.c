#include <msp430.h>

// unsigned int memo_capt=0;
// unsigned int capt;
// unsigned int diff_capt;
int compteur_0 = 0;
int compteur_1 = 0;

 
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

  BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
  DCOCTL = CALDCO_1MHZ;
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

int main(void){
  WDTCTL = WDTPW +WDTHOLD; // desactivation Watch Dog Timer
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

  P2OUT &= ~(BIT5);
  P2OUT |= BIT1;

  P2IFG &= ~(BIT0|BIT3);

  config_signal();
  // __enable_interrupt();
  while(1);
}
