/*******************************************************************************
* FileName:	P2P.c                           MODIFIED FROM ORIGINAL STACK!!!
* Dependencies:
* Processor:	PIC18, PIC24, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710
* Complier:     Microchip C18 v3.04 or higher
*		Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher
* Company:	Microchip Technology, Inc.
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
*  This is the P2P stack
*
* Change History:
*  Rev   Date         Author    Description
*  0.1   1/03/2008    yfy       Initial revision
*  2.0   4/15/2009    yfy       MiMAC and MiApp revision
*  2.1   6/20/2009    yfy       Add LCD support
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
*
* MICROCHIP CWSN-LSI STACK - JUAN.
* IMPORTANT!: This file has suffered modifications that affect the behaviour of
* the protocol P2P, due to having more than one transceiver working at the same
* time, sharing the connection table, etc. I try to explain with my comments all
* the changes and some of the original code lines that might seem confusing.
*******************************************************************************/

/************************ HEADERS *********************************************/
#include "WirelessProtocols/ConfigApp.h"
#include "NodeHAL.h"
#if defined(PROTOCOL_P2P)

#include "WirelessProtocols/MSPI.h"
#include "WirelessProtocols/P2P/P2P.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "WirelessProtocols/Console.h"
#include "WirelessProtocols/NVM.h"
#include "WirelessProtocols/SymbolTime.h"
#include "Transceivers/MCHP_MAC.h"
#include "Transceivers/ConfigTransceivers.h"
#include "WirelessProtocols/MCHP_API.h"
#include "WirelessProtocols/EEPROM.h"

/************************ VARIABLES *******************************************/
// permanent address definition
#if MY_ADDRESS_LENGTH == 8
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3,
                                             EUI_4, EUI_5, EUI_6, EUI_7};
#elif MY_ADDRESS_LENGTH == 7
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3,
                                             EUI_4, EUI_5, EUI_6};
#elif MY_ADDRESS_LENGTH == 6
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4,
                                             EUI_5};
#elif MY_ADDRESS_LENGTH == 5
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4};
#elif MY_ADDRESS_LENGTH == 4
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3};
#elif MY_ADDRESS_LENGTH == 3
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2};
#elif MY_ADDRESS_LENGTH == 2
    BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1};
#endif

#if defined(ENABLE_ED_SCAN) || defined(ENABLE_ACTIVE_SCAN) || \
    defined(ENABLE_FREQUENCY_AGILITY)
    // Scan Duration formula for P2P Connection:
    //  60 * (2 ^ n + 1) symbols, where one symbol equals 16us
    #define SCAN_DURATION_0     SYMBOLS_TO_TICKS(120)
    #define SCAN_DURATION_1     SYMBOLS_TO_TICKS(180)
    #define SCAN_DURATION_2     SYMBOLS_TO_TICKS(300)
    #define SCAN_DURATION_3     SYMBOLS_TO_TICKS(540)
    #define SCAN_DURATION_4     SYMBOLS_TO_TICKS(1020)
    #define SCAN_DURATION_5     SYMBOLS_TO_TICKS(1980)
    #define SCAN_DURATION_6     SYMBOLS_TO_TICKS(3900)
    #define SCAN_DURATION_7     SYMBOLS_TO_TICKS(7740)
    #define SCAN_DURATION_8     SYMBOLS_TO_TICKS(15420)
    #define SCAN_DURATION_9     SYMBOLS_TO_TICKS(30780)
    #define SCAN_DURATION_10    SYMBOLS_TO_TICKS(61500)
    #define SCAN_DURATION_11    SYMBOLS_TO_TICKS(122940)
    #define SCAN_DURATION_12    SYMBOLS_TO_TICKS(245820)
    #define SCAN_DURATION_13    SYMBOLS_TO_TICKS(491580)
    #define SCAN_DURATION_14    SYMBOLS_TO_TICKS(983100)
    const ROM DWORD ScanTime[15] = {SCAN_DURATION_0, SCAN_DURATION_1,
        SCAN_DURATION_2,  SCAN_DURATION_3,  SCAN_DURATION_4,  SCAN_DURATION_5,
        SCAN_DURATION_6,  SCAN_DURATION_7,  SCAN_DURATION_8,  SCAN_DURATION_9,
        SCAN_DURATION_10, SCAN_DURATION_11, SCAN_DURATION_12, SCAN_DURATION_13,
        SCAN_DURATION_14};
#endif

// COMMON VARIABLES FOR ALL TRANSCEIVERS -------------------------------------//
extern nodeStatus NodeStatus;       //CWSN LSI - Node Hardware Abstraction Level
extern unsigned int coreTMRvals[];
extern BYTE coreTMRptr;

#ifdef ENABLE_INDIRECT_MESSAGE
    /***************************************************************************
     * "#pragma udata" is used to specify the starting address of a global
     * variable. The address may be MCU dependent on RAM available. If the size
     * of the global variable is small, such manual assignment may not be
     * necessary. Developer can comment out the address assignment.
     **************************************************************************/
    #if defined(__18CXX)
        #pragma udata INDIRECT_BUFFER
    #endif
    INDIRECT_MESSAGE indirectMessages[INDIRECT_MESSAGE_SIZE];
    // structure to store indirect messages for nodes with radio off duing idle

    #if defined(__18CXX)
        #pragma udata
    #endif
#endif

/*******************************************************************************
 * "#pragma udata" is used to specify the starting address of a global variable.
 * The address may be MCU dependent on RAM available. If the size of the global
 * variable is small, such manual assignment may not be necessary. Developer can
 * comment out the address assignment.
 ******************************************************************************/
#if defined(__18CXX)
    #pragma udata BIGvariables1
#endif
CONNECTION_ENTRY ConnectionTable[CONNECTION_SIZE];
// The peer device records for P2P connections

#if defined(__18CXX)
    #pragma udata
#endif

#if defined(IEEE_802_15_4)
    WORD_VAL  myPANID;      // the PAN Identifier for the device
#endif

extern BYTE AdditionalNodeID[];
// The additional information regarding the device that would like to share with
// the peer on the other side of P2P connection. This information is application
// specific.

BYTE LatestConnection;

#if defined(ENABLE_ACTIVE_SCAN)
    BYTE ActiveScanResultIndex;
    ACTIVE_SCAN_RESULT ActiveScanResults[ACTIVE_SCAN_RESULT_SIZE];
    // The results for active scan, including the PAN identifier, signal
    // strength and operating channel
#endif

#if defined(ENABLE_SECURITY)
    DWORD_VAL IncomingFrameCounter[CONNECTION_SIZE];
    // If authentication is used, IncomingFrameCounter can prevent replay attack
#endif

#if defined(ENABLE_NETWORK_FREEZER)
    MIWI_TICK nvmDelayTick;
#endif

// SPECIFIC VARIABLES FOR EACH TRANSCEIVER -----------------------------------//
#if defined MRF24J40
    BYTE MIWI2400_currentChannel = MIWI2400DfltChannel + MIWI2400ConfChannelOffset;
    /***************************************************************************
    * "#pragma udata" is used to specify the starting address of a global
    * variable. The address may be MCU dependent on RAM available. If the size
    * of the global variable is small, such manual assignment may not be
    * necessary. Developer can comment out the address assignment.
    ***************************************************************************/
    #if defined(__18CXX)
        #pragma udata TRX_BUFFER
    #endif
    BYTE MRF24J40_TxBuffer[MRF24J40_TX_BUF_SIZE];
    #if defined(__18CXX)
        #pragma udata
    #endif
    BYTE MRF24J40_TxData;
    volatile P2P_STATUS MRF24J40_P2PStatus;
    RECEIVED_MESSAGE MIWI2400_rxMessage;       //For storing the received packet
    MAC_RECEIVED_PACKET MRF24J40_MACRxPacket;  //Transceiver received packet
    BYTE MIWI2400_ConnMode = DISABLE_ALL_CONN;
    P2P_CAPACITY MIWI2400CapacityInfo;
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF24J40_DataReqTimer;
    #endif
#endif

#if defined MRF49XA_1
    /***************************************************************************
    * "#pragma udata" is used to specify the starting address of a global
    * variable. The address may be MCU dependent on RAM available. If the size
    * of the global variable is small, such manual assignment may not be
    * necessary. Developer can comment out the address assignment.
    ***************************************************************************/
    #if defined(__18CXX)
        #pragma udata TRX_BUFFER
    #endif
    BYTE MRF49XA_1_TxBuffer[MRF49XA_TX_BUF_SIZE];
    #if defined(__18CXX)
        #pragma udata
    #endif
    BYTE MRF49XA_1_TxData;
    volatile P2P_STATUS MRF49XA_1_P2PStatus;
    #if defined MRF49XA_1_IN_434
        RECEIVED_MESSAGE MIWI0434_rxMessage;
        BYTE MIWI0434_currentChannel = MIWI0434DfltChannel + MIWI0434ConfChannelOffset;
        P2P_CAPACITY MIWI0434CapacityInfo;
        BYTE MIWI0434_ConnMode = DISABLE_ALL_CONN;
    #elif defined MRF49XA_1_IN_868
        RECEIVED_MESSAGE MIWI0868_rxMessage;
        BYTE MIWI0868_currentChannel = MIWI0868DfltChannel + MIWI0868ConfChannelOffset;
        P2P_CAPACITY MIWI0868CapacityInfo;
        BYTE MIWI0868_ConnMode = DISABLE_ALL_CONN;
    #endif
    MAC_RECEIVED_PACKET MRF49XA_1_MACRxPacket; //Transceiver received packet
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF49XA_1_DataReqTimer;
    #endif
#endif

#if defined MRF49XA_2
    /***************************************************************************
    * "#pragma udata" is used to specify the starting address of a global
    * variable. The address may be MCU dependent on RAM available. If the size
    * of the global variable is small, such manual assignment may not be
    * necessary. Developer can comment out the address assignment.
    ***************************************************************************/
    #if defined(__18CXX)
        #pragma udata TRX_BUFFER
    #endif
    BYTE MRF49XA_2_TxBuffer[MRF49XA_TX_BUF_SIZE];
    #if defined(__18CXX)
        #pragma udata
    #endif
    BYTE MRF49XA_2_TxData;
    volatile P2P_STATUS MRF49XA_2_P2PStatus;
    #if defined MRF49XA_2_IN_434
        RECEIVED_MESSAGE MIWI0434_rxMessage;
        BYTE MIWI0434_currentChannel = MIWI0434DfltChannel + MIWI0434ConfChannelOffset;
        BYTE MIWI0434_ConnMode = DISABLE_ALL_CONN;
        P2P_CAPACITY MIWI0434CapacityInfo;
    #elif defined MRF49XA_2_IN_868
        RECEIVED_MESSAGE MIWI0868_rxMessage;
        BYTE MIWI0868_currentChannel = MIWI0868DfltChannel + MIWI0868ConfChannelOffset;
        P2P_CAPACITY MIWI0868CapacityInfo;
        BYTE MIWI0868_ConnMode = DISABLE_ALL_CONN;
    #endif
    MAC_RECEIVED_PACKET MRF49XA_2_MACRxPacket; //Transceiver received packet
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF49XA_2_DataReqTimer;
    #endif
#endif

#if defined MRF89XA
    BYTE MIWI0868_currentChannel = MIWI0868DfltChannel + MIWI0868ConfChannelOffset;
    /***************************************************************************
    * "#pragma udata" is used to specify the starting address of a global
    * variable. The address may be MCU dependent on RAM available. If the size
    * of the global variable is small, such manual assignment may not be
    * necessary. Developer can comment out the address assignment.
    ***************************************************************************/
    #if defined(__18CXX)
        #pragma udata TRX_BUFFER
    #endif
    BYTE MRF89XA_TxBuffer[MRF89XA_TX_BUF_SIZE];
    #if defined(__18CXX)
        #pragma udata
    #endif
    BYTE MRF89XA_TxData;
    volatile P2P_STATUS MRF89XA_P2PStatus;
    RECEIVED_MESSAGE MIWI0868_rxMessage;       //For storing the received packet
    MAC_RECEIVED_PACKET MRF89XA_MACRxPacket;   //Transceiver received packet
    BYTE MIWI0868_ConnMode = DISABLE_ALL_CONN;
    P2P_CAPACITY MIWI0868CapacityInfo;
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF89XA_DataReqTimer;
    #endif
#endif

#if defined(ENABLE_TIME_SYNC)
    #if defined(ENABLE_SLEEP)
        #if defined MRF24J40
            WORD_VAL MRF24J40_WakeupTimes;
            WORD_VAL MRF24J40_CounterValue;
        #endif
        #if defined MRF49XA_1
            WORD_VAL MRF49XA_1_WakeupTimes;
            WORD_VAL MRF49XA_1_CounterValue;
        #endif
        #if defined MRF49XA_2
            WORD_VAL MRF49XA_2_WakeupTimes;
            WORD_VAL MRF49XA_2_CounterValue;
        #endif
        #if defined MRF89XA
            WORD_VAL MRF89XA_WakeupTimes;
            WORD_VAL MRF89XA_CounterValue;
        #endif
    #elif defined(ENABLE_INDIRECT_MESSAGE)
        BYTE TimeSyncSlot = 0;
        MIWI_TICK TimeSyncTick;
        MIWI_TICK TimeSlotTick;
    #endif
#endif

/************************ FUNCTION DEFINITION *********************************/
BYTE AddConnection(INPUT miwi_band mb);
BOOL isSameAddress(INPUT BYTE *Address1, INPUT BYTE *Address2);

#if defined(IEEE_802_15_4)
    BOOL SendPacket(INPUT BOOL Broadcast, INPUT WORD_VAL DestinationPANID,
                    INPUT BYTE *DestinationAddress, INPUT BOOL isCommand,
                    INPUT BOOL SecurityEnabled, INPUT miwi_band mb);
#else
    BOOL SendPacket(INPUT BOOL Broadcast,
                    INPUT BYTE *DestinationAddress, INPUT BOOL isCommand,
                    INPUT BOOL SecurityEnabled, INPUT miwi_band mb);
#endif

#ifdef ENABLE_FREQUENCY_AGILITY
    void StartChannelHopping(INPUT BYTE OptimalChannel, INPUT miwi_band mb);
#endif

BOOL CheckForData(miwi_band mb);

//LSI-CWSN new functions:
    static void Transceivers_Tasks(BYTE transceiver);
    static void AuxMAC_DiscardPacket(BYTE transceiver);
    static BOOL AuxMAC_SendPacket(MAC_TRANS_PARAM *MTP, BYTE transceiver);

/******************************* FUNCTIONS ************************************/

/******************************************************************************/
// C18 compiler cannot optimize the code with a macro. Instead of calling macro
// Nop in a big function, we define a wrapper to call Nop in order to be able to
// optimize the code efficiently.
/******************************************************************************/
void MacroNop(void) {
    Nop();
}

/*******************************************************************************
 * Function:    void P2PTasks( void )
 * Overview:    This function maintains the operation of the stack. It should be
 *              called as often as possible.
 * PreCondition:None
 * Input:       None
 * Output:      None
 * Side Effects:The stack receives, handles, buffers, and transmits packets. It
 *              also handles all of the joining
 ******************************************************************************/
void P2PTasks(void) {
    BYTE i;
    MIWI_TICK tmpTick;
    #ifdef ENABLE_INDIRECT_MESSAGE
        // check indirect message periodically. If an indirect message is not
        // acquired within time of INDIRECT_MESSAGE_TIMEOUT
        for(i = 0; i < INDIRECT_MESSAGE_SIZE; i++){
            if(indirectMessages[i].flags.bits.isValid){
                tmpTick = MiWi_TickGet();
                if(MiWi_TickGetDiff(tmpTick, indirectMessages[i].TickStart) \
                   > INDIRECT_MESSAGE_TIMEOUT){
                    indirectMessages[i].flags.Val = 0x00;
                    Printf("\r\nIndirect message expired");
                }
            }
        }
    #endif
    #ifdef ENABLE_SLEEP
        // check if a response for Data Request has been received with in time
        // of RFD_DATA_WAIT, defined in P2P.h. Expire the Data Request to let
        // device goes to sleep, if no response is received. Save battery power
        // even if something wrong with associated device
        #if defined MRF24J40
            if(MRF24J40_P2PStatus.bits.DataRequesting){
                tmpTick = MiWi_TickGet();
                if( MiWi_TickGetDiff(tmpTick, MRF24J40_DataReqTimer) > RFD_DATA_WAIT ) {
                    Printf("Data Request Expired\r\n");
                    MRF24J40_P2PStatus.bits.DataRequesting = 0;
                    #if defined(ENABLE_TIME_SYNC)
                        MRF24J40_WakeupTimes.Val = RFD_WAKEUP_INTERVAL / 16;
                        MRF24J40_CounterValue.Val = 0xFFFF - ((WORD)4000*(RFD_WAKEUP_INTERVAL % 16));
                    #endif
                }
            }
        #endif
        #if defined MRF49XA_1
            if(MRF49XA_1_P2PStatus.bits.DataRequesting){
                tmpTick = MiWi_TickGet();
                if( MiWi_TickGetDiff(tmpTick, MRF49XA_1_DataReqTimer) > RFD_DATA_WAIT ) {
                    Printf("Data Request Expired\r\n");
                    MRF49XA_1_P2PStatus.bits.DataRequesting = 0;
                    #if defined(ENABLE_TIME_SYNC)
                        MRF49XA_1_WakeupTimes.Val = RFD_WAKEUP_INTERVAL / 16;
                        MRF49XA_1_CounterValue.Val = 0xFFFF - ((WORD)4000*(RFD_WAKEUP_INTERVAL % 16));
                    #endif
                }
            }
        #endif
        #if defined MRF49XA_2
            if(MRF49XA_2_P2PStatus.bits.DataRequesting){
                tmpTick = MiWi_TickGet();
                if( MiWi_TickGetDiff(tmpTick, MRF49XA_2_DataReqTimer) > RFD_DATA_WAIT ) {
                    Printf("Data Request Expired\r\n");
                    MRF49XA_2_P2PStatus.bits.DataRequesting = 0;
                    #if defined(ENABLE_TIME_SYNC)
                        MRF49XA_2_WakeupTimes.Val = RFD_WAKEUP_INTERVAL / 16;
                        MRF49XA_2_CounterValue.Val = 0xFFFF - ((WORD)4000*(RFD_WAKEUP_INTERVAL % 16));
                    #endif
                }
            }
        #endif
        #if defined MRF89XA
            if(MRF89XA_P2PStatus.bits.DataRequesting){
                tmpTick = MiWi_TickGet();
                if( MiWi_TickGetDiff(tmpTick, MRF89XA_DataReqTimer) > RFD_DATA_WAIT ) {
                    Printf("Data Request Expired\r\n");
                    MRF89XA_P2PStatus.bits.DataRequesting = 0;
                    #if defined(ENABLE_TIME_SYNC)
                        MRF89XA_WakeupTimes.Val = RFD_WAKEUP_INTERVAL / 16;
                        MRF89XA_CounterValue.Val = 0xFFFF - ((WORD)4000*(RFD_WAKEUP_INTERVAL % 16));
                    #endif
                }
            }
        #endif
    #endif

    #if defined(ENABLE_NETWORK_FREEZER)
    //This section needs NVM functions to be adapted!!!
        #if defined MRF24J40
            if(MRF24J40_P2PStatus.bits.SaveConnection){
                tmpTick = MiWi_TickGet();
                if(MiWi_TickGetDiff(tmpTick, nvmDelayTick) > (ONE_SECOND)){
                    MRF24J40_P2PStatus.bits.SaveConnection = 0;
                    nvmPutConnectionTable(ConnectionTable);
                    //Printf("\r\nSave Connection\r\n");
                }
            }
        #endif
        #if defined MRF49XA_1
            if(MRF49XA_1_P2PStatus.bits.SaveConnection){
                tmpTick = MiWi_TickGet();
                if(MiWi_TickGetDiff(tmpTick, nvmDelayTick) > (ONE_SECOND)){
                    MRF49XA_1_P2PStatus.bits.SaveConnection = 0;
                    nvmPutConnectionTable(ConnectionTable);
                    //Printf("\r\nSave Connection\r\n");
                }
            }
        #endif
        #if defined MRF49XA_2
            if(MRF49XA_2_P2PStatus.bits.SaveConnection){
                tmpTick = MiWi_TickGet();
                if(MiWi_TickGetDiff(tmpTick, nvmDelayTick) > (ONE_SECOND)){
                    MRF49XA_2_P2PStatus.bits.SaveConnection = 0;
                    nvmPutConnectionTable(ConnectionTable);
                    //Printf("\r\nSave Connection\r\n");
                }
            }
        #endif
        #if defined MRF89XA
            if(MRF89XA_P2PStatus.bits.SaveConnection){
                tmpTick = MiWi_TickGet();
                if(MiWi_TickGetDiff(tmpTick, nvmDelayTick) > (ONE_SECOND)){
                    MRF89XA_P2PStatus.bits.SaveConnection = 0;
                    nvmPutConnectionTable(ConnectionTable);
                    //Printf("\r\nSave Connection\r\n");
                }
            }
        #endif
    #endif

    #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && \
        defined(ENABLE_INDIRECT_MESSAGE)
        tmpTick = MiWi_TickGet();
        if(MiWi_TickGetDiff(tmpTick, TimeSyncTick) > ((ONE_SECOND)*RFD_WAKEUP_INTERVAL)){
            TimeSyncTick.Val += ((DWORD)(ONE_SECOND) * RFD_WAKEUP_INTERVAL);
            if( TimeSyncTick.Val > tmpTick.Val ) {
                TimeSyncTick.Val = tmpTick.Val;
            }
            TimeSyncSlot = 0;
        }
    #endif
    // Check if the transceivers have received any message. ------------------//
    //coreTMRvals[1] = ReadCoreTimer();
    #if defined MRF49XA_1
        Transceivers_Tasks(1);
    #endif
    #if defined MRF49XA_2
        Transceivers_Tasks(2);
    #endif
    //coreTMRvals[2] = ReadCoreTimer();
    #if defined MRF24J40
        Transceivers_Tasks(3);
    #endif
    //coreTMRvals[3] = ReadCoreTimer();
    return;
    //------------------------------------------------------------------------//
}

