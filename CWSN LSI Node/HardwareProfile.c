/*******************************************************************************
 *              HardwareProfile.c -- Hardware Profile
 *******************************************************************************
 * FileName:        HardwareProfile.c
 * Dependencies:
 * Processor:       PIC18, PIC24, PIC32, dsPIC30, dsPIC33
 * Compiler:        C18 02.20.00 or higher
 *                  C30 02.03.00 or higher
 *                  C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright ? 2007-2010 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, modify, copy and distribute 
 * Software only when embedded on a Microchip microcontroller or digital 
 * signal controller and used with a Microchip radio frequency transceiver, 
 * which are integrated into your product or third party product (pursuant 
 * to the terms in the accompanying license agreement).   
 *
 * You should refer to the license agreement accompanying this Software for 
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED ?AS IS? WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A 
 * PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE 
 * LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, 
 * CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY 
 * DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO 
 * ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
 * LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, 
 * TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT 
 * NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************
 * File Description:
 *
 * This file provides configuration and basic hardware functionality based on 
 * chosen hardware. It has been adapted for LSI CWSN node from demo boards and
 * examples. Configuration chosen is applied by invoking the initialization
 * functions.
 *
 * Change History:
 *  Rev   Date         Description
 *  0.1   2/17/2009    Initial revision
 *  3.1   5/28/2010    MiWi DE 3.1
 * Author: Juan Domingo Rebollo - Laboratiorio de Sistemas Integrados (LSI-UPM)
 ******************************************************************************/
#include "Compiler.h"
#include "WirelessProtocols/Console.h"
#include "Common/TimeDelay.h"
#include "HardwareProfile.h"
#include "NodeHAL.h"

BOOL PUSH_BUTTON1_pressed;
BOOL PUSH_BUTTON2_pressed;

#if defined(__PIC32MX__)
    /* MAX SPI CLOCK FREQ SUPPORTED FOR MIWI TRANSCIEVER */
    #if defined(MRF89XA)
        #define MAX_SPI_CLK_FREQ_FOR_P2P             (1000000)
    #else
        #define MAX_SPI_CLK_FREQ_FOR_P2P             (1000000)
    #endif
    #define        Sleep()      PowerSaveSleep()
#endif

/*******************************************************************************
 * Function:    BoardInit(void)
 * PreCondition:None
 * Input:       None
 * Output:      None
 * Overview:    SPI pins and SFR, Maintenance Tasks Timer, External Interrupts,
 *              and other board issues initialization.
 * Note:        This routine needs to be called before initialising MiWi stack
 *              or invoking other function that operates on MiWi stack.
 ******************************************************************************/
