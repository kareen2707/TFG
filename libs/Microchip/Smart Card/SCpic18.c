/********************************************************************
 FileName:		SCpic18.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18 Microcontrollers
 Hardware:		This demo is natively intended to be used on LPC & HPC Exp board.
 				This demo can be modified for use on other hardware platforms.
 Complier:  	Microchip C18 (for PIC18)
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
  1.0   Initial release
  1.01  Cleaned up unnecessary variables
  1.02  Optimized code by removing redundant code in "SCdrv_SetBRG"
        function.
********************************************************************/

#include 	<p18cxxx.h>
#include	"stdio.h"
#include 	"Smart Card/pps18.h"
#include 	"string.h"
#include	"GenericTypeDefs.h"
#include	"Smart Card/SCpic18.h"
#include    "sc_config.h"

unsigned int gBitTimeDelay;
unsigned int bitTime;
unsigned int initialBaudReg;
unsigned long scReferenceClock;

unsigned long baudRate;
unsigned int factorF = 372;
BYTE factorDNumerator = 1;
BYTE factorDdenominator = 1;
BOOL delayLapsedFlag = FALSE;

////////////////////////////////////////
// Generate delay in microsec resolution
// 0 - 2  us
// 1 - 3 us
// 10 - 12 us
// ...
////////////////////////////////////////
void WaitMicroSec( WORD microSec )
{
	while( microSec )
	{
		Nop();
		microSec--;
	}
}	

void WaitMilliSec( WORD ms )
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

	//calculate parity
	_asm
     swapf    data,0,0  //reducing byte to nibble
     xorwf    data,0,0
     addlw    0x41    // bit 1 becomes B0^B1 and bit 7 becomes B6^B7
     iorlw    0x7C    // for carry propagation from bit 1 to bit 7
     addlw    2       // Done! the parity bit is bit 7 of W
	_endasm
	
	TXSTAbits.TX9D = WREG >> 7; //copy calc parity bit to the transmitter
	
	TXREG = data;
	TXSTAbits.TXEN = 1;

	while( !TXSTAbits.TRMT ) //wait for Tx Complete
	{
		Nop();
	}	
	
	TXSTAbits.TXEN = 0;

	//turn off uart
	RCSTAbits.SPEN = 0;

	if( !SCdrv_GetRxPinData() )  // The Receiver did not like our data. it is pulling line low 
	{					  // to indicate PE or FR errors
		noError = FALSE;
	
		RCSTAbits.SPEN = 1;
						
		//now retransmit the data
		if( txRetryCounter < 5 )
		{
			txRetryCounter++;
			SCdrv_SendTxData(data);
		}
	}
	
	if( noError ) //no error detected
		txRetryCounter = 0;

	RCSTAbits.SPEN = 1;
	
	if( RCSTA & 0x6 )	//FERR or OERR occured
	{
		RCSTAbits.CREN = 0; //clear error flags
		RCSTAbits.CREN = 1;
	}	

	while(1)	// remove rx data recvd from our Tx line
	{
		BYTE temp;		
		if( PIR1bits.RCIF )
		{
			temp = RCREG;
		}	
		else
			break;
	}	
}

/////////////////////////////////////////////
/////////////////////////////////////////////
BOOL SCdrv_GetRxData( BYTE* pDat, unsigned long nTrys )
{
	//wait for data byte
	while( !(PIR1 & 0x20) )
	{
		if( nTrys-- == 0 )
			return FALSE;
	}
	
	//calculate parity
	_asm
     swapf    RCREG,0,0  //reducing byte to nibble
     xorwf    RCREG,0,0
     addlw    0x41    // bit 1 becomes B0^B1 and bit 7 becomes B6^B7
     iorlw    0x7C    // for carry propagation from bit 1 to bit 7
     addlw    2       // Done! the parity bit is bit 7 of W
     RLNCF    WREG,0,0		//rotate left W reg
     xorwf    RCSTA,0,0
	_endasm

	if( WREG & 1 )	//Parity Error detected
	{
		RCSTAbits.SPEN = 0;
		SCdrv_TxPin_Direction(0);  //pull it low to tell the card that there was error receiving data
		
		WaitMicroSec(gBitTimeDelay );
				
		SCdrv_TxPin_Direction(1); //release line. Card should retransmit now.
				
		RCSTAbits.SPEN = 1;
		return SCdrv_GetRxData( pDat, nTrys );	//Read the data from retransmission 
	}

	*pDat = RCREG;
	return TRUE;
}