// Microchip LSI Stack
// Extracted from P2PTasks and modified to do all transceiver tasks by invoking
// the function with different parameters.
static void Transceivers_Tasks(BYTE transceiver){
    BYTE *pbuffer, *TxData, *currentChannel;
    BYTE ChannelOffset, ConnMode;
    volatile P2P_STATUS *pstatus;
    P2P_CAPACITY *P2PCapacityInfo;
    RECEIVED_MESSAGE *MIWI_rxMsg;
    MAC_RECEIVED_PACKET *MAC_rxPckt;
    miwi_band mb;
    BYTE NOT_RI_MASK;
    BYTE p;
    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
        WORD_VAL *WakeupTimes;
        WORD_VAL *CounterValue;
    #endif
    switch (transceiver){
        case 1:
            #if defined MRF49XA_1
                if(MiMAC_MRF49XA_ReceivedPacket(1)){
                    #if defined MRF49XA_1_IN_434
                        MIWI_rxMsg = &MIWI0434_rxMessage;
                        P2PCapacityInfo = &MIWI0434CapacityInfo;
                        ConnMode = MIWI0434_ConnMode;
                        currentChannel = &MIWI0434_currentChannel;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        mb = ISM_434;
                        NOT_RI_MASK = ~(MIWI_0434_RI_MASK);
                    #elif defined MRF49XA_1_IN_868
                        MIWI_rxMsg = &MIWI0868_rxMessage;
                        P2PCapacityInfo = &MIWI0868CapacityInfo;
                        ConnMode = MIWI0868_ConnMode;
                        currentChannel = &MIWI0868_currentChannel;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        mb = ISM_868;
                        NOT_RI_MASK = ~(MIWI_0868_RI_MASK);
                    #endif
                    MAC_rxPckt = &MRF49XA_1_MACRxPacket;
                    TxData = & MRF49XA_1_TxData;
                    pbuffer = MRF49XA_1_TxBuffer;
                    pstatus = &MRF49XA_1_P2PStatus;
                    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                        WakeupTimes = &MRF49XA_1_WakeupTimes;
                        CounterValue = &MRF49XA_1_WakeupTimes;
                    #endif
                    break;
                } else {
                    return;     //Nothing to do for this transceiver.
                }
            #else
                Printf("\rInvalid call to MRF49XA_1 tasks. This transceiver is unavailable.");
                return;
            #endif
        case 2:
            #if defined MRF49XA_2
                if(MiMAC_MRF49XA_ReceivedPacket(2)){
                    #if defined MRF49XA_2_IN_434
                        MIWI_rxMsg = &MIWI0434_rxMessage;
                        P2PCapacityInfo = &MIWI0434CapacityInfo;
                        ConnMode = MIWI0434_ConnMode;
                        currentChannel = &MIWI0434_currentChannel;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        mb = ISM_434;
                        NOT_RI_MASK = ~(MIWI_0434_RI_MASK);
                    #elif defined MRF49XA_2_IN_868
                        MIWI_rxMsg = &MIWI0868_rxMessage;
                        P2PCapacityInfo = &MIWI0868CapacityInfo;
                        ConnMode = MIWI0868_ConnMode;
                        currentChannel = &MIWI0868_currentChannel;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        mb = ISM_868;
                        NOT_RI_MASK = ~(MIWI_0868_RI_MASK);
                    #endif
                    MAC_rxPckt = &MRF49XA_2_MACRxPacket;
                    TxData = & MRF49XA_2_TxData;
                    pbuffer = MRF49XA_2_TxBuffer;
                    pstatus = &MRF49XA_2_P2PStatus;
                    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                        WakeupTimes = &MRF49XA_2_WakeupTimes;
                        CounterValue = &MRF49XA_2_WakeupTimes;
                    #endif
                    break;
                } else {
                    return;     //Nothing to do for this transceiver.
                }
            #else
                Printf("\rInvalid call to MRF49XA_2 tasks. This transceiver is unavailable.");
                return;
            #endif
        case 3:
            #if defined MRF24J40
                if(MiMAC_MRF24J40_ReceivedPacket()){
                    MIWI_rxMsg = &MIWI2400_rxMessage;
                    P2PCapacityInfo = &MIWI2400CapacityInfo;
                    ConnMode = MIWI2400_ConnMode;
                    currentChannel = &MIWI2400_currentChannel;
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    mb = ISM_2G4;
                    NOT_RI_MASK = ~(MIWI_2400_RI_MASK);
                    MAC_rxPckt = &MRF24J40_MACRxPacket;
                    TxData = & MRF24J40_TxData;
                    pbuffer = MRF24J40_TxBuffer;
                    pstatus = &MRF24J40_P2PStatus;
                    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                        WakeupTimes = &MRF24J40_WakeupTimes;
                        CounterValue = &MRF24J40_WakeupTimes;
                    #endif
                    break;
                } else {
                    return;     //Nothing to do for this transceiver.
                }
            #else
                Printf("\rInvalid call to MRF24J40 tasks. This transceiver is unavailable.");
                return;
            #endif
        default:
            Printf("\rInvalid call to transceivers tasks. Unknown transceiver.");
            return;
    }
    BYTE i;
    MIWI_rxMsg->flags.Val = 0;
    MIWI_rxMsg->SourcePANID.Val = MAC_rxPckt->SourcePANID.Val;
    MIWI_rxMsg->flags.bits.broadcast = MAC_rxPckt->flags.bits.broadcast;
    MIWI_rxMsg->flags.bits.secEn = MAC_rxPckt->flags.bits.secEn;
    MIWI_rxMsg->flags.bits.command = (MAC_rxPckt->flags.bits.packetType == \
                                             PACKET_TYPE_COMMAND) ? 1:0;
    MIWI_rxMsg->flags.bits.srcPrsnt = MAC_rxPckt->flags.bits.sourcePrsnt;
    if(MAC_rxPckt->flags.bits.sourcePrsnt) {
        MIWI_rxMsg->SourceAddress = MAC_rxPckt->SourceAddress;
    }
    #if defined(IEEE_802_15_4) && !defined(TARGET_SMALL)
    if(transceiver == 3){
        MIWI_rxMsg->SourcePANID.Val = MAC_rxPckt->SourcePANID.Val;
    }
    #endif

    MIWI_rxMsg->PayloadSize = MAC_rxPckt->PayloadLen;
    MIWI_rxMsg->Payload = MAC_rxPckt->Payload;

    #ifndef TARGET_SMALL
        MIWI_rxMsg->PacketLQI = MAC_rxPckt->LQIValue;
        MIWI_rxMsg->PacketRSSI = MAC_rxPckt->RSSIValue;
    #endif

    if (MIWI_rxMsg->flags.bits.command) {
        // if comes here, we know it is a command frame
        switch(MIWI_rxMsg->Payload[0] ) {
            #if defined(ENABLE_HAND_SHAKE)
                case CMD_P2P_CONNECTION_REQUEST:
                    
                    //Printf("\r\nAlguien quiere estar conectado a mi :) ");
                    {
                        BYTE status = STATUS_SUCCESS;

                        // if a device goes to sleep, it can only have one
                        // connection, as the result, it cannot accept new
                        // connection request
                        #ifdef ENABLE_SLEEP
                            AuxMAC_DiscardPacket(transceiver);
                            break;
                        #else
                            // if channel does not match, it may be a
                            // sub-harmonics signal, ignore the request
                            if(*currentChannel != MIWI_rxMsg->Payload[1]){
                                AuxMAC_DiscardPacket(transceiver);
                                break;
                            }

                            // if new connection is not allowed, ignore
                            // the request
                            if( ConnMode == DISABLE_ALL_CONN ) {
                                AuxMAC_DiscardPacket(transceiver);
                                break;
                            }

                            #if !defined(TARGET_SMALL) && defined(IEEE_802_15_4)
                                // if PANID does not match, ignore the request
                                if((MIWI_rxMsg->SourcePANID.Val != 0xFFFF) &&
                                   (MIWI_rxMsg->SourcePANID.Val != myPANID.Val) &&
                                   (MIWI_rxMsg->PayloadSize > 2) && (transceiver == 3)){
                                    //Juan: added the last condition for only entrying
                                    //this point with MRF24J40.
                                    status = STATUS_NOT_SAME_PAN;
                                }
                                else
                            #endif
                            {
                                // request accepted, try to add the requesting
                                // device into P2P Connection Entry
                                status = AddConnection(mb);
                            }

                            if((ConnMode == ENABLE_PREV_CONN) && (status != STATUS_EXISTS && \
                                status != STATUS_UPDATED && status != STATUS_ACTIVE_SCAN)){
                                //Juan: added UPDATED condition in case the
                                //connection was updated. If so, let's permit
                                //the peer node know that it can use another
                                //interface in its connection with this node.

                                status = STATUS_NOT_PERMITTED;
                                //Juan: status if it was a new connection.
                            }

                            if((status == STATUS_SUCCESS || status == STATUS_EXISTS) &&
                                MiApp_CB_AllowConnection(LatestConnection) == FALSE ) {

                                //Juan: this if{} seems absurd to me, as in P2P
                                //is intended to check the connection table in
                                //cases SUCCESS and EXISTS with MiAppCB function
                                //and it is defined as always TRUE in a macro...
                                ConnectionTable[LatestConnection].status.Val = 0;
                                status = STATUS_NOT_PERMITTED;
                            }

                            // prepare the P2P_CONNECTION_RESPONSE command
                            MiApp_FlushTx(mb);
                            MiApp_WriteData(CMD_P2P_CONNECTION_RESPONSE, mb);
                            MiApp_WriteData(status, mb);
                            //if(status == STATUS_SUCCESS || status == STATUS_EXISTS){ //Juan: modified as below.
                            if(status == STATUS_SUCCESS || status == STATUS_EXISTS || status == STATUS_UPDATED){
                                MiApp_WriteData(P2PCapacityInfo->Val, mb);
                                #if ADDITIONAL_NODE_ID_SIZE > 0
                                    for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                                        MiApp_WriteData(AdditionalNodeID[i], mb);
                                    }
                                #endif
                            }

                            AuxMAC_DiscardPacket(transceiver);
                            
                                                       

                            // unicast the response to the requesting device
                            #ifdef TARGET_SMALL
                                #if defined(IEEE_802_15_4)
                                    SendPacket(FALSE, myPANID, MIWI_rxMsg->SourceAddress, \
                                               TRUE, MIWI_rxMsg->flags.bits.secEn, mb);
                                #else
                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, TRUE, \
                                               MIWI_rxMsg->flags.bits.secEn, mb);
                                #endif
                            #else
                                #if defined(IEEE_802_15_4)
                                    SendPacket(FALSE, MIWI_rxMsg->SourcePANID,  \
                                               MIWI_rxMsg->SourceAddress, TRUE, \
                                               MIWI_rxMsg->flags.bits.secEn, mb);
                                #else
                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, \
                                               TRUE, MIWI_rxMsg->flags.bits.secEn, mb);
                                #endif
                            #endif

                            #if defined(ENABLE_NETWORK_FREEZER)
                                if( status == STATUS_SUCCESS ) {
                                    nvmPutConnectionTableIndex(&(ConnectionTable[LatestConnection]), \
                                                               LatestConnection);
                                }
                            #endif                     
                        #endif  // end of ENABLE_SLEEP
                    }
                    break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
                case CMD_P2P_NEW_COMMAND:
                    {   
                        BYTE i,j;
                        BYTE size = 1;
                        BYTE new = 1;
                        BYTE addressReceived[MY_ADDRESS_LENGTH];
                        Printf("\r\nTamaño mensaje recibido: \n");
                        PrintDec(MIWI_rxMsg->PayloadSize);
                        Printf("\r\n EMISOR: ");
                        for (j=0; j < MY_ADDRESS_LENGTH; j++){
                            PrintChar(MIWI_rxMsg->SourceAddress[j]);
                        }
                            
                        //FIRST WE CHECK THE NEW ADDRESS DOESN'T EXIST IN THE CONNECTION TABLE
                        while (size < MIWI_rxMsg->PayloadSize){ 
                            
                            //get the address from the packet
                            Printf("\r\nDIRECCION EN PAQUETE: ");
                            for (j=0; j < MY_ADDRESS_LENGTH; j++){
                                 addressReceived[j] = MIWI_rxMsg->Payload[j + size];
                                 PrintChar(addressReceived[j]); 
                            }
                            Printf("\r\n");
                            
                            //if the address already exists we don't have nothing to do but validating it
                            for (i=0; i< CONNECTION_SIZE; i++){ 
                                    if (isSameAddress(ConnectionTable[i].Address, addressReceived)){
                                        ConnectionTable[i].status.bits.isValid = 1;
                                        new = 0;
                                    } 
                            }
                            
                            //if the address doesn't exist we save it 
                            //if (new){
                                for (i=0; i< CONNECTION_SIZE; i++){
                                    if(!ConnectionTable[i].status.bits.isValid && new){
                                        ConnectionTable[i].status.Val = 0;
                                        ConnectionTable[i].MiWiInterfaces = 0;
                                        for (j=0; j < MY_ADDRESS_LENGTH; j++){
                                             ConnectionTable[i].Address[j] = addressReceived[j];
                                        } 
                                        ConnectionTable[i].status.bits.isValid = 1;
                                        new = 0;
                                    }
                                }
                            //}
                                    
                            size += MY_ADDRESS_LENGTH;
                            new = 1;
                        }
                        MiApp_DiscardMessage(mb);
                        size = 0;
                }break;
                    
                    
                case CMD_P2P_ACTIVE_SCAN_REQUEST:
                    //Printf("\r\nME QUIEREN ESCANEAR :) ");
                    {   
                        
                    #ifdef ENABLE_SLEEP
                        AuxMAC_DiscardPacket(transceiver);
                        break;
                    #endif
                        if(ConnMode > ENABLE_ACTIVE_SCAN_RSP){
                            AuxMAC_DiscardPacket(transceiver);
                            break;
                        }
                        if(*currentChannel != MIWI_rxMsg->Payload[1]){
                            AuxMAC_DiscardPacket(transceiver);
                            break;
                        }

                        MiApp_FlushTx(mb);
                        MiApp_WriteData(CMD_P2P_ACTIVE_SCAN_RESPONSE, mb);
                        MiApp_WriteData(P2PCapacityInfo->Val, mb);
                        #if ADDITIONAL_NODE_ID_SIZE > 0
                            for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                                MiApp_WriteData(AdditionalNodeID[i], mb);
                            }
                        #endif
                        AuxMAC_DiscardPacket(transceiver);

                        // unicast the response to the requesting device
                        #ifdef TARGET_SMALL
                            #if defined(IEEE_802_15_4)
                                SendPacket(FALSE, myPANID, MIWI_rxMsg->SourceAddress, \
                                           TRUE, MIWI_rxMsg->flags.bits.secEn, mb);
                            #else
                                SendPacket(FALSE, MIWI_rxMsg->SourceAddress, TRUE, \
                                           MIWI_rxMsg->flags.bits.secEn, mb);
                            #endif
                        #else
                            #if defined(IEEE_802_15_4)
                                SendPacket(FALSE, MIWI_rxMsg->SourcePANID, \
                                           MIWI_rxMsg->SourceAddress, TRUE,\
                                           MIWI_rxMsg->flags.bits.secEn, mb);
                            #else
                                SendPacket(FALSE, MIWI_rxMsg->SourceAddress, \
                                           TRUE, MIWI_rxMsg->flags.bits.secEn, mb);
                            #endif
                        #endif
                    }
                    break;

                #ifndef TARGET_SMALL
                    case CMD_P2P_CONNECTION_REMOVAL_REQUEST:
                        {
                            MiApp_FlushTx(mb);
                            MiApp_WriteData(CMD_P2P_CONNECTION_REMOVAL_RESPONSE, mb);

                            for(i = 0; i < CONNECTION_SIZE; i++){
                                // if the record is valid
                                if(ConnectionTable[i].status.bits.isValid){
                                    // if the record is the same as the requesting device
                                    if(isSameAddress(MIWI_rxMsg->SourceAddress, \
                                                     ConnectionTable[i].Address)){
                                        // find the record.

                                        //Juan: added. Update the slot removing
                                        //the current interface.
                                        ConnectionTable[i].MiWiInterfaces &= NOT_RI_MASK;
                                        if (ConnectionTable[i].MiWiInterfaces == 0){
                                            //Juan: if it was the only interface
                                            //using this connection, disable the
                                            //record and set status to be SUCCESS
                                            ConnectionTable[i].status.Val = 0;
                                            MiApp_WriteData(STATUS_SUCCESS, mb);
                                        }
                                        else{
                                            MiApp_WriteData(STATUS_UPDATED, mb);
                                        }

                                        #if defined(ENABLE_NETWORK_FREEZER)
                                            nvmPutConnectionTableIndex(&(ConnectionTable[i]), i);
                                        #endif
                                        break;
                                    }
                                }
                            }

                            AuxMAC_DiscardPacket(transceiver);

                            if(i == CONNECTION_SIZE){
                                // not found, the requesting device is not my peer
                                MiApp_WriteData(STATUS_ENTRY_NOT_EXIST, mb);
                            }
                            #ifdef TARGET_SMALL
                                #if defined(IEEE_802_15_4)
                                    SendPacket(FALSE, myPANID, MIWI_rxMsg->SourceAddress, \
                                               TRUE, MIWI_rxMsg->flags.bits.secEn, mb);
                                #else
                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, TRUE, \
                                               MIWI_rxMsg->flags.bits.secEn, mb);
                                #endif
                            #else
                                #if defined(IEEE_802_15_4)
                                    SendPacket(FALSE, MIWI_rxMsg->SourcePANID, \
                                               MIWI_rxMsg->SourceAddress, TRUE,\
                                               MIWI_rxMsg->flags.bits.secEn, mb);
                                #else
                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, \
                                               TRUE, MIWI_rxMsg->flags.bits.secEn, mb);
                                #endif
                            #endif
                        }
                        break;
                #endif

                case CMD_P2P_CONNECTION_RESPONSE:
                    {
                        //Printf("\r\nRespuesta a una conexión, ole yo");
                        switch(MIWI_rxMsg->Payload[1]){
                            case STATUS_SUCCESS:
                            case STATUS_EXISTS:
                            case STATUS_UPDATED:
                                #if defined(IEEE_802_15_4)
                                    //Juan: added transceiver condition
                                    if(transceiver == 3){
                                        if(myPANID.Val == 0xFFFF){
                                            myPANID.Val = MIWI_rxMsg->SourcePANID.Val;
                                            {
                                                WORD tmp = 0xFFFF;
                                                MiMAC_SetAltAddress((BYTE *)&tmp, (BYTE *)&myPANID.Val);
                                            }
                                            #if defined(ENABLE_NETWORK_FREEZER)
                                                nvmPutMyPANID(myPANID.v);
                                            #endif
                                        }
                                    }
                                #endif
                                AddConnection(mb);
                                #if defined(ENABLE_NETWORK_FREEZER)
                                    pstatus->bits.SaveConnection = 1;
                                    nvmDelayTick = MiWi_TickGet();
                                #endif
                                break;
                            default:
                                if(transceiver == 3){
                                    myPANID.Val = 0xFFFF;
                                    WORD tmp = 0xFFFF;
                                    MiMAC_SetAltAddress((BYTE *)&tmp, (BYTE *)&myPANID.Val);
                                }
                                break;
                        }
                    }
                    AuxMAC_DiscardPacket(transceiver);
                    break;

                case CMD_P2P_ACTIVE_SCAN_RESPONSE:
                    {
                    //Printf("\r\nRESPUESTA DE UN ESCANER, ole yo :) ");
                        if(pstatus->bits.Resync){
                            pstatus->bits.Resync = 0;
                        }
                        #ifdef ENABLE_ACTIVE_SCAN
                            else{
                                i = 0;
                                for(; i < ActiveScanResultIndex; i++){
                                    if((ActiveScanResults[i].Channel==*currentChannel)&&
                                        isSameAddress(ActiveScanResults[i].Address, MIWI_rxMsg->SourceAddress)){
                                        //Juan: modified. Added transceiver condition.
                                        #if defined(IEEE_802_15_4)
                                            if((ActiveScanResults[i].PANID.Val==MIWI_rxMsg->SourcePANID.Val)&&
                                                (transceiver == 3))
                                        #endif
                                            {
                                            break;
                                            }
                                    }
                                }
                                if(i==ActiveScanResultIndex && (i < ACTIVE_SCAN_RESULT_SIZE)){
                                    ActiveScanResults[ActiveScanResultIndex].Channel=*currentChannel;
                                    ActiveScanResults[ActiveScanResultIndex].RSSIValue=MIWI_rxMsg->PacketRSSI;
                                    ActiveScanResults[ActiveScanResultIndex].LQIValue=MIWI_rxMsg->PacketLQI;
                                    #if defined(IEEE_802_15_4)
                                        if (transceiver == 3){
                                            ActiveScanResults[ActiveScanResultIndex].PANID.Val=MIWI_rxMsg->SourcePANID.Val;
                                        }
                                    #endif
                                    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                                        ActiveScanResults[ActiveScanResultIndex].Address[i]=MIWI_rxMsg->SourceAddress[i];
                                    }
                                    ActiveScanResults[ActiveScanResultIndex].Capability.Val=MIWI_rxMsg->Payload[1];
                                    #if ADDITIONAL_NODE_ID_SIZE > 0
                                        for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                                            ActiveScanResults[ActiveScanResultIndex].PeerInfo[i]=MIWI_rxMsg->Payload[2+i];
                                        }
                                    #endif
                                    ActiveScanResultIndex++;
                                }
                            }
                        #endif
                        AuxMAC_DiscardPacket(transceiver);
                    }
                    break;

                #ifndef TARGET_SMALL
                    case CMD_P2P_CONNECTION_REMOVAL_RESPONSE:
                    {
                        //Juan: added UPDATED condition.
                        if((MIWI_rxMsg->Payload[1] == STATUS_SUCCESS) || (MIWI_rxMsg->Payload[1] == STATUS_UPDATED)){
                            for(i = 0; i < CONNECTION_SIZE; i++){
                                // if the record is valid
                                if(ConnectionTable[i].status.bits.isValid){
                                    // if the record address is the same as the requesting device
                                    if(isSameAddress(MIWI_rxMsg->SourceAddress, ConnectionTable[i].Address)){

                                        //Juan: added. Update the slot removing
                                        //the current interface.
                                        ConnectionTable[i].MiWiInterfaces &= NOT_RI_MASK;
                                        if (ConnectionTable[i].MiWiInterfaces == 0){
                                            //Juan: if it was the only interface
                                            //using this connection, disable the
                                            //record and set status to be SUCCESS
                                            ConnectionTable[i].status.Val = 0;
                                        }

                                        #if defined(ENABLE_NETWORK_FREEZER)
                                            nvmPutConnectionTableIndex(&(ConnectionTable[i]), i);
                                        #endif
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    AuxMAC_DiscardPacket(transceiver);
                    break;
                #endif
            #endif

            #ifdef ENABLE_INDIRECT_MESSAGE
                case CMD_DATA_REQUEST:
                case CMD_MAC_DATA_REQUEST:
                    {
                        Printf("\r\n Me piden mensaje que en teoría tengo");
                        BOOL isCommand = FALSE;
                        WORD_VAL tmpW;

                        MiApp_FlushTx(mb);

                        #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                            MiApp_WriteData(CMD_TIME_SYNC_DATA_PACKET, mb);
                            isCommand = TRUE;
                            tmpTick = MiWi_TickGet();
                            //tmpW.Val=(((ONE_SECOND)*RFD_WAKEUP_INTERVAL)-(tmpTick.Val-TimeSyncTick.Val)\
                                        +(TimeSlotTick.Val*TimeSyncSlot)) / (ONE_SECOND * 16);
                            //tmpW.Val=(((ONE_SECOND)*RFD_WAKEUP_INTERVAL)-(tmpTick.Val-TimeSyncTick.Val)\
                                        +(TimeSlotTick.Val*TimeSyncSlot)) / SYMBOLS_TO_TICKS((DWORD)0xFFFF* \
                                        MICRO_SECOND_PER_COUNTER_TICK / 16);
                            tmpW.Val = (((ONE_SECOND)*RFD_WAKEUP_INTERVAL)-(tmpTick.Val-TimeSyncTick.Val)\
                                        +(TimeSlotTick.Val*TimeSyncSlot)) / SYMBOLS_TO_TICKS((DWORD)0xFFFF* \
                                        MICRO_SECOND_PER_COUNTER_TICK / 16);
                            MiApp_WriteData(tmpW.v[0], mb);
                            MiApp_WriteData(tmpW.v[1], mb);
                            //tmpW.Val= 0xFFFF-(WORD)((TICKS_TO_SYMBOLS((((ONE_SECOND)*RFD_WAKEUP_INTERVAL)- \
                                        (tmpTick.Val - TimeSyncTick.Val) + (TimeSlotTick.Val * TimeSyncSlot)+ \
                                        TimeSlotTick.Val/2 - (ONE_SECOND * tmpW.Val * 16) )) * 16 / 250));
                            //tmpW.Val= 0xFFFF-(WORD)((TICKS_TO_SYMBOLS((((ONE_SECOND)*RFD_WAKEUP_INTERVAL)- \
                                        (tmpTick.Val - TimeSyncTick.Val) + (TimeSlotTick.Val * TimeSyncSlot)+ \
                                        TimeSlotTick.Val/2-((DWORD)0xFFFF*tmpW.Val)))*16/MICRO_SECOND_PER_COUNTER_TICK));
                            tmpW.Val= 0xFFFF-(WORD)((TICKS_TO_SYMBOLS((((ONE_SECOND)*RFD_WAKEUP_INTERVAL) - \
                                      (tmpTick.Val - TimeSyncTick.Val)  + ( TimeSlotTick.Val * TimeSyncSlot )+\
                                      TimeSlotTick.Val/2 - SYMBOLS_TO_TICKS((DWORD)0xFFFF*MICRO_SECOND_PER_COUNTER_TICK\
                                      / 16 * tmpW.Val))) * 16 / MICRO_SECOND_PER_COUNTER_TICK));
                            if(TimeSyncSlot < TIME_SYNC_SLOTS){
                                TimeSyncSlot++;
                            }
                            MiApp_WriteData(tmpW.v[0], mb);
                            MiApp_WriteData(tmpW.v[1], mb);
                        #endif

                        for(i = 0; i < INDIRECT_MESSAGE_SIZE; i++){
                            if(indirectMessages[i].flags.bits.isValid){
                                BYTE j;

                                #ifdef ENABLE_BROADCAST_TO_SLEEP_DEVICE
                                    if(indirectMessages[i].flags.bits.isBroadcast){
                                        for(j = 0; j < CONNECTION_SIZE; j++){
                                            if( indirectMessages[i].DestAddress.DestIndex[j] != 0xFF &&
                                                isSameAddress(ConnectionTable[indirectMessages[i].DestAddress.DestIndex[j]].Address,\
                                                              MIWI_rxMsg->SourceAddress)){
                                                indirectMessages[i].DestAddress.DestIndex[j] = 0xFF;
                                                for(j = 0; j < indirectMessages[i].PayLoadSize; j++){
                                                    MiApp_WriteData(indirectMessages[i].PayLoad[j], mb);
                                                }
                                                #if defined(ENABLE_TIME_SYNC)
                                                    if(indirectMessages[i].flags.bits.isCommand){
                                                        *pbuffer = CMD_TIME_SYNC_COMMAND_PACKET;
                                                    }
                                                #endif
                                                #if defined(IEEE_802_15_4)
                                                    SendPacket(FALSE, indirectMessages[i].DestPANID, \
                                                               MIWI_rxMsg->SourceAddress, isCommand,\
                                                               indirectMessages[i].flags.bits.isSecured, mb);
                                                #else
                                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, \
                                                               isCommand, indirectMessages[i].flags.bits.isSecured, mb);
                                                #endif
                                                //goto DiscardPacketHere;
                                                goto END_OF_SENDING_INDIRECT_MESSAGE;
                                            }
                                        }
                                    }
                                    else
                                #endif
                                if(isSameAddress(indirectMessages[i].DestAddress.DestLongAddress, \
                                                 MIWI_rxMsg->SourceAddress)){
                                    for(j = 0; j < indirectMessages[i].PayLoadSize; j++){
                                        MiApp_WriteData(indirectMessages[i].PayLoad[j], mb);
                                    }
                                    #if defined(ENABLE_TIME_SYNC)
                                        if (indirectMessages[i].flags.bits.isCommand){
                                            *pbuffer = CMD_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                    #if defined(IEEE_802_15_4)
                                       j = SendPacket(FALSE, indirectMessages[i].DestPANID, \
                                                   indirectMessages[i].DestAddress.DestLongAddress,\
                                                   isCommand, (BOOL)indirectMessages[i].flags.bits.isSecured, mb);
                                       if (j) Printf("Mensaje indirecto devuelto");
                                       else Printf("Mensaje indirecto NO devuelto");
                                    #else
                                        SendPacket(FALSE, indirectMessages[i].DestAddress.DestLongAddress,\
                                                   isCommand, (BOOL)indirectMessages[i].flags.bits.isSecured, mb);
                                    #endif
                                    indirectMessages[i].flags.Val = 0;
                                    goto END_OF_SENDING_INDIRECT_MESSAGE;
                                }
                            }
                        }

                        if(i == INDIRECT_MESSAGE_SIZE){
                            #ifdef TARGET_SMALL
                                #if defined(IEEE_802_15_4)
                                    SendPacket(FALSE, myPANID, MIWI_rxMsg->SourceAddress,\
                                               isCommand, FALSE, mb);
                                #else
                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, \
                                               isCommand, FALSE, mb);
                                #endif
                            #else
                                #if defined(IEEE_802_15_4)
                                    SendPacket(FALSE, MIWI_rxMsg->SourcePANID, \
                                               MIWI_rxMsg->SourceAddress, isCommand, FALSE, mb);
                                #else
                                    SendPacket(FALSE, MIWI_rxMsg->SourceAddress, \
                                               isCommand, FALSE, mb);
                                #endif
                            #endif
                        }

