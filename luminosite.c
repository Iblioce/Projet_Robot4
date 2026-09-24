//Luminosité ambiante

void luminosite(){
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
