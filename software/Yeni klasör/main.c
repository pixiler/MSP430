#include <msp430.h> 

/*MSP430FR2433 UART example*/

unsigned char RXData = 0;
unsigned char TXData = 1;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	PM5CTL0 &= ~LOCKLPM5;       //Disable the GPIO power-on default high-impedance mode
	                            //to activate previously configured port settings

	P1DIR |= BIT0;
	P1OUT |= ~BIT0;

	//Configure UART pins

	P1SEL0 |= BIT4 | BIT5;      //set 2- UART pin as second function

	//Configure UART for power up default clock rate

	UCA0CTLW0 |= UCSWRST;          // Put eUSCI in reset
	UCA0CTLW0 |= UCSSEL__SMCLK;   // Set equal to SMCLK

	//Baud Rate calculation 115200 baud
	UCA0BR0 = 8;                //1000000/115200 = 8.68
	UCA0MCTLW = 0xD600;         //1000000/115200 = INT(1000000/115200) = 0.68
	                            // UCBRSx value = 0xD6 (see SLAU445G 23.3.10)
	//9600 Baud
//	UCA0BR0 = 104;              // 1000000/9600 = 104.167
//	UCA0MCTLW = 0x1100;         // 0x11 = 0.167

	UCA0CTLW0 &= ~UCSWRST;     // Initialize eUSCI
	UCA0IE |= UCRXIE;           // Enable USCI_A0 RX interrupt

	__bis_SR_register(LPM0_bits | GIE);  // Enter LPM0 CPU off, SMCLK running

 return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(UCA0IV)
    {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        while(!(UCTXIFG & UCA0IFG));
        UCA0TXBUF = UCA0RXBUF;              // Load data onto buffer
        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;

    }
}