END_OF_SENDING_INDIRECT_MESSAGE:
                        #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                            if(MAC_rxPckt->PayloadLen > 1){
                                MIWI_rxMsg->Payload = &(MAC_rxPckt->Payload[1]);
                                MIWI_rxMsg->PayloadSize--;
                                pstatus->bits.RxHasUserData = 1;
                            }
                            else
                        #endif
                        AuxMAC_DiscardPacket(transceiver);
                    }
                    break;
            #endif


            #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                case CMD_TIME_SYNC_DATA_PACKET:
                case CMD_TIME_SYNC_COMMAND_PACKET:
                    {
                        *WakeupTimes.v[0] = MIWI_rxMsg->Payload[1];
                        *WakeupTimes.v[1] = MIWI_rxMsg->Payload[2];
                        *CounterValue.v[0] = MIWI_rxMsg->Payload[3];
                        *CounterValue.v[1] = MIWI_rxMsg->Payload[4];

                        if(MIWI_rxMsg->PayloadSize > 5){
                            if(MIWI_rxMsg->Payload[0] == CMD_TIME_SYNC_DATA_PACKET){
                                MIWI_rxMsg->flags.bits.command = 0;
                            }
                            MIWI_rxMsg->PayloadSize -= 5;
                            MIWI_rxMsg->Payload = &(MIWI_rxMsg->Payload[5]);
                            pstatus->bits.RxHasUserData = 1;
                        }
                        else{
                            pstatus->bits.DataRequesting = 0;
                            AuxMAC_DiscardPacket(transceiver);
                        }
                    }
                    break;
            #endif

            #if defined(ENABLE_FREQUENCY_AGILITY)
                case CMD_CHANNEL_HOPPING:
                    if(MIWI_rxMsg->Payload[1] != *currentChannel){
                        AuxMAC_DiscardPacket(transceiver);
                        break;
                    }
                    StartChannelHopping(MIWI_rxMsg->Payload[2], mb);
                    Printf("\r\nHopping Channel to ");
                    PrintDec(*currentChannel - ChannelOffset);
                    AuxMAC_DiscardPacket(transceiver);
                    break;
            #endif

            default:
                // let upper application layer to handle undefined command frame
                pstatus->bits.RxHasUserData = 1;
                break;
        }
    }
    else{
       pstatus->bits.RxHasUserData = 1;
    }

    #ifdef ENABLE_SLEEP
        if(pstatus->bits.DataRequesting && pstatus->bits.RxHasUserData){
            pstatus->bits.DataRequesting = 0;
        }
    #endif

    if(MIWI_rxMsg->PayloadSize == 0 || pstatus->bits.SearchConnection || \
       pstatus->bits.Resync){
        pstatus->bits.RxHasUserData = 0;
        AuxMAC_DiscardPacket(transceiver);
    }
}


