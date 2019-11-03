#include <msp430.h>

/*MSP430FR2433 UART example*/
/*esp8266 AT command*/

void RxInput();
void UARTSendArray(char TxArray[]);

unsigned char data;     // received char data
char MyString [25];     // work buffer
char SSID[8] = "DESKTOP";
char password[9] = "@x037R06";

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	// clock system setup
	__bis_SR_register(SCG0);    // disable FLL
	CSCTL3 |= SELREF__REFOCLK;  // Set REF0CLK AS FLL reference source
	CSCTL0 = 0;                 // clear DC0 and MOD register
	CSCTL1 &= ~(DCORSEL_7);     // Clear DC0 frequency select bits first
	CSCTL1 |= DCORSEL_3;        // Set DCOCLK = 8Mhz
	CSCTL2 = FLLD_1 + 121;      // FLLD = 1, DCOIV = 4Mhz

	__delay_cycles(3);
	__bic_SR_register(SCG0);    // enable FLL

	while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));      // Poll until FLL is locked
	CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK;      // set ACLK = XT1 = 32768Hz, DCOCLK as MCLK and SMCLK source
	CSCTL5 |= DIVM1;                                //SMCLK = MCLK = DCODIV / 2 = 1 Mhz, by default
	
	PM5CTL0 &= ~LOCKLPM5;       //Disable the GPIO power-on default high-impedance mode
	                            //to activate previously configured port settings
/*burada kaldým*/
	P1DIR |= BIT0 | BIT1;       // RED = 0, GREEN = 1
	P1OUT &= ~(BIT0 | BIT1);    // Clear P1.0

	//Configure hardware UART

	P1SEL0 |= BIT4 | BIT5;      //set 2- UART pin as second function P1.4 - TX P1.5 RX

	//Configure UART

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

	//wait for input
	UARTSendArray("AT+CWJAP=DESKTOP,@x037R06\r\n");
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
//        while(!(UCTXIFG & UCA0IFG));
//        UCA0TXBUF = UCA0RXBUF;              // Load data onto buffer
        data = UCA0RXBUF;                     // read the received char - also clears Interrupt
//        RxInput();                            // Process the received char
        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;

    }
}

void RxInput()
{
    //strcat(MyString, "Received command ");
//    strcat(MyString, (char*)data);
//    UARTSendArray(MyString);
    switch (data) {
        case 'R':
            P1OUT |= BIT0;
            break;
        case 'r':
            P1OUT &= ~BIT0;
            break;
        case 'G':
            P1OUT |= BIT1;
            break;
        case 'g':
            P1OUT &= ~BIT1;
            break;
//        case 'L':
//            int eren = 0;
//            strcat(MyString, "MyString Value: ");
//            for(i=0; i<10; i++)
//            {
//                strcat(MyString, (char*)i);
//                UARTSendArray(MyString);
//            }
//            break;
        default:
            UARTSendArray("Unknown Command: \n");
            break;
    }
}

void UARTSendArray(char *TxArray)
{
    while(*TxArray)     // loop until binary zero (EOS)
    {
        while(!(UCTXIFG & UCA0IFG));        // Wait for TX buffer to be ready for new data
        UCA0TXBUF = *TxArray++;             // Write the character at the location specified py the pointer
    }
}
