#include <msp430.h>

void config_timer1(void)
{
   TA1CTL = 0| (TASSEL_2 | ID_1);// source SMCLK, prédiv
  TA1CCR0 = 6555;
  TA1CCR1= 4951; // voir texte
  TA1CCTR2 = 4000;
  TA1CTL |= TAIE;// autorisation interruption TAIE
  TA1CTL |= MC_1;// comptage en mode up     
  TA1CCTL1 |= OUTMOD_7;// activation du signal      
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_ISR(void)
{
    if ((TA0CTL & TAIFG) == TAIFG)
    {
        P1OUT &= ~(BIT2);
        TA0CTL &= MC_0; //arret du comptage
        TA0CTL &= ~(TAIFG);
    }
}


void main(void)
{
  WDTCTL = WDTPW | WDTHOLD; // desactivation Watch Dog Timer

  P2SEL &= ~( BIT1 | BIT2 | BIT4| BIT5 );  // Select I/O function for red LED
  P2SEL2 &= ~( BIT1 | BIT3 | BIT2 | BIT4 | BIT5 );
  P2SEL |= (BIT2 | BIT4);
  
  P2DIR |= (BIT1 | BIT2 | BIT4 | BIT5); //
  P2DIR &= ~(BIT2 | BIT4);

  P2OUT &= ~(BIT2 | BIT4);
  P2OUT |= (BIT5 | BIT1); 

  /*BCSCTL1 = CALBC1_1MHZ;          // frequence d’horloge 1MHz
  DCOCTL = CALDCO_1MHZ;           //    "
  */
  config_timer1();
  
   

  P1IE |= BIT3;     // initialisation de l'interruption
  P1IES |= BIT3;    // interruption sur front descendant
                    // donc appui car bouton connecte a la masse
  P1IFG &= ~(BIT2); // RAZ flag d’interruption
  __enable_interrupt();
  while (1);// boucle infinie
}