BOOL MiApp_ProtocolInit(BOOL bNetworkFreezer){
    BYTE i;

    MACINIT_PARAM initValue;

    #if defined(ENABLE_NVM)
        #if defined(ENABLE_NVM_MAC)
            if( MY_ADDRESS_LENGTH > 6 ){
                for(i = 0; i < 3; i++){
                    EEPROMRead(&(myLongAddress[MY_ADDRESS_LENGTH-1-i]), EEPROM_MAC_ADDR+i, 1);
                }
                myLongAddress[4] = 0xFF;
                if( MY_ADDRESS_LENGTH > 7 ){
                    myLongAddress[3] = 0xFE;
                }
                for(i = 0; i < 3; i++){
                    EEPROMRead(&(myLongAddress[2-i]), EEPROM_MAC_ADDR+3+i, 1);
                }
            }
            else{
                for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                    EEPROMRead(&(myLongAddress[MY_ADDRESS_LENGTH-1-i]), EEPROM_MAC_ADDR+i, 1);
                }
            }
        #endif
    #endif

    #if defined(ENABLE_NETWORK_FREEZER)
        NVMInit();
    #endif

    //clear all status bits
    #if defined MRF24J40
        MRF24J40_P2PStatus.Val = 0;
    #endif
    #if defined MRF49XA_1
        MRF49XA_1_P2PStatus.Val = 0;
    #endif
    #if defined MRF49XA_2
        MRF49XA_2_P2PStatus.Val = 0;
    #endif
    #if defined MRF89XA
        MRF89XA_P2PStatus.Val = 0;
    #endif

    for(i = 0; i < CONNECTION_SIZE; i++){
        ConnectionTable[i].status.Val = 0;
    }

    InitSymbolTimer();

    #if defined MRF24J40
        MRF24J40_TxData = PAYLOAD_START;
    #endif
    #if defined MRF49XA_1
        MRF49XA_1_TxData = PAYLOAD_START;
    #endif
    #if defined MRF49XA_2
        MRF49XA_2_TxData = PAYLOAD_START;
    #endif
    #if defined MRF89XA
        MRF89XA_TxData = PAYLOAD_START;
    #endif

    #ifdef ENABLE_INDIRECT_MESSAGE
        for(i = 0; i < INDIRECT_MESSAGE_SIZE; i++){
            indirectMessages[i].flags.Val = 0;
        }
    #endif
    #if defined(ENABLE_SECURITY)
        for(i = 0; i < CONNECTION_SIZE; i++){
            IncomingFrameCounter[i].Val = 0;
        }
    #endif
    #if defined(ENABLE_NETWORK_FREEZER)
        if(bNetworkFreezer){
            // WARNING NVM NOT ADAPTED!!! Needs modifying nvm functions (which
            // radio interface is involved)
            #if defined MIWI_0434_RI
                nvmGetCurrentChannel(&MIWI0434_currentChannel);
                if(MIWI0434_currentChannel >= 32){
                    return FALSE;
                }
            #endif
            #if defined MIWI_0868_RI
                nvmGetCurrentChannel(&MIWI0868_currentChannel);
                if(MIWI0868_currentChannel >= 32){
                    return FALSE;
                }
            #endif
            #if defined MIWI_2400_RI
                nvmGetCurrentChannel(&MIWI2400_currentChannel);
                if(MIWI2400_currentChannel >= 32){
                    return FALSE;
                }
            #endif

            #if defined(IEEE_802_15_4)
                nvmGetMyPANID(myPANID.v);
            #endif

            nvmGetConnMode(&MIWI0434_ConnMode);
            nvmGetConnMode(&MIWI0868_ConnMode);
            nvmGetConnMode(&MIWI2400_ConnMode);
            nvmGetConnectionTable(ConnectionTable);

            #if defined(IEEE_802_15_4)
                Printf("\r\nPANID:");
                PrintChar(myPANID.v[1]);
                PrintChar(myPANID.v[0]);
            #endif

            #if defined MIWI_0434_RI
                Printf(" Channel MIWI at 434 MHz:");
                PrintChar(MIWI0434_currentChannel - MIWI0434ConfChannelOffset);
            #endif
            #if defined MIWI_0868_RI
                Printf(" Channel MIWI at 868 MHz:");
                PrintChar(MIWI0868_currentChannel - MIWI0868ConfChannelOffset);
            #endif
            #if defined MIWI_2400_RI
                Printf(" Channel MIWI at 2,4 GHz:");
                PrintChar(MIWI2400_currentChannel - MIWI2400ConfChannelOffset);
            #endif
        }
        else{
            #if defined(IEEE_802_15_4)
                myPANID.Val = MY_PAN_ID;
                nvmPutMyPANID(myPANID.v);
            #endif

            #if defined MIWI_0434_RI
                nvmPutCurrentChannel(&MIWI0434_currentChannel);
            #endif
            #if defined MIWI_0868_RI
                nvmPutCurrentChannel(&MIWI0868_currentChannel);
            #endif
            #if defined MIWI_2400_RI
                nvmPutCurrentChannel(&MIWI2400_currentChannel);
            #endif

            nvmPutConnMode(&MIWI0434_ConnMode);
            nvmPutConnMode(&MIWI0868_ConnMode);
            nvmPutConnMode(&MIWI2400_ConnMode);
            nvmPutConnectionTable(ConnectionTable);
        }
    #else
        #if defined(IEEE_802_15_4)
            myPANID.Val = MY_PAN_ID;
        #endif
    #endif

    initValue.PAddress = myLongAddress;
    initValue.actionFlags.bits.CCAEnable = 1;
    initValue.actionFlags.bits.PAddrLength = MY_ADDRESS_LENGTH;
    initValue.actionFlags.bits.NetworkFreezer = bNetworkFreezer;
    initValue.actionFlags.bits.RepeaterMode = 0;

    #if defined MRF49XA_1
        MiMAC_MRF49XA_Init(initValue, 1);
        #if defined MRF49XA_1_IN_434
            MiApp_SetChannel(MIWI0434_currentChannel, ISM_434);
        #elif defined MRF49XA_1_IN_868
            MiApp_SetChannel(MIWI0868_currentChannel, ISM_868);
        #endif
    #endif
    #if defined MRF49XA_2
        MiMAC_MRF49XA_Init(initValue, 2);
        #if defined MRF49XA_2_IN_434
            MiApp_SetChannel(MIWI0434_currentChannel, ISM_434);
        #elif defined MRF49XA_2_IN_868
            MiApp_SetChannel(MIWI0868_currentChannel, ISM_868);
        #endif
    #endif
    #if defined MRF24J40
        MiMAC_MRF24J40_Init(initValue);
        MiApp_SetChannel(MIWI2400_currentChannel, ISM_2G4);
    #endif
//    #if defined MRF89XA
//        MiMAC_MRF89XA_Init(initValue);
//        MiApp_SetChannel(MIWI0868_currentChannel, ISM_868);
//    #endif

    #if defined(IEEE_802_15_4)
        {
            // Microchip LSI Stack - Only MRF24J40 supports Alt. Address as it
            // is for IEEE 802.15.4 in 2,4 GHz band.
            WORD tmp = 0xFFFF;
            MiMAC_SetAltAddress((BYTE *)&tmp, (BYTE *)&myPANID.Val);
        }
    #endif

    #if defined(ENABLE_TIME_SYNC)
        #if defined(ENABLE_SLEEP)
            #if defined MRF24J40
                MRF24J40_WakeupTimes.Val = 0;
                MRF24J40_CounterValue.Val = 61535;  //(0xFFFF - 4000) one second
            #endif
            #if defined MRF49XA_1
                MRF49XA_1_WakeupTimes.Val = 0;
                MRF49XA_1_CounterValue.Val = 61535; //(0xFFFF - 4000) one second
            #endif
            #if defined MRF49XA_2
                MRF49XA_2_WakeupTimes.Val = 0;
                MRF49XA_2_CounterValue.Val = 61535; //(0xFFFF - 4000) one second
            #endif
            #if defined MRF89XA
                MRF89XA_WakeupTimes.Val = 0;
                MRF89XA_CounterValue.Val = 61535;   //(0xFFFF - 4000) one second
            #endif
        #elif defined(ENABLE_INDIRECT_MESSAGE)
            TimeSlotTick.Val=((ONE_SECOND)*RFD_WAKEUP_INTERVAL)/TIME_SYNC_SLOTS;
        #endif
    #endif

    BYTE P2PCapacityInfo = 0;
    #if !defined(ENABLE_SLEEP)
        P2PCapacityInfo |= 0x01;
    #endif
    #if defined(ENABLE_SECURITY)
        P2PCapacityInfo |= 0x08;
    #endif
    #if defined MRF49XA_1
        #if defined MRF49XA_1_IN_434
            MIWI0434CapacityInfo.Val = P2PCapacityInfo | (MIWI0434_ConnMode << 4);
        #elif defined MRF49XA_1_IN_868
            MIWI0868CapacityInfo.Val = P2PCapacityInfo | (MIWI0868_ConnMode << 4);
        #endif
    #endif
    #if defined MRF49XA_2
        #if defined MRF49XA_2_IN_434
            MIWI0434CapacityInfo.Val = P2PCapacityInfo | (MIWI0434_ConnMode << 4);
        #elif defined MRF49XA_2_IN_868
            MIWI0868CapacityInfo.Val = P2PCapacityInfo | (MIWI0868_ConnMode << 4);
        #endif
    #endif
    #if defined MRF24J40
        MIWI2400CapacityInfo.Val = P2PCapacityInfo | (MIWI2400_ConnMode << 4);
    #endif

    #if defined MRF24J40
        MRF24J40_IE = 1;
    #endif
    #if defined MRF49XA_1
        MRF49XA_1_IE = 1;
    #endif
    #if defined MRF49XA_2
        MRF49XA_2_IE = 1;
    #endif
    #if defined MRF89XA
        PHY_IRQ1 = 1;
    #endif

    return TRUE;
}

#ifdef ENABLE_SLEEP
    /***************************************************************************
     * Function:    BYTE MiApp_TransceiverPowerState(BYTE Mode)
     * Summary:     This function put the RF transceiver into different power
     *              state. i.e. Put the RF transceiver into sleep or wake it up.
     * Description: This is the primary user interface functions for the
     *              application layer to put RF transceiver into sleep or wake
     *              it up. This function is only available to those wireless
     *              nodes that may have to disable the transceiver to save
     *              battery power.
     * PreCondition: Protocol initialization has been done.
     * Parameters:  BYTE Mode - The mode of power state for the RF transceiver
     *                          to be set. The possible power states are
     *                          following
     *                  * POWER_STATE_SLEEP     The deep sleep mode for RF
     *                                          transceiver
     *                  * POWER_STATE_WAKEUP    Wake up state, or operating
     *                                          state for RF transceiver
     *                  * POWER_STATE_WAKEUP_DR Put device into wakeup mode and
     *                                          then transmit a data request to
     *                                          the device's associated device
     * Returns:     The status of the operation. The following are the possible
     *              status
     *                  * SUCCESS           Operation successful
     *                  * ERR_TRX_FAIL      Transceiver fails to go to sleep or
     *                                      wake up
     *                  * ERR_TX_FAIL       Transmission of Data Request command
     *                                      failed. Only available if the input
     *                                      mode is POWER_STATE_WAKEUP_DR.
     *                  * ERR_RX_FAIL       Failed to receive any response to
     *                                      Data Request command. Only available
     *                                      if input mode is POWER_STATE_WAKEUP_DR.
     *                  * ERR_INVLAID_INPUT Invalid input mode.
     * Example:
     *      <code>
     *      //put RF transceiver into sleep
     *      MiApp_TransceiverPowerState(POWER_STATE_SLEEP;
     *      //Put the MCU into sleep
     *      Sleep();
     *      //wakes up the MCU by WDT, external interrupt or any other means
     *      //make sure that RF transceiver to wake up and send out Data Request
     *      MiApp_TransceiverPowerState(POWER_STATE_WAKEUP_DR);
     *      </code>
     * Remarks:     None
     **************************************************************************/
    BYTE MiApp_TransceiverPowerState(INPUT BYTE Mode, INPUT miwi_band mb){
        BYTE status;
        volatile P2P_STATUS *P2Pstatus;
        BYTE transceiver;
        switch (mb){
            case ISM_434:
                #ifndef MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    #if defined MRF49XA_1_IN_434
                        P2Pstatus = & MRF49XA_1_P2PStatus;
                        transceiver = 1;
                    #elif defined MRF49XA_2_IN_434
                        P2Pstatus = & MRF49XA_2_P2PStatus;
                        transceiver = 2;
                    #else
                        return ERR_INVALID_INPUT; //Error
                    #endif
                    break;
                #endif
            case ISM_868:
                #ifndef MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    #if defined MRF49XA_1_IN_868
                        P2Pstatus = & MRF49XA_1_P2PStatus;
                        transceiver = 1;
                    #elif defined MRF49XA_2_IN_868
                        P2Pstatus = & MRF49XA_2_P2PStatus;
                        transceiver = 2;
                    #elif defined MRF89XA
                        P2Pstatus = & MRF89XA_P2PStatus;
                        transceiver = 4;
                    #else
                        return ERR_INVALID_INPUT; //Error
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #ifndef MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    P2Pstatus = & MRF24J40_P2PStatus;
                    transceiver = 3;
                    break;
                #endif
            default:
                return ERR_INVALID_INPUT;    //Error
        }
        BOOL ok;
        switch(Mode){
            case POWER_STATE_SLEEP:
                #if defined(ENABLE_NETWORK_FREEZER)
                    if(P2Pstatus->bits.SaveConnection){
                        nvmPutConnectionTable(ConnectionTable);
                        P2Pstatus->bits.SaveConnection = 0;
                    }
                #endif

                #if defined MRF49XA_1
                    if (transceiver == 1){
                        ok = MiMAC_MRF49XA_PowerState(POWER_STATE_DEEP_SLEEP, 1);
                    }else
                #endif
                #if defined MRF49XA_2
                    if (transceiver == 2){
                        ok = MiMAC_MRF49XA_PowerState(POWER_STATE_DEEP_SLEEP, 2);
                    }else
                #endif
                #if defined MRF24J40
                    if (transceiver == 3){
                        ok = MiMAC_MRF24J40_PowerState(POWER_STATE_DEEP_SLEEP);
                    }else
                #endif
                #if defined MRF89XA
                    if (transceiver ==4){
                        ok = MiMAC_MRF89XA_PowerState(POWER_STATE_DEEP_SLEEP);
                    }else
                #endif
                    {return HAL_INTERNAL_ERROR;} //Else transceivers. Error. Nop.

                if(ok){
                    P2Pstatus->bits.Sleeping = 1;
                    return SUCCESS;
                }
                return ERR_TRX_FAIL;

            case POWER_STATE_WAKEUP:
                #if defined MRF49XA_1
                    if (transceiver == 1){
                        ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 1);
                    }else
                #endif
                #if defined MRF49XA_2
                    if (transceiver == 2){
                        ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 2);
                    }else
                #endif
                #if defined MRF24J40
                    if (transceiver == 3){
                        ok = MiMAC_MRF24J40_PowerState(POWER_STATE_OPERATE);
                    }else
                #endif
                #if defined MRF89XA
                    if (aux ==4){
                        ok = MiMAC_MRF89XA_PowerState(POWER_STATE_OPERATE);
                    }else
                #endif
                    {return HAL_INTERNAL_ERROR;} //Else transceivers. Error. Nop.

                if (ok){
                    P2Pstatus->bits.Sleeping = 0;
                    return SUCCESS;
                }
                return ERR_TRX_FAIL;

            case POWER_STATE_WAKEUP_DR:
                #if defined MRF49XA_1
                    if (transceiver == 1){
                        ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 1);
                    }else
                #endif
                #if defined MRF49XA_2
                    if (transceiver == 2){
                        ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 2);
                    }else
                #endif
                #if defined MRF24J40
                    if (transceiver == 3){
                        ok = MiMAC_MRF24J40_PowerState(POWER_STATE_OPERATE);
                    }else
                #endif
                #if defined MRF89XA
                    if (aux ==4){
                        ok = MiMAC_MRF89XA_PowerState(POWER_STATE_OPERATE);
                    }else
                #endif
                    {return HAL_INTERNAL_ERROR;} //Else transceivers. Error. Nop.

                if(ok == FALSE){
                    return ERR_TRX_FAIL;
                }
                P2Pstatus->bits.Sleeping = 0;
                if(CheckForData(mb) == FALSE){
                    Printf("\r\n Error en checkData");
                    return ERR_TX_FAIL;
                }
                Printf("\r\nCheckForData was success");
                while(P2Pstatus->bits.DataRequesting){
                    P2PTasks();
                }
                return SUCCESS;

            default:
                return ERR_INVALID_INPUT;
        }
    }

     /**************************************************************************
     * Function:    BOOL CheckForData(void)
     *
     * Overview:    This function sends out a Data Request to the peer device of
     *              the first P2P connection.
     * PreCondition:Transceiver is initialized and fully waken up
     * Input:       None
     * Output:      None
     * Side Effects:The P2P stack is waiting for the response from the peer
     *              device. A data request timer has been started. In case there
     *              is no response from the peer device, the data request will
     *              time-out itself.
     **************************************************************************/
     BOOL CheckForData(INPUT miwi_band mb){
        BYTE tmpTxData;
        BYTE *pbuffer;
        BYTE *TxData;
        BYTE myParent, i;
        BYTE transceiver;
        volatile P2P_STATUS *status;
        #ifdef ENABLE_SLEEP
            MIWI_TICK *DataReqTimer;
        #endif
        switch (mb){
            case ISM_434:
                #ifndef MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_434
                        tmpTxData = MRF49XA_1_TxData;
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                        status  = & MRF49XA_1_P2PStatus;
                        transceiver = 1;
                        #ifdef ENABLE_SLEEP
                            DataReqTimer = & MRF49XA_1_DataReqTimer;
                        #endif
                    #elif defined MRF49XA_2_IN_434
                        tmpTxData = MRF49XA_2_TxData;
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                        status  = & MRF49XA_2_P2PStatus;
                        transceiver = 1;
                        #ifdef ENABLE_SLEEP
                            DataReqTimer = & MRF49XA_2_DataReqTimer;
                        #endif
                    #else
                        return FALSE;       //Error
                    #endif
                #endif
                       
                        
                break;
            case ISM_868:
                #ifndef MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_868
                        tmpTxData = MRF49XA_1_TxData;
                        TxData  = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                        status  = & MRF49XA_1_P2PStatus;
                        transceiver = 2;
                        #ifdef ENABLE_SLEEP
                            DataReqTimer = & MRF49XA_1_DataReqTimer;
                        #endif
                    #elif defined MRF49XA_2_IN_868
                        tmpTxData = MRF49XA_2_TxData;
                        TxData  = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                        status  = & MRF49XA_2_P2PStatus;
                        transceiver = 2;
                        #ifdef ENABLE_SLEEP
                            DataReqTimer = & MRF49XA_2_DataReqTimer;
                        #endif
                    #elif defined MRF89XA
                        tmpTxData = MRF89XA_TxData;
                        TxData  = & MRF89XA_TxData;
                        pbuffer = MRF89XA_TxBuffer;
                        status  = & MRF89XA_P2PStatus;
                        transceiver = 2;
                        #ifdef ENABLE_SLEEP
                            DataReqTimer = & MRF89XA_DataReqTimer;
                        #endif
                    #else
                        return FALSE;       //Error
                    #endif
                #endif
                break;
            case ISM_2G4:
                #ifndef MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    tmpTxData = MRF24J40_TxData;
                    TxData  = & MRF24J40_TxData;
                    pbuffer = MRF24J40_TxBuffer;
                    status  = & MRF24J40_P2PStatus;
                    transceiver = 3;
                    #ifdef ENABLE_SLEEP
                        DataReqTimer = & MRF24J40_DataReqTimer;
                    #endif
                #endif
                break;
            default:
                return FALSE;   //Error.

        }
        BYTE firstByte = *pbuffer;

        MiApp_FlushTx(mb);
        MiApp_WriteData(CMD_MAC_DATA_REQUEST, mb);

        for (i=0; i<CONNECTION_SIZE; i++){
            if (ConnectionTable[i].status.bits.isValid && (ConnectionTable[i].MiWiInterfaces & (0x01 << (transceiver-1))) ){
                myParent = i;  
            }
        }
        
        #if !defined(ENABLE_ENHANCED_DATA_REQUEST)
            if(tmpTxData > 0 ){
                BYTE i;
                for( i = tmpTxData; i > 1; i--){
                    *(pbuffer+i) = *(pbuffer+i-1);
                }
                *(pbuffer+1) = firstByte;
                *TxData = tmpTxData + 1;
            }
        #endif

        #if defined(IEEE_802_15_4)
            #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                if(SendPacket(FALSE, myPANID, ConnectionTable[myParent].Address, TRUE,\
                              status->bits.Enhanced_DR_SecEn, mb))
            #else
                if(SendPacket(FALSE, myPANID, ConnectionTable[myParent].Address, TRUE,\
                              FALSE, mb))
            #endif
        #else
            #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                if(SendPacket(FALSE, ConnectionTable[myParent].Address, TRUE, \
                              status->bits.Enhanced_DR_SecEn, mb))
            #else
                if(SendPacket(FALSE, ConnectionTable[myParent].Address, TRUE, FALSE, mb))
            #endif
        #endif
        {
                    
            status->bits.DataRequesting = 1;
            #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                status->bits.Enhanced_DR_SecEn = 0;
            #endif
            *DataReqTimer = MiWi_TickGet();
            *pbuffer = firstByte;
            *TxData = tmpTxData;

            #if defined(ENABLE_TIME_SYNC)
                #if defined(__18CXX)
                    TMR3H = 0;
                    TMR3L = 0;
                #elif defined(__dsPIC33F__) || defined(__PIC24F__) || \
                      defined(__PIC24FK__)  || defined(__PIC24H__)
                    PR1 = 0xFFFF;
                    TMR1 = 0;
                #elif defined(__PIC32MX__)
                    PR1 = 0xFFFF;
                    while(T1CONbits.TWIP);
                    TMR1 = 0;
                #endif
            #endif
            return TRUE;
        }

        *pbuffer = firstByte;
        *TxData = tmpTxData;
        #if defined(ENABLE_ENHANCED_DATA_REQUEST)
            status->bits.Enhanced_DR_SecEn = 0;
        #endif
        return FALSE;
     }
