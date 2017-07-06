/*******************************************************************************
* FileName:		P2P.h
* Dependencies: SymbolTime.h
* Processor:	PIC18, PIC24F, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Hardware:		PICDEM Z, Explorer 16, PIC18 Explorer
* Complier:     Microchip C18 v3.30 or higher
*				Microchip C30 v2.03 or higher	
*               Microchip C32 v1.02 or higher
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice for P2P Software:
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
********************************************************************************
* File Description:
*
*  This is the header file for P2P stack.  
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   1/03/2008    yfy       Initial revision
*  2.0   4/15/2009    yfy       MiMAC and MiApp revision
*  2.1   06/20/2009   yfy       Add LCD support
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
*******************************************************************************/

#ifndef __P2P_H_
#define __P2P_H_

#define INPUT
#define OUTPUT
#define IOPUT

#include "WirelessProtocols/ConfigApp.h"

#if defined(PROTOCOL_P2P)
/******************************* HEADERS **************************************/
#include "WirelessProtocols/P2P/ConfigP2P.h"
#include "WirelessProtocols/SymbolTime.h"
#include "Transceivers/Transceivers.h"
#include "Transceivers/MCHP_MAC.h"
#include "WirelessProtocols/MCHP_API.h"        //added for recognising miwi_band

/****************************** DEFINITIONS ***********************************/
#define PAYLOAD_START                           0

#define STATUS_SUCCESS                          0x00
#define STATUS_EXISTS                           0x01
#define STATUS_ACTIVE_SCAN                      0x02
#define STATUS_UPDATED                          0x10    //Juan: added.
#define STATUS_ENTRY_NOT_EXIST                  0xF0
#define STATUS_NOT_ENOUGH_SPACE                 0xF1
#define STATUS_NOT_SAME_PAN                     0xF2
#define STATUS_NOT_PERMITTED                    0xF3

#define CMD_P2P_CONNECTION_REQUEST              0x81
#define CMD_P2P_CONNECTION_REMOVAL_REQUEST      0x82
#define CMD_DATA_REQUEST                        0x83
#define CMD_CHANNEL_HOPPING                     0x84
#define CMD_TIME_SYNCHRONIZATION_REQUEST        0x85
#define CMD_TIME_SYNCHRONIZATION_NOTIFICATION   0x86
#define CMD_P2P_ACTIVE_SCAN_REQUEST             0x87

#define CMD_TIME_SYNC_DATA_PACKET               0x8A
#define CMD_TIME_SYNC_COMMAND_PACKET            0x8B

#define CMD_P2P_NEW_COMMAND                     0x88
#define CMD_P2P_CONNECTION_RESPONSE             0x91
#define CMD_P2P_CONNECTION_REMOVAL_RESPONSE     0x92
#define CMD_P2P_ACTIVE_SCAN_RESPONSE            0x97

#define CMD_MAC_DATA_REQUEST                    0x04

#if defined(ENABLE_ED_SCAN) && defined(ENABLE_FREQUENCY_AGILITY)
    #define FREQUENCY_AGILITY_STARTER
#endif

#define MICRO_SECOND_PER_COUNTER_TICK   (1000000ul / ( COUNTER_CRYSTAL_FREQ / 8 ))          //REVIEW!!
 
/******************************** DATA TYPE ***********************************/
 
/*******************************************************************************
 * Overview:
 * The capacity information for a MiWi(TM) P2P device. It is the definition of
 * the first byte of PeerInfo defined in P2P_CONNECTION_ENTRY. The highest bit
 * also be used to indicate if the P2P connection entry is a valid entry
 ******************************************************************************/
//Juan: This structure was defined by MCHP but not used in the code... Original
//code defined a BYTE P2PCapacity which was treated bit by bit as this struct.
//Most of the flags were not used.
//In order to make the code more legible, I'm using this definition, and also
//defining this BYTE for each MiWi RI, as long as Conn Mode and other flags,
//may be different for each transceiver in a specific instant of time.
typedef union{
    BYTE    Val;                            // the value of the P2P capacitity 
    struct _P2P_CAPACITY_BITS
    {
        BYTE RXOnWhileIdle       : 1;   // if device turns on radio when idle
        BYTE DataRequestNeeded   : 1;   
            // if data request is required when device turns off the radio when
            // idle. It is used to decide if an indirect message is necessary
            // to be stored.
        BYTE TimeSynchronization : 1;   // reserved bit for future development
        BYTE SecurityCapacity    : 1;
            // if the device is capable of handling encrypted information
        BYTE ConnectionMode      : 2;
        BYTE filler              : 1;
        BYTE isValid             : 1;
            // use this bit to indicate that this entry is a valid entry
    }bits;
}P2P_CAPACITY;         

