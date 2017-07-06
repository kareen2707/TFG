
#include "Compiler.h"
#include "Common/timer.h"
#include "HardwareProfile.h"

#define __DELAY_C

#if defined(__C32__)


//Guillermo. Le cambio el nombre porque hay conflicto con TimeDelay.c.
//void DelayMs(WORD ms)
//Y el nuevo nombre
void DelayMsTimer(WORD ms)
{
    unsigned char i;
    while(ms--)
    {
        i=4;
        while(i--)
        {
            Delay10us(25);
        }
    }
}

//Guillermo. Le cambio el nombre porque hay conflicto con TimeDelay.c.
//void Delay10us(DWORD dwCount)
//Y el nuevo nombre
void Delay10usTimer(DWORD dwCount)
{
	volatile DWORD _dcnt;

	_dcnt = dwCount*((DWORD)(0.00001/(1.0/GetInstructionClock())/10));
	while(_dcnt--)
	{
		#if defined(__C32__)
			Nop();
			Nop();
			Nop();
		#endif
	}
}
#endif

