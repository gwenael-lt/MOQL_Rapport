#include <msp430g2553.h>
#include <stdio.h>
#include <Afficheur.h>
#include <ADC.h>

unsigned char cmd[10];


void Init(void)
{
    P1OUT |= BIT5;
    P1DIR |= BIT5;

    P1SEL |= (BIT5 | BIT6 | BIT7);                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= (BIT5 | BIT6 | BIT7);                    // P1.1 = RXD, P1.2=TXD

    UCB0CTL1 = UCSWRST;                         // SOFTWARE RESET
    UCB0CTL0 |= (UCCKPH | UCMSB | UCMST | UCSYNC);
    UCB0CTL1 |= UCSSEL_2;               // division clock par 2 (par 3 possible)

    UCB0BR0 = 0x02;
    UCB0BR1 = 0x00;
    UCA0MCTL = 0x00;

    UCB0CTL1 &= ~UCSWRST; //Initialisation USCI

    /* Enable USCI_A0 RX interrupt */
    IE2 |= UCA0RXIE;

    /* calibration d'usine de l'oscillateur pour le servomoteur
     * if(CALBC1_1MHZ==0xFF || CALDCO_1MHZ==0xFF)
     *      __bis_SR_register(LPM4_bits);
     *  else
     *  {
     *      DC0CTL =0;
     *      BCSCTL1 = CALBC1_1MHZ;
     *      DCOCTL = CALDCO_1MHZ;
     *  }
     *
     */
}


void TXdata(unsigned char c)
{
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = c;
}

void envoi_text(unsigned char *texte)
{
    unsigned int i = 0;
    for(i=0 ; texte[i] != 0x00 ; i++)
    {
        while(!(IFG2 & UCA0TXIFG));
        UCA0TXBUF=texte[i];
    }
}


/* TIMER
 * #define MOTORS_FREQUENCY 33333
 *
 * void init_timer(void)
 * {
 *      TA1CTL &= ~MC_0;
 *      TA1CCR0 = MOTORS_FREQUENCY;
 *      TA1CTL = (TASSEL_2 | MC_1 | ID_0 | TACLR);
 *      TA1CCTL1 = 0 | OUTMOD_7;
 * }
 */


int avancer(unsigned int vitA, unsigned int vitB)
{
    P2DIR &= ~(BIT0|BIT1|BIT3); //activation moteurs + sens avant moteur A
    P2DIR |=(BIT2|BIT4);  //Vitesse moteur
    P2DIR |=(BIT5); // sens avant moteur B

    P2OUT |=(BIT0|BIT1|BIT3|BIT5);

    P2SEL |= (BIT2|BIT4);
    P2SEL2 &= ~BIT2;
    P2SEL2 &= ~BIT4;
    TA1CTL = TASSEL_2 | MC_1;
    TA1CCTL1 |= OUTMOD_7;
    TA1CCTL2 |= OUTMOD_7;
    TA1CCR0 = 100; // determine la periode du signal
    TA1CCR1 = vitA; // determine le rapport cyclique du signal
    TA1CCR2 = vitB;
    return 0;
}

int reculer(unsigned int vitA, unsigned int vitB)
{
    P2DIR &= ~(BIT0|BIT3|BIT5); //activation moteurs + sens arriere moteur A
    P2DIR |=(BIT2|BIT4);  //Vitesse moteur
    P2DIR |=(BIT1); // sens arriere moteur B

    P2OUT |=(BIT0|BIT1|BIT3|BIT5);

    P2SEL |= (BIT2|BIT4);
    P2SEL2 &= ~BIT2;
    P2SEL2 &= ~BIT4;
    TA1CTL = TASSEL_2 | MC_1;
    TA1CCTL1 |= OUTMOD_7;
    TA1CCTL2 |= OUTMOD_7;
    TA1CCR0 = 100; // determine la periode du signal
    TA1CCR1 = vitA; // determine le rapport cyclique du signal
    TA1CCR2 = vitB;
    return 0;
}

int droite(unsigned int vitA)
{
    P2DIR &= ~(BIT0|BIT1);
    P2DIR |=(BIT2);

    P2OUT |=(BIT0|BIT1);

    P2SEL |= (BIT2);
    P2SEL2 &= ~BIT2;
    TA1CTL = TASSEL_2 | MC_1;
    TA1CCTL1 |= OUTMOD_7;
    TA1CCR0 = 100; // determine la periode du signal
    TA1CCR1 = vitA; // determine le rapport cyclique du signal
    return 0;
}

int gauche(unsigned int vitB)
{
    P2DIR &= ~(BIT3|BIT4);
    P2DIR |=(BIT5);

    P2OUT |=(BIT3|BIT5);

    P2SEL |= (BIT4);
    P2SEL2 &= ~BIT4;
    TA1CTL = TASSEL_2 | MC_1;
    TA1CCTL2 |= OUTMOD_7;
    TA1CCR0 = 100; // determine la periode du signal
    TA1CCR2 = vitB; // determine le rapport cyclique du signal
    return 0;
}



void commandes (void)
{
    if(strcmp((const char *)cmd,"h") == 0) //help
        {
            envoi_text("\r\nCommandes :");
            envoi_text("\r\nHelp : h");
            envoi_text("\r\nMarche avant : ");
            envoi_text("\r\nMarche arrière : ");
            envoi_text("\r\nTourner à droite : ");
            envoi_text("\r\nTourner à gauche : ");
        }
    //ROUE GAUCHE = BIT 0/1/2 (activation, vitesse, sens)
    //ROUE DROITE = BIT 3/4/5
    if(strcmp((const char *)cmd,"273") == 0) //fleche avant
        {
            avancer(20,20);
        }
    if(strcmp((const char *)cmd,"274") == 0) //fleche arrière
        {
            reculer(20,20);
        }
    if(strcmp((const char *)cmd,"275") == 0) //fleche droite
        {//ACTIVATION ROUE GAUCHE
            droite(15);
        }
    if(strcmp((const char *)cmd,"276") == 0) //fleche gauche
        {//ACTIVATION ROUE DROITE
            gauche(15);
        }
}

 // main.c
int main(void)
{
    int capt_infr;


    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer


    // init_timer();


	if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xFF)
	    {
	        __bis_SR_register(LPM4_bits); // Low Power Mode #trap on Error
	    }
	    else
	    {
	        // Parametres usine
	        BCSCTL1 = CALBC1_1MHZ;
	        DCOCTL = CALDCO_1MHZ;
	    }

	    Init();
	    __bis_SR_register(GIE); // interrupts enabled
	    envoi_text(' > Tapez "h" pour voir les commandes');



	    while (1){

	        if(capt_infr < 275)
	        {
	            avancer(20, 20);
	        }
	        if(capt_infr > 330)
	        {
	            reculer(15, 30);

	        }

	    /*P1OUT &= ~BIT5; // sélection bit

	    while (!(IFG2 & UCB0TXIFG));
	        {
	            UCB0TXBUF = 0xAA; //envoi données
	        }
	    while (!(IFG2 & UCB0RXIFG));
	        {
	            char_recu = UCB0RXBUF; //réception et stockage données
	        }

	    P1OUT |= BIT5;*/
	    }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    unsigned char c;
    c = UCA0RXBUF;
    TXdata(c);
}
