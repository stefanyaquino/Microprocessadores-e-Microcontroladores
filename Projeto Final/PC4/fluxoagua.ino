#include <msp430g2553.h> 
#include <legacymsp430.h>

#define RX BIT1
#define TX BIT2
#define BAUD_9600   0
#define BAUD_19200  1
#define BAUD_38400  2
#define BAUD_56000  3
#define BAUD_115200 4
#define BAUD_128000 5
#define BAUD_256000 6
#define NUM_BAUDS   7

int vazao; //Variável para armazenar o valor em L/min
int media=0; //Variável para tirar a média a cada 1 minuto
int contaPulso=0; //Variável para a quantidade de pulsos
int i=0; //Variável para contagem

void Send_Data(unsigned char c);
void Send_Int(int n);
void Send_String(char str[]);
void Init_UART(unsigned int baud_rate_choice);

int main (void) 
{   
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer 
  
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  //P1DIR = BIT0;
  //P1OUT = BIT0; 
  
  Init_UART(BAUD_9600);
  
  //P1IES  = BIT4; // Configurar interrupção no botão por borda de descida 
  P1IE   = BIT4; // Habilitar interrupção 
  
  while(1)
 {
   _BIS_SR(GIE); 
   
    vazao = contaPulso / 5.5; //Converte para L/min
    media=media+vazao; //Soma a vazão para o calculo da media
    i++;
 
   //   Send_Int(vazao); //Imprime na serial o valor da vazão
     // Send_String(" L/min - "); //Imprime L/min
//    Send_Int(i); //Imprime a contagem i (segundos)
//    Send_String("s"); //Imprime s indicando que está em segundos
  
//   if(i==60)
//   {
//    media = media/60; //Tira a media dividindo por 60
//   // Send_String("\nMedia por minuto = "); //Imprime a frase Media por minuto =
//    Send_Int(media); //Imprime o valor da media
//    Send_String(" L/min - "); //Imprime L/min
//    media = 0; //Zera a variável media para uma nova contagem
//    i=0; //Zera a variável i para uma nova contagem
//    //Send_String("\n\nInicio\n\n"); //Imprime Inicio indicando que a contagem iniciou
//   }
 }
}

interrupt(PORT1_VECTOR) P1_ISR(void) 
{
 contaPulso++;
 P1IFG = 0; 
}  

void Send_Data(unsigned char c)
{
  while((IFG2&UCA0TXIFG)==0);
  UCA0TXBUF = c;
}

void Send_Int(int n)
{
  int casa, dig;
  if(n==0)
  {
    Send_Data('0');
    return;
  }
  if(n<0)
  {
    Send_Data('-');
    n = -n;
  }
  for(casa = 1; casa<=n; casa *= 10);
  casa /= 10;
  while(casa>0)
  {
    dig = (n/casa);
    Send_Data(dig+'0');
    n -= dig*casa;
    casa /= 10;
  }
}

void Send_String(char str[])
{
  int i;
  for(i=0; str[i]!= '\0'; i++)
    Send_Data(str[i]);
}

void Init_UART(unsigned int baud_rate_choice)
{
  unsigned char BRs[NUM_BAUDS] = {104, 52, 26, 17, 8, 7, 3};
  unsigned char MCTLs[NUM_BAUDS] = {UCBRF_0+UCBRS_1,
                    UCBRF_0+UCBRS_0,
                    UCBRF_0+UCBRS_0,
                    UCBRF_0+UCBRS_7,
                    UCBRF_0+UCBRS_6,
                    UCBRF_0+UCBRS_7,
                    UCBRF_0+UCBRS_7};
  if(baud_rate_choice<NUM_BAUDS)
  {
    // Habilita os pinos para transmissao serial UART
    P1SEL2 = P1SEL = RX+TX;
    // Configura a transmissao serial UART com 8 bits de dados,
    // sem paridade, comecando pelo bit menos significativo,
    // e com um bit de STOP
    UCA0CTL0 = 0;
    // Escolhe o SMCLK como clock para a UART
    UCA0CTL1 = UCSSEL_2;
    // Define a baud rate
    UCA0BR0 = BRs[baud_rate_choice];
    UCA0BR1 = 0;
    UCA0MCTL = MCTLs[baud_rate_choice];
  }
}

