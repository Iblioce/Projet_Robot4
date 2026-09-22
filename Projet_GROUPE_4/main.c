#include <msp430.h>
 
 
// void config_timer1(void)
// {
//   TA1CTL = 0| (TASSEL_2 | ID_1);// source SMCLK, prédiv
//   TA1CCR0 = 6555;
//   TA1CCR1= 4951; // voir texte
//   // TA1CCR2 = 4000;
//   TA1CTL |= TAIE;// autorisation interruption TAIE
//   TA1CTL |= MC_1;// comptage en mode up    
//   TA1CCTL2 |= OUTMOD_7;// activation du signal      
// }
 


 
 
// void main(void)
// {
//   WDTCTL = WDTPW | WDTHOLD; // desactivation Watch Dog Timer

// // Button



//  P1SEL &= ~BIT3;  
//  P1SEL2 &= ~BIT3; 
 
//   P2SEL &= ~( BIT0 | BIT1 | BIT2 | BIT3 | BIT4| BIT5 );  // Select I/O function for red LED
//   P2SEL2 &= ~( BIT0 | BIT1 | BIT3 | BIT2 | BIT4 | BIT5 );
//   P2SEL |= (BIT2 | BIT4);
 
//   P2DIR |= (BIT1 | BIT2 | BIT4 | BIT5);
//   P2DIR &= ~(BIT2 | BIT4);
 
  
//   P2OUT |= (BIT5 | BIT1);
 
//   BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
//   DCOCTL = CALDCO_1MHZ;           //    "
  
//   config_timer1();

 
 
//   P2IE |= (BIT0|BIT3);     // initialisation de l'interruption
//   P2IES |= (BIT0|BIT3);    // interruption sur front descendant
//   P2IFG &= ~(BIT0|BIT3); // RAZ flag d’interruption
//   // __enable_interrupt();

//    P2OUT |= (BIT2 | BIT4 | BIT1 | BIT5);
//   while (1); // boucle infinie
// }



// // #pragma vector=PORT2_VECTOR
// // __interrupt void TIMER0_ISR(void)
// // {  
// //   // 
// //   //  P2IFG &= ~(BIT0|BIT3);

// //  if(P2IFG & BIT0){
// //     P2IFG &= ~BIT0;
// //  }
// //  if(P2IFG & BIT3){
// //   P2IFG &= ~BIT3;
// //  }
// //   }

void config_signal(){

  BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
  DCOCTL = CALDCO_1MHZ;
  TA1CTL = 0| (TASSEL_2 | ID_1);// source SMCLK, prédiv
  TA1CTL |= TAIE; // autorisation interruption TAIE
  TA1CTL |= MC_1; // comptage en mode up  

  //roue1
  TA1CCTL2 |= OUTMOD_7;// activation du signal   1ere roue 
  TA1CCR0 = 2000;
  //TA1CCR1 = 2000;
  TA1CCR2 = 1500;

  //roue2
  TA1CCR1 = 2000;
  TA1CCTL1 |= OUTMOD_7;// activation du signal  2e roue

}
int main(void){
  WDTCTL = WDTPW +WDTHOLD;
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

  P2OUT &= ~(BIT5);
  P2OUT |= BIT1;

  config_signal();

 /* BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
  DCOCTL = CALDCO_1MHZ;           //    "

  TA1CTL = 0| (TASSEL_2 | ID_1);// source SMCLK, prédiv
  TA1CCR0 = 2000;
  //TA1CCR1 = 2000;
  TA1CCR2 = 1500;
  TA1CTL |= TAIE; // autorisation interruption TAIE
  TA1CTL |= MC_1; // comptage en mode up  
  TA1CCTL2 |= OUTMOD_7;// activation du signal   1ere roue 
  TA1CCTL1 |= OUTMOD_7;// activation du signal  2e roue
  */
}
  
  