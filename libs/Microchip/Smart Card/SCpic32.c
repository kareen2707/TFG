/********************************************************************
 FileName:		SCpic32.c
 Dependencies:	See INCLUDES section
 Processor:		PIC32 Microcontrollers
 Hardware:		This demo is natively intended to be used on Exp 16 board.
 				This demo can be modified for use on other hardware platforms.
 Complier:  	Microchip C32 (for PIC32)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
 File Description:

 Change History:
  Rev   Description
  ----  -----------------------------------------
  1.02  Initial release
********************************************************************/

#include 	"string.h"
#include	"GenericTypeDefs.h"
#include 	"sc_config.h"
#include    "./Smart Card/SCpic32.h"

unsigned long baudRate;
unsigned long scReferenceClock; // Smart Card Reference Clock
unsigned int factorF = 372;
BYTE factorDNumerator = 1;
BYTE factorDdenominator = 1;
BOOL delayLapsedFlag = FALSE;

void WaitMicroSec( unsigned int microSec )
{
	BYTE temp = (BYTE)(FCY/1000000UL);

	while(--microSec)
	{
		// Set the Timer Count as per the delay needed
		SCdrv_SetDelayTimerCnt(0xFFFF - temp);

		// Enable the delay timer
		SCdrv_EnableDelayTimer();

		// Wait until the delay is elapsed
		while(!delayLapsedFlag);

		// Clear the delay flag
		delayLapsedFlag = FALSE;
	}
}	

void WaitMilliSec( unsigned int ms )
{
	while( ms-- )
		WaitMicroSec( 800 );
}


/*******************************************************************************
  Function:
    void SC_Delay(void)
	
  Description:
    This function waits for delay to get completed

  Precondition:
    None.

  Parameters:
    unsigned int instructionCount - Number of instruction counts to be waited

  Return Values:
    None
	
  Remarks:
    None.
  *****************************************************************************/
