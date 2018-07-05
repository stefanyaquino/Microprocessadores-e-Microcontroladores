#include <msp430g2553.h> 
#include <legacymsp430.h>

int contaPulso=0;

int main (void) 
{   
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer 
  
  P1DIR = BIT0;
  P1OUT = BIT0; 
  
  //P1IES  = BIT4; // Configurar interrupção no botão por borda de descida 
  P1IE   = BIT4; // Habilitar interrupção 
  
  while(1)
 {
   _BIS_SR(GIE); 
   
   if(contaPulso>100)
   {
     P1DIR = BIT6;
     P1OUT = BIT6; 
     contaPulso=0;
   }
 }
}

interrupt(PORT1_VECTOR) P1_ISR(void) 
{
 contaPulso++;
 P1IFG = 0;   
}  
