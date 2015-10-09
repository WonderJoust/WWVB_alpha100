#include <msp430g2553.h>
#include "1602A-4.h"

//----------------------------------------------------------------------
//----------------------------------------------------------------------

#define LCD0 0x001
#define LCD1 0x002
#define LCD2 0x004
#define LCD3 0x008

#define LCDRegSel 0x040
#define LCDEnable 0x080

volatile char LCDOutPut[2][16];
volatile char PrvLCDOutPut[2][16];

int main(void);
void init(void);
void outPutPlz(void);

//----------------------------------------------------------------------MAIN
int main(void) {

	init();

	outPutPlz();

    return 0;
}
//----------------------------------------------------------------------/MAIN
//----------------------------------------------------------------------/

//----------------------------------------------------------------------INIT
void init(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P1DIR = LCD0 + LCD1 +LCD2 +LCD3 + LCDRegSel + LCDEnable;		//LCD outputs

    lcdinit();

	unsigned int j=0;

	for (j=0; j<16; j++)
	{
		PrvLCDOutPut[0][j] = 0;
		PrvLCDOutPut[1][j] = 0;
		LCDOutPut[0][j] = 'b';
		LCDOutPut[1][j] = 'a';
	}
	//---GENERAL OUTPUT FORMAT INIT
	//			0123456789ABCDEF
	//LINE1--> 	_DD/MM/YY_HH:MM_		(running clock)
	//LINE2--> 	!DD/MM/YY_HH:MM!		(last time rcvd)

	LCDOutPut[0][0] = ' ';
	LCDOutPut[0][3] = '/';
	LCDOutPut[0][6] = '/';
	LCDOutPut[0][9] = ' ';
	LCDOutPut[0][12] = ':';
	LCDOutPut[0][15] = ' ';

	LCDOutPut[1][0] = '!';
	LCDOutPut[1][3] = '/';
	LCDOutPut[1][6] = '/';
	LCDOutPut[1][9] = ' ';
	LCDOutPut[1][12] = ':';
	LCDOutPut[1][15] = '!';

	//---/GENERAL OUTPUT FORMAT INIT
	
	//ADC Stuff
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	ADC10CTL1 = SHS_1 + CONSEQ_2 + INCH_1;    // TA1 trigger sample start
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
	__enable_interrupt();                     // Enable interrupts.
	TACCR0 = 30;                              // Delay to allow Ref to settle
	TACCTL0 |= CCIE;                          // Compare-mode interrupt.
	TACTL = TASSEL_2 + MC_1;                  // TACLK = SMCLK, Up mode.
	LPM0;                                     // Wait for delay.
	TACCTL0 &= ~CCIE;                         // Disable timer Interrupt
	__disable_interrupt();
	ADC10CTL0 |= ENC;                         // ADC10 Enable
	ADC10AE0 |= 0x02;                         // P1.1 ADC10 option select
	P1DIR |= 0x01;                            // Set P1.0 output
	TACCR0 = 2048-1;                          // PWM Period
	TACCTL1 = OUTMOD_3;                       // TACCR1 set/reset
	TACCR1 = 2047;                            // TACCR1 PWM Duty Cycle
	TACTL = TASSEL_1 + MC_1;                  // ACLK, up mode

	__bis_SR_register(LPM3_bits + GIE);       // Enter LPM3 w/ interrupts


}
//----------------------------------------------------------------------/INIT
//----------------------------------------------------------------------/

//----------------------------------------------------------------------OUTPUTPLZ
void outPutPlz(void) {

	unsigned int i;
	unsigned int j;

	for (i=0; i<2; i++)
	{
		for (j=0; j<16; j++)
		{
			if (LCDOutPut[i][j] != PrvLCDOutPut[i][j])
			{
				gotoXy(j,i);
				lcdData(LCDOutPut[i][j]);

				PrvLCDOutPut[i][j] = LCDOutPut[i][j];
			}
		}
	}
}
//----------------------------------------------------------------------/OUTPUTPLZ
//----------------------------------------------------------------------


// ADC10 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
  if (ADC10MEM < 0x155)                     // ADC10MEM = A1 > 0.5V?
    P1OUT &= ~0x01;                         // Clear P1.0 LED off
  else
    P1OUT |= 0x01;                          // Set P1.0 LED on
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ta0_isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) ta0_isr (void)
#else
#error Compiler not supported!
#endif
{
  TACTL = 0;
  LPM0_EXIT;                                // Exit LPM0 on return
}
