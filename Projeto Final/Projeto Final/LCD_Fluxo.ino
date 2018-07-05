#include <msp430g2553.h>
#include <legacymsp430.h>

#define CAP_IN BIT1
#define LED_VERMELHO BIT0
#define LED_VERDE BIT6

#define BTN BIT3
#define LCD_OUT P2OUT
#define LCD_DIR P2DIR
#define D4 BIT0
#define D5 BIT1
#define D6 BIT2
#define D7 BIT3
#define RS BIT4
#define E  BIT5
#define DADOS 1
#define COMANDO 0
#define CMND_DLY 1000
#define DATA_DLY 1000
#define BIG_DLY  20000
#define CLR_DISPLAY  Send_Byte(1, COMANDO, BIG_DLY)
#define POS0_DISPLAY Send_Byte(2, COMANDO, BIG_DLY)

void Atraso_us(volatile unsigned int us);
void Send_Nibble(volatile unsigned char nibble, volatile unsigned char dados, volatile unsigned int microsegs);
void Send_Byte(volatile unsigned char byte, volatile unsigned char dados, volatile unsigned int microsegs);
void Send_Data(volatile unsigned char byte);
void Send_String(char str[]);
void Send_Int(int n);
void InitLCD(void);

float litros=0; //Variável para armazenar o valor em L/min
int cont=0; //Variável para a quantidade de pulsos
int i=0;
int aux=0;

int main(void)
{
	WDTCTL = WDTHOLD + WDTPW;
	
        BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	
        P1DIR |= (LED_VERMELHO + LED_VERDE); //Habilita LEDs na porta P1 como saída (Launchpad)
        P1OUT |= LED_VERMELHO; //Liga LED Vermelho no bit P1.0
        P1OUT &= ~LED_VERDE;

        P1DIR &= ~CAP_IN; //Habilitar captura de sinal externo via Timer_A
        P1SEL |= CAP_IN; //Habilitar captura de sinal externo via Timer_A
        P1SEL2 &= ~CAP_IN; //Habilitar captura de sinal externo via Timer_A
        
        //Modo de captura do canal 0: pino externo CCI0A,
        //captura das bordas de subida (CM_1), gerando interrupção
        TACCTL0 = CCIS_0 + CM_1 + CAP + CCIE + SCS;
        //Timer_A em modo contínuo, com SMCLK @ 1MHz
        TACTL = TASSEL_2 + ID_0 + MC_2;
        
        InitLCD();
	Send_String("Hidrometro");

        //Modo de baixo consumo com SMCLK ligado
        _BIS_SR(LPM0_bits+GIE);
        
        while(1)
        {
           P1OUT &= ~LED_VERDE;
        };
	
	return 0;
}
interrupt(TIMER0_A0_VECTOR) TA0_ISR(void)
{
   litros = litros + 0.0039;
   CLR_DISPLAY;
   POS0_DISPLAY;
   aux = litros * 1000;
   Send_String("Gasto: ");
   Send_Int(aux);
   Send_String("mL");
   
  P1OUT ^= LED_VERDE;
  TACCTL0 &= ~CCIFG; //Apaga flag de interrupção
}

void Atraso_us(volatile unsigned int us)
{
	
        TA1CCR0 = us-1;
	TA1CTL = TASSEL_2 + ID_0 + MC_1 + TAIE;
	while((TA1CTL & TAIFG)==0);
	TA1CTL = TACLR;
	TA1CTL = 0;
}

void Send_Nibble(volatile unsigned char nibble, volatile unsigned char dados, volatile unsigned int microsegs)
{
	LCD_OUT |= E;
	LCD_OUT &= ~(RS + D4 + D5 + D6 + D7);
	LCD_OUT |= RS*(dados==DADOS) +
		D4*((nibble & BIT0)>0) +
		D5*((nibble & BIT1)>0) +
		D6*((nibble & BIT2)>0) +
		D7*((nibble & BIT3)>0);
	LCD_OUT &= ~E;
	Atraso_us(microsegs);
}

void Send_Byte(volatile unsigned char byte, volatile unsigned char dados, volatile unsigned int microsegs)
{
	Send_Nibble(byte >> 4, dados, microsegs/2);
	Send_Nibble(byte & 0xF, dados, microsegs/2);
}

void Send_Data(volatile unsigned char byte)
{
	Send_Byte(byte, DADOS, DATA_DLY);
}

void Send_String(char str[])
{
	while((*str)!='\0')
	{
		Send_Data(*(str++));
	}
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
	for(casa = 10000; casa>n; casa /= 10);
	while(casa>0)
	{
		dig = (n/casa);
		Send_Data(dig+'0');
		n -= dig*casa;
		casa /= 10;
	}
}

void InitLCD(void)
{
	unsigned char CMNDS[] = {0x20, 0x14, 0xC, 0x6};
	unsigned int i;
	// Atraso de 10ms para o LCD fazer o boot
	Atraso_us(10000);
	LCD_DIR |= D4+D5+D6+D7+RS+E;
	Send_Nibble(0x2, COMANDO, CMND_DLY);
	for(i=0; i<4; i++)
		Send_Byte(CMNDS[i], COMANDO, CMND_DLY);
	CLR_DISPLAY;
	POS0_DISPLAY;
}