#endif

#ifdef ENABLE_INDIRECT_MESSAGE
    /***************************************************************************
     * Function:    BOOL IndirectPacket (BOOL Broadcast,
     *                      WORD_VAL DestinationPANID, BYTE *DestinationAddress,
     *                      BOOL isCommand, BOOL SecurityEnabled)
     * Overview:    This function store the indirect message for node that turns
     *              off radio when idle
     * PreCondition:None
     * Input:   Broadcast          - Boolean to indicate if the indirect message
     *                               a broadcast message
     *          DestinationPANID   - The PAN Identifier of the destination node
     *          DestinationAddress - The pointer to the destination long address
     *          isCommand          - The boolean to indicate if the packet is
     *                               command
     *          SecurityEnabled    - The boolean to indicate if the packet needs
     *                               encryption
     * Output:      boolean to indicate if operation successful
     * Side Effects: An indirect message stored and waiting to deliever to
     *               sleeping device. An indirect message timer has started to
     *               expire the indirect message in case RFD does not acquire
     *               data in predefined interval
     **************************************************************************/
    #if defined(IEEE_802_15_4)
        BOOL IndirectPacket(INPUT BOOL Broadcast,
                            INPUT WORD_VAL DestinationPANID,
                            INPUT BYTE *DestinationAddress,
                            INPUT BOOL isCommand,
                            INPUT BOOL SecurityEnabled,
                            INPUT miwi_band mb)
    #else
        BOOL IndirectPacket(INPUT BOOL Broadcast,
                            INPUT BYTE *DestinationAddress,
                            INPUT BOOL isCommand,
                            INPUT BOOL SecurityEnabled,
                            INPUT miwi_band mb)
    #endif
    {
        BYTE i;
        #ifndef ENABLE_BROADCAST_TO_SLEEP_DEVICE
            if(Broadcast){
                return FALSE;
            }
        #endif

        BYTE *TxData;
        BYTE *pbuffer;
        if (mb == ISM_434){
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    TxData = & MRF49XA_1_TxData;
                    pbuffer = MRF49XA_1_TxBuffer;
                #elif defined MRF49XA_2_IN_434
                    TxData = & MRF49XA_2_TxData;
                    pbuffer = MRF49XA_2_TxBuffer;
                #endif
            #endif
        }
        else if (mb == ISM_868){
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    TxData = & MRF49XA_1_TxData;
                    pbuffer = MRF49XA_1_TxBuffer;
                #elif defined MRF49XA_2_IN_868
                    TxData = & MRF49XA_2_TxData;
                    pbuffer = MRF49XA_2_TxBuffer;
                #elif defined MRF89XA
                    TxData = & MRF89XA_TxData;
                    pbuffer = MRF89XA_TxBuffer;
                #endif
            #endif
        }
        else if(mb == ISM_2G4){
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return FALSE;
            #else
                TxData = & MRF24J40_TxData;
                pbuffer = MRF24J40_TxBuffer;
            #endif
        }
        else {return FALSE;}

        // loop through the available indirect message buffer and locate
        // the empty message slot
        for(i = 0; i < INDIRECT_MESSAGE_SIZE; i++)  {
            if(indirectMessages[i].flags.bits.isValid == 0){
                BYTE j;

                // store the message
                indirectMessages[i].flags.bits.isValid = TRUE;
                indirectMessages[i].flags.bits.isBroadcast = Broadcast;
                indirectMessages[i].flags.bits.isCommand = isCommand;
                indirectMessages[i].flags.bits.isSecured = SecurityEnabled;
                indirectMessages[i].MiWiFreqBand = mb;
                #if defined(IEEE_802_15_4)
                    //Juan: added mb condition. mb condition is equivalent to
                    //transceiver condition if the only transceiver available
                    //for ISM_2G4 is MRF24J40 (transceiver compliant with IEEE
                    //802.15.4 specification)
                    if(mb == ISM_2G4){
                        indirectMessages[i].DestPANID.Val = DestinationPANID.Val;
                    }
                #endif
                if (DestinationAddress != NULL){
                    for(j = 0; j < MY_ADDRESS_LENGTH; j++){
                        indirectMessages[i].DestAddress.DestLongAddress[j] = DestinationAddress[j];
                    }
                }
                #ifdef ENABLE_BROADCAST_TO_SLEEP_DEVICE
                else {
                    BYTE k = 0;
                    for(j = 0; j < CONNECTION_SIZE; j++){
                        //if( (ConnectionTable[j].PeerInfo[0] & 0x83) == 0x82 )
                        if (ConnectionTable[j].status.bits.isValid &&
                            ConnectionTable[j].status.bits.RXOnWhenIdle == 0) {
                            indirectMessages[i].DestAddress.DestIndex[k++] = j;
                        }
                    }
                    for(; k < CONNECTION_SIZE; k++){
                        indirectMessages[i].DestAddress.DestIndex[k] = 0xFF;
                    }
                }
                #endif

                indirectMessages[i].PayLoadSize = *TxData;
                for(j = 0; j < *TxData; j++){
                    indirectMessages[i].PayLoad[j] = *(pbuffer+j);
                }
                indirectMessages[i].TickStart = MiWi_TickGet();
                return TRUE;
            }
        }
        //Juan: if there isn't a free slot, returns false...
        return FALSE;
    }
#endif

/*******************************************************************************
 * Function:    BOOL SendPacket(BOOL Broadcast, WORD_VAL DestinationPANID,
 *                              BYTE *DestinationAddress, BOOL isCommand,
 *                              BOOL SecurityEnabled)
 * Overview:    This function sends the packet
 * PreCondition:Transceiver is initialized
 * Input:   BOOL     Broadcast           If packet to send needs to be broadcast
 *          WORD_VAL DestinationPANID    Destination PAN Identifier
 *          BYTE *   DestinationAddress  Pointer to destination long address
 *          BOOL     isCommand           If packet to send is a command packet
 *          BOOL     SecurityEnabled     If packet to send needs encryption
 * Output:  BOOL, If operation successful
 * Side Effects: Transceiver is triggered to transmit a packet
 ******************************************************************************/
#if defined(IEEE_802_15_4)
    BOOL SendPacket(INPUT BOOL Broadcast,
                    INPUT WORD_VAL DestinationPANID,
                    INPUT BYTE *DestinationAddress,
                    INPUT BOOL isCommand,
                    INPUT BOOL SecurityEnabled,
                    INPUT miwi_band mb)
#else
    BOOL SendPacket(INPUT BOOL Broadcast,
                    INPUT BYTE *DestinationAddress,
                    INPUT BOOL isCommand,
                    INPUT BOOL SecurityEnabled,
                    INPUT miwi_band mb)
#endif
{
    BYTE transceiver;
    BYTE *TxData;
    switch (mb){
        case ISM_434:
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    transceiver = 1;
                    TxData = & MRF49XA_1_TxData;
                #elif defined MRF49XA_2_IN_434
                    transceiver = 2;
                    TxData = & MRF49XA_2_TxData;
                #endif
                break;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    transceiver = 1;
                    TxData = & MRF49XA_1_TxData;
                #elif defined MRF49XA_2_IN_868
                    transceiver = 2;
                    TxData = & MRF49XA_2_TxData;
                #elif defined MRF89XA
                    transceiver = 4;
                    TxData = & MRF89XA_TxData;
                #endif
                break;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return FALSE;
            #else
                transceiver = 3;
                TxData = & MRF24J40_TxData;
                break;
            #endif
        default:
            return FALSE;
    }

    MAC_TRANS_PARAM tParam;

    tParam.flags.Val = 0;
    tParam.flags.bits.packetType = (isCommand)? PACKET_TYPE_COMMAND : PACKET_TYPE_DATA;
    tParam.flags.bits.ackReq = (Broadcast) ? 0 : 1;
    tParam.flags.bits.broadcast = Broadcast;
    tParam.flags.bits.secEn = SecurityEnabled;
    #if defined(IEEE_802_15_4)
        if (transceiver ==3){
            tParam.altSrcAddr = 0;
            tParam.altDestAddr = (Broadcast)? TRUE : FALSE;
        }
    #endif

    #if defined(INFER_DEST_ADDRESS)
        tParam.flags.bits.destPrsnt = 0;
    #else
        tParam.flags.bits.destPrsnt = (Broadcast)? 0:1;
    #endif

    #if defined(SOURCE_ADDRESS_ABSENT)
        if(tParam.flags.bits.packetType == PACKET_TYPE_COMMAND){
            tParam.flags.bits.sourcePrsnt = 1;
        }
        else{
            tParam.flags.bits.sourcePrsnt = 0;
        }
    #else
        tParam.flags.bits.sourcePrsnt = 1;
    #endif

    tParam.DestAddress = DestinationAddress;
    
    #if defined(IEEE_802_15_4)
        if(transceiver == 3){
            tParam.DestPANID.Val = DestinationPANID.Val;
        }
    #endif
    if (AuxMAC_SendPacket(&tParam, transceiver)){
        //Printf("TRUE.");
        *TxData = PAYLOAD_START;
        return TRUE;
    }
    else{
        //Printf("FALSE.");
        return FALSE;
    }
}

/*******************************************************************************
 * Function:    BOOL MiApp_BroadcastPacket(BOOL SecEn)
 * Summary:     This function broadcast a message in the TxBuffer.
 * Description: This is the primary user interface function for the application
 *              layer to broadcast a message. The application payload is filled
 *              in the global char array TxBuffer.
 * PreCondition:Protocol initialization has been done.
 * Parameters:  BOOL SecEn - The boolean indicates if the application payload
 *                           needs to be secured before transmission.
 * Returns:     A boolean to indicates if the broadcast procedure is
 *              succcessful.
 * Example:
 *      <code>
 *      // Secure and then broadcast the message stored in TxBuffer
 *      MiApp_BroadcastPacket(TRUE);
 *      </code>
 * Remarks:     None
 ******************************************************************************/
BOOL MiApp_BroadcastPacket(INPUT BOOL SecEn, INPUT miwi_band mb){
    volatile P2P_STATUS *pstatus;
    switch (mb){
        case ISM_434:
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    #if defined MRF49XA_1_IN_434
                        pstatus = &MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_434
                        pstatus = &MRF49XA_2_P2PStatus;
                    #endif
                #endif
                break;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    #if defined MRF49XA_1_IN_868
                        pstatus = &MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_868
                        pstatus = &MRF49XA_2_P2PStatus;
                    #elif defined MRF89XA
                        pstatus = &MRF89XA_P2PStatus;
                    #endif
                #endif
                break;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    pstatus = &MRF24J40_P2PStatus;
                #endif
                break;
            #endif
        default:
            return FALSE;
    }

    #ifdef ENABLE_INDIRECT_MESSAGE
        BYTE i;
        for(i = 0; i < CONNECTION_SIZE; i++){
            if(ConnectionTable[i].status.bits.isValid &&
               ConnectionTable[i].status.bits.RXOnWhenIdle == 0){
                #if defined(IEEE_802_15_4)
                    if(IndirectPacket(TRUE, myPANID, NULL, FALSE, SecEn, mb))
                #else
                    if(IndirectPacket(TRUE, NULL, FALSE, SecEn, mb))
                #endif
                    {
                        Printf("\rIndirect packet saved.");
                    }else{
                        Printf("\rIndirect packet couldn't be saved.");
                    }
                break;
            }
        }
    #endif

    #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
        if(pstatus->bits.Sleeping){
            pstatus->bits.Enhanced_DR_SecEn = SecEn;
            return TRUE;
        }
    #endif

    #if defined(IEEE_802_15_4)
        return SendPacket(TRUE, myPANID, NULL, FALSE, SecEn, mb);
    #else
        return SendPacket(TRUE, NULL, FALSE, SecEn, mb);
    #endif
}

/*******************************************************************************
 * Function:    BOOL MiApp_UnicastConnection(BYTE ConnectionIndex, BOOL SecEn)
 * Summary:     This function unicast a message in the TxBuffer to the device
 *              with the input ConnectionIndex in the connection table.
 * Description: This is one of the primary user interface functions for the
 *              application layer to unicast a message. The destination device
 *              is in the connection table specified by the input parameter
 *              ConnectionIndex. The application payload is filled in the global
 *              char array TxBuffer.
 * PreCondition:Protocol initialization has been done. The input parameter
 *              ConnectionIndex points to a valid peer device in the connection
 *              table.
 * Parameters:  BYTE ConnectionIndex - The index of the destination device in
 *                                     the connection table.
 *              BOOL SecEn - The boolean indicates if the application payload
 *                           needs to be secured before transmission
 * Returns:     A boolean to indicates if the unicast procedure is succcessful.
 * Example:
 *      <code>
 *      // Secure and then unicast the message stored in TxBuffer to the first
 *      // device in the connection table
 *      MiApp_UnicastConnection(0, TRUE);
 *      </code>
 * Remarks:     None
 ******************************************************************************/
BOOL MiApp_UnicastConnection(INPUT BYTE ConnectionIndex, INPUT BOOL SecEn, \
                             INPUT miwi_band mb){
    volatile P2P_STATUS *pstatus;
    switch (mb){
        case ISM_434:
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                    #if defined MRF49XA_1_IN_434
                        pstatus = &MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_434
                        pstatus = &MRF49XA_2_P2PStatus;
                    #endif
                #endif
                break;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                    #if defined MRF49XA_1_IN_868
                        pstatus = &MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_868
                        pstatus = &MRF49XA_2_P2PStatus;
                    #elif defined MRF89XA
                        pstatus = &MRF89XA_P2PStatus;
                    #endif
                #endif
                break;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                    pstatus = &MRF24J40_P2PStatus;
                #endif
                break;
            #endif
        default:
            return FALSE;
    }

    if(ConnectionTable[ConnectionIndex].status.bits.isValid){
        #ifdef ENABLE_INDIRECT_MESSAGE
            // check if RX on when idle
            //if((ConnectionTable[ConnectionIndex].PeerInfo[0] & 0x03) == 0x02)
            if(ConnectionTable[ConnectionIndex].status.bits.RXOnWhenIdle == 0){
                #if defined(IEEE_802_15_4)
                    return IndirectPacket(FALSE, myPANID, ConnectionTable[ConnectionIndex].Address,\
                                          FALSE, SecEn, mb);
                #else
                    // Define the additional step to assign the address to
                    // bypass compiler bug in C18 v3.38.
                    BYTE address[MY_ADDRESS_LENGTH];
                    BYTE i;

                    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                        address[i] = ConnectionTable[ConnectionIndex].Address[i];
                    }

                    //return IndirectPacket(FALSE, ConnectionTable[ConnectionIndex].Address,\
                    //                      FALSE, SecEn, mb);
                    return IndirectPacket(FALSE, address, FALSE, SecEn, mb);
                #endif
            }
        #endif

        #if defined(ENABLE_ENHANCED_DATA_REQUEST)
            if (pstatus->bits.Sleeping){
                pstatus->bits.Enhanced_DR_SecEn = SecEn;
                return TRUE;
            }
        #endif

        return MiApp_UnicastAddress(ConnectionTable[ConnectionIndex].Address, \
                                    TRUE, SecEn, mb);
    }
    return FALSE;
}

/*******************************************************************************
 * Function:    BOOL MiApp_UnicastAddress(BYTE *DestinationAddress, \
 *                                        BOOL PermanentAddr, BOOL SecEn)
 * Summary:     This function unicast a message in the TxBuffer to the device
 *              with DestinationAddress
 * Description: This is one of the primary user interface functions for the
 *              application layer to unicast a message. The destination device
 *              is specified by the input parameter DestinationAddress. The
 *              application payload is filled in the global char array TxBuffer.
 * PreCondition:Protocol initialization has been done.
 * Parameters:  BYTE *DestinationAddress - The destination address of the unicast
 *              BOOL PermanentAddr - The boolean to indicate if the destination
 *                                   address above is a permanent address or
 *                                   alternative network address. This parameter
 *                                   is only used in a network protocol.
 *              BOOL SecEn - The boolean indicates if the application payload
 *                           needs to be secured before transmission.
 * Returns:     A boolean to indicates if the unicast procedure is succcessful.
 * Example:
 *      <code>
 *      // Secure and then broadcast the message stored in TxBuffer to the
 *      // permanent address specified in the input parameter.
 *      MiApp_UnicastAddress(DestAddress, TRUE, TRUE);
 *      </code>
 * Remarks:     None
 ******************************************************************************/
BOOL MiApp_UnicastAddress(INPUT BYTE *DestinationAddress, INPUT BOOL PermanentAddr, \
                          INPUT BOOL SecEn, INPUT miwi_band mb){
    volatile P2P_STATUS *pstatus;
    switch (mb){
        case ISM_434:
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    #if defined MRF49XA_1_IN_434
                        pstatus = &MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_434
                        pstatus = &MRF49XA_2_P2PStatus;
                    #endif
                #endif
                break;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    #if defined MRF49XA_1_IN_868
                        pstatus = &MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_868
                        pstatus = &MRF49XA_2_P2PStatus;
                    #elif defined MRF89XA
                        pstatus = &MRF89XA_P2PStatus;
                    #endif
                #endif
                break;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return FALSE;
            #else
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    pstatus = &MRF24J40_P2PStatus;
                #endif
                break;
            #endif
        default:
            return FALSE;
    }

    //Juan: IMPORTANT! Modified protocol's behaviour.
    //Original: If the destination node (dest. node) is a sleeping device and
    //indirect messages are enabled, save the packet as an indirect packet. The
    //dest. node must request data within the indirect packet timeout limit for
    //receiving such packet. If indirect messages are disabled, send the packet
    //anyway.
    //LSI-CWSN stack. Try to send the packet first, independently of whether the
    //destination node is a sleeping device or not. 
    //If failure, then: if the sleeping device is in the connection table and
    //indirect messages are enabled, save the packet as an indirect message to
    //give the dest. node the chance of requesting for it.
    //If node was awake, transmission was reported as a failure and the dest.
    //node request for the packet it may receive a duplicate. At least it
    //probably received the packet if it was awake.
    BOOL ok;
    #if defined(IEEE_802_15_4)
        ok = SendPacket(FALSE, myPANID, DestinationAddress, FALSE, SecEn, mb);
    #else
        ok = SendPacket(FALSE, DestinationAddress, FALSE, SecEn, mb);
    #endif

    #ifdef ENABLE_INDIRECT_MESSAGE
    if (ok == FALSE){
        BYTE i;
        for(i = 0; i < CONNECTION_SIZE; i++){
            // check if RX on when idle
            if(ConnectionTable[i].status.bits.isValid && \
               (ConnectionTable[i].status.bits.RXOnWhenIdle == 0) && \
               isSameAddress(DestinationAddress, ConnectionTable[i].Address)) {
                #if defined(IEEE_802_15_4)
                    if(IndirectPacket(FALSE, myPANID, DestinationAddress, \
                                          FALSE, SecEn, mb))
                #else
                    if(IndirectPacket(FALSE, DestinationAddress, FALSE, SecEn, mb))
                #endif
                    {
                        Printf("\r\n\nIndirect packet saved.");
                    }
                    else{
                        Printf("\r\n\nIndirect packet couldn't be saved.");
                    }
            }
        }
    }
    #endif

    #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
        //Juan: Enhanced data request not adapted
        if (pstatus->bits.Sleeping){
            pstatus->bits.Enhanced_DR_SecEn = SecEn;
            return TRUE;
        }
    #endif

    return ok;
}