///////////////////////////////////////////////////
//////////////////////////////////////////////////
void SCdrv_SetBRG( BYTE speedCode )
{
	BYTE tempCode;
	unsigned int baudReg;

	// If you are not using internal clock in Smart Card & are 
	// using external clock to drive Smart Card than calculate 
	// factor 'F' from TA1 value
	#ifdef ENABLE_SC_EXTERNAL_CLOCK

		tempCode = (speedCode & 0xF0) >> 4;
		
		if(tempCode == 0x00)	// If internal clock used in Smart Card
		{
			baudReg = (unsigned int)((FCY * factorDdenominator)/(unsigned long)(9600 * (unsigned long)factorDNumerator) - 1);
		}
		else	// If externa; clock used to drive Smart Card
		{
			baudRate = (scReferenceClock * factorDNumerator)/(unsigned long)(factorF * (unsigned long)factorDdenominator);
			baudReg = (unsigned int)((FCY/baudRate) - 1);
		}

	#else	// If internal clock used in Smart Card

		baudReg = (unsigned char)((FCY * factorDdenominator)/(unsigned long)(9600 * (unsigned long)factorDNumerator) - 1);

	#endif

	SPBRG = (unsigned char)baudReg;
	SPBRGH = (unsigned char)(baudReg >> 8);

	//recalculate bit time
	gBitTimeDelay = baudReg;
	gBitTimeDelay *= bitTime;	//micro sec for 1 bit @ 9.6kbps
	gBitTimeDelay /= initialBaudReg; 		
}	

///////////////////////////////////////////////////
//////////////////////////////////////////////////
void SCdrv_CloseUART(void)
{
	SCdrv_DisableClock();	//turn off Clock
	RCSTAbits.SPEN = 0;		//turn off UART
	RCSTAbits.CREN = 0;
}


///////////////////////////////////////////////////
//////////////////////////////////////////////////
void SCdrv_InitUART(void)
{
	unsigned int power2Value = 1;
	BYTE power2temp;

	SCdrv_PowerPin_Direction(0);	//set RB0 as output to power the Smart Card
	SCdrv_ResetPin_Direction(0);	//set RB4 as output for Smart Card Reset Pin
	SCdrv_CardPresent_Direction(1);  //RB3 Input Card Present
	SCdrv_SimPresent_Direction(1);   //RB1 Input Card Present
	
	SCdrv_SetTxPinData(0);
	SCdrv_TxPin_Direction(1);	// TX1 also act as gpio to pull the line low
	SCdrv_RxPin_Direction(1);	
	
	SCdrv_EnableCardSimPresentPinPullUp();	//enable pull up, active low

	SCdrv_SetSwitchCardPower(0);	//Turn off power to smart card
	SCdrv_SetSwitchCardReset(0); //keep card in reset state		

	// Set the clock value that you want to trigger the smart card.
	// Dont enable the clock,only set it.
	Scdrv_ClockSet();

	power2temp = REF_CLOCK_POWER2_VALUE;

	while(power2temp--)
	{
		power2Value = power2Value * (BYTE)2;
	}

	scReferenceClock = REF_CLOCK_CIRCUIT_INPUT_CLK/(power2Value + REF_CLOCK_DIVISOR_VALUE);

	baudRate = scReferenceClock/(unsigned int)372;

	initialBaudReg = ((FCY/baudRate) - 1);
	SPBRG = (unsigned char)initialBaudReg;
	SPBRGH = (unsigned char)(initialBaudReg >> 8);
	bitTime = (1000000UL)/baudRate + 1;

	gBitTimeDelay = initialBaudReg;
	gBitTimeDelay *= bitTime;	//micro sec for 9.6k baud
	gBitTimeDelay /= initialBaudReg;  

	TXSTAbits.TX9  = 1; //select 9 bit transmission
	TXSTAbits.SYNC = 0; //Async mode	
	TXSTAbits.BRGH = 1;
	BAUDCONbits.BRG16 = 1;

	RCSTAbits.CREN = 1;
	RCSTAbits.RX9  = 1;

	// Enable Delay Timer Interrupts for future use
	SCdrv_EnableDelayTimerIntr();
}