void SC_Delay(unsigned int instructionCount)
{
	// Set the Timer Count as per the delay needed
	SCdrv_SetDelayTimerCnt(0xFFFF - instructionCount);

	// Enable the delay timer
	SCdrv_EnableDelayTimer();

	// Wait until the delay is elapsed
	while(!delayLapsedFlag);

	// Clear the delay flag
	delayLapsedFlag = FALSE;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
void SCdrv_SendTxData( BYTE data )
{
	BYTE txRetryCounter = 0;
	BOOL noError = TRUE;

	U1STAbits.URXEN	= 0;	//Recieve Disable
	U1STAbits.UTXEN = 1;
	U1TXREG = data;		

	while( !U1STAbits.TRMT )
	{
		Nop();
		Nop();
	}
	
	U1STAbits.UTXEN = 0;

	U1MODEbits.UARTEN = 0;	// Disable UART Module

	WaitMicroSec( 1 );
	
	if( !SCdrv_GetRxPinData() )  // The Receiver did not like our data. it is pulling line low 
	{					  // to indicate PE or FR errors
		noError = FALSE;

//		WaitMicroSec((U1BRG * 170)/371);  //wait two etu before repeating
		
		U1MODEbits.UARTEN = 1;
						
		//now retransmit the data
		if( txRetryCounter < 5 )
		{
			txRetryCounter++;
			SCdrv_SendTxData(data);
		}
	}
	else
	{	
//		WaitMicroSec((U1BRG * 140)/371);  //wait 1.5 etu
	}

	if( noError ) //no error detected
		txRetryCounter = 0;

	U1MODEbits.UARTEN = 1;	// Enable UART Module
	U1STAbits.URXEN	= 1;	//Recieve Enable	
	U1STAbits.OERR = 0;	//clear any overflow error that we caused
	
	while(1)	// remove rx data recvd from our Tx line
	{
		WORD temp;		
		if( U1STAbits.URXDA )
			temp = U1RXREG;
		else
			break;
	}	
}

/////////////////////////////////////////////
/////////////////////////////////////////////
BOOL SCdrv_GetRxData( BYTE* pDat, unsigned long nTrys )
{
	//wait for data byte
	while( !U1STAbits.URXDA && nTrys-- );
	
	if( !U1STAbits.URXDA )
		return FALSE;
	
	if( U1STAbits.PERR )	//Parity Error detected
	{
		SCdrv_TxPin_Direction(0);  //pull it low to tell the card that there was error receiving data
		U1MODEbits.RXINV = 1;  //do not recognize this low state as a valid start bit
					
		//Read the data from UART to clear the error flag
		*pDat = U1RXREG;		
	
		WaitMicroSec((U1BRG * 116)/371);  //for 9600 baud, 116 us. for 250kbps, 5us

		SCdrv_TxPin_Direction(1); //release RD10. Card should retransmit now.

		U1MODEbits.RXINV = 0;

		return SCdrv_GetRxData(pDat, 10000);	//Read the data from retransmission
	}
	else
	{
		//Read the data from UART
		*pDat = U1RXREG;
	}

	return TRUE;
}

///////////////////////////////////////////////////
//////////////////////////////////////////////////
void SCdrv_SetBRG( BYTE speedCode )
{
	BYTE tempCode;

	// If you are not using internal clock in Smart Card & are 
	// using external clock to drive Smart Card than calculate 
	// factor 'F' from TA1 value
	#ifdef ENABLE_SC_EXTERNAL_CLOCK

		tempCode = (speedCode & 0xF0) >> 4;

		if(tempCode == 0x00)	// If internal clock used in Smart Card
		{
			U1BRG = (unsigned int)((unsigned long)((unsigned long)(FCY * factorDdenominator)/(unsigned long)(4 * 9600 * (unsigned long)factorDNumerator)) - 1);   //Internal clk used in card
		}
		else	// If externa; clock used to drive Smart Card
		{
			baudRate = (unsigned long long)((unsigned long long)((unsigned long long)scReferenceClock * factorDNumerator)/(unsigned long)(factorF * (unsigned long)factorDdenominator));

			U1BRG = (unsigned int)((unsigned long)((unsigned long)FCY/(4 * baudRate)) - 1);   //10752bps with 4Mhz clk to card
		}

	#else	// If internal clock used in Smart Card

		U1BRG = (unsigned int)((unsigned long)((unsigned long)(FCY * factorDdenominator)/(unsigned long)(4 * 9600 * (unsigned long)factorDNumerator)) - 1);   //Internal clk used in card

	#endif
}	
	
///////////////////////////////////////////////////
//////////////////////////////////////////////////
void SCdrv_CloseUART(void)
{
	SCdrv_DisableClock();		// turn off Clock
	U1MODEbits.UARTEN = 0;  // Turn off UART
	U1STAbits.OERR = 0;;

	// Disable Pull-ups at Tx & Rx pins
	SCdrv_DisableTxPinPullUp();
	SCdrv_DisableRxPinPullUp();	
}

///////////////////////////////////////////////////
//////////////////////////////////////////////////
void SCdrv_InitUART(void)
{
	unsigned int power2Value = 1;
	BYTE power2temp;

	#ifdef ENABLE_SC_POWER_THROUGH_PORT_PIN
		SCdrv_PowerPin_Direction(0);	//set RG8 as output to power the Smart Card
	#endif
	
	SCdrv_ResetPin_Direction(0);	//set RD0 as output for Smart Card Reset Pin
	SCdrv_CardPresent_Direction(1);   //RD3 Input Card Present - SmartCard Conn Active Hi
	SCdrv_SimPresent_Direction(1);   //RG12 Input Card Present - SimCard Conn Active Low
	
	SCdrv_SetTxPinData(0);
	SCdrv_TxPin_Direction(1);	// use as gpio to pull the line low

	//Turn on the pull-up on both RX and TX line for faster transitions.
	SCdrv_EnableTxPinPullUp();
	SCdrv_EnableRxPinPullUp();

	// Enable Pull-ups for card present/sim present pins
	SCdrv_EnableCardPresentPinPullUp();
	SCdrv_EnableSimPresentPinPullUp();

	#ifdef ENABLE_SC_POWER_THROUGH_PORT_PIN
		SCdrv_SetSwitchCardPower(0);	//Turn off power to smart card
	#endif

	SCdrv_SetSwitchCardReset(0); //keep card in reset state		

	MapUART1RxPin();		// Map UART1 Rx pin
	MapUART1TxPin();		// Map UART1 Tx pin

	Scdrv_ClockSet();

	// Initial Baud Rate of Smart Card for external Clock
	#ifdef ENABLE_SC_EXTERNAL_CLOCK

		power2temp = REF_CLOCK_POWER2_VALUE;

		while(power2temp--)
		{
			power2Value = power2Value * (BYTE)2;
		}

		scReferenceClock = REF_CLOCK_CIRCUIT_INPUT_CLK/(power2Value + REF_CLOCK_DIVISOR_VALUE);
	
		baudRate = scReferenceClock/372;

		U1BRG = (unsigned int)((unsigned long)((unsigned long)FCY/(4 * baudRate)) - 1);   //10752bps with 4Mhz clk to card
	
	#else
	
		U1BRG = (unsigned int)((unsigned long)((unsigned long)FCY/(4 * (unsigned int)9600)) - 1);   //Internal clk used in card

	#endif

	U1STAbits.URXEN	= 1;	//Recieve Enable
	U1MODEbits.PDSEL	= 1;	//8bits + even parity
	U1MODEbits.STSEL	= 0;	//1 stop bit
	U1MODEbits.BRGH		= 1;

	// Enable Delay Timer Interrupts for future use
	SCdrv_EnableDelayTimerIntr();

}