/*******************************************************************************
 * Function:    BOOL isSameAddress(BYTE *Address1, BYTE *Address2)
 * Overview:    This function compares two long addresses and returns the
 *              boolean to indicate if they are the same
 * PreCondition:
 * Input:       Address1 - Pointer to the first long address to be compared
 *              Address2 - Pointer to the second long address to be compared
 * Output:      If the two address are the same
 * Side Effects:
 ******************************************************************************/
BOOL isSameAddress(INPUT BYTE *Address1, INPUT BYTE *Address2){
    BYTE i;
    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
        if(Address1[i] != Address2[i]){
            return FALSE;
        }
    }
    return TRUE;
}

#if defined(ENABLE_HAND_SHAKE)
    BOOL MiApp_StartConnection(BYTE Mode, BYTE ScanDuration, DWORD ChannelMap, \
                               miwi_band mb){
        BYTE ChannelOffset;
        switch (mb){
            case ISM_434:
                #ifndef MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return FALSE;
                #else
                    ChannelOffset = MIWI0434ConfChannelOffset;
                    break;
                #endif
            case ISM_868:
                #ifndef MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return FALSE;
                #else
                    ChannelOffset = MIWI0868ConfChannelOffset;
                    break;
                #endif
            case ISM_2G4:
                #ifndef MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return FALSE;
                #else
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    break;
                #endif
            default:
                return FALSE;
        }

        switch (Mode){
            WORD tmp;
            BYTE channel;
            BYTE RSSIValue;
            case START_CONN_DIRECT:
                #if defined(IEEE_802_15_4)
                    #if MY_PAN_ID == 0xFFFF
                        myPANID.v[0] = TMRL;
                        myPANID.v[1] = TMRL+0x51;
                    #else
                        myPANID.Val = MY_PAN_ID;
                    #endif

                    #if defined MRF24J40
                    // Microchip LSI Stack - Only MRF24J40 supports Alt. Address
                    // as it is for IEEE 802.15.4 in 2,4 GHz band...
                    if (mb == ISM_2G4){
                        tmp = 0xFFFF;
                        MiMAC_SetAltAddress((BYTE *)&tmp, (BYTE *)&myPANID.Val);
                    }
                    #endif
                #endif
                #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && \
                    defined(ENABLE_INDIRECT_MESSAGE)
                    TimeSyncTick = MiWi_TickGet();
                #endif
                return TRUE;
            case START_CONN_ENERGY_SCN:
                #if defined(ENABLE_ED_SCAN)
                    #if defined(IEEE_802_15_4)
                        //if (mb == ISM_2G4){
                            //Microchip LSI Stack - Only MRF24J40 supports Alt.
                            //Address as it is for IEEE 802.15.4 in 2,4 GHz band
                            //Short addr in P2P must be 0xFFFF (broadcast)
                            //See doc. AN1204
                            #if MY_PAN_ID == 0xFFFF
                                myPANID.v[0] = TMRL;
                                myPANID.v[1] = TMRL+0x51;
                            #else
                                myPANID.Val = MY_PAN_ID;
                            #endif
                        if (mb == ISM_2G4){
                            tmp = 0xFFFF;
                            MiMAC_SetAltAddress((BYTE *)&tmp, (BYTE *)&myPANID.Val);
                        }
                    #endif
                    channel = MiApp_NoiseDetection(ChannelMap, ScanDuration, \
                                           NOISE_DETECT_ENERGY, &RSSIValue, mb);
                    MiApp_SetChannel(channel, mb);
                    Printf("\r\nStart Wireless Communication on Channel ");
                    PrintDec(channel - ChannelOffset);
                    Printf("\r\n");
                    #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && \
                        defined(ENABLE_INDIRECT_MESSAGE)
                        TimeSyncTick = MiWi_TickGet();
                    #endif
                    return TRUE;
                #else
                    return FALSE;
                #endif
            case START_CONN_CS_SCN:
                // Juan: Carrier sense scan is not supported for current
                // available Microchip transceivers...
                return FALSE;
            default:
                break;
        }
        return FALSE;
    }

    /***************************************************************************
     * Function:    BYTE MiApp_CoConnection(BYTE ActiveScanIndex,
     *                                             BYTE Mode)
     * Summary:     This function establish a connection with one or more nodes
     *              in an existing PAN.
     * Description: This is the primary user interface function for the
     *              application layer to start communication with an existing
     *              PAN. For P2P protocol, this function call can establish one
     *              or more connections. For network protocol, this function can
     *              be used to join the network, or establish a virtual socket
     *              connection with a node out of the radio range. There are
     *              multiple ways to establish connection(s), all depends on the
     *              input parameters.
     * PreCondition: Protocol initialization has been done. If only to establish
     *              connection with a predefined device, an active scan must be
     *              performed before and valid active scan result has been saved
     * Parameters:  BYTE ActiveScanIndex - The index of the target device in the
     *              ActiveScanResults array, if a predefined device is targeted.
     *              If the value of ActiveScanIndex is 0xFF, the protocol stack
     *              will try to establish a connection with any device.
     *              BYTE Mode - The mode to establish a connection. This
     *                          parameter is generally valid in a network
     *                          protocol. The possible modes are:
     *                  * CONN_MODE_DIRECT   Establish a connection without
     *                                       radio range.
     *                  * CONN_MODE_INDIRECT Establish a virtual connection with
     *                                       a device that may be in or out of
     *                                       the radio range. This mode
     *                                       sometimes is called cluster socket,
     *                                       which is only valid for network
     *                                       protocol. The PAN Coordinator will
     *                                       be involved to establish a virtual
     *                                       indirect socket connection.
     * Returns:     The index of the peer device on the connection table.
     * Example:
     *      <code>
     *      // Establish one or more connections with any device
     *      PeerIndex = MiApp_EstablishConnection(0xFF, CONN_MODE_DIRECT);
     *      </code>
     * Remarks:     If more than one connections have been established through
     *              this function call, the return value points to the index of
     *              one of the peer devices.
     **************************************************************************/
    BYTE MiApp_EstablishConnection(INPUT BYTE ActiveScanIndex, INPUT BYTE Mode,\
                                   INPUT miwi_band mb){
        BYTE *currentChannel, *ConnMode;
        volatile P2P_STATUS *status;
        P2P_CAPACITY *P2PCapacityInfo;
        switch (mb){
            case ISM_434:
                #ifndef MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return 0xFF;
                #else
                    #if defined MRF49XA_1_IN_434
                        status = & MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_434
                        status = & MRF49XA_2_P2PStatus;
                    #else
                        return 0xFF;    //Error.
                    #endif
                    P2PCapacityInfo = & MIWI0434CapacityInfo;
                    ConnMode = & MIWI0434_ConnMode;
                    currentChannel = & MIWI0434_currentChannel;
                    break;
                #endif
            case ISM_868:
                #ifndef MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return 0xFF;
                #else
                    #if defined MRF49XA_1_IN_868
                        status = & MRF49XA_1_P2PStatus;
                    #elif defined MRF49XA_2_IN_868
                        status = & MRF49XA_2_P2PStatus;
                    #elif defined MRF89XA
                        status = & MRF89XA_P2PStatus;
                    #else
                        return 0xFF     //Error
                    #endif
                    P2PCapacityInfo = & MIWI0868CapacityInfo;
                    ConnMode = & MIWI0868_ConnMode;
                    currentChannel = & MIWI0868_currentChannel;
                    break;
                #endif
            case ISM_2G4:
                #ifndef MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return 0xFF;
                #else
                    status = & MRF24J40_P2PStatus;
                    P2PCapacityInfo = & MIWI2400CapacityInfo;
                    ConnMode = & MIWI2400_ConnMode;
                    currentChannel = & MIWI2400_currentChannel;
                    break;
                #endif
            default:
                return 0xFF;     //Error
        }

        BYTE tmpConnectionMode = *ConnMode;
        BYTE retry = CONNECTION_RETRY_TIMES;
        BYTE connectionInterval = 0;
        MIWI_TICK t1, t2;
        int resultado;
        
        //P2P Protocol does not support indirect mode.
        if(Mode == CONN_MODE_INDIRECT){return 0xFF;}

        t1 = MiWi_TickGet();
        t1.Val -= (ONE_SECOND);
        *ConnMode = ENABLE_ALL_CONN;

        status->bits.SearchConnection = 1;
        while(status->bits.SearchConnection){
            t2 = MiWi_TickGet();
            if(MiWi_TickGetDiff(t2, t1) > (ONE_SECOND)) { 
                t1 = t2;
                if(connectionInterval-- > 0){
                    continue;
                }
                connectionInterval = CONNECTION_INTERVAL-1;
                if(retry-- == 0){
                    status->bits.SearchConnection = 0;
                    if(mb == ISM_2G4){
                        myPANID.Val = 0xFFFF;
                    }
                    return 0xFF;
                }

                //Juan: Looking for a peer. Sending a request with my P2P info
                //attached...
                MiApp_FlushTx(mb);
                MiApp_WriteData(CMD_P2P_CONNECTION_REQUEST, mb);
                MiApp_WriteData(*currentChannel, mb);
                MiApp_WriteData(P2PCapacityInfo->Val, mb);

                #if ADDITIONAL_NODE_ID_SIZE > 0
                    BYTE i;
                    for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                        MiApp_WriteData(AdditionalNodeID[i], mb);
                    }
                #endif

                //myPANID.Val = ActiveScanResults[ActiveScanIndex].PANID.Val;
                    
                #if defined(IEEE_802_15_4)
                    #if defined(ENABLE_ACTIVE_SCAN)
                        //Juan: If ActiveScanIndex is 0xFF, the protocol stack
                        //will try to establish a connection with any device.
                        if(ActiveScanIndex == 0xFF){
                            SendPacket(TRUE, myPANID, NULL, TRUE, FALSE, mb);
                        }
                        else{
                            //Juan: Joining the network found whose info is
                            //stored in ActiveScanIndex slot.
                            //Printf("\r\nconexion con: ");
                            //PrintDec(ActiveScanIndex);
                            resultado = MiApp_SetChannel(ActiveScanResults[ActiveScanIndex].Channel, mb);
                            resultado = SendPacket(FALSE, ActiveScanResults[ActiveScanIndex].PANID, ActiveScanResults[ActiveScanIndex].Address, TRUE, FALSE, mb);
                            //if(resultado) Printf("\r\n send ok");
                            //else Printf("\r\n send fail");
                        }
                    #else
                        SendPacket(TRUE, myPANID, NULL, TRUE, FALSE, mb);
                    #endif
                    //Juan: request has been sent.
                #else
                    #if defined(ENABLE_ACTIVE_SCAN)
                        if(ActiveScanIndex == 0xFF) {
                            SendPacket(TRUE, NULL, TRUE, FALSE, mb);
                        }
                        else {
                            MiApp_SetChannel(ActiveScanResults[ActiveScanIndex].Channel, mb);
                            SendPacket(FALSE, ActiveScanResults[ActiveScanIndex].Address, \
                                       TRUE, FALSE, mb);
                        }
                    #else
                        SendPacket(TRUE, NULL, TRUE, FALSE, mb);
                    #endif
                #endif

                if(MiApp_MessageAvailable(mb)){
                    //Juan: P2PTasks has been called for dealing with responses.
                    MiApp_DiscardMessage(mb);
                }
                //P2PTasks();
                *ConnMode = tmpConnectionMode;

                continue;
            }
        }
    }
#endif

void MiApp_DiscardMessage(miwi_band mb){
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_434
                    MRF49XA_1_P2PStatus.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(1);
                    return;
                #elif defined MRF49XA_2_IN_434
                    MRF49XA_2_P2PStatus.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(2);
                    return;
                #else
                    return;     //Error
                #endif
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_868
                    MRF49XA_1_P2PStatus.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(1);
                    return;
                #elif defined MRF49XA_2_IN_868
                    MRF49XA_2_P2PStatus.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(2);
                    return;
                #elif defined MRF89XA
                    MRF89XA_P2PStatus.bits.RxHasUserData = 0;
                    MiMAC_MRF89XA_DiscardPacket();
                    return;
                #else
                    return;     //Error
                #endif
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return;
            #else
                MRF24J40_P2PStatus.bits.RxHasUserData = 0;
                MiMAC_MRF24J40_DiscardPacket();
                return;
            #endif
        default:
            return;         //Error
    }
}

BOOL MiApp_SetChannel(BYTE channel, miwi_band mb){
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 1))
                #elif defined MRF49XA_2_IN_434
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 2))
                #else
                    return FALSE; //Error
                #endif
                {
                    MIWI0434_currentChannel = channel;
                    NodeStatus.MIWI0434_OpChannel = channel - MIWI0434ConfChannelOffset;
                    #if defined(ENABLE_NETWORK_FREEZER)
                        nvmPutCurrentChannel(&MIWI0434_currentChannel);
                    #endif
                    return TRUE;
                }
                return FALSE;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 1))
                #elif defined MRF49XA_2_IN_868
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 2))
                #elif defined MRF89XA
                    if(MiMAC_MRF89XA_SetChannel(channel, 0))
                #else
                    return FALSE; //Error
                #endif
                {
                    MIWI0868_currentChannel = channel;
                    NodeStatus.MIWI0868_OpChannel = channel - MIWI0868ConfChannelOffset;
                    #if defined(ENABLE_NETWORK_FREEZER)
                        nvmPutCurrentChannel(&MIWI0868_currentChannel);
                    #endif
                    return TRUE;
                }
                return FALSE;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return;
            #else
                if(MiMAC_MRF24J40_SetChannel(channel, 0)) {
                    MIWI2400_currentChannel = channel;
                    NodeStatus.MIWI2400_OpChannel = channel - MIWI2400ConfChannelOffset;
                    #if defined(ENABLE_NETWORK_FREEZER)
                        nvmPutCurrentChannel(&MIWI2400_currentChannel);
                    #endif
                    return TRUE;
                }
                return FALSE;
            #endif
        default:
            return FALSE;
    }
}