void BoardInit(void){
    #if defined(__PIC32MX__)

    // RADIO INTERFACES & SPI INIT -------------------------------------------//
        #if defined HARDWARE_SPI
            /* Peripheral Bus Frequency = System Clock / PB Divider */
            unsigned int pbFreq;
            pbFreq = (DWORD) CLOCK_FREQ/(1 << mOSCGetPBDIV());

            unsigned int SPI_Clk_Freq;
            unsigned char SPI_Brg;
        #endif

        #if defined MRF24J40
            PHY_CS_TRIS = OUTPUT_PIN;
            PHY_CS = 1;
            PHY_RESETn_TRIS = OUTPUT_PIN;
            PHY_RESETn = 1;

            MRF24J40_INT_TRIS = INPUT_PIN;

            SDI_TRIS = INPUT_PIN;
            SDO_TRIS = OUTPUT_PIN;
            SCK_TRIS = OUTPUT_PIN;
            SPI_SDO = 0;
            SPI_SCK = 0;

            PHY_WAKE_TRIS = OUTPUT_PIN;
            PHY_WAKE = 1;

            MRF24J40_PWR_TRIS = OUTPUT_PIN;
            MRF24J40_PWR = 1;

            SPICONCLR = 0xFFFFFFFF;             // Clear SPIxCON register

            #ifdef HARDWARE_SPI
                /* Enable SPI, Set to Master Mode & Set CKE bit : Serial output
                 * data changes on transition from active clock state to Idle
                 * clock state */
                SPICON = 0x00008120;

                /* PB Frequency can be maximum 40 MHz */
                if(pbFreq > (2 * MAX_SPI_CLK_FREQ_FOR_P2P)){
                    SPI_Brg = 1;
                    /* Continue the loop till you find SPI Baud Rate Reg Value */
                    while(1){
                        /* SPI Clock Calculation as per PIC32 Manual */
                        SPI_Clk_Freq = pbFreq / (2 * (SPI_Brg + 1));

                        if(SPI_Clk_Freq <= MAX_SPI_CLK_FREQ_FOR_P2P){
                            break;
                        }
                        SPI_Brg++;
                    }
                    #if defined MRF24J40_IN_SPI1
                    mSpiChnSetBrg (1, SPI_Brg);
                    #elif defined MRF24J40_IN_SPI2
                    mSpiChnSetBrg (2, SPI_Brg);
                    #elif defined MRF24J40_IN_SPI3
                        mSpiChnSetBrg (1A, SPI_Brg);
                    #elif defined MRF24J40_IN_SPI4
                        mSpiChnSetBrg (3A, SPI_Brg);
                    #endif
               }
               else{
                    #if defined MRF24J40_IN_SPI1
                    mSpiChnSetBrg (1, 0);
                    #elif defined MRF24J40_IN_SPI2
                    mSpiChnSetBrg (2, 0);
                    #elif defined MRF24J40_IN_SPI3
                        mSpiChnSetBrg (1A, 0);
                    #elif defined MRF24J40_IN_SPI4
                        mSpiChnSetBrg (3A, SPI_Brg);
                    #endif
               }
            #endif
        #endif
        #if defined(MRF49XA_1)
            // pruebas de funcionamiento
   /*        MRF49XA_1_PHY_CS_TRIS    = OUTPUT_PIN;
             MRF49XA_1_PHY_CS = 0;
             MRF49XA_1_PHY_CS = 1;

             MRF49XA_1_PHY_RESETn_TRIS  = OUTPUT_PIN;
             MRF49XA_1_PHY_RESETn = 0;
             MRF49XA_1_PHY_RESETn = 1;

             MRF49XA_1_INT_TRIS = OUTPUT_PIN;
             MRF49XA_1_INT_PIN = 0;
             MRF49XA_1_INT_PIN = 1;

             MRF49XA_1_SDI_TRIS = OUTPUT_PIN;
             MRF49XA_1_SPI_SDI = 0;
             MRF49XA_1_SPI_SDI = 1;

             MRF49XA_1_SDO_TRIS = OUTPUT_PIN;
             MRF49XA_1_SPI_SDO = 0;
             MRF49XA_1_SPI_SDO = 1;

             MRF49XA_1_SCK_TRIS = OUTPUT_PIN;
             MRF49XA_1_SPI_SCK = 0;
             MRF49XA_1_SPI_SCK = 1;

             MRF49XA_1_nFSEL_TRIS = OUTPUT_PIN;
             MRF49XA_1_nFSEL = 0;
             MRF49XA_1_nFSEL = 1;

             MRF49XA_1_FINT_TRIS = OUTPUT_PIN;
             MRF49XA_1_FINT = 0;
             MRF49XA_1_FINT = 1;
*/
            // configuration.   Juan: Added; Agus: Modified to a standard way
            MRF49XA_1_PHY_CS_TRIS = OUTPUT_PIN;
            MRF49XA_1_PHY_CS = 1;
            MRF49XA_1_PHY_RESETn_TRIS = OUTPUT_PIN;
            MRF49XA_1_PHY_RESETn = 1;

            SWDelay(50);

            MRF49XA_1_INT_TRIS = INPUT_PIN;

            MRF49XA_1_SDI_TRIS = INPUT_PIN;
            MRF49XA_1_SDO_TRIS = OUTPUT_PIN;
            MRF49XA_1_SCK_TRIS = OUTPUT_PIN;
            MRF49XA_1_SPI_SDO = 0;
            MRF49XA_1_SPI_SCK = 0;

            MRF49XA_1_nFSEL_TRIS = OUTPUT_PIN;
            MRF49XA_1_FINT_TRIS = INPUT_PIN;
            MRF49XA_1_nFSEL = 1;          // nFSEL inactive





            #ifdef cNGD_PLATFORM
//                MRF49XA_1_PWR_TRIS = OUTPUT_PIN;
//                MRF49XA_1_PWR = 1;
            #endif

            MRF49XA_1_SPICONCLR = 0xFFFFFFFF;       //Clear SPIxCON register

            #ifdef HARDWARE_SPI
                /* Enable SPI1, Set to Master Mode & Set CKE bit : Serial output
                * data changes on transition from active clock state to Idle
                * clock state */
                MRF49XA_1_SPICON = 0x00008120;

                /* PB Frequency can be maximum 40 MHz */
                if(pbFreq > (2 * MAX_SPI_CLK_FREQ_FOR_P2P)){
                    SPI_Brg = 1;
                    /* Continue the loop till you find SPI Baud Rate Reg Value */
                    while(1){
                        /* SPI Clock Calculation as per PIC32 Manual */
                        SPI_Clk_Freq = pbFreq / (2 * (SPI_Brg + 1));
                        if(SPI_Clk_Freq <= MAX_SPI_CLK_FREQ_FOR_P2P){
                            break;
                        }
                        SPI_Brg++;
                    }
                    #if defined MRF49XA_1_IN_SPI1
                    mSpiChnSetBrg (1, SPI_Brg);
                    #elif defined MRF49XA_1_IN_SPI2
                    mSpiChnSetBrg (2, SPI_Brg);
                    #elif defined MRF49XA_1_IN_SPI3
                        mSpiChnSetBrg (1A, SPI_Brg);
                    #endif
               }
               else{
                    #if defined MRF49XA_1_IN_SPI1
                    mSpiChnSetBrg (1, 0);
                    #elif defined MRF49XA_1_IN_SPI2
                    mSpiChnSetBrg (2, 0);
                    #elif defined MRF49XA_1_IN_SPI3
                        mSpiChnSetBrg (1A, 0);
                    #endif
               }
            #endif
        #endif
        #if defined(MRF49XA_2)

            MRF49XA_2_PHY_CS_TRIS = OUTPUT_PIN;
            MRF49XA_2_PHY_CS = 1;
            MRF49XA_2_PHY_RESETn_TRIS = OUTPUT_PIN;
            MRF49XA_2_PHY_RESETn = 1;

            SWDelay(50);

            MRF49XA_2_INT_TRIS = 1;

            MRF49XA_2_SDI_TRIS = INPUT_PIN;
            MRF49XA_2_SDO_TRIS = OUTPUT_PIN;
            MRF49XA_2_SCK_TRIS = OUTPUT_PIN;
            MRF49XA_2_SPI_SDO = 0;
            MRF49XA_2_SPI_SCK = 0;

            MRF49XA_2_nFSEL_TRIS = OUTPUT_PIN;
            MRF49XA_2_FINT_TRIS = INPUT_PIN;
            MRF49XA_2_nFSEL = 1;          // nFSEL inactive

            #ifdef cNGD_PLATFORM
                MRF49XA_2_PWR_TRIS = OUTPUT_PIN;
                MRF49XA_2_PWR = 1;
            #endif

            MRF49XA_2_SPICONCLR = 0xFFFFFFFF;       // Clear SPIxCON register

            #ifdef HARDWARE_SPI
                /* Enable SPI1, Set to Master Mode & Set CKE bit : Serial output
                * data changes on transition from active clock state to Idle
                * clock state */
                MRF49XA_2_SPICON = 0x00008120;

                /* PB Frequency can be maximum 40 MHz */
                if(pbFreq > (2 * MAX_SPI_CLK_FREQ_FOR_P2P)){
                    SPI_Brg = 1;
                    /* Continue the loop till you find SPI Baud Rate Reg Value */
                    while(1){
                        /* SPI Clock Calculation as per PIC32 Manual */
                        SPI_Clk_Freq = pbFreq / (2 * (SPI_Brg + 1));
                        if(SPI_Clk_Freq <= MAX_SPI_CLK_FREQ_FOR_P2P){
                            break;
                        }
                        SPI_Brg++;
                    }
                    #if defined MRF49XA_2_IN_SPI1
                    mSpiChnSetBrg (1, SPI_Brg);
                    #elif defined MRF49XA_2_IN_SPI2
                    mSpiChnSetBrg (2, SPI_Brg);
                    #elif defined MRF49XA_2_IN_SPI3
                        mSpiChnSetBrg (1A, SPI_Brg);
                    #endif
               }
               else{
                    #if defined MRF49XA_2_IN_SPI1
                    mSpiChnSetBrg (1, 0);
                    #elif defined MRF49XA_2_IN_SPI2
                    mSpiChnSetBrg (2, 0);
                    #elif defined MRF49XA_2_IN_SPI3
                        mSpiChnSetBrg (1A, 0);
                    #endif
               }
            #endif
        #endif
        #if defined MRF89XA
            Data_nCS_TRIS = 0;
            Config_nCS_TRIS = 0;
            Data_nCS = 1;
            Config_nCS = 1;
            PHY_IRQ1_TRIS = 1;

            //... REVIEW...
        #endif

    // SPI & EXTERNAL INTERRUPTS PINS AND CONFIGURATION ----------------------//
        /* Set the SPI Port Directions (SDO, SDI, SCK) for every SPI module.*/
            #if defined SPI1_IN_USE
  
                SDI1_TRIS = INPUT_PIN;   //DIGITAL IN
                SDO1_TRIS = OUTPUT_PIN;  //DIGITAL OUT
                SCK1_TRIS = OUTPUT_PIN;  //DIGITAL OUT

            #endif

            #if defined SPI2_IN_USE

                SDI2_TRIS = INPUT_PIN;   //DIGITAL IN
                SDO2_TRIS = OUTPUT_PIN;  //DIGITAL OUT
                SCK2_TRIS = OUTPUT_PIN;  //DIGITAL OUT

            #endif

            #if defined SPI3_IN_USE

                SDI3_TRIS = INPUT_PIN;   //DIGITAL IN
                SDO3_TRIS = OUTPUT_PIN;  //DIGITAL OUT
                SCK3_TRIS = OUTPUT_PIN;  //DIGITAL OUT

            #endif

            #if defined SPI4_IN_USE

                SDI4_TRIS = INPUT_PIN;   //DIGITAL IN
                SDO4_TRIS = OUTPUT_PIN;  //DIGITAL OUT
                SCK4_TRIS = OUTPUT_PIN;  //DIGITAL OUT

            #endif

        /* Set the external interrups Pin Directions and Priority*/
            #if defined INT1_IN_USE

                INT1_TRIS = INPUT_PIN; // DIGITAL IN
                mINT1SetIntPriority(4);
                mINT1SetIntSubPriority(2);
                mINT1SetEdgeMode(0);                //0: Falling Edge.
                // Enable INT1
                mINT1IntEnable(1); 
            #endif
            #if defined INT2_IN_USE
                
                INT2_TRIS = INPUT_PIN; // DIGITAL IN
                mINT2SetIntPriority(4);
                mINT2SetIntSubPriority(2);
                mINT2SetEdgeMode(0);                //0: Falling Edge.
                /* Enable INT2 */
                mINT2IntEnable(1);
            #endif
            #if defined INT3_IN_USE
                
                INT3_TRIS = INPUT_PIN; // DIGITAL IN
                mINT3SetIntPriority(4);
                mINT3SetIntSubPriority(2);
                mINT3SetEdgeMode(0);                //0: Falling Edge.
                /* Enable INT3 */
                mINT3IntEnable(1);
            #endif
            #if defined INT4_IN_USE
               
                INT4_TRIS = INPUT_PIN; // DIGITAL IN
                mINT4SetIntPriority(4);
                mINT4SetIntSubPriority(2);
                mINT4SetEdgeMode(0);                //0: Falling Edge.
                /* Enable INT4 */
                mINT4IntEnable(1);
            #endif

     // LEDs
        #ifdef cNGD_PLATFORM
               mJTAGPortEnable(0); //Needed due to multiplexed pins
            
               LED1_TRIS = OUTPUT_PIN;
               LED2_TRIS = OUTPUT_PIN;
               LED3_TRIS = OUTPUT_PIN;
               LED1 = 1;
               LED2 = 1; 
               LED3 = 1; 
                              
        #endif


    // TIMER 1 FOR TIME_SYNC -------------------------------------------------//
        #if defined(ENABLE_TIME_SYNC)   
        //TIMER 1 MAY BE USED FOR SLEEP MODE AND/OR FOR STACKS MAINTENANCE. IT
        //NEEDS ADAPTATION BEFORE ENABLING TIME_SYNC WITH TIMER 1 TOO!
            T1CON = 0;
            T1CON = 0x0012;
            T1CONSET = 0x8000;
            PR1 = 0xFFFF;
            IFS0bits.T1IF = 0;

            mT1IntEnable(1);
            mT1SetIntPriority(4);

            while(T1CONbits.TWIP);
            TMR1 = 0;
        #endif
    // TIMER 1 FOR NODE STACKS AUTO-MAINTENANCE ------------------------------//
        #if defined NODE_DOES_MAINTENANCE_TASKS
            T1CON = 0x0070;             //Disable timer, PBCLK source, PS=256
            TMR1  = 0x0000;             //Reset count
            PR1   = MAINTENANCE_PERIOD; //Set period.

            IPC1SET = 0x00000005;   //Set Priority level 1, Subpriority level 1
            IFS0CLR = 0x00000010;   //Clear T1IF
            IEC0SET = 0x00000010;   //Set T1IE
            //Timer will be triggered after initialization.
        #endif
//************************************* TODO
    // IOPORT CN - For waking up the node manually. --------------------------//
        mPORTDSetPinsDigitalIn(BIT_5); // CN14
        CNCON = 0x8000;         //Module enabled.
        CNEN = 0x00004000;      //Enable CN14
        CNPUE = 0x00004000;     //Enable CN14 weak pull-up.
        ReadBUTTONS();          //Clear PORT mismatch condition.
        IFS1CLR = 0x00000001;   //Clear the CN interrupt flag status bit
        IPC6SET = 0x00180000;   //Set CN priority 6, subpriority 0.
        //It will be enabled only during sleep mode time interval
    //------------------------------------------------------------------------//
																										// Lo modifico en el wifi config
        #if defined(ENABLE_NVM)     //REVIEW
            //EE_nCS_TRIS = 0;//FERNANDO, CUIDADO NO SE SI LA PILA REALMENTE FUNCIONA CON FLASH MEMORY
            //EE_nCS = 1;
        #endif

    // INTERRUPTION FLAGS AND EXT_INT PIN FINAL SETTINGS ---------------------//
        #if defined MRF49XA_1
            MRF49XA_1_IF = 0;
            if(MRF49XA_1_INT_PIN == 0){
                MRF49XA_1_IF = 1;
            }
        #endif
        #if defined MRF49XA_2
            MRF49XA_2_IF = 0;
            if(MRF49XA_2_INT_PIN == 0){
                MRF49XA_2_IF = 1;
            }
        #endif
        #if defined MRF89XA
            PHY_IRQ1 = 0;
        #endif
        #if defined MRF24J40
            MRF24J40_IF = 0;
            if(MRF24J40_INT_PIN == 0){
                MRF24J40_IF = 1;
            }
        #endif
    
    //CONFIGURAMOS LA INTERRUPCIÓN DEL BOTÓN 1
    INTEnableSystemMultiVectoredInt();
    mCNOpen(CN_ON | CN_IDLE_CON, CN14_ENABLE | CN16_ENABLE, CN_PULLUP_DISABLE_ALL);
    //mCNOpen(CN_ON | CN_IDLE_CON, CN16_ENABLE , CN_PULLUP_DISABLE_ALL);
    PORTSetPinsDigitalIn(IOPORT_D, BIT_5 | BIT_7);
    //PORTSetPinsDigitalIn(IOPORT_D, BIT_7);
    //mPORTDRead();
    ConfigIntCN(CHANGE_INT_ON | CHANGE_INT_PRI_3);//6);
    
    #else   //Not PIC32.
        #error "Unknown target board." 
    #endif
}