/*******************************************************************************
 * Overview:
 * The structure to store indirect messages for devices which turn off radio
 * when idle
 ******************************************************************************/
typedef struct{
    MIWI_TICK TickStart;    // start time of the indirect message. Used for
                            // checking indirect message time out

    #if defined(IEEE_802_15_4)
        WORD_VAL DestPANID; // the PAN identifier for the destination node
    #endif
    
    // destination address for the indirect message, either unicast or broadcast
    union{
        BYTE DestLongAddress[MY_ADDRESS_LENGTH];    // unicast long address
        BYTE DestIndex[CONNECTION_SIZE];            
            // broadcast index of the P2P Connection Entries for RFD devices
    } DestAddress;

    union{
        BYTE Val;       // value for the flags
        struct{
            BYTE isValid     : 1; //If this indirect message is valid
            BYTE isBroadcast : 1; //If this indirect message is for broadcasting
            BYTE isCommand   : 1; //If this indirect message a command
            BYTE isSecured   : 1; //If this indirect message requires encryption
        } bits;         // bit map of the flags
    } flags;        // flags for indirect message
    
    BYTE PayLoadSize;   // the indirect message pay load size

    #if (TX_BUFFER_SIZE) > 110
        BYTE PayLoad[110];
    #else
        BYTE PayLoad[TX_BUFFER_SIZE];   // the indirect message payload
    #endif
    miwi_band MiWiFreqBand;    //Microchip CWSN-LSI stack. Frequency band used for this message.
} INDIRECT_MESSAGE; 

/******************************************************************
 * Overview: structure to indicate the status of P2P stack
 *****************************************************************/
typedef union{
    BYTE Val;                   // The value of the P2P status flags
    struct{
        BYTE Sleeping          :1;  // indicate if the device in sleeping state
        BYTE SaveConnection    :1;  
            // indicate if it is time for Network Freezer module to save valid
            // connections. Important because there may be more than one
            //response coming back and NVM operation is blocking
        BYTE DataRequesting    :1;  
            // indicate that device is in the process of data request from its
            // parent. Only effective if device enables sleeping
        BYTE RxHasUserData     :1;     
            // indicate if the received frame needs processing from the
            // application layer
        BYTE SearchConnection  :1;     
            // indicate if the stack is currently in the process of looking for
            // new connection
        BYTE Resync            :1;  
            // indicate if the stack is currently in the process of
            //resynchronizing connection with the peer device
        BYTE Enhanced_DR_SecEn :1;
    }bits;      // bit map of the P2P status
} P2P_STATUS;                               

/************************** EXTERNAL VARIABLES ********************************/
#if defined MRF24J40
    extern BYTE MRF24J40_TxBuffer[];
    extern BYTE MRF24J40_TxData;
    extern volatile P2P_STATUS MRF24J40_P2PStatus;
#endif
#if defined MRF49XA_1
    extern BYTE MRF49XA_1_TxBuffer[];
    extern BYTE MRF49XA_1_TxData;
    extern volatile P2P_STATUS MRF49XA_1_P2PStatus;
#endif
#if defined MRF49XA_2
    extern BYTE MRF49XA_2_TxBuffer[];
    extern BYTE MRF49XA_2_TxData;
    extern volatile P2P_STATUS MRF49XA_2_P2PStatus;
#endif
#if defined MRF89XA
    extern BYTE MRF89XA_TxBuffer[];
    extern BYTE MRF89XA_TxData;
    extern volatile P2P_STATUS MRF89XA_P2PStatus;
#endif

#if defined(IEEE_802_15_4)
    extern WORD_VAL myPANID;
#endif
extern WORD_VAL CounterVal;

/***************************** FUNCTION PROTOTYPES ****************************/

BOOL isSameAddress(INPUT BYTE *Address1, INPUT BYTE *Address2);

#endif

#endif
    
    