BYTE MiApp_MessageAvailable(miwi_band mb){
    P2PTasks();
    BYTE mbWithData = 0x00;
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return mbWithData;
            #else
                #if defined MRF49XA_1_IN_434
                    if (MRF49XA_1_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_434
                    if (MRF49XA_2_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #endif
                return mbWithData;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return mbWithData;
            #else
                #if defined MRF49XA_1_IN_868
                    if (MRF49XA_1_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_868
                    if (MRF49XA_2_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #endif
                return mbWithData;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return FALSE;
            #else
                if (MRF24J40_P2PStatus.bits.RxHasUserData){
                    mbWithData |= MIWI_2400_RI_MASK;
                }
                return mbWithData;
            #endif
        case ALL_ISM:
            #if defined MIWI_0434_RI
                #if defined MRF49XA_1_IN_434
                    if (MRF49XA_1_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_434
                    if (MRF49XA_2_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #endif
            #endif
            #if defined MIWI_0868_RI
                #if defined MRF49XA_1_IN_868
                    if (MRF49XA_1_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_868
                    if (MRF49XA_2_P2PStatus.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #endif
            #endif
            #if defined MIWI_2400_RI
                if (MRF24J40_P2PStatus.bits.RxHasUserData){
                    mbWithData |= MIWI_2400_RI_MASK;
                }
            #endif
            return mbWithData;
        default:
            return mbWithData;
    }
}

#ifdef ENABLE_DUMP
    /***************************************************************************
     * FUnction:    void DumpConnection(BYTE index)
     * Overview:    This function prints out the content of the connection with
     *              the input index of the P2P Connection Entry
     * PreCondition:
     * Input:       index - The index of the P2P Connection Entry to be printed
     *                      out
     * Output:      None
     *
     * Side Effects:The content of the connection pointed by the index of the
     *              P2P Connection Entry will be printed out
     **************************************************************************/
    void DumpConnection(INPUT BYTE index){
        BYTE i, j;
        if(index > CONNECTION_SIZE){
            Printf("\r\n\r\nMy Address: 0x");
            for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                PrintChar(myLongAddress[MY_ADDRESS_LENGTH-1-i]);
            }
            #if defined(IEEE_802_15_4)
                Printf("  PANID: 0x");
                PrintChar(myPANID.v[1]);
                PrintChar(myPANID.v[0]);
            #endif
            #if defined MIWI_0434_RI
                Printf("\r\nChannel MiWi at 434 MHz: ");
                PrintDec(MIWI0434_currentChannel - MIWI0434ConfChannelOffset);
            #endif
            #if defined MIWI_0868_RI
                Printf("\r\nChannel MiWi at 868 MHz: ");
                PrintDec(MIWI0868_currentChannel - MIWI0868ConfChannelOffset);
            #endif
            #if defined MIWI_2400_RI
                Printf("\r\nChannel MiWi at 2,4 GHz: ");
                PrintDec(MIWI2400_currentChannel - MIWI2400ConfChannelOffset);
            #endif
        }

        if(index < CONNECTION_SIZE){
            Printf("\r\nConnection     PeerLongAddress     434M 868M 2.4G     PeerInfo\r\n");
            if(ConnectionTable[index].status.bits.isValid){
                PrintChar(index);
                Printf("             ");
                for(i = 0; i < 8; i++){
                    if(i < MY_ADDRESS_LENGTH){
                        PrintChar(ConnectionTable[index].Address[MY_ADDRESS_LENGTH-1-i]);
                    }
                    else{
                        Printf("  ");
                    }
                }
                Printf("    ");
                for(i=0; i<3; i++){
                    if (ConnectionTable[index].MiWiInterfaces & (0x01 << i)){
                        // Juan: This part has been added. It begins checking
                        // MIWI_0434_RI flag and ends up checking MIWI_2400_RI.
                        Printf("YES  ");
                    }
                    else{
                        Printf("NO   ");
                    }
                }
                Printf("    ");
                #if ADDITIONAL_NODE_ID_SIZE > 0
                    for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                        PrintChar( ConnectionTable[index].PeerInfo[i] );
                    }
                #endif
                Printf("\r\n");
            }
        }
        else{
            Printf("\r\nConnection     PeerLongAddress     434M 868M 2.4G     PeerInfo\r\n");
            for(i = 0; i < CONNECTION_SIZE; i++){
                if(ConnectionTable[i].status.bits.isValid){
                    PrintChar(i);
                    Printf("             ");
                    for(j = 0; j < 8; j++){
                        if(j < MY_ADDRESS_LENGTH){
                            PrintChar(ConnectionTable[i].Address[MY_ADDRESS_LENGTH-1-j]);
                        }
                        else {
                            Printf("  ");
                        }
                    }
                    Printf("    ");
                    for(j=0; j<3; j++){
                        if (ConnectionTable[i].MiWiInterfaces & (0x01 << j)){
                            //Juan: This part has been added. It begins checking
                            //MIWI_0434_RI flag and end up checking MIWI_2400_RI.
                            Printf("YES  ");
                        }
                        else{
                            Printf("NO   ");
                        }
                    }
                    Printf("    ");
                    #if ADDITIONAL_NODE_ID_SIZE > 0
                        for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                            PrintChar( ConnectionTable[i].PeerInfo[j] );
                        }
                    #endif
                    Printf("\r\n");
                }
            }
        }
    }
#endif

#if defined(ENABLE_HAND_SHAKE)
    /***************************************************************************
     * Function:    BYTE AddConnection(void)
     * Overview:    This function create a new P2P connection entry
     * PreCondition:A P2P Connection Request or Response has been received and
     *              stored in MIWIxxxx_rxMessage structure
     * Input:       None
     * Output:      The index of the P2P Connection Entry for the newly added
     *              connection
     * Side Effects:A new P2P Connection Entry is created. The search connection
     *              operation ends if an entry is added successfully
     **************************************************************************/
    BYTE AddConnection(miwi_band mb) {
        RECEIVED_MESSAGE *rcvdmessage;
        volatile P2P_STATUS *P2Pstatus;
        BYTE ConnMode, RI_MASK;
        if(mb == ISM_434){
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return 0xFF;
            #else
                rcvdmessage = & MIWI0434_rxMessage;
                ConnMode = MIWI0434_ConnMode;
                RI_MASK = MIWI_0434_RI_MASK;
                #if defined MRF49XA_1_IN_434
                    P2Pstatus = & MRF49XA_1_P2PStatus;
                #elif defined MRF49XA_2_IN_434
                    P2Pstatus = & MRF49XA_2_P2PStatus;
                #endif
            #endif
        } else if(mb == ISM_868){
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return 0xFF;
            #else
                rcvdmessage = & MIWI0868_rxMessage;
                ConnMode = MIWI0868_ConnMode;
                RI_MASK = MIWI_0868_RI_MASK;
                #if defined MRF49XA_1_IN_868
                    P2Pstatus = & MRF49XA_1_P2PStatus;
                #elif defined MRF49XA_2_IN_868
                    P2Pstatus = & MRF49XA_2_P2PStatus;
                #endif
            #endif
        } else if(mb == ISM_2G4){
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return 0xFF;
            #else
                rcvdmessage = & MIWI2400_rxMessage;
                ConnMode = MIWI2400_ConnMode;
                RI_MASK = MIWI_2400_RI_MASK;
                P2Pstatus = & MRF24J40_P2PStatus;
            #endif
        }
        else {return 0xFF;}    //Error

        BYTE i;
        BYTE status = STATUS_SUCCESS;
        BYTE connectionSlot = 0xFF;

        // if no peerinfo attached, this is only an active scan request,
        // so do not save the source device's info
        #ifdef ENABLE_ACTIVE_SCAN
            if(rcvdmessage->PayloadSize <3){
                return STATUS_ACTIVE_SCAN;
            }
        #endif

        // loop through all entry and locate an proper slot
        for(i = 0; i < CONNECTION_SIZE; i++){
            // check if the entry is valid
            if(ConnectionTable[i].status.bits.isValid) {
                // check if the entry address matches source address of current received packet
                if(isSameAddress(rcvdmessage->SourceAddress, ConnectionTable[i].Address)){
                    //Juan: This part is new. Check if we know that this node
                    //has the current miwi_band or we can update the information
                    connectionSlot = i;
                    if(ConnectionTable[i].MiWiInterfaces & RI_MASK){
                        status = STATUS_EXISTS;
                    }
                    else{
                        ConnectionTable[i].MiWiInterfaces |= RI_MASK;
                        status = STATUS_UPDATED;
                    }
                    break;
                }
            }
            else if(connectionSlot == 0xFF){
                // store the first empty slot
                connectionSlot = i;
            }
        }

        if(connectionSlot == 0xFF){
            return STATUS_NOT_ENOUGH_SPACE;
        }
        else {
            if(ConnMode >= ENABLE_PREV_CONN){
                return status;
            }

            for(i = 0; i < 8; i++){
                ConnectionTable[connectionSlot].Address[i] = rcvdmessage->SourceAddress[i];
            }
            // store the capacity info and validate the entry
            ConnectionTable[connectionSlot].status.bits.isValid = 1;
            ConnectionTable[connectionSlot].status.bits.RXOnWhenIdle = (rcvdmessage->Payload[2] & 0x01);

            //Juan: added the line below for linking this slot with current RI.
            ConnectionTable[connectionSlot].MiWiInterfaces |= RI_MASK;

            // store possible additional connection payload
            #if ADDITIONAL_NODE_ID_SIZE > 0
                for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++) {
                    ConnectionTable[connectionSlot].PeerInfo[i] = rcvdmessage->Payload[3+i];
                }
            #endif
            #ifdef ENABLE_SECURITY
                // if security is enabled, clear the incoming frame control
                IncomingFrameCounter[connectionSlot].Val = 0;
            #endif
            LatestConnection = connectionSlot;

            P2Pstatus->bits.SearchConnection = 0;

            return status;
        }
    }
#endif

    s
    

#ifdef ENABLE_ACTIVE_SCAN
    /***************************************************************************
     * Function:    BYTE MiApp_SearchConnection(BYTE ScanDuartion,
     *                                          DWORD ChannelMap)
     * Summary:     This function perform an active scan to locate operating
     *              PANs in the neighborhood.
     * Description: This is the primary user interface function for the
     *              application layer to perform an active scan. After this
     *              function call, all active scan response will be stored in
     *              the global variable ActiveScanResults in the format of
     *              structure ACTIVE_SCAN_RESULT. The return value indicates the
     *              total number of valid active scan response in the active
     *              scan result array.
     * PreCondition: Protocol initialization has been done.
     * Parameters:  BYTE ScanDuration - The maximum time to perform scan on
     *                                  single channel. The value is from 5 to
     *                                  14. The real time to perform scan can be
     *                                  calculated in following formula from
     *                                  IEEE 802.15.4 specification
     *                                  960*(2^ScanDuration + 1)* 10^(-6) second
     *              DWORD ChannelMap - The bit map of channels to perform noise
     *                                 scan. The 32-bit double word parameter
     *                                 use one bit to represent corresponding
     *                                 channels from 0 to 31. For instance,
     *                                 0x00000003 represent to scan channel 0
     *                                 and channel 1.
     * Returns:     The number of valid active scan response stored in the
     *              global variable ActiveScanResults.
     * Example:
     *      <code>
     *      // Perform an active scan on all possible channels
     *      NumOfActiveScanResponse = MiApp_SearchConnection(10, 0xFFFFFFFF);
     *      </code>
     * Remarks:     None
     **************************************************************************/
    BYTE MiApp_SearchConnection(INPUT BYTE ScanDuration, INPUT DWORD ChannelMap,\
                                INPUT miwi_band mb){
        BYTE backupChannel, ChannelOffset;
        DWORD FullChannelMap;
        BYTE *currentChannel, test;
        ActiveScanResultIndex = 0;      //Juan: PANs found... by now 0.
        if(mb == ISM_434){
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return ActiveScanResultIndex;
            #else
                #if defined MRF49XA_1_IN_434
                    FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                #elif defined MRF49XA_2_IN_434
                    FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                #else
                    return ActiveScanResultIndex; //Error
                #endif
                ChannelOffset = MIWI0434ConfChannelOffset;
                backupChannel = MIWI0434_currentChannel;
                currentChannel = & MIWI0434_currentChannel;
            #endif
        } else if(mb == ISM_868){
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return ActiveScanResultIndex;
            #else
                #if defined MRF49XA_1_IN_868
                    FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                #elif defined MRF49XA_2_IN_868
                    FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                #elif defined MRF89XA
                    FullChannelMap = MRF89XA_FULL_CHANNEL_MAP;
                #else
                    return ActiveScanResultIndex; //Error
                #endif
                ChannelOffset = MIWI0868ConfChannelOffset;
                backupChannel = MIWI0868_currentChannel;
                currentChannel = & MIWI0868_currentChannel;
            #endif
        } else if(mb == ISM_2G4){
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return ActiveScanResultIndex;
            #else
                ChannelOffset = MIWI2400ConfChannelOffset;
                backupChannel = MIWI2400_currentChannel;
                FullChannelMap = MRF24J40_FULL_CHANNEL_MAP;
                currentChannel = & MIWI2400_currentChannel;
            #endif
        }
        else {return ActiveScanResultIndex;}    //Error. Search aborted.

        BYTE i;
        DWORD channelMask = 0x00000001;

        MIWI_TICK t1, t2;

        for(i = 0; i < ACTIVE_SCAN_RESULT_SIZE; i++){
            //Juan: Node can store info about a number of [ACTIVE_SCAN_RESULTS_SIZE] PANs
            ActiveScanResults[i].Channel = 0xFF;
            //Juan: By now, operating channels are set to invalid.
        }

        i = 0;
        //Juan: Now the search for PANs begins...
        while(i < 32){
            if(ChannelMap & FullChannelMap & (channelMask << i)){
                #if defined(IEEE_802_15_4)
                    WORD_VAL tmpPANID;
                #endif

                Printf("\r\nScan Channel ");
                PrintDec(i - ChannelOffset);
                /* choose appropriate channel */
                MiApp_SetChannel(i, mb);

                MiApp_FlushTx(mb);
                MiApp_WriteData(CMD_P2P_ACTIVE_SCAN_REQUEST, mb);
                MiApp_WriteData(*currentChannel, mb);

                #if defined(IEEE_802_15_4)
                    tmpPANID.Val = 0xFFFF;
                    //Juan: destination PANID will be ignored for subGHz bands.
                   test = SendPacket(TRUE, tmpPANID, NULL, TRUE, FALSE, mb);
                   //if (test) Printf("\r\n\nOK");
                   if (!(test)) Printf("\r\n\nFAIL");
                #else
                    SendPacket(TRUE, NULL, TRUE, FALSE, mb);
                #endif

                t1 = MiWi_TickGet();
                while(1){
                    if(MiApp_MessageAvailable(mb)){
                        //Juan: P2PTasks has been called and commands received
                        //have been processed... ActiveScanResults may have been
                        //updated!
                        MiApp_DiscardMessage(mb);
                    }
                    //P2PTasks();
                    t2 = MiWi_TickGet();
                    if(MiWi_TickGetDiff(t2, t1) > ((DWORD)(ScanTime[ScanDuration]))){
                        // if scan time exceed scan duration, prepare to scan the next channel
                        break;
                    }
                }

            }
            i++;
        }
        MiApp_SetChannel(backupChannel, mb);
        return ActiveScanResultIndex;
    }
#endif

#ifdef ENABLE_ED_SCAN
     /**************************************************************************
     * Function:    BYTE MiApp_NoiseDetection (DWORD ChannelMap,
     *                  BYTE ScanDuration, BYTE DetectionMode, BYTE *NoiseLevel)
     * Summary:     This function perform a noise scan and returns the channel
     *              with least noise
     * Description: This is the primary user interface functions for the
     *              application layer to perform noise detection on multiple
     *              channels.
     * PreCondition:Protocol initialization has been done.
     * Parameters:  DWORD ChannelMap - The bit map of channels to perform noise
     *                                 scan. The 32-bit double word parameter
     *                                 use one bit to represent corresponding
     *                                 channels from 0 to 31. For instance,
     *                                 0x00000003 represent to scan channel 0
     *                                 and channel 1.
     *              BYTE ScanDuration - The maximum time to perform scan on
     *                                  single channel. The value is from 5 to
     *                                  14. The real time to perform scan can
     *                                  be calculated in following formula from
     *                                  IEEE 802.15.4 specification
     *                                  960*(2^ScanDuration + 1)* 10^(-6) second
     *              BYTE DetectionMode - The noise detection mode to perform the
     *                                   scan. The two possible scan modes are:
     *                  * NOISE_DETECT_ENERGY Energy detection scan mode
     *                  * NOISE_DETECT_CS     Carrier sense detection scan mode
     *              BYTE *NoiseLevel - The noise level at the channel with least
     *                                 noise level
     * Returns:     The channel that has the lowest noise level
     * Example:
     *      <code>
     *      BYTE NoiseLevel;
     *      OptimalChannel = MiApp_NoiseDetection(0xFFFFFFFF, 10,
     *                                        NOISE_DETECT_ENERGY, &NoiseLevel);
     *      </code>
     * Remarks:     None
     **************************************************************************/
    BYTE MiApp_NoiseDetection (INPUT DWORD ChannelMap, INPUT BYTE ScanDuration,\
          INPUT BYTE DetectionMode, OUTPUT BYTE *RSSIValue, INPUT miwi_band mb){

        if(DetectionMode != NOISE_DETECT_ENERGY){
            return 0xFF;
        }
        //Carrier sense detection not supported by MIWI STACK...

        BYTE i, OptimalChannel, ChannelOffset, transceiver;
        MIWI_TICK t1, t2;
        DWORD FullChannelMap;

        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return 0xFF;
                #else
                    #if defined MRF49XA_1_IN_434
                        FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        transceiver = 1;
                        break;
                    #elif defined MRF49XA_2_IN_434
                        FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        transceiver = 2;
                        break;
                    #else
                        return 0xFF;        //Error
                    #endif
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return 0xFF;
                #else
                    #if defined MRF49XA_1_IN_868
                        FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        transceiver = 1;
                        break;
                    #elif defined MRF49XA_2_IN_868
                        FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        transceiver = 2;
                        break;
                    #elif defined MRF89XA
                        FullhannelMap = MRF89XA_FULL_CHANNEL_MAP;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        transceiver = 4;
                        break;
                    #else
                        return 0xFF;        //Error
                    #endif
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return 0xFF;
                #else
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    FullChannelMap = MRF24J40_FULL_CHANNEL_MAP;
                    transceiver = 3;
                    break;
                #endif
            default:
                return 0xFF;
        }
        BYTE minRSSI = 0xFF;
        DWORD channelMask = 0x00000001;
//        ConsolePutROMString((ROM char*)"\r\nEnergy Scan Results:");
        i = 0;
        while (i < 32) {
            if(ChannelMap & FullChannelMap & (channelMask << i)) {
                BYTE RSSIcheck;
                BYTE maxRSSI = 0;
                BYTE j, k;

                /* choose appropriate channel */
                MiApp_SetChannel(i, mb);

                t1 = MiWi_TickGet();

                while(1) {
                    #if defined MRF49XA_1
                    if(transceiver == 1){
                        RSSIcheck = MiMAC_MRF49XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, 1);
                    }else
                    #endif
                    #if defined MRF49XA_2
                    if(transceiver == 2){
                        RSSIcheck = MiMAC_MRF49XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, 2);
                    }else
                    #endif
                    #if defined MRF24J40
                    if(transceiver == 3){
                        RSSIcheck = MiMAC_MRF24J40_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT);
                    }else
                    #endif
                    #if defined MRF89XA
                    if(transceiver == 4){
                        RSSIcheck = MiMAC_MRF89XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT);
                    }else
                    #endif
                        {return 0xFF;}  //Else transceivers. Error. Abort.

                    if(RSSIcheck > maxRSSI){
                        maxRSSI = RSSIcheck;
                    }

                    t2 = MiWi_TickGet();
                    if( MiWi_TickGetDiff(t2, t1) > ((DWORD)(ScanTime[ScanDuration])) ) {
                        // if scan time exceed scan duration, prepare to scan the next channel
                        break;
                    }
                }

//                Printf("\r\nChannel ");
//                PrintDec(i-ChannelOffset);
//                Printf(": ");
//                j = maxRSSI/5;
//                for(k = 0; k < j; k++){
//                    ConsolePut('-');
//                }
//                Printf(" ");
//                PrintChar(maxRSSI);

                //Refresh scan variables in NodeStatus
                if(mb == ISM_434){
                    #if defined MIWI_0434_RI
                        NodeStatus.scanMIWI0434[i-ChannelOffset]= maxRSSI;
                    #endif
                } else if(mb == ISM_868){
                    #if defined MIWI_0868_RI
                        NodeStatus.scanMIWI0868[i-ChannelOffset] = maxRSSI;
                    #endif
                } else if(mb == ISM_2G4){
                    #if defined MIWI_2400_RI
                        NodeStatus.scanMIWI2400[i-ChannelOffset] = maxRSSI;
                    #endif
                }

                if(maxRSSI < minRSSI){
                    minRSSI = maxRSSI;
                    OptimalChannel = i;
                    if(RSSIValue){
                        *RSSIValue = minRSSI;
                    }
                }
            }
            i++;
        }
        return OptimalChannel;
    }
#endif


