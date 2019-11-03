#include <msp430.h> 

/**
 * main.c
 */

unsigned char data;     // received char data
unsigned char receivedData[20][100] = {};
volatile unsigned int raw = 0;
volatile unsigned int col = 0;
unsigned char checking[100] = {};

volatile int found = -1;       //-1 default   0 new line      1 ok

//unsigned char ssid[] = "DESKTOP";
unsigned char ssid[] = "AT+CWJAP=\"DESKTOP\",\"@037R068\"\r\n";
unsigned char password[] = "@03R067";
unsigned char dede[100] = {};

void espSendArray(char TxArray[]);
void espSerialInit(void);
void espGetArray(char espData);
void wait(int connection);
int __ok(void);
int __connect(void);
void clear(void);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;       //Disable the GPIO power-on default high-impedance mode
	                                //to activate previously configured port settings

	 P1DIR |= BIT0 | BIT1;       // RED = 0, GREEN = 1
	 P1OUT &= ~(BIT0 | BIT1);    // Clear P1.0


	//espSerial Initiation
	espSerialInit();

	espSendArray("AT\r\n");
	 __bis_SR_register(LPM0_bits | GIE);  // Enter LPM0 CPU off, SMCLK running

	 wait(1);
	 found = __ok();
	 //P1OUT ^= BIT1;
	//espSendArray("ATE0\r\n");

	//espSendArray("AT+CWMODE=1\r\n");


    espSendArray("AT+CWJAP=\"DESKTOP\",\"@037R068\"\r\n");
	//espSendArray("ATE0");         //Enable / Disable echo
    wait(1);                             //wait esp return "OK"
    found = __ok();
    //P1OUT ^= BIT1;

    espSendArray("AT+CIPSTART=\"TCP\",\"192.168.137.159\",80\r\n");
    wait(1);
    found = __connect();
    //P1OUT ^= BIT1;

    espSendArray("AT+CIPSEND=62\r\n");
    wait(1);
    P1OUT ^= BIT1;
    espSendArray("GET / HTTP/1.1\r\n");
    espSendArray("Host: 192.168.137.159\r\n");
    espSendArray("Connection: close\r\n");
    espSendArray("\r\n");
    espSendArray("\r\n");

	P1OUT ^= BIT1;

	return 0;
}
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    switch(UCA0IV)
    {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        data = UCA0RXBUF;                     // read the received char - also clears Interrupt
        _BIC_SR_IRQ(LPM0_bits);
        espGetArray(data);                           // Process the received char
        P1OUT |= BIT0;
        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;

    }
}

void espSerialInit(void)
{
    P1SEL0 |= BIT4 | BIT5 ;             // P1.4 = TX    ,    P1.5 = RX

    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;

    //UCA0BR0 = 8;      // 1MHz SMCLK/11200 BAUD
    //UCA0MCTLW = 0xD600;         //1000000/115200 = INT(1000000/115200) = 0.68
    UCA0BR0 = 107;      // 1MHz SMCLK/9600 BAUD
    UCA0BR1 = 0x00;
    UCA0MCTLW = 0x1100; // | UCOS16 | UCBRF_1;
    UCA0CTLW0 &= ~UCSWRST;     UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

}
void espSendArray(char *TxArray)
{
    while(*TxArray)     // loop until binary zero (EOS)
    {
        while(!(UCTXIFG & UCA0IFG));        // Wait for TX buffer to be ready for new data
        UCA0TXBUF = *TxArray++;             // Write the character at the location specified py the pointer
    }
}
void espGetArray(char espData)
{
    receivedData[col][raw++] = espData;
    //check(espData);
    if(espData == '\n'){
        raw = 0;
        col++;
    }
}

int __ok(void)     //if return 0 "OK" not founded // if return 1 "OK" founded
{
    unsigned int i;
    unsigned int k;

    for(i=0; i<20;i++)
    {
        for(k=0; k<100; k++)
            {
                if(receivedData[i][k] == 'O')
                {
                    if(receivedData[i][k+1] == 'K')
                    {
                        if(receivedData[i][k+2] == '\r')
                            return 1;
                    }

                }
            }
    }
    return 0;
}
int __connect(void)     //if return 0 "CONNECT" not founded // if return 1 "CONNECT" founded
{
    unsigned int i;
    unsigned int k;

    for(i=0; i<20;i++)
    {
        for(k=0; k<100; k++)
            {
                if(receivedData[i][k] == 'C')
                {
                    if(receivedData[i][k+1] == 'O')
                    {
                        if(receivedData[i][k+2] == 'N')
                            return 1;
                    }

                }
            }
    }
    return 0;
}
void wait(int connection)     //wait esp return "OK"
{
    /*1 __ok
     * 2 __connection*/
    switch (connection) {

    //col = 0;
        case 1:
            //P1OUT ^= BIT1;
            while(__ok() == 0)
                {
                    __no_operation();
                }
            break;
        case 2:
            //P1OUT ^= BIT1;
            while(__connect() == 0)
            {
                __no_operation();
            }
            break;
        default:
            break;
    }
    clear();
}

void clear(void)
{
    unsigned int i=0;
    unsigned int k=0;
    for(i = 0; i<20; i++)
    {
        for(k = 0; k<100; k++)
        {
            receivedData[i][k] = '\0';
        }
    }
    col= 0;
    raw = 0;
}