void __ISR(_CHANGE_NOTICE_VECTOR, ipl6AUTO)IntCN(void)
{

    mPORTDRead();
    if (!BUTTON_2) {
        PUSH_BUTTON2_pressed = TRUE;

    }
    if (!BUTTON_1) {
        PUSH_BUTTON1_pressed = TRUE;
    }
    mCNClearIntFlag();
}


/*********************************************************************
 * Function:        BYTE ButtonPressed(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Byte to indicate which button has been pressed.
 *                  Return 0 if no button pressed.
 *
 * Side Effects:
 *
 * Overview:        This function check if any button has been pressed
 *
 * Note:            If you use Clara´s code, discomment this function
 ********************************************************************/
/*BYTE ButtonPressed(void)
{
    if (PUSH_BUTTON1_pressed) {
        PUSH_BUTTON1_pressed = FALSE;
        return 1;
    } 
    if (PUSH_BUTTON2_pressed){
        PUSH_BUTTON2_pressed = FALSE;
        return 2;
    }
    else {
        return 0;
    }
}*/
/*********************************************************************
 * Function:        BYTE s2Pressed(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Byte to indicate if button s2 has been pressed.
 *
 * Side Effects:
 *
 * Overview:        This function check if any button has been pressed
 *
 * Author:          Karen Flores
 ********************************************************************/
BYTE s2Pressed(void){
    if(PUSH_BUTTON1_pressed){
        PUSH_BUTTON1_pressed = FALSE;
        return 1;
    }
    return 0;
}
/*********************************************************************
 * Function:        BYTE s3Pressed(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Byte to indicate if button s3 has been pressed.
 *
 * Side Effects:
 *
 * Overview:        This function check if any button has been pressed
 *
 * Author:          Karen Flores
 ********************************************************************/
BYTE s3Pressed(void){
    if(PUSH_BUTTON2_pressed){
        PUSH_BUTTON2_pressed = FALSE;
        return 1;
    }
    return 0;
}

#if defined ENABLE_CONSOLE
#if defined DEBUG_USB


#else
void WriteStringPC(const char *string){
    while (*string != '\0'){    // If next char is not the end of the string.
        while(TxNotRdyUART());  // Wait for Debug UART Tx buffer to accept more data.
        WriteUART(*string);     // Write a single char data.
        string++;               // Prepare for next data
        while(BusyUART());      // Wait for the transmission to finish.
    }
}
#endif
#endif