#ifdef ENABLE_FREQUENCY_AGILITY
    /***************************************************************************
     * Function:    void StartChannelHopping(BYTE OptimalChannel)
     * Overview:    This function broadcast the channel hopping command and
     *              after that, change operating channel to the input optimal
     *              channel.
     * PreCondition: Transceiver has been initialized
     * Input:       OptimalChannel - The channel to hop to
     * Output:      None
     * Side Effects: The operating channel for current device will change to the
     *              specified channel
     **************************************************************************/
    void StartChannelHopping(INPUT BYTE OptimalChannel, miwi_band mb){

        BYTE *currentChannel;
        switch (mb){
            case ISM_434:
                #ifndef MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return;
                #else
                    currentChannel = & MIWI0434_currentChannel;
                    break;
                #endif
            case ISM_868:
                #ifndef MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return;
                #else
                    currentChannel = & MIWI0868_currentChannel;
                    break;
                #endif
            case ISM_2G4:
                #ifndef MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return;
                #else
                    currentChannel = & MIWI2400_currentChannel;
                    break;
                #endif
            default:
                return;     //Error
        }

        BYTE i;
        MIWI_TICK t1, t2;

        for(i = 0; i < FA_BROADCAST_TIME; i++){
            t1 = MiWi_TickGet();
            while(1){
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > SCAN_DURATION_9){
                    MiApp_FlushTx(mb);
                    MiApp_WriteData(CMD_CHANNEL_HOPPING, mb);
                    MiApp_WriteData(*currentChannel, mb);
                    MiApp_WriteData(OptimalChannel, mb);
                    #if defined(IEEE_802_15_4)
                        SendPacket(TRUE, myPANID, NULL, TRUE, FALSE, mb);
                    #else
                        SendPacket(TRUE, NULL, TRUE, FALSE, mb);
                    #endif
                    break;
                }
            }
        }
        MiApp_SetChannel(OptimalChannel, mb);
    }


    /***************************************************************************
     * Function:    MiApp_ResyncConnection(BYTE ConnectionIndex,
     *                                     DWORD ChannelMap, miwi_band mb)
     * Summary:     This function tries to resynchronize the lost connection
     *              with peers, probably due to channel hopping
     * Description: This is the primary user interface function for the
     *              application to resynchronize a lost connection. For a RFD
     *              device that goes to sleep periodically, it may not receive
     *              the channel hopping command that is sent when it is sleep.
     *              The sleeping RFD device depends on this function to hop to
     *              the channel that the rest of the PAN has jumped to. This
     *              function call is usually triggered by continously
     *              communication failure with the peers.
     * PreCondition:Transceiver has been initialized
     * Parameters:  DWORD ChannelMap - The bit map of channels to perform noise
     *                                 scan. The 32-bit double word parameter
     *                                 use one bit to represent corresponding
     *                                 channels from 0 to 31. For instance,
     *                                 0x00000003 represent to scan channel 0
     *                                 and channel 1.
     * Returns:     a boolean to indicate if resynchronization of connection is
     *              successful
     * Example:
     *      <code>
     *      // Sleeping RFD device resync with its associated device, usually
     *      // the first peer in the connection table
     *      MiApp_ResyncConnection(0, 0xFFFFFFFF);
     *      </code>
     * Remark:      If operation is successful, the wireless node will be hopped
     *              to the channel that the rest of the PAN is operating on.
     **************************************************************************/

    BOOL MiApp_ResyncConnection(INPUT BYTE ConnectionIndex, \
                                INPUT DWORD ChannelMap, INPUT miwi_band mb){
        DWORD FullChannelMap;
        BYTE backupChannel;
        BYTE *currentChannel;
        volatile P2P_STATUS *status;
        BOOL aux;
        BYTE ChannelOffset, NOT_RI_MASK;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_434
                        FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                        status = & MRF49XA_1_P2PStatus;
                        MRF49XA_1_P2PStatus.bits.Resync = 1;
                    #elif defined MRF49XA_2_IN_434
                        FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                        status = & MRF49XA_2_P2PStatus;
                        MRF49XA_2_P2PStatus.bits.Resync = 1;
                    #else
                        return FALSE;   //Error
                    #endif
                    backupChannel = MIWI0434_currentChannel;
                    currentChannel = & MIWI0434_currentChannel;
                    ChannelOffset = MIWI0434ConfChannelOffset;
                    NOT_RI_MASK = ~(MIWI_0434_RI_MASK);
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_868
                        FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                        status = & MRF49XA_1_P2PStatus;
                        MRF49XA_1_P2PStatus.bits.Resync = 1;
                    #elif defined MRF49XA_2_IN_868
                        FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                        status = & MRF49XA_2_P2PStatus;
                        MRF49XA_2_P2PStatus.bits.Resync = 1;
                    #elif defined MRF89XA
                        FullhannelMap = MRF89XA_FULL_CHANNEL_MAP;
                        status = & MRF89XA_P2PStatus;
                        MRF89XA_P2PStatus.bits.Resync = 1;
                    #else
                        return FALSE;   //Error
                    #endif
                    backupChannel = MIWI0868_currentChannel;
                    currentChannel = & MIWI0868_currentChannel;
                    ChannelOffset = MIWI0868ConfChannelOffset;
                    NOT_RI_MASK = ~(MIWI_0868_RI_MASK);
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return FALSE;
                #else
                    FullChannelMap = MRF24J40_FULL_CHANNEL_MAP;
                    backupChannel = MIWI2400_currentChannel;
                    status =  & MRF24J40_P2PStatus;
                    currentChannel = & MIWI2400_currentChannel;
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    NOT_RI_MASK = ~(MIWI_2400_RI_MASK);
                    MRF24J40_P2PStatus.bits.Resync = 1;
                    break;
                #endif
            default:
                return FALSE;
        }
        BYTE i;
        BYTE j;
        MIWI_TICK t1, t2;

        t1 = MiWi_TickGet();
        for(i = 0; i < RESYNC_TIMES; i++){
            DWORD ChannelMask = 0x00000001;
            j = 0;

            while(status->bits.Resync){
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > SCAN_DURATION_9){
                    t1.Val = t2.Val;

                    if(j > 31){
                        break;
                    }
                    while((ChannelMap & FullChannelMap & (ChannelMask << j)) == 0){
                        if(++j > 31){
                            goto GetOutOfLoop;
                        }
                    }

                    Printf("\r\nChecking Channel ");
                    PrintDec(j - ChannelOffset);
                    MiApp_SetChannel(j, mb);
                    j++;

                    MiApp_FlushTx(mb);
                    MiApp_WriteData(CMD_P2P_ACTIVE_SCAN_REQUEST, mb);
                    MiApp_WriteData(*currentChannel, mb);

                    #if defined(IEEE_802_15_4)
                        SendPacket(FALSE, myPANID, ConnectionTable[ConnectionIndex].Address, \
                                   TRUE, FALSE, mb);
                    #else
                        SendPacket(FALSE, ConnectionTable[ConnectionIndex].Address, \
                                   TRUE, FALSE, mb);
                    #endif
                }
                if(MiApp_MessageAvailable(mb)){
                    MiApp_DiscardMessage(mb);
                }
                //P2PTasks();
            }

            if(status->bits.Resync == 0){
                Printf("\r\nResynchronized Connection to Channel ");
                PrintDec(*currentChannel - ChannelOffset);
                Printf("\r\n");
                return TRUE;
            }
GetOutOfLoop:
            MacroNop();
        }
        //Juan: resync failure. Additionally, if the connection slot had the
        //current miwi interface linked and resync have failed, erase it from
        //the MiWiInterfaces field.
        MiApp_SetChannel(backupChannel, mb);
        ConnectionTable[ConnectionIndex].MiWiInterfaces &= NOT_RI_MASK; //Juan: Added
        if(ConnectionTable[ConnectionIndex].MiWiInterfaces == 0){       //Juan: Added
            ConnectionTable[ConnectionIndex].status.Val = 0;            //Juan: Added
        }                                                               //Juan: Added
        status->bits.Resync = 0;
        return FALSE;
    }

    #ifdef FREQUENCY_AGILITY_STARTER
        /***********************************************************************
         * Function:    BOOL MiApp_InitChannelHopping(DWORD ChannelMap)
         * Summary:     This function tries to start a channel hopping
         *              (frequency agility) procedure
         * Description: This is the primary user interface function for the
         *              application to do energy scan to locate the channel with
         *              least noise. If the channel is not current operating
         *              channel, process of channel hopping will be started.
         * PreCondition: Transceiver has been initialized
         * Parameters:  DWORD ChannelMap - The bit map of the candicate channels
         *                                 which can be hopped to
         * Returns:     a boolean to indicate if channel hopping is initiated
         * Example:
         *      <code>
         *      // if condition meets, scan all possible channels and hop
         *      // to the one with least noise
         *      MiApp_InitChannelHopping(0xFFFFFFFF);
         *      </code>
         * Remark:      The operating channel will change to the optimal channel
         *              with least noise
         **********************************************************************/
        BOOL MiApp_InitChannelHopping (INPUT DWORD ChannelMap, miwi_band mb){
            BYTE backupChannel, backupConnMode, ChannelOffset;
            switch (mb){
                case ISM_434:
                    #ifndef MIWI_0434_RI
                        Printf("Error: MiWi ISM 434 MHz band is not available.");
                        return FALSE;
                    #else
                        backupChannel = MIWI0434_currentChannel;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        backupConnMode = MIWI0434_ConnMode;
                        break;
                    #endif
                case ISM_868:
                    #ifndef MIWI_0868_RI
                        Printf("Error: MiWi ISM 868 MHz band is not available.");
                        return FALSE;
                    #else
                        backupChannel = MIWI0868_currentChannel;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        backupConnMode = MIWI0868_ConnMode;
                        break;
                    #endif
                case ISM_2G4:
                    #ifndef MIWI_2400_RI
                        Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                        return FALSE;
                    #else
                        backupChannel = MIWI2400_currentChannel;
                        ChannelOffset = MIWI2400ConfChannelOffset;
                        backupConnMode = MIWI2400_ConnMode;
                        break;
                    #endif
                default:
                    return FALSE;   //Error
            }

            BYTE RSSIValue;
            BYTE optimalChannel;

            MiApp_ConnectionMode(DISABLE_ALL_CONN, mb);
            optimalChannel = MiApp_NoiseDetection(ChannelMap, 10,
                                           NOISE_DETECT_ENERGY, &RSSIValue, mb);
            MiApp_ConnectionMode(backupConnMode, mb);
            MiApp_SetChannel(backupChannel, mb);
            if(optimalChannel == backupChannel){
                return FALSE;
            }

            Printf("\r\nHopping to Channel ");
            PrintDec(optimalChannel - ChannelOffset);
            Printf("\r\n");
            StartChannelHopping(optimalChannel, mb);
            return TRUE;
        }
    #endif
#endif

#if !defined(TARGET_SMALL)
    /***************************************************************************
     * Function:    void MiApp_RemoveConnection(BYTE ConnectionIndex)
     * Summary:     This function remove connection(s) in connection table
     * Description: This is the primary user interface function to disconnect
     *              connection(s). For a P2P protocol, it simply remove the
     *              connection. For a network protocol, if the device referred
     *              by the input parameter is the parent of the device calling
     *              this function, the calling device will get out of network
     *              along with its children. If the device referred by the input
     *              parameter is children of the device calling this function,
     *              the target device will get out of network.
     * PreCondition: Transceiver has been initialized. Node has establish one or
     *              more connections
     * Parameters:  BYTE ConnectionIndex - The index of the connection in the
     *                                     connection table to be removed
     * Returns:     None
     * Example:
     *      <code>
     *      MiApp_RemoveConnection(0x00);
     *      </code>
     * Remarks:     None
     **************************************************************************/
    void MiApp_RemoveConnection(INPUT BYTE ConnectionIndex, miwi_band mb){
        BYTE NOT_RI_MASK;
        switch (mb){
            case ISM_434:
                #ifndef MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return;
                #endif
                NOT_RI_MASK = ~(MIWI_0434_RI_MASK);
                break;
            case ISM_868:
                #ifndef MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return;
                #endif
                NOT_RI_MASK = ~(MIWI_0868_RI_MASK);
                break;
            case ISM_2G4:
                #ifndef MIWI_2400_RI
                    Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                    return;
                #endif
                NOT_RI_MASK = ~(MIWI_2400_RI_MASK);
                break;
            default:
                return;   //Error
        }
        //Juan: it seems that if this function is invoked with FF, it tries to
        //remove every connection.
        if(ConnectionIndex == 0xFF) {
            BYTE i;
            for(i = 0; i < CONNECTION_SIZE; i++) {
                WORD j;
                if(ConnectionTable[i].status.bits.isValid) {
                    MiApp_FlushTx(mb);
                    MiApp_WriteData(CMD_P2P_CONNECTION_REMOVAL_REQUEST, mb);
                    #if defined(IEEE_802_15_4)
                        SendPacket(FALSE, myPANID, ConnectionTable[i].Address, \
                                   TRUE, FALSE, mb);
                    #else
                        SendPacket(FALSE, ConnectionTable[i].Address, TRUE, FALSE, mb);
                    #endif
                    for(j = 0; j < 0xFFF; j++) {}   // delay
                }
                //Juan: added MiWiInterfaces assignment and "if 0". If it's the
                //only MIWi Interface using this connection then mark it as
                //invalid (for reusing its slot).
                ConnectionTable[i].MiWiInterfaces &= NOT_RI_MASK;
                if (ConnectionTable[i].MiWiInterfaces == 0){
                    ConnectionTable[i].status.Val = 0;
                    #if defined(ENABLE_NETWORK_FREEZER)
                        nvmPutConnectionTableIndex(&(ConnectionTable[i]), i);
                    #endif
                }
            }
        }
        else if(ConnectionTable[ConnectionIndex].status.bits.isValid){
            WORD j;

            MiApp_FlushTx(mb);
            MiApp_WriteData(CMD_P2P_CONNECTION_REMOVAL_REQUEST, mb);
            #if defined(IEEE_802_15_4)
                SendPacket(FALSE, myPANID, ConnectionTable[ConnectionIndex].Address, \
                           TRUE, FALSE, mb);
            #else
                SendPacket(FALSE, ConnectionTable[ConnectionIndex].Address,
                           TRUE, FALSE, mb);
            #endif
            for(j = 0; j < 0xFFF; j++) {}   // delay

            //Juan: added MiWiInterfaces assignment and "if 0". If it's the only
            //MIWi Interface using this connection then mark it as invalid (for
            //reusing its slot).
            ConnectionTable[ConnectionIndex].MiWiInterfaces &= NOT_RI_MASK;
            if (ConnectionTable[ConnectionIndex].MiWiInterfaces == 0){
                ConnectionTable[ConnectionIndex].status.Val = 0;
                #if defined(ENABLE_NETWORK_FREEZER)
                    nvmPutConnectionTableIndex(&(ConnectionTable[ConnectionIndex]),\
                                               ConnectionIndex);
                #endif
            }
        }
    }
#endif

/*******************************************************************************
 * Function:    void MiApp_ConnectionMode(BYTE Mode)
 * Summary:     This function set the current connection mode.
 * Description: This is the primary user interface function for the application
 *              layer to configure the way that the host device accept
 *              connection request.
 * PreCondition: Protocol initialization has been done.
 * Parameters:  BYTE Mode - The mode to accept connection request. The privilege
 *                          for those modes decreases gradually as defined. The
 *                          higher privilege mode has all the rights of the
 *                          lower privilege modes. The possible modes are
 *                  * ENABLE_ALL_CONN  Enable response to all connection request
 *                  * ENABLE_PREV_CONN Enable response to connection request
 *                                     from device already in the connection
 *                                     table.
 *                  * ENABLE_ACTIVE_SCAN_RSP Enable response to active scan only
 *                  * DISABLE_ALL_CONN Disable response to connection request,
 *                                     including an acitve scan request.
 * Returns:     None
 * Example:
 *      <code>
 *      // Enable all connection request
 *      MiApp_ConnectionMode(ENABLE_ALL_CONN);
 *      </code>
 * Remarks:     None
 ******************************************************************************/
void MiApp_ConnectionMode(INPUT BYTE Mode, miwi_band mb){
    if(Mode > 3){
        return;
    }
    switch (mb){
        case ISM_434:
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return;
            #else
                MIWI0434_ConnMode = Mode;
                MIWI0434CapacityInfo.Val = (MIWI0434CapacityInfo.Val & 0x0F) | (Mode << 4);
                break;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return;
            #else
                MIWI0868_ConnMode = Mode;
                MIWI0868CapacityInfo.Val = (MIWI0868CapacityInfo.Val & 0x0F) | (Mode << 4);
                break;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return;
            #else
                MIWI2400_ConnMode = Mode;
                MIWI2400CapacityInfo.Val = (MIWI2400CapacityInfo.Val & 0x0F) | (Mode << 4);
                break;
            #endif
        default:
            return;   //Error
    }

    #if defined(ENABLE_NETWORK_FREEZER)
        nvmPutConnMode(&ConnMode);      //Adapt functions... Then save new mode
    #endif
}

void MiApp_FlushTx(miwi_band mb){
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_434
                    MRF49XA_1_TxData = PAYLOAD_START;
                #elif defined MRF49XA_2_IN_434
                    MRF49XA_2_TxData = PAYLOAD_START;
                #else
                    //Error
                #endif
                return;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_868
                    MRF49XA_1_TxData = PAYLOAD_START;
                #elif defined MRF49XA_2_IN_868
                    MRF49XA_2_TxData = PAYLOAD_START;
                #elif defined MRF89XA
                    MRF89XA_TxData = PAYLOAD_START;
                #else
                    //Error
                #endif
                return;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return;
            #else
                MRF24J40_TxData = PAYLOAD_START;
                return;
            #endif
        default:
            return;
    }
}


void MiApp_WriteData(BYTE data, miwi_band mb){
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_434
                    MRF49XA_1_TxBuffer[MRF49XA_1_TxData++] = data;
                #elif defined MRF49XA_2_IN_434
                    MRF49XA_2_TxBuffer[MRF49XA_2_TxData++] = data;
                #else
                    //Error
                #endif
                return;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_868
                    MRF49XA_1_TxBuffer[MRF49XA_1_TxData++] = data;
                #elif defined MRF49XA_2_IN_868
                    MRF49XA_2_TxBuffer[MRF49XA_2_TxData++] = data;
                #elif defined MRF89XA
                    MRF89XA_TxBuffer[MRF89XA_TxData++] = data;
                #else
                    //Error
                #endif
                return;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return;
            #else
                MRF24J40_TxBuffer[MRF24J40_TxData++] = data;
                return;
            #endif
        default:
            return;
    }
}

// LSI-CWSN new functions
static BOOL AuxMAC_SendPacket(MAC_TRANS_PARAM *MTP, BYTE transceiver){
    BOOL ok;
    switch (transceiver){
        case 1:
            #if defined MRF49XA_1
                ok = MiMAC_MRF49XA_SendPacket(*MTP, MRF49XA_1_TxBuffer, MRF49XA_1_TxData, 1);
                return ok;
            #else
                return FALSE;   // ERROR here! transceiver has an invalid value
            #endif
        case 2:
            #if defined MRF49XA_2
                ok = MiMAC_MRF49XA_SendPacket(*MTP, MRF49XA_2_TxBuffer, MRF49XA_2_TxData, 2);
                return ok;
            #else
                return FALSE;   // ERROR here! transceiver has an invalid value
            #endif
        case 3:
            #if defined MRF24J40
                ok = MiMAC_MRF24J40_SendPacket(*MTP, MRF24J40_TxBuffer, MRF24J40_TxData);
                return ok;
            #else
                return FALSE;   // ERROR here! transceiver has an invalid value
            #endif
        default:
            return FALSE;       //Wrong transceiver!
    }
}

static void AuxMAC_DiscardPacket(BYTE transceiver){
    switch (transceiver){
        case 1:
            #if defined MRF49XA_1
                MiMAC_MRF49XA_DiscardPacket(1);
            #endif
            return;
        case 2:
            #if defined MRF49XA_2
                MiMAC_MRF49XA_DiscardPacket(2);
            #endif
            return;
        case 3:
            #if defined MRF24J40
                MiMAC_MRF24J40_DiscardPacket();
            #endif
            return;
        default:
            return;       //Wrong transceiver!
    }
}



#else  // defined PROTOCOL_P2P
    /*******************************************************************
     * C18 compiler cannot compile an empty C file. define following
     * bogus variable to bypass the limitation of the C18 compiler if
     * a different protocol is chosen.
     ******************************************************************/
    extern char bogusVariable;
#endif