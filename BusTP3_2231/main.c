#include <msp430.h>


int miliseconds1,miliseconds2, Tlimite=0;
int distance,distance2,moyenne;
long sensor,sensor2;


int avancer( TA1CCR1, TA1CCR2) {

         P1DIR |= BIT6;
         //P1SEL |=(BIT6);
         P1OUT = BIT6;
         return 0;
}

int reculer(TA1CCR1, TA1CCR2){

             return 0;
}

/**
 * main.c
 */
void main(void)
{

     WDTCTL = WDTPW | WDTHOLD;
      BCSCTL1 = CALBC1_1MHZ;
      DCOCTL = CALDCO_1MHZ;
      WDTCTL = WDTPW + WDTHOLD;

  //robot__avance
      P1DIR &= ~(BIT0|BIT1|BIT3);
      P1DIR |=(BIT2|BIT4);
      P1DIR |=(BIT5);
      P1OUT |=(BIT0|BIT1|BIT3|BIT5);

    //capteur
      TA0CCTL0 |= CCIE;     //activer lesinterruptions
      TA0CCR0 = 1000;       // CCR0+période
      TA0CTL = 0|(TASSEL_2 | ID_0); // SMCLK, upmode, prediv de 1
      TA0CTL |= MC_1;
      TACCTL0 |= CCIE;     //activer lesinterruptions
      TACTL = 0|(TASSEL_2 | ID_0);
      TACTL |= MC_1;
      P1IFG &= ~0xFF;

      P1DIR |= (BIT1|BIT4);         // pin 1.1 et 1.4 (TRIGGER)
      P1DIR &= ~ (BIT2|BIT5);       // pin 1.2 et 1.5 (ECHO)

      P1IE &= ~(BIT1|BIT4);

      P1IE |= (BIT2|BIT5);         // permet interruption ECHO pin
      P1IES &= ~(BIT2|BIT5);       //front montant ECHO pin

      P1DIR &= ~BIT0 ;             //pin 1.0 en entrée
      P1SEL &= ~BIT0 ;

      __enable_interrupt();        // autorisation generale des interruptions

     while(1){
        //envoi impulsion pour detection
        P1OUT |= (BIT1|BIT4);
        __delay_cycles(10);
        P1OUT &= ~(BIT1|BIT4);      // arret impulsion

        distance = sensor/58;     // conversion en cm

        if(distance >10)
        {
            P1DIR |= BIT6;
            P1OUT |= BIT6;
            __delay_cycles(1000);

        }
            P1DIR |= BIT6;
            P1OUT &= ~(BIT6);
        /*if(((distance1-distance2) <1) || ((distance2-distance1) <1)){
           if(distance1 < distance2)
              avancer(11,16);
           else
             avancer(14,13);
        }
        else if(moyenne < 15 && moyenne >13){
                    //robot ralentit

                }
                if(moyenne < 13){
                    //robot recule
                    reculer(11, 13);
                 }
                if(moyenne >15){
                    //robot avance
                    avancer(18,20);
                }*/
    }// fin while

}// fin MAIN

               #pragma vector=PORT1_VECTOR
               __interrupt void Port_1(void)
               {
                   if((P1IFG & BIT5)==BIT5 || (P1IFG & BIT2)==BIT2)
                       {
                         if(!((P1IES & BIT5)== BIT5) || !((P1IES & BIT2)== BIT2))
                         {
                           TA0CTL |= TACLR;
                           miliseconds1 = 0;
                           miliseconds2 = 0;
                           P1IES |= (BIT2|BIT5);  //front descendant
                         }
                         else
                         {
                           sensor = (long)miliseconds1*1000 + (long)TAR;    //calcul ECHO
                           P1IES &= ~BIT5;
                           __delay_cycles(10);
                           sensor2=  (long)miliseconds2*1000 + (long)TAR;
                           P1IES &= ~BIT2;
                         }
                   P1IFG &= ~(BIT1|BIT2|BIT4|BIT5);
                   }
               }


               #pragma vector=TIMER0_A0_VECTOR
               __interrupt void Timer_A (void)
               {
                 miliseconds1++;
                 miliseconds2++;
                 TA0CTL &= ~TAIFG ;
               }


               #pragma vector=TIMER1_A0_VECTOR
               __interrupt void Timer_B (void)
               {
                 Tlimite++;
                 TACTL &= ~TAIFG ;
               }
