/********************************************************************
* FileName:		MSPI.c
* Dependencies: MSPI.h
* Processor:	PIC18, PIC24F, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710
* Complier:     Microchip C18 v3.04 or higher
*				Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice
*
* Copyright © 2007-2010 Microchip Technology Inc.  All rights reserved.
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
* SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY
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
*
*********************************************************************
* File Description:
*
*   Configures and enables usage of the SPI ports
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   11/09/2006   yfy       Initial revision
*  1.0   01/09/2007   yfy       Initial release
*  2.0   4/15/2009    yfy       MiMAC and MiApp revision
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
********************************************************************/

/************************ HEADERS **********************************/
#include "WirelessProtocols/ConfigApp.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "WirelessProtocols/Console.h"

#if defined(__32MX795F512H__) || defined (__32MX795F512L__) || defined(__32MX675F256L__) //Agus
UINT8 SPI_ERROR_COUNTER;



/*******************************************************************************
* Function:     void SPIPut(UINT8 mod, BYTE v)
* PreCondition: SPI has been configured
* Input:        mod - selects SPI module, from 1 to 4, depending on availability
 *              v - is the byte that needs to be transfered
* Output:	none
* Side Effects:	SPI transmits the byte
* Overview:	This function will send a byte over the SPI
* Note:		None
*******************************************************************************/
void SPIPut(UINT8 mod, BYTE v){

    BYTE i;
    switch (mod){
        #if !defined (__32MX795F512H__)    //PIC32MX795F512H doesn't support SPI1 //Agus
        case 1: //SPI1
            #if !defined MRF49XA_1_IN_SPI1 && !defined MRF49XA_2_IN_SPI1 && \
                !defined MRF24J40_IN_SPI1  && !defined MRF24WB0M_IN_SPI1
                Printf("\rError: SPI1 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO1 = 0;
                    SPI_SCK1 = 0;
                    for(i = 0; i < 8; i++){
                        SPI_SDO1 = (v >> (7-i));
                        SPI_SCK1 = 1;
                        SPI_SCK1 = 0;
                    }
                    SPI_SDO1 = 0;
                    return;
                #else
                    SpiChnPutC(SPI_CHANNEL1, v);
                    i = SpiChnGetC(SPI_CHANNEL1);
                    return;
                #endif
            #endif
        #endif
        case 2: //SPI2
            #if !defined MRF49XA_1_IN_SPI2 && !defined MRF49XA_2_IN_SPI2 && \
                !defined MRF24J40_IN_SPI2  && !defined MRF24WB0M_IN_SPI2
                Printf("\rError: SPI2 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO2 = 0;
                    SPI_SCK2 = 0;
                    for(i = 0; i < 8; i++){
                        SPI_SDO2 = (v >> (7-i));
                        SPI_SCK2 = 1;
                        SPI_SCK2 = 0;
                    }
                    SPI_SDO2 = 0;
                    return;
                #else
                    SpiChnPutC(SPI_CHANNEL2, v);
                    i = SpiChnGetC(SPI_CHANNEL2);
                    return;
                #endif
            #endif
        case 3: //SPI3
            #if !defined MRF49XA_1_IN_SPI3 && !defined MRF49XA_2_IN_SPI3 && \
                !defined MRF24J40_IN_SPI3  && !defined MRF24WB0M_IN_SPI3
                Printf("\rError: SPI3 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO3 = 0;
                    SPI_SCK3 = 0;
                    for(i = 0; i < 8; i++){
                        SPI_SDO3 = (v >> (7-i));
                        SPI_SCK3 = 1;
                        SPI_SCK3 = 0;
                    }
                    SPI_SDO3 = 0;
                    return;
                #else
                    SpiChnPutC(SPI_CHANNEL3, v);
                    i = SpiChnGetC(SPI_CHANNEL3);
                    return;
                #endif
            #endif
        case 4: //SPI4
            #if !defined MRF49XA_1_IN_SPI4 && !defined MRF49XA_2_IN_SPI4 && \
                !defined MRF24J40_IN_SPI4  && !defined MRF24WB0M_IN_SPI4
                Printf("\rError: SPI4 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO4 = 0;
                    SPI_SCK4 = 0;
                    for(i = 0; i < 8; i++){
                        SPI_SDO4 = (v >> (7-i));
                        SPI_SCK4 = 1;
                        SPI_SCK4 = 0;
                    }
                    SPI_SDO4 = 0;
                    return;
                #else
                    SpiChnPutC(SPI_CHANNEL4, v);
                    i = SpiChnGetC(SPI_CHANNEL4);
                    return;
                #endif
            #endif
        default:
            Printf("\rError: Incorrect SPI Module.");
            SPI_ERROR_COUNTER++;
            return;
    }
}

/*******************************************************************************
* Function:     BYTE SPIGet(UINT8 mod)
* PreCondition: SPI has been configured
* Input:        mod - selects SPI module, from 1 to 4, depending on availability
* Output:       BYTE - the byte that was last received by the SPI
* Side Effects:	none
* Overview:	This function will read a byte over the SPI
* Note:         None
*******************************************************************************/
BYTE SPIGet(UINT8 mod){
    BYTE spidata = 0;
    BYTE i;
    switch (mod){
        #if !defined (__32MX795F512H__)    //PIC32MX795F512H doesn't support SPI1 //Agus
        case 1: //SPI1
            #if !defined MRF49XA_1_IN_SPI1 && !defined MRF49XA_2_IN_SPI1 && \
                !defined MRF24J40_IN_SPI1  && !defined MRF24WB0M_IN_SPI1
                Printf("\rError: SPI1 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO1 = 0;
                    SPI_SCK1 = 0;
                    for(i = 0; i < 8; i++){
                        spidata = (spidata << 1) | SPI_SDI1;
                        SPI_SCK1 = 1;
                        SPI_SCK1 = 0;
                    }
                    return spidata;
                #else
                    SpiChnPutC(SPI_CHANNEL1, 0x00);
                    spidata = SpiChnGetC(SPI_CHANNEL1);
                    return spidata;
                #endif
            #endif
        #endif
        case 2: //SPI2
            #if !defined MRF49XA_1_IN_SPI2 && !defined MRF49XA_2_IN_SPI2 && \
                !defined MRF24J40_IN_SPI2  && !defined MRF24WB0M_IN_SPI2
                Printf("\rError: SPI2 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO2 = 0;
                    SPI_SCK2 = 0;
                    for(i = 0; i < 8; i++){
                        spidata = (spidata << 1) | SPI_SDI2;
                        SPI_SCK2 = 1;
                        SPI_SCK2 = 0;
                    }
                    return spidata;
                #else
                    SpiChnPutC(SPI_CHANNEL2, 0x00);
                    spidata = SpiChnGetC(SPI_CHANNEL2);
                    return spidata;
                #endif
            #endif
        case 3: //SPI3
            #if !defined MRF49XA_1_IN_SPI3 && !defined MRF49XA_2_IN_SPI3 && \
                !defined MRF24J40_IN_SPI3  && !defined MRF24WB0M_IN_SPI3
                Printf("\rError: SPI3 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO3 = 0;
                    SPI_SCK3 = 0;
                    for(i = 0; i < 8; i++){
                        spidata = (spidata << 1) | SPI_SDI3;
                        SPI_SCK3 = 1;
                        SPI_SCK3 = 0;
                    }
                    return spidata;
                #else
                    SpiChnPutC(SPI_CHANNEL3, 0x00);
                    spidata = SpiChnGetC(SPI_CHANNEL3);
                    return spidata;
                #endif
            #endif
        case 4: //SPI4
            #if !defined MRF49XA_1_IN_SPI4 && !defined MRF49XA_2_IN_SPI4 && \
                !defined MRF24J40_IN_SPI4  && !defined MRF24WB0M_IN_SPI4
                Printf("\rError: SPI4 is not available.");
                SPI_ERROR_COUNTER++;
                return;
            #else
                #if !defined(HARDWARE_SPI)
                    SPI_SDO4 = 0;
                    SPI_SCK4 = 0;
                    for(i = 0; i < 8; i++){
                        spidata = (spidata << 1) | SPI_SDI4;
                        SPI_SCK4 = 1;
                        SPI_SCK4 = 0;
                    }
                    return spidata;
                #else
                    SpiChnPutC(SPI_CHANNEL4, 0x00);
                    spidata = SpiChnGetC(SPI_CHANNEL4);
                    return spidata;
                #endif
            #endif
        default:
            Printf("\rError: Incorrect SPI Module.");
            SPI_ERROR_COUNTER++;
            return;
    }
}

//For debugging.
    UINT8 GetSPIErrorCounter(){
        return SPI_ERROR_COUNTER;
    }

    void ResetSPIErrorCounter(){
        SPI_ERROR_COUNTER = 0;
    }

#else
    #error "Unknown processor.  See Compiler.h"
#endif




