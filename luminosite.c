//Luminosité ambiante

void luminosite(){
  Aff_init();
  ADC_init();
  Init_USI_SPI();

 
  ADC_Demarrer_conversion( 2);
  int res = ADC_Lire_resultat();
  res = res/1023*100;

  if(res <= 50){
    P1OUT |= (BIT6|BIT0); 
  }
  else{
    P1OUT &= ~(BIT6|BIT0)
  }
}


//Nouvelle fonction config_signal

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

  luminosite();

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

  //Luminosité
  P1SEL &= ~(BIT0|BIT6);
  P1SEL2 &= ~(BIT0|BIT6);

  //Directions : 0 entree et 1 sortie
  P2DIR &= ~(BIT0|BIT3); // entrees à 0
  P2DIR |= (BIT1|BIT2|BIT4|BIT5); // sorties à 1
  P1DIR |= (BIT0|BIT6); // sortie des LEDs

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

