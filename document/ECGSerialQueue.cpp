// Serial output with queue - from ECG code 20160427


#define MAXQUE 25

 char qr,qw;  // read, write, and count
 char mqb[MAXQUE];  // buffer  // always 1 less than MAX can be stored



//prototypes
  char pushque( char val);
  char popque();
  char isempty(void){return (qr == qw);} // true =  empty};
  void initque(void);


  // Configure GPIOs to it's lowest power state
  P1OUT = 0;                                // All P1.x reset
  P1DIR = 0xFF;                             // All P1.x outputs
//  P2OUT = 0;                                // All P2.x reset
//  P2DIR = 0xFF;                             // All P2.x outputs
  P2SEL0 |= BIT0 | BIT1;                    // USCI_A0 UART operation
  P2SEL1 &= ~(BIT0 | BIT1);

  // Clock System Setup
  CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
//  CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers
  // leave off CSCTL4 &= ~LFXTOFF;    // turn on LFXT (external crystal)

// check exteral oscillator is running
//  do
//  {
//    CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
//    SFRIFG1 &= ~OFIFG;
//  } while (SFRIFG1 & OFIFG);                // Test oscillator fault flag


   // Configure USCI_A0 for UART mode
  UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 21-4: UCBRSx = 0x04
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA0BR0 = 52;                             // 8000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
 // UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt



    initque();  // set indexes
 
  __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, enable interrupts
  __no_operation();  
==========================================

// add TXISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG: break;
    case USCI_UART_UCTXIFG: 
     
        UCA0TXBUF = popque();     
        if(isempty()) 
        {
          UCA0IE &= !UCTXIE;  // turn off interrupt
        }
    
    break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}


void OutChVal(int seq, unsigned char Chan)
{
   unsigned char TXByte;
 // seq = ((seq>>2)&0x3ff)+512;
              // seq = seq+512;
              TXByte = seq & 0x003F;        // Set TXByte lower 6 bits first
              // lower byte has 0x00 for Chan code
              //TXByte = TXByte+58;                // 58->122 char
              // transmit char
//            while(!(UCA0IFG & UCTXIFG)); //wait for TX available
//               UCA0TXBUF = TXByte;
              pushque(TXByte);  // place on queue
              UCA0IE |= UCTXIE;   // turn on tx interrupt (even if on)
  
              TXByte = (seq >> 6);        // Set TXByte to the upper 4 bits
              TXByte = TXByte & 0x003F;        //
              TXByte = TXByte | Chan;            // from 41->57 char + msb
  
              //Transmit
//             while(!(UCA0IFG & UCTXIFG)); //wait for TX available
//             UCA0TXBUF = TXByte;
               pushque(TXByte);
}

char pushque(char val)
// still want to start txbuf and ie if pushque not empty
{  //w is always nextw
     mqb[qw] = val;  // write
        val = 0; // <<<<<not full
     // compute next write
     qw++;
    if(qw >=MAXQUE) qw=0;
        if(qw== qr) // then bump r, overwrite oldest
        {  // queue full, so bump oldest
            qr= qr+1;
          if( qr>=MAXQUE) qr=0;
          val = 1; //<<<<<full
        }

     return val;
}


char popque(void)
{   //if empty, count==0, so return same '0' value
     if(qr == qw) // then empty
      {  
        //undefined value
        return 0; //<<<<<<empty
     }
      char tmp = mqb[qr];  // do read
        if(++qr >=MAXQUE) qr=0; // 0-(MAX-1) 0-9
        return tmp;  // <<<<<<<not empty
}


void initque(void)

{
    qr=0;
    qw=0;
}

