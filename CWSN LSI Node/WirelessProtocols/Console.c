/*******************************************************************************
* FileName:	Console.c                       MODIFIED FROM ORIGINAL STACK!!
* Dependencies: Console.h
* Processor:	PIC18, PIC24F, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710
* Hardware:	PICDEM Z, Explorer 16, PIC18 Explorer
* Complier:     Microchip C18 v3.04 or higher
*		Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher
* Company:	Microchip Technology, Inc.
*
* Copyright � 2007-2010 Microchip Technology Inc.  All rights reserved.
*
* Microchip licenses to you the right to use, modify, copy and distribute
* Software only when embedded on a Microchip microcontroller or digital signal
* controller and used with a Microchip radio frequency transceiver, which are
* integrated into your product or third party product (pursuant to the terms in
* the accompanying license agreement).
*
* You should refer to the license agreement accompanying this Software for
* additional information regarding your rights and obligations.
*
* SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
* MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
* CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
* OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
* INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
* CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
* SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
* (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
********************************************************************************
* File Description:
*
*   This file configures and provides the function for using the
*   UART to transmit data over RS232 to the computer.
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   11/09/2006   yfy       Initial revision
*  1.0   01/09/2007   yfy       Initial release
*  2.0   4/24/2009    yfy       Modified for MiApp interface
*  2.1   6/20/2009    yfy       Add LCD support
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
*******************************************************************************/

/********************************** HEADERS ***********************************/
#include "WirelessProtocols/Console.h"
#include "WirelessProtocols/ConfigApp.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "NodeHAL.h"

#include "USB/usb_config.h"
#include "./USB/usb.h"
#include "./USB/usb_function_cdc.h"
#include "USB/usb_device.h"

#if defined(ENABLE_CONSOLE)
#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || \
    defined(__PIC24H__) || defined(__PIC32MX__)

/******************************* VARIABLES ************************************/
ROM unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A',
                                    'B','C','D','E','F'};
unsigned char TempCharacterArray[] = {'a','a','a'};

#if defined DEBUG_USB

    char USB_Out_Buffer[CDC_DATA_OUT_EP_SIZE];
    char RS232_Out_Data[CDC_DATA_IN_EP_SIZE];
    char USB_In_Buffer[CDC_DATA_IN_EP_SIZE];//XXX-Willy.
    BOOL USB_Console_Initialized = FALSE; //XXX_Willy.

    unsigned char  NextUSBOut;
    unsigned char    NextUSBOut;

    unsigned char    LastRS232Out;  // Number of characters in the buffer
    unsigned char    RS232cp;       // current position within the buffer
    unsigned char RS232_Out_Data_Rdy = 0;
    USB_HANDLE  lastTransmission;

#endif

/******************************* FUNCTIONS ************************************/
/******************************************************************************
 * Function:        BYTE USB_Console_Tasks(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          1 si esta listo para procesar la I/O.
 *
 * Side Effects:    None
 *
 * Overview:        Hace efectivo el env�o. Debe ejecutarse al menos una vez
 *                  por bucle.
 *
 * Note:            None
 *****************************************************************************/
BYTE USB_Console_Tasks(void)
{
    CDCTxService();
    #if defined(USB_POLLING)
            // Check bus status and service USB interrupts.
        USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
                                          // this function periodically.  This function will take care
                                          // of processing and responding to SETUP transactions
                                          // (such as during the enumeration process when you first
                                          // plug in).  USB hosts require that USB devices should accept
                                          // and process SETUP packets in a timely fashion.  Therefore,
                                          // when using polling, this function should be called
                                          // regularly (such as once every 1.8ms or faster** [see
                                          // inline code comments in usb_device.c for explanation when
                                          // "or faster" applies])  In most cases, the USBDeviceTasks()
                                          // function does not take very long to execute (ex: <100
                                          // instruction cycles) before it returns.
    #endif
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return 0;
    return 1;
}

/*******************************************************************************
* Function:         void ConsoleInit(void)
* PreCondition:     none
* Input:            none
* Output:	    none
* Side Effects:	    UART selected for debugging is configured
* Overview:         This function will configure the UART with the options:
*                   8 bits data, 1 stop bit, no flowcontrol mode
* Note:		    None
*******************************************************************************/
void ConsoleInit(void){
#if defined __32MX795F512H__ || defined __32MX795F512L__ || defined __32MX675F256L__ // MCU names
        #if defined DEBUG_UART1
            U1BRG = ((CLOCK_FREQ/(1<<mOSCGetPBDIV()))/(UBRGH_DIV*BAUD_RATE))-1; //
            U1STA = 0;
            U1MODE = UMODE_CONF;
            U1STA = (1 << 12)|UART_TX_ENABLE;   //0x1400    TX/RX EN
        #elif defined DEBUG_UART2
            U2BRG = ((CLOCK_FREQ/(1<<mOSCGetPBDIV()))/(UBRGH_DIV*BAUD_RATE))-1;
            U2STA = 0;
            U2MODE = UMODE_CONF;
            U2STA = (1 << 12)|UART_TX_ENABLE;   //0x1400    TX/RX EN
        #elif defined DEBUG_UART3
            U3BRG = ((CLOCK_FREQ/(1<<mOSCGetPBDIV()))/(UBRGH_DIV*BAUD_RATE))-1;
            U3STA = 0;
            U3MODE = UMODE_CONF;
            U3STA = (1 << 12)|UART_TX_ENABLE;   //0x1400    TX/RX EN
        #elif defined DEBUG_UART4
            U4BRG = ((CLOCK_FREQ/(1<<mOSCGetPBDIV()))/(UBRGH_DIV*BAUD_RATE))-1;
            U4STA = 0;
            U4MODE = UMODE_CONF;
            U4STA = (1 << 12)|UART_TX_ENABLE;   //0x1400    TX/RX EN
        #elif defined DEBUG_UART5
            U5BRG = ((CLOCK_FREQ/(1<<mOSCGetPBDIV()))/(UBRGH_DIV*BAUD_RATE))-1;
            U5STA = 0;
            U5MODE = UMODE_CONF;
            U5STA = (1 << 12)|UART_TX_ENABLE;   //0x1400    TX/RX EN
        #elif defined DEBUG_UART6
            U6BRG = ((CLOCK_FREQ/(1<<mOSCGetPBDIV()))/(UBRGH_DIV*BAUD_RATE))-1;
            U6STA = 0;
            U6MODE = UMODE_CONF;
            U6STA = (1 << 12)|UART_TX_ENABLE;   //0x1400    TX/RX EN
        #elif defined DEBUG_USB

            AD1PCFG = 0xFFFF;


        //	The USB specifications require that USB peripheral devices must never source
        //	current onto the Vbus pin.  Additionally, USB peripherals should not source
        //	current on D+ or D- when the host/hub is not actively powering the Vbus line.
        //	When designing a self powered (as opposed to bus powered) USB peripheral
        //	device, the firmware should make sure not to turn on the USB module and D+
        //	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
        //	firmware needs some means to detect when Vbus is being powered by the host.
        //	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
        // 	can be used to detect when Vbus is high (host actively powering), or low
        //	(host is shut down or otherwise not supplying power).  The USB firmware
        // 	can then periodically poll this I/O pin to know when it is okay to turn on
        //	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
        //	peripheral device, it is not possible to source current on D+ or D- when the
        //	host is not actively providing power on Vbus. Therefore, implementing this
        //	bus sense feature is optional.  This firmware can be made to use this bus
        //	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
        //	HardwareProfile.h file.
            #if defined(USE_USB_BUS_SENSE_IO)
                 tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
            #endif

        //	If the host PC sends a GetStatus (device) request, the firmware must respond
        //	and let the host know if the USB peripheral device is currently bus powered
        //	or self powered.  See chapter 9 in the official USB specifications for details
        //	regarding this request.  If the peripheral device is capable of being both
        //	self and bus powered, it should not return a hard coded value for this request.
        //	Instead, firmware should check if it is currently self or bus powered, and
        //	respond accordingly.  If the hardware has been configured like demonstrated
        //	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
        //	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2"
        //	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
        //	has been defined in HardwareProfile.h, and that an appropriate I/O pin has been mapped
        //	to it in HardwareProfile.h.

            #if defined(USE_SELF_POWER_SENSE_IO)
                tris_self_power = INPUT_PIN;	// See HardwareProfile.h
            #endif


            // USER INIT

            UARTConfigure(UART3, UART_ENABLE_PINS_TX_RX_ONLY);
            UARTSetFifoMode(UART3, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
            UARTSetLineControl(UART3, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
            UARTSetDataRate(UART3, GetPeripheralClock(), USB_BAUD_RATE);
            UARTEnable(UART3, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

            unsigned char i;
            // 	 Initialize the arrays
            for (i=0; i<sizeof(USB_Out_Buffer); i++)
                {
                    USB_Out_Buffer[i] = 0;
                }

             NextUSBOut = 0;
             LastRS232Out = 0;
             lastTransmission = 0;

             USBDeviceInit();

              #if defined(USB_INTERRUPT)
                USBDeviceAttach();
              #endif

           /*     //se termina de inicializar consola pulsando 's'
                char tecla = ConsoleGet();
                while(tecla!='s'){
                    tecla = ConsoleGet();
                }
                BYTE lines = 20;
                for(i=0;i<lines;i++){
                    Printf("\n\r");
                } */

                SWDelay(5000);

        #endif
    #else
        #error Microcontroller not supported.
    #endif
}

/*******************************************************************************
* Function:         void ConsolePut(BYTE c)
* PreCondition:     none
* Input:	    c - character to be printed
* Output:           none
* Side Effects:	    c is printed to the console
* Overview:         This function will print the inputed character
* Note:             Do not power down the microcontroller until the transmission
*                   is complete or the last transmission of the string can be
*                   corrupted.
*******************************************************************************/
void ConsolePut(BYTE c){
    #if defined DEBUG_UART1
        while(U1STAbits.TRMT == 0);
        U1TXREG = c;
    #elif defined DEBUG_UART2
        while(U2STAbits.TRMT == 0);
        U2TXREG = c;
    #elif defined DEBUG_UART3
        while(U3STAbits.TRMT == 0);
        U3TXREG = c;
    #elif defined DEBUG_UART4
        while(U4STAbits.TRMT == 0);
        U4TXREG = c;
    #elif defined DEBUG_UART5
        while(U5STAbits.TRMT == 0);
        U5TXREG = c;
    #elif defined DEBUG_UART6
        while(U6STAbits.TRMT == 0);
        U6TXREG = c;
    #elif defined DEBUG_USB
        TempCharacterArray[0] = c;
        TempCharacterArray[1] = '\0';
        ConsolePutROMString(&TempCharacterArray[0]);
    #endif
}

/*******************************************************************************
* Function:         BYTE ConsoleGet(void)
* PreCondition:     none
* Input:	    none
* Output:	    one byte received by UART
* Side Effects:	    none
* Overview:	    This function will receive one byte from UART
* Note:             Do not power down the microcontroller until the transmission
*                   is complete or the last transmission of the string can be
*                   corrupted.
*******************************************************************************/
BYTE ConsoleGet(void){
    char Temp;
    //BYTE numBytes;

    #if defined DEBUG_UART1
        while(IFS0bits.U1RXIF == 0);
        Temp = U1RXREG;
        IFS0bits.U1RXIF = 0;
    #elif defined DEBUG_UART2
        while(IFS1bits.U2RXIF == 0);
        Temp = U2RXREG;
        IFS1bits.U2RXIF = 0;
    #elif defined DEBUG_UART3
        while(IFS1bits.U3RXIF == 0);
        Temp = U3RXREG;
        IFS1bits.U3RXIF = 0;
    #elif defined DEBUG_UART4
        while(IFS2bits.U4RXIF == 0);
        Temp = U4RXREG;
        IFS2bits.U4RXIF = 0;
    #elif defined DEBUG_UART5
        while(IFS2bits.U5RXIF == 0);
        Temp = U5RXREG;
        IFS2bits.U5RXIF = 0;
    #elif defined DEBUG_UART6
        while(IFS2bits.U6RXIF == 0);
        Temp = U6RXREG;
        IFS2bits.U6RXIF = 0;
    #elif defined DEBUG_USB
        while(!USB_Console_Tasks());
        while(getsUSBUSART(USB_In_Buffer,sizeof(USB_In_Buffer))== 0); // PRUEBA
      //  if(getsUSBUSART(USB_In_Buffer,sizeof(USB_In_Buffer)) > 0)
        //{
            Temp = USB_In_Buffer[0];
        //we received numBytes bytes of data and they are copied into
        //  the "buffer" variable.  We can do something with the data
        //  here.
        //}else{Temp= 0;}
     #endif

    return Temp;
}

/*******************************************************************************
* Function:         void PrintChar(BYTE toPrint)
* PreCondition:     none
* Input:	    toPrint - character to be printed
* Output:           none
* Side Effects:	    toPrint is printed to the console
* Overview:         This function will print the inputed BYTE to the console in
*                   hexidecimal form
* Note:             Do not power down the microcontroller until the transmission
*                   is complete or the last transmission of the string can be
*                   corrupted.
*******************************************************************************/
void PrintChar(BYTE toPrint){
    BYTE PRINT_VAR;
    PRINT_VAR = toPrint;
    toPrint = (toPrint>>4)&0x0F;

    ConsolePut(CharacterArray[toPrint]);

    toPrint = (PRINT_VAR)&0x0F;

    ConsolePut(CharacterArray[toPrint]);

}

/*******************************************************************************
* Function:         void ConsolePutROMString(ROM char* str)
* PreCondition:     none
* Input:	    str - ROM string that needs to be printed
* Output:           none
* Side Effects:	    str is printed to the console
* Overview:         This function will print the inputed ROM string
* Note:             Do not power down the microcontroller until the transmission
*                   is complete or the last transmission of the string can be
*                   corrupted.
*******************************************************************************/
void ConsolePutROMString(ROM char* str){

#if defined DEBUG_USB
    while (!USB_Console_Tasks());
        while(!USBUSARTIsTxTrfReady())
            CDCTxService();
        putrsUSBUSART(str);

#else

    BYTE c;
    while((c = *str++))
        ConsolePut(c);

#endif

}

/*******************************************************************************
* Function:         void PrintDec(BYTE toPrint)
* PreCondition:     none
* Input:	    toPrint - character to be printed. Range is 0-99
* Output:           none
* Side Effects:	    toPrint is printed to the console in decimal
* Overview:         This function will print the inputed BYTE to the console in
*                   decimal form
* Note:             Do not power down the microcontroller until the transmission
*                   is complete or the last transmission of the string can be
*                   corrupted.
*******************************************************************************/
void PrintDec(BYTE toPrint){
    BYTE a = toPrint/10;
    BYTE b = toPrint%10;
    char aa;
    if(toPrint>9){
        aa = CharacterArray[a];
        }else{
        aa = '0';
        }
    char bb = CharacterArray[b];
    TempCharacterArray[0]=aa;
    TempCharacterArray[1]=bb;
    TempCharacterArray[2]='\0';
    ConsolePutROMString(&TempCharacterArray[0]);
}

/********************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user
 *                  routines. It is a mixture of both USB and
 *                  non-USB tasks.
 *
 * Note:            None
 *******************************************************************/
void ProcessIO(void)
{

char tecla;
tecla=ConsoleGet();
if(tecla!=0)
{
    switch(tecla)
    {
        case 'a':
            Printf("\r\nRecibido una a.");
            break;
        case 'b':
            Printf("\r\nRecibido una b.");
            break;
        case 'c':
            Printf("\r\nRecibido una c.");
            break;
        default:
            Printf("\r\nLo que se ha recibido no se contemplaba.");
            break;
    }
}

}//end ProcessIO





/******************************************************************************
 pruebecita para probar funcionalidad
 *****************************************************************************/
int mainApppppp(void)
{
    int counter = 0;
    //USB_Console_Init();
    ConsoleInit();
    while(1)
    {
        //DelayMs(1000);
        //Printf("\r\n Starting MiWi(TM) LSI-CWSN Stack ...");
       // counter = counter + 1;
    	char tecla;
        tecla=ConsoleGet();

        char* str = "\r\ney! ";
        char* str1 = "\r\noh!";
        //char* both = malloc(strlen(str) + strlen(&counter) + 1);
        //strcpy(both, str);
        //strcat(both, &counter);

        if(tecla!=0){
            if(counter == 0){
             Printf(str);
             counter = 1;
            }else{
                Printf(str1);
                counter = 0;}

        }
        //USB_Console_Tasks();
        }

            /*if(USB_Console_Tasks())
        {
            //ProcessIO();
              Printf("\n1 Starting MiWi(TM) LSI-CWSN Stack ...");
                Printf("\n2 Starting MiWi(TM) LSI-CWSN Stack ...");
                Printf("\n3 Starting MiWi(TM) LSI-CWSN Stack ...");
            Printf("\n4 Starting MiWi(TM) LSI-CWSN Stack ...");

        }*/


}//end main

#else
    #error Unknown processor.  See Compiler.h
#endif

#endif  //ENABLE_CONSOLE
