/*******************************************************************************
* FileName:     MiWi.c                        ¡¡¡MODIFIED FROM ORIGINAL STACK!!!
* Dependencies:    
* Processor:	PIC18, PIC24, PIC32, dsPIC30, dsPIC33
*               tested with 18F4620, dsPIC33FJ256GP710	
* Complier:     Microchip C18 v3.11 or higher
*				Microchip C30 v2.03 or higher
*               Microchip C32 v1.02 or higher		
* Company:		Microchip Technology, Inc.
*
* Copyright and Disclaimer Notice for MiWi Software:
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
********************************************************************************
* File Description:
*
* Change History:
*  Rev   Date         Author	Description
*  0.1   11/09/2006   df        Initial revision
*  1.0   01/09/2007   yfy       Initial release
*  3.1   5/28/2010    yfy       MiWi DE 3.1
*  4.1   6/3/2011     yfy       MAL v2011-06
*******************************************************************************/

/******************************* HEADERS **************************************/

#include "WirelessProtocols/ConfigApp.h"
#include "NodeHAL.h"

#if defined(PROTOCOL_MIWI)
    #include "WirelessProtocols/MSPI.h"
    #include "WirelessProtocols/MiWi/MiWi.h"
    #include "Compiler.h"
    #include "GenericTypeDefs.h"
    #include "WirelessProtocols/Console.h"
    #include "WirelessProtocols/NVM.h"
    #include "WirelessProtocols/SymbolTime.h"
    #include "Transceivers/MCHP_MAC.h"
    #include "Transceivers/ConfigTransceivers.h"
    #include "WirelessProtocols/MCHP_API.h"
    #include "WirelessProtocols/EEPROM.h"

/****************************** DEFINITIONS ***********************************/
    // Scan Duration formula 
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
    const ROM DWORD ScanTime[15] = {SCAN_DURATION_0, SCAN_DURATION_1, \
        SCAN_DURATION_2,  SCAN_DURATION_3,  SCAN_DURATION_4,  SCAN_DURATION_5, \
        SCAN_DURATION_6,  SCAN_DURATION_7,  SCAN_DURATION_8,  SCAN_DURATION_9, \
        SCAN_DURATION_10, SCAN_DURATION_11, SCAN_DURATION_12, SCAN_DURATION_13,\
        SCAN_DURATION_14};

    typedef union{
        BYTE    Val;
        struct{
            BYTE    Sleep           :1;
            BYTE    Role            :2;
            BYTE    Security        :1;
            BYTE    ConnMode        :2;
            BYTE    CoordCap        :1;
        }bits;
    } MIWI_CAPACITY_INFO;

//JUAN: ADDED. DEFINITIONS FOR MIWI'S CONNECTION_STATUS ----------------------//
//Original MiWi stack values (for each group, 1 MiWi transceiver only, original
//struct definition): 0x8C, 0x8D, 0xB7, 0xBF

//Open Socket Default RXOFF values. Set flags Valid, LongValid, short????Valid
#define CONNSTAT_DFLT_OFF_0434  0x001C //ISM_434
#define CONNSTAT_DFLT_OFF_0868  0x010C //ISM_868
#define CONNSTAT_DFLT_OFF_2400  0x100C //ISM_2G4
//Open Socket Default RX0N values. Sets also RXon flag.
#define CONNSTAT_DFLT_ON_0434   0x001D //ISM_434
#define CONNSTAT_DFLT_ON_0868   0x010D //ISM_868
#define CONNSTAT_DFLT_ON_2400   0x100D //ISM_2G4
//Network Association Request. Valid, LongValid, RxOn, FinishJoin, Direct, Family.
#define CONNSTAT_NW0434_ASREQ   0x00ED //ISM_434
#define CONNSTAT_NW0868_ASREQ   0x0E0D //ISM_868
#define CONNSTAT_NW2400_ASREQ   0xE00D //ISM_2G4
//Network Association Response. As request, but also sets Short????Valid flag.
#define CONNSTAT_NW0434_ASRESP   0x00FD //ISM_434
#define CONNSTAT_NW0868_ASRESP   0x0F0D //ISM_868
#define CONNSTAT_NW2400_ASRESP   0xF00D //ISM_2G4
//----------------------------------------------------------------------------//

/******************************* VARIABLES ************************************/
    // permanent address definition
    #if MY_ADDRESS_LENGTH == 8
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3, \
                                                 EUI_4, EUI_5, EUI_6, EUI_7};
    #elif MY_ADDRESS_LENGTH == 7
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3, \
                                                 EUI_4, EUI_5, EUI_6};
    #elif MY_ADDRESS_LENGTH == 6
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3, \
                                                EUI_4, EUI_5};
    #elif MY_ADDRESS_LENGTH == 5
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3, \
                                                 EUI_4};
    #elif MY_ADDRESS_LENGTH == 4
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2, EUI_3};
    #elif MY_ADDRESS_LENGTH == 3
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1, EUI_2};
    #elif MY_ADDRESS_LENGTH == 2
        BYTE myLongAddress[MY_ADDRESS_LENGTH] = {EUI_0, EUI_1};
    #endif

// COMMON VARIABLES FOR ALL TRANSCEIVERS -------------------------------------//
extern nodeStatus NodeStatus;
extern unsigned coreTMRvals[];
extern BYTE coreTMRptr;

extern BYTE AdditionalNodeID[];
// The additional information regarding the device that would like to share with
// the peer on the other side of P2P connection. This information is application
// specific.

BYTE ActiveScanResultIndex;
ACTIVE_SCAN_RESULT ActiveScanResults[ACTIVE_SCAN_RESULT_SIZE];
// The results for active scan, including the PAN identifier, signal strength
// and operating channel

#if defined(__18CXX)
    #pragma udata BIGvariables1
#endif
CONNECTION_ENTRY ConnectionTable[CONNECTION_SIZE]; //Juan: Shared connection table
#if defined(__18CXX)
    #pragma udata
#endif

MAC_TRANS_PARAM MTP;            //Auxiliar variable for sending a packet.

#ifdef NWK_ROLE_COORDINATOR
    #if defined(ENABLE_INDIRECT_MESSAGE)
        #if defined(__18CXX)
            #pragma udata INDIRECT_BUFFER
        #endif
        INDIRECT_MESSAGE indirectMessages[INDIRECT_MESSAGE_SIZE];
        #if defined(__18CXX)
            #pragma udata
        #endif
    #endif
    BYTE role;
#endif

#if defined(ENABLE_SECURITY)
    DWORD_VAL IncomingFrameCounter[CONNECTION_SIZE];
#endif

#if defined(ENABLE_NETWORK_FREEZER)
    MIWI_TICK nvmDelayTick;
#endif

#if defined(ENABLE_MIWI_ACKNOWLEDGEMENT)
    BOOL MiWiAckRequired = TRUE;
#else
    BOOL MiWiAckRequired = FALSE;
#endif

BYTE defaultHops = MAX_HOPS;

struct _BROADCAST_RECORD{
    WORD_VAL    AltSourceAddr;
    BYTE        MiWiSeq;
    BYTE        RxCounter;
    MIWI_TICK   StartTick;
    miwi_band   MiWiFreqBand;               //Juan: Added
} BroadcastRecords[BROADCAST_RECORD_SIZE];

//SPECIFIC VARIABLES FOR EACH TRANSCEIVER ------------------------------------//
#if defined MRF24J40
    BYTE MIWI2400_currentChannel;                   //Channel (stack usage)
    BYTE my2400Parent;
    BYTE MIWI2400_ConnMode = DISABLE_ALL_CONN;      //Default connection mode
    RECEIVED_MESSAGE MIWI2400_rxMessage;    //Structure for the received packet
    OPEN_SOCKET Socket2400Info;             //For establishing indirect links
    MIWI_CAPACITY_INFO MIWI2400CapacityInfo;
    WORD_VAL myShort2400Addr;
    WORD_VAL myPAN2400ID;
    WORD_VAL Ack2400Addr;
    BYTE Ack2400SeqNum = 0xFF;
    BYTE MIWI2400SeqNum;
    TEMP_NODE_INFO temp2400;
    
    BYTE auxiliar,prueba, k;

    BYTE MRF24J40_TxData;
    MIWI_STATE_MACHINE MRF24J40_MiWiStateMachine;
    MAC_RECEIVED_PACKET MRF24J40_MACRxPacket;
    #if defined(__18CXX)
        #pragma udata TRX_BUFFER
    #endif
    #if (MRF24J40_TX_BUF_SIZE+MIWI_HEADER_LEN) > 110
        BYTE MRF24J40_TxBuffer[110];
    #else
        BYTE MRF24J40_TxBuffer[MRF24J40_TX_BUF_SIZE+MIWI_HEADER_LEN];
    #endif
    #if defined(__18CXX)
    #pragma udata
    #endif
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF24J40_DataReqTimer;
    #endif
    #ifdef NWK_ROLE_COORDINATOR
        BYTE Routing2400Table[8];
        BYTE Router2400Failures[8];
        BYTE known2400Coordinators;
    #endif
#endif

#if defined MRF49XA_1
    #if defined MRF49XA_1_IN_434
        BYTE MIWI0434_currentChannel;                   //Channel (stack usage)
        BYTE my0434Parent;
        BYTE MIWI0434_ConnMode = DISABLE_ALL_CONN;      //Default connection mode
        RECEIVED_MESSAGE MIWI0434_rxMessage;    //Structure for the received packet
        OPEN_SOCKET Socket0434Info;             //For establishing indirect links
        MIWI_CAPACITY_INFO MIWI0434CapacityInfo;
        WORD_VAL myShort0434Addr;
        WORD_VAL myPAN0434ID;
        WORD_VAL Ack0434Addr;
        BYTE Ack0434SeqNum = 0xFF;
        BYTE MIWI0434SeqNum;
        TEMP_NODE_INFO temp0434;
        #ifdef NWK_ROLE_COORDINATOR
            BYTE Routing0434Table[8];
            BYTE Router0434Failures[8];
            BYTE known0434Coordinators;
        #endif
    #elif defined MRF49XA_1_IN_868
        BYTE MIWI0868_currentChannel;                   //Channel (stack usage)
        BYTE my0868Parent;
        BYTE MIWI0868_ConnMode = DISABLE_ALL_CONN;      //Default connection mode
        RECEIVED_MESSAGE MIWI0868_rxMessage;    //Structure for the received packet
        OPEN_SOCKET Socket0868Info;             //For establishing indirect links
        MIWI_CAPACITY_INFO MIWI0868CapacityInfo;
        WORD_VAL myShort0868Addr;
        WORD_VAL myPAN0868ID;
        WORD_VAL Ack0868Addr;
        BYTE Ack0868SeqNum = 0xFF;
        BYTE MIWI0868SeqNum;
        TEMP_NODE_INFO temp0868;
        #ifdef NWK_ROLE_COORDINATOR
            BYTE Routing0868Table[8];
            BYTE Router0868Failures[8];
            BYTE known0868Coordinators;
        #endif
    #endif

    BYTE MRF49XA_1_TxData;
    MIWI_STATE_MACHINE MRF49XA_1_MiWiStateMachine;
    MAC_RECEIVED_PACKET MRF49XA_1_MACRxPacket;
    #if defined(__18CXX)
    #pragma udata TRX_BUFFER
    #endif
    #if (MRF49XA_TX_BUF_SIZE+MIWI_HEADER_LEN) > 110
        BYTE MRF49XA_1_TxBuffer[110];
    #else
        BYTE MRF49XA_1_TxBuffer[MRF49XA_TX_BUF_SIZE+MIWI_HEADER_LEN];
    #endif
    #if defined(__18CXX)
    #pragma udata
    #endif
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF49XA_1_DataReqTimer;
    #endif
#endif

#if defined MRF49XA_2
    #if defined MRF49XA_2_IN_434
        BYTE MIWI0434_currentChannel;                   //Channel (stack usage)
        BYTE my0434Parent;
        BYTE MIWI0434_ConnMode = DISABLE_ALL_CONN;      //Default connection mode
        RECEIVED_MESSAGE MIWI0434_rxMessage;    //Structure for the received packet
        OPEN_SOCKET Socket0434Info;             //For establishing indirect links
        MIWI_CAPACITY_INFO MIWI0434CapacityInfo;
        WORD_VAL myShort0434Addr;
        WORD_VAL myPAN0434ID;
        WORD_VAL Ack0434Addr;
        BYTE Ack0434SeqNum = 0xFF;
        BYTE MIWI0434SeqNum;
        TEMP_NODE_INFO temp0434;
        #ifdef NWK_ROLE_COORDINATOR
            BYTE Routing0434Table[8];
            BYTE Router0434Failures[8];
            BYTE known0434Coordinators;
        #endif
    #elif defined MRF49XA_2_IN_868
        BYTE MIWI0868_currentChannel;                   //Channel (stack usage)
        BYTE my0868Parent;
        BYTE MIWI0868_ConnMode = DISABLE_ALL_CONN;      //Default connection mode
        RECEIVED_MESSAGE MIWI0868_rxMessage;    //Structure for the received packet
        OPEN_SOCKET Socket0868Info;             //For establishing indirect links
        MIWI_CAPACITY_INFO MIWI0868CapacityInfo;
        WORD_VAL myShort0868Addr;
        WORD_VAL myPAN0868ID;
        WORD_VAL Ack0868Addr;
        BYTE Ack0868SeqNum = 0xFF;
        BYTE MIWI0868SeqNum;
        TEMP_NODE_INFO temp0868;
        #ifdef NWK_ROLE_COORDINATOR
            BYTE Routing0868Table[8];
            BYTE Router0868Failures[8];
            BYTE known0868Coordinators;
        #endif
    #endif

    BYTE MRF49XA_2_TxData;
    MIWI_STATE_MACHINE MRF49XA_2_MiWiStateMachine;
    MAC_RECEIVED_PACKET MRF49XA_2_MACRxPacket;
    #if defined(__18CXX)
    #pragma udata TRX_BUFFER
    #endif
    #if (MRF49XA_TX_BUF_SIZE+MIWI_HEADER_LEN) > 110
        BYTE MRF49XA_2_TxBuffer[110];
    #else
        BYTE MRF49XA_2_TxBuffer[MRF49XA_TX_BUF_SIZE+MIWI_HEADER_LEN];
    #endif
    #if defined(__18CXX)
    #pragma udata
    #endif
    #ifdef ENABLE_SLEEP
        MIWI_TICK MRF49XA_2_DataReqTimer;
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

/********************************** FUNCTIONS *********************************/
    
#if defined(IEEE_802_15_4)
    BOOL SendMACPacket(BYTE *PANID, BYTE *Address, BYTE PacketType, BYTE ModeMask, miwi_band mb);
#else
    BOOL SendMACPacket(BYTE *Address, BYTE PacketType, miwi_band mb);
#endif

BOOL RouteMessage(WORD_VAL PANID, WORD_VAL ShortAddress, BOOL SecEn, miwi_band mb);
void StartChannelHopping(INPUT BYTE OptimalChannel, miwi_band mb);
void SendBeacon(miwi_band mb);

#if defined(IEEE_802_15_4)
    BOOL SaveIndirectMessage(INPUT BOOL Broadcast,
                             INPUT WORD_VAL DestinationPANID,
                             INPUT BYTE *DestinationAddress,
                             INPUT BOOL isAltAddr,
                             INPUT BOOL SecurityEnabled,
                             INPUT miwi_band mb);
#else
    BOOL SaveIndirectMessage(INPUT BOOL Broadcast,
                             INPUT BYTE *DestinationAddress,
                             INPUT BOOL isAltAddr,
                             INPUT BOOL SecurityEnabled,
                             INPUT miwi_band mb);
#endif

//LSI-CWSN new functions:

static void Transceivers_Tasks(BYTE transceiver);
static BOOL AuxMAC_SendPacket(MAC_TRANS_PARAM *MTP, BYTE transceiver);
static void AuxMAC_FlushTx(BYTE transceiver);

//----------------------------------------------------------------------------//

    /**************************************************************************/
    // C18 compiler cannot optimize the code with a macro. Instead of calling 
    // macro Nop in a big function, we define a wrapper to call Nop in order to
    // be able to optimize the code efficiently.
    /**************************************************************************/
    void MacroNop(void){
        Nop();
    }   
    
    #if !defined(TARGET_SMALL)
        /**********************************************************************/
        // Function:    void BroadcastJitter(BYTE range)
        // PreCondition:None
        // Input:       range   - The range of random delay in millisecond.
        // Output:      None
        // Side Effect: A random delay between 0 to range in millisecond
        // Overview:    This function introduce a random delay between 0 to
        //              range in millisecond. This delay helps multiple devices
        //              to rebroadcast the message at different time, thus lower
        //              the possibility of packet collision in rebroadcast.
        /**********************************************************************/
        void BroadcastJitter(BYTE range){
            BYTE jitter = TMRL % range;
            MIWI_TICK t1, t2;

            while( jitter > 0 ){
                jitter--;
                t1 = MiWi_TickGet();
                while(1)
                {
                    t2 = MiWi_TickGet();
                    if( MiWi_TickGetDiff(t2, t1) > ONE_MILI_SECOND )
                    {
                        break;
                    }
                }        
            }  
        }
    #endif
    
/*******************************************************************************
 * Function:    void MiWiTasks( void )
 * PreCondition:None
 * Input:       None
 * Output:      None
 * Side Effects:The stack receives, handles, buffers, and transmits packets.
 *              It also handles all of the joining
 * Overview:    This function maintains the operation of the stack.
 *              It should be called as often as possible.
 ******************************************************************************/
void MiWiTasks(void){
    BYTE i;
    MIWI_TICK t1;
//Transceiver tasks. Check if the transceivers have received any message. ----//
    #if defined MRF49XA_1
        //coreTMRvals[1] = ReadCoreTimer();
        Transceivers_Tasks(1);
    #endif
    #if defined MRF49XA_2
        Transceivers_Tasks(2);
    #endif
    #if defined MRF24J40
        //coreTMRvals[2] = ReadCoreTimer();
        Transceivers_Tasks(3);
    #endif
//----------------------------------------------------------------------------//
    //coreTMRvals[3] = ReadCoreTimer();
    t1 = MiWi_TickGet();          //Juan: Capture tick time for tasks.
    //if there really isn't anything going on
    #if defined(NWK_ROLE_COORDINATOR) && defined(ENABLE_INDIRECT_MESSAGE)
        // check indirect message periodically. If an indirect message is
        // not acquired within time of INDIRECT_MESSAGE_TIMEOUT
        for(i = 0; i < INDIRECT_MESSAGE_SIZE; i++){
            if(indirectMessages[i].flags.bits.isValid){
                if(MiWi_TickGetDiff(t1, indirectMessages[i].TickStart) > INDIRECT_MESSAGE_TIMEOUT){
                    indirectMessages[i].flags.Val = 0x00;
                    Printf("\r\nIndirect message expired.");   //Juan: Added.
                }
            }
        }
    #endif //COORDINATOR_CAPABLE

    #if !defined(ENABLE_SLEEP)
        for(i = 0; i < BROADCAST_RECORD_SIZE; i++){
            if(BroadcastRecords[i].RxCounter > 0){
                if(MiWi_TickGetDiff(t1, BroadcastRecords[i].StartTick) > BROADCAST_RECORD_TIMEOUT){
                    BroadcastRecords[i].RxCounter = 0;
                    Printf("\r\nBroadcast record expired.");   //Juan: Added.
                }
            }
        }
    #endif

//The following code must be executed for every radio interface --------------//
    //Variables to be used.
    MIWI_TICK t2;
    MIWI_TICK *DataReqTimer;
    OPEN_SOCKET *OpenSocketInfo;
    MIWI_STATE_MACHINE *msm;
    #if defined(ENABLE_TIME_SYNC)
        WORD_VAL *WakeupTimes;
        WORD_VAL *CounterValue;
    #endif
    miwi_band mb, next_mb;
    mb = NO_ISM;

LOAD_RI_VARIABLES:
    //Load variables for the current radio interface.
    switch(mb){     //Select the next radio interface...
        case NONE:
            #if defined MIWI_0434_RI
                next_mb = ISM_434;
            #elif defined MIWI_0868_RI
                next_mb = ISM_868;
            #elif defined MIWI_2400_RI
                next_mb = ISM_2G4;
            #endif
            break;
        case ISM_434:
            #if defined MIWI_0868_RI
                next_mb = ISM_868;
            #elif defined MIWI_2400_RI
                next_mb = ISM_2G4;
            #else
                next_mb = NO_ISM;
            #endif
            break;
        case ISM_868:
            #if defined MIWI_2400_RI
                next_mb = ISM_2G4;
            #else
                next_mb = NO_ISM;
            #endif
            break;
        case ISM_2G4:
            next_mb = NO_ISM;
            break;
        default:
            Printf("\r\nERROR IN MIWI_TASKS\r");
            return;
    }
    if (next_mb == NO_ISM){
        //Done for all!!
        return;
    }
    mb = next_mb;       

    switch(mb){                                     //Load variables...
        case ISM_434:
            #if defined MIWI_0434_RI
                OpenSocketInfo = & Socket0434Info;
                #if defined MRF49XA_1_IN_434
                    msm = & MRF49XA_1_MiWiStateMachine;
                    #if defined(ENABLE_SLEEP)
                        DataReqTimer = & MRF49XA_1_DataReqTimer;
                    #endif
                    #if defined(ENABLE_TIME_SYNC)
                        WakeupTimes = & MRF49XA_1_WakeupTimes;
                        CounterValue = & MRF49XA_1_CounterValue;
                    #endif
                #elif defined MRF49XA_2_IN_434
                    msm = & MRF49XA_2_MiWiStateMachine;
                    #if defined(ENABLE_SLEEP)
                        DataReqTimer = & MRF49XA_2_DataReqTimer;
                    #endif
                    #if defined(ENABLE_TIME_SYNC)
                        WakeupTimes = & MRF49XA_2_WakeupTimes;
                        CounterValue = & MRF49XA_2_CounterValue;
                    #endif
                #endif
            #endif
            break;
        case ISM_868:
            #if defined MIWI_0868_RI
                OpenSocketInfo = & Socket0868Info;
                #if defined MRF49XA_1_IN_868
                    msm = & MRF49XA_1_MiWiStateMachine;
                    #if defined(ENABLE_SLEEP)
                        DataReqTimer = & MRF49XA_1_DataReqTimer;
                    #endif
                    #if defined(ENABLE_TIME_SYNC)
                        WakeupTimes = & MRF49XA_1_WakeupTimes;
                        CounterValue = & MRF49XA_1_CounterValue;
                    #endif
                #elif defined MRF49XA_2_IN_868
                    msm = & MRF49XA_2_MiWiStateMachine;
                    #if defined(ENABLE_SLEEP)
                        DataReqTimer = & MRF49XA_2_DataReqTimer;
                    #endif
                    #if defined(ENABLE_TIME_SYNC)
                        WakeupTimes = & MRF49XA_2_WakeupTimes;
                        CounterValue = & MRF49XA_2_CounterValue;
                    #endif
                #endif
            #endif
            break;
        case ISM_2G4:
            #if defined MIWI_2400_RI
                OpenSocketInfo = & Socket2400Info;
                msm = & MRF24J40_MiWiStateMachine;
                #if defined(ENABLE_SLEEP)
                    DataReqTimer = & MRF24J40_DataReqTimer;
                #endif
                #if defined(ENABLE_TIME_SYNC)
                    WakeupTimes = & MRF24J40_WakeupTimes;
                    CounterValue = & MRF24J40_CounterValue;
                #endif
            #endif
            break;
        default:
            return;
    }

    
    //Tasks...
    #if defined(ENABLE_SLEEP)
        //if we are an RFD and a member of the network
        if(msm->bits.memberOfNetwork){
            if(msm->bits.DataRequesting){
                t2.Val = MiWi_TickGetDiff(t1, (*DataReqTimer));
                if(t2.Val > RFD_DATA_WAIT){
                    msm->bits.DataRequesting = 0;
                    #if defined(ENABLE_TIME_SYNC)
                        WakeupTimes->Val = RFD_WAKEUP_INTERVAL / 16;
                        CounterValue->Val = 0xFFFF - ((WORD)4000*(RFD_WAKEUP_INTERVAL % 16));
                    #endif
                }
            }
        }
    #endif

    #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && defined(ENABLE_INDIRECT_MESSAGE)
        if(MiWi_TickGetDiff(t1, TimeSyncTick) > ((ONE_SECOND) * RFD_WAKEUP_INTERVAL)){
            TimeSyncTick.Val += ((DWORD)(ONE_SECOND) * RFD_WAKEUP_INTERVAL);
            if(TimeSyncTick.Val > t1.Val){
                TimeSyncTick.Val = t1.Val;
            }
            TimeSyncSlot = 0;
        }
    #endif


    #if defined(ENABLE_NETWORK_FREEZER) && defined(NWK_ROLE_COORDINATOR)
   //THIS PART IS NOT ADAPTED.
        if(msm->bits.saveConnection){
            if(MiWi_TickGetDiff(t1, nvmDelayTick) > (ONE_SECOND)){
                msm->bits.saveConnection = 0;
                nvmPutKnownCoordinators(&knownCoordinators);
                nvmPutRoutingTable(RoutingTable);
                #if !defined(IEEE_802_15_4)
                    nvmPutConnectionTable(ConnectionTable);
                #endif
            }
        }
    #endif

    //Juan: broadcast record code moved to common tasks in this function.

    //clean up an old socket if one still exists
    if(OpenSocketInfo->status.bits.requestIsOpen){
        //Juan: At least, I have received an open_socket_request from the first
        //peer (who is searching for a second peer to establish an indirect
        //connection)
        if(OpenSocketInfo->status.bits.matchFound){
            //Juan: I have received the second request from another peer so I
            //have got a "match". In TransceiversTasks, the match was notified 
            //to the first peer sending out a response with the info about the  
            //second peer. So, if I'm a coordinator (in charge of handling
            //sockets), I still have to response to the second peer, attaching
            //the info about the first peer involved.
            #ifdef NWK_ROLE_COORDINATOR
                BYTE j;

                ConsolePutROMString((ROM char*)"sending out second response\r\n");

                MiApp_FlushTx(mb);
                MiApp_WriteData(MIWI_STACK_REPORT_TYPE, mb);    //Report Type
                MiApp_WriteData(OPEN_SOCKET_RESPONSE, mb);      //Report ID

                //copy the long and short address from the Rx Buffer
                for(j=0; j<MY_ADDRESS_LENGTH; j++){
                    MiApp_WriteData(OpenSocketInfo->LongAddress1[j], mb);
                }
                MiApp_WriteData(OpenSocketInfo->ShortAddress1.v[0], mb);
                MiApp_WriteData(OpenSocketInfo->ShortAddress1.v[1], mb);

                #if ADDITIONAL_NODE_ID_SIZE > 0
                    for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                        MiApp_WriteData(OpenSocketInfo->AdditionalNodeID1[j], mb);
                    }
                #endif

                MiApp_UnicastAddress(OpenSocketInfo->ShortAddress2.v, FALSE, FALSE, mb);

                //Juan: socket has been established. The socket is marked as not
                //open for enabling new indirect connections.
                OpenSocketInfo->status.bits.requestIsOpen = 0;
            #endif
            //openSocketInfo.status.bits.requestIsOpen = 0;
        }
        else{
            //Juan: there is no match yet... check if the request has expired.
            t2.Val = MiWi_TickGetDiff(t1, OpenSocketInfo->socketStart);

            if(t2.Val > OPEN_SOCKET_TIMEOUT){
                //invalidate the expired socket request
                OpenSocketInfo->status.bits.requestIsOpen = 0;
            }
        }
    }
    
    //Repeat for the next radio interface.
    goto LOAD_RI_VARIABLES;
}

// Microchip LSI Stack
// Extracted from MiWiTasks and modified to do all transceiver tasks by invoking
// the function with different parameters.
static void Transceivers_Tasks(BYTE transceiver){
    BYTE *pbuffer, *TxData, *currentChannel, *myParent, *MiWiSeqNum, *AckSeqNum;
    BYTE ChannelOffset, ConnMode;
    WORD_VAL *myPANID, *myShortAddress, *AckAddr;
    OPEN_SOCKET *OpenSocketInfo;
    TEMP_NODE_INFO *TempNodeInfo;
    MIWI_STATE_MACHINE *msm;
    MIWI_CAPACITY_INFO *CapacityInfo;
    RECEIVED_MESSAGE *MIWI_rxMsg;
    MAC_RECEIVED_PACKET *MAC_rxPckt;
    miwi_band mb;
    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
        WORD_VAL *WakeupTimes;
        WORD_VAL *CounterValue;
    #endif
    #ifdef NWK_ROLE_COORDINATOR
        BYTE *RoutingTable;
        BYTE *RouterFailures;
        BYTE *knownCoordinators;
    #endif
    switch (transceiver){
        case 1:
            #if defined MRF49XA_1
                if(MiMAC_MRF49XA_ReceivedPacket(1)){
                    if(MRF49XA_1_MiWiStateMachine.bits.RxHasUserData){
                       return;
                    }
                    // Else: init. local variables to use MRF49XA_1's variables and go on.
                    #if defined MRF49XA_1_IN_434
                        MIWI_rxMsg = & MIWI0434_rxMessage;
                        currentChannel = &MIWI0434_currentChannel;
                        myParent = & my0434Parent;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        ConnMode = MIWI0434_ConnMode;
                        mb = ISM_434;
                        myPANID = & myPAN0434ID;
                        myShortAddress = & myShort0434Addr;
                        OpenSocketInfo = & Socket0434Info;
                        TempNodeInfo = & temp0434;
                        AckAddr = & Ack0434Addr;
                        CapacityInfo = & MIWI0434CapacityInfo;
                        #ifdef NWK_ROLE_COORDINATOR
                            RoutingTable = &Routing0434Table[0];
                            RouterFailures = &Router0434Failures[0];
                            knownCoordinators = & known0434Coordinators;
                        #endif
                    #elif defined MRF49XA_1_IN_868
                        MIWI_rxMsg = & MIWI0868_rxMessage;
                        currentChannel = & MIWI0868_currentChannel;
                        myParent = & my0868Parent;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        ConnMode = MIWI0868_ConnMode;
                        mb = ISM_868;
                        myPANID = & myPAN0868ID;
                        myShortAddress = & myShort0868Addr;
                        OpenSocketInfo = & Socket0868Info;
                        TempNodeInfo = & temp0868;
                        AckAddr = & Ack0868Addr;
                        CapacityInfo = & MIWI0868CapacityInfo;
                        #ifdef NWK_ROLE_COORDINATOR
                            RoutingTable = & Routing0868Table[0];
                            RouterFailures = & Router0868Failures[0];
                            knownCoordinators = & known0868Coordinators;
                        #endif
                    #endif
                    MAC_rxPckt = &MRF49XA_1_MACRxPacket;
                    TxData = & MRF49XA_1_TxData;
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                        WakeupTimes = & MRF49XA_1_WakeupTimes;
                        CounterValue = & MRF49XA_1_WakeupTimes;
                    #endif
                    break;
                }
                return;
            #else
		Printf("\r\nInvalid call to MRF49XA_1 tasks. This transceiver is unavailable.");
		return;
            #endif
        case 2:
            #if defined MRF49XA_2
                if(MiMAC_MRF49XA_ReceivedPacket(2)){
                    if(MRF49XA_2_MiWiStateMachine.bits.RxHasUserData){
                       return;
                    }
                    // Else: init. local variables to use MRF49XA_2's variables and go on.
                    #if defined MRF49XA_2_IN_434
                        MIWI_rxMsg = & MIWI0434_rxMessage;
                        currentChannel = & MIWI0434_currentChannel;
                        myParent = & my0434Parent;
                        ChannelOffset = MIWI0434ConfChannelOffset;
                        ConnMode = MIWI0434_ConnMode;
                        mb = ISM_434;
                        myPANID = & myPAN0434ID;
                        myShortAddress = & myShort0434Addr;
                        OpenSocketInfo = & Socket0434Info;
                        TempNodeInfo = & temp0434;
                        AckAddr = & Ack0434Addr;
                        CapacityInfo = & MIWI0434CapacityInfo;
                        #ifdef NWK_ROLE_COORDINATOR
                            RoutingTable = & Routing0434Table[0]; // AGUS CAMBIADO
                            RouterFailures = & Router0434Failures[0]; // AGUS CAMBIADO
                            knownCoordinators = & known0434Coordinators;
                        #endif
                    #elif defined MRF49XA_2_IN_868
                        MIWI_rxMsg = & MIWI0868_rxMessage;
                        currentChannel = & MIWI0868_currentChannel;
                        myParent = & my0868Parent;
                        ChannelOffset = MIWI0868ConfChannelOffset;
                        ConnMode = MIWI0868_ConnMode;
                        mb = ISM_868;
                        myPANID = & myPAN0868ID;
                        myShortAddress = & myShort0868Addr;
                        OpenSocketInfo = & Socket0868Info;
                        TempNodeInfo = & temp0868;
                        AckAddr = & Ack0868Addr;
                        CapacityInfo = & MIWI0868CapacityInfo;
                        #ifdef NWK_ROLE_COORDINATOR
                            RoutingTable = & Routing0868Table[0]; //AGUS CAMBIADO
                            RouterFailures = & Router0868Failures[0]; //AGUS CAMBIADO
                            knownCoordinators = & known0868Coordinators;
                        #endif
                    #endif
                    MAC_rxPckt = & MRF49XA_2_MACRxPacket;
                    TxData = & MRF49XA_2_TxData;
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;

                    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                        WakeupTimes = & MRF49XA_2_WakeupTimes;
                        CounterValue = & MRF49XA_2_WakeupTimes;
                    #endif
                    break;
                }
                return;
            #else
		Printf("\r\nInvalid call to MRF49XA_2 tasks. This transceiver is unavailable.");
		return;
            #endif
        case 3:
            #if defined MRF24J40
                if(MiMAC_MRF24J40_ReceivedPacket()){
                    if(MRF24J40_MiWiStateMachine.bits.RxHasUserData){
                        return;
                    }
                    // Else: init. local variables to use MRF24J40's variables and go on.
                    MIWI_rxMsg = & MIWI2400_rxMessage;
                    currentChannel = & MIWI2400_currentChannel;
                    myParent = & my2400Parent;
                    ChannelOffset = MIWI2400ConfChannelOffset;
                    ConnMode = MIWI2400_ConnMode;
                    mb = ISM_2G4;
                    myPANID = & myPAN2400ID;
                    myShortAddress = & myShort2400Addr;
                    OpenSocketInfo = &Socket2400Info;
                    TempNodeInfo = & temp2400;
                    AckAddr = & Ack2400Addr;
                    CapacityInfo = & MIWI2400CapacityInfo;

                    MAC_rxPckt = & MRF24J40_MACRxPacket;
                    TxData = & MRF24J40_TxData;
                    pbuffer = MRF24J40_TxBuffer;
                    msm = & MRF24J40_MiWiStateMachine;
                    #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                        WakeupTimes = & MRF24J40_WakeupTimes;
                        CounterValue = & MRF24J40_WakeupTimes;
                    #endif
                    #ifdef NWK_ROLE_COORDINATOR
                        RoutingTable = & Routing2400Table[0];
                        RouterFailures = & Router2400Failures[0];
                        knownCoordinators = & known2400Coordinators;
                    #endif
                    break;
                }
                return;
            #else
		Printf("\r\nInvalid call to MRF24J40 tasks. This transceiver is unavailable.");
		return;
            #endif
        default:
	    Printf("\r\nInvalid call to transceiver tasks. Unknown transceiver.");
	    return;
    }
    BYTE i;

    MIWI_rxMsg->flags.Val = 0;
    MIWI_rxMsg->flags.bits.broadcast = MAC_rxPckt->flags.bits.broadcast;
    MIWI_rxMsg->flags.bits.secEn = MAC_rxPckt->flags.bits.secEn;
    MIWI_rxMsg->flags.bits.command = (MAC_rxPckt->flags.bits.packetType == PACKET_TYPE_COMMAND) ? 1:0;
    MIWI_rxMsg->flags.bits.srcPrsnt = MAC_rxPckt->flags.bits.sourcePrsnt;
    
    //Printf("\r\nPANID mensaje");
    //PrintChar(MAC_rxPckt->SourcePANID.v[1]);
    //PrintChar(MAC_rxPckt->SourcePANID.v[0]);
    
    if(MAC_rxPckt->flags.bits.sourcePrsnt){
        //Juan: added transceiver condition.
        if(transceiver == 3){
            #if defined(IEEE_802_15_4)
                MIWI_rxMsg->flags.bits.altSrcAddr = MAC_rxPckt->altSourceAddress;
            #endif
        }else{
            MIWI_rxMsg->flags.bits.altSrcAddr = 1;
        }
        MIWI_rxMsg->SourceAddress = MAC_rxPckt->SourceAddress;
    }

    //Juan: added transceiver condition.
    if(transceiver == 3){
        #if defined(IEEE_802_15_4)
            MIWI_rxMsg->SourcePANID.Val = MAC_rxPckt->SourcePANID.Val;
        #endif
    }
    MIWI_rxMsg->PacketLQI = MAC_rxPckt->LQIValue;
    MIWI_rxMsg->PacketRSSI = MAC_rxPckt->RSSIValue;

    msm->bits.RxHasUserData = 0;

    //determine what type of packet it is.
    switch(MAC_rxPckt->flags.bits.packetType){
        ////////////////////////////////////////////////////////////////////////
        case PACKET_TYPE_DATA:                          //if it is a data packet
        ////////////////////////////////////////////////////////////////////////
        //The guts of MiWi ...
        {
            WORD_VAL destPANID, sourcePANID;
            WORD_VAL sourceShortAddress, destShortAddress;
HANDLE_DATA_PACKET:
            #if defined(ENABLE_SLEEP)
                #if defined(ENABLE_BROADCAST_TO_SLEEP_DEVICE)
                    for(i = 0; i < BROADCAST_RECORD_SIZE; i++){
                        if( BroadcastRecords[i].RxCounter > 0){
                            BroadcastRecords[i].RxCounter--;
                        }
                    }
                #endif

                // If it is just an empty packet, ignore here.
                if(MAC_rxPckt->PayloadLen == 0 ){
                    msm->bits.DataRequesting = 0;
                    break;
                }
            #endif

            if(MAC_rxPckt->PayloadLen < 10){
                break;
            }

            // Load the source and destination address information.
            destPANID.v[0] = MAC_rxPckt->Payload[2];
            destPANID.v[1] = MAC_rxPckt->Payload[3];
            destShortAddress.v[0] = MAC_rxPckt->Payload[4];
            destShortAddress.v[1] = MAC_rxPckt->Payload[5];
            sourcePANID.v[0] = MAC_rxPckt->Payload[6];
            sourcePANID.v[1] = MAC_rxPckt->Payload[7];
            sourceShortAddress.v[0] = MAC_rxPckt->Payload[8];
            sourceShortAddress.v[1] = MAC_rxPckt->Payload[9];

            MIWI_rxMsg->flags.Val = 0;
            MIWI_rxMsg->flags.bits.secEn = MAC_rxPckt->flags.bits.secEn;
            // if this is a broadcast
            if(MIWI_rxMsg->flags.bits.broadcast || destShortAddress.Val == 0xFFFF){
                // if this broadcast is from myself
                if( sourceShortAddress.Val == myShortAddress->Val &&
                    sourcePANID.Val == myPANID->Val){
                    break;
                }

                #ifdef NWK_ROLE_COORDINATOR
                    // Consider to rebroadcast the message
                    if(MAC_rxPckt->Payload[0]>1){
                        MAC_rxPckt->Payload[0]--;
                        AuxMAC_FlushTx(transceiver);
                        for(i = 0; i < MAC_rxPckt->PayloadLen; i++){
                            MiApp_WriteData(MAC_rxPckt->Payload[i], mb);
                        }

                        MTP.flags.Val = MAC_rxPckt->flags.Val;

                        //Juan: added transceiver condition.
                        if (transceiver == 3){
                            #if defined(IEEE_802_15_4)
                                MTP.DestPANID.Val = myPANID->Val;
                                MTP.altSrcAddr = MAC_rxPckt->altSourceAddress;
                            #endif
                        }

                        #if !defined(TARGET_SMALL)
                            // introduce a random delay to avoid
                            // rebroadcast at the same time
                            BroadcastJitter(20);
                        #endif

                        AuxMAC_SendPacket(&MTP, transceiver);

                        #if defined(ENABLE_INDIRECT_MESSAGE)
                            for(i = 0; i < CONNECTION_SIZE; i++){
                                if(ConnectionTable[i].status.bits.isValid &&
                                  (ConnectionTable[i].status.bits.RXOnWhenIdle == 0)){
                                    //Juan: check "isFamily" flag of the current interface
                                    switch(mb){
                                        case ISM_434:
                                            if(ConnectionTable[i].status.bits.is0434Family)
                                                goto SAVE_INDIRECT_MESSAGE; //
                                            else
                                                break;  //Next i...
                                        case ISM_868:
                                            if(ConnectionTable[i].status.bits.is0868Family)
                                                goto SAVE_INDIRECT_MESSAGE;
                                            else
                                                break;  //Next i...
                                        case ISM_2G4:
                                            if(ConnectionTable[i].status.bits.is2400Family)
                                                goto SAVE_INDIRECT_MESSAGE;
                                            else
                                                break;  //Next i...
                                        default:
                                            return;     //ERROR
                                    }
                                }
                            }
SAVE_INDIRECT_MESSAGE:
                            if( i < CONNECTION_SIZE ){
                                #if defined(IEEE_802_15_4)
                                    if (SaveIndirectMessage (TRUE, destPANID, NULL,
                                            FALSE, MAC_rxPckt->flags.bits.secEn, mb))
                                #else
                                    if (SaveIndirectMessage (TRUE, NULL, FALSE,
                                            MAC_rxPckt->flags.bits.secEn, mb))
                                #endif
                                    {
                                        Printf("\r\n\nIndirect packet saved.");
                                    }
                                    else{
                                        Printf("\r\n\nIndirect packet couldn't be saved.");
                                    }

                                    
                            }
                        #endif
                    }
                #endif

                //since this is a broadcast we need to parse the packet as well.
                //Juan: added last condition.
                for(i = 0; i < BROADCAST_RECORD_SIZE; i++){
                    if( BroadcastRecords[i].RxCounter &&
                        BroadcastRecords[i].AltSourceAddr.Val == sourceShortAddress.Val &&
                        BroadcastRecords[i].MiWiSeq == MAC_rxPckt->Payload[10] &&
                        BroadcastRecords[i].MiWiFreqBand == mb){
                        break;
                    }
                }

                // if the broadcast is already in the broadcast record
                if( i < BROADCAST_RECORD_SIZE ){
                    #if defined(ENABLE_SLEEP)
                        msm->bits.DataRequesting = 0;
                    #endif
                    break;
                }

                // save the broadcast information into the broadcast record.
                for(i = 0; i < BROADCAST_RECORD_SIZE; i++){
                    if( BroadcastRecords[i].RxCounter == 0 ){
                        break;
                    }
                }

                if( i < BROADCAST_RECORD_SIZE ){
                    BroadcastRecords[i].AltSourceAddr.Val = sourceShortAddress.Val;
                    BroadcastRecords[i].MiWiSeq = MAC_rxPckt->Payload[10];
                    BroadcastRecords[i].RxCounter = INDIRECT_MESSAGE_TIMEOUT_CYCLE + 1;
                    BroadcastRecords[i].MiWiFreqBand = mb;  //Juan: added.
                    #if !defined(ENABLE_SLEEP)
                        BroadcastRecords[i].StartTick = MiWi_TickGet();
                    #endif
                }

                MIWI_rxMsg->flags.bits.broadcast = 1;
                goto ThisPacketIsForMe;
            }

            // this is unicast
            if((destPANID.Val == myPANID->Val) && (destShortAddress.Val == myShortAddress->Val)){
                //need to check to see if the MiWi Ack is set
                if(MAC_rxPckt->Payload[1] & MIWI_ACK_REQ) {
                    *(pbuffer)      = defaultHops;		//number of hops
                    *(pbuffer + 1)  = 0x02;		        //Frame Control
                    *(pbuffer + 2)  = sourcePANID.v[0];
                    *(pbuffer + 3)  = sourcePANID.v[1];
                    *(pbuffer + 4)  = sourceShortAddress.v[0];
                    *(pbuffer + 5)  = sourceShortAddress.v[1];
                    *(pbuffer + 6)  = myPANID->v[0];		//source PANID LSB
                    *(pbuffer + 7)  = myPANID->v[1];		//source PANID MSB
                    *(pbuffer + 8)  = myShortAddress->v[0];	//source address LSB
                    *(pbuffer + 9)  = myShortAddress->v[1];	//source address MSB
                    *(pbuffer + 10) = MAC_rxPckt->Payload[MIWI_HEADER_LEN-1];   //sequence number
                    *(pbuffer + 11) = MIWI_STACK_REPORT_TYPE;
                    *(pbuffer + 12) = ACK_REPORT_TYPE;
                    *TxData = 13;

                    #if defined(NWK_ROLE_COORDINATOR)
                        if(sourceShortAddress.v[1] != myShortAddress->v[1]){
                            RouteMessage(sourcePANID, sourceShortAddress, FALSE, mb);
                        }
                        else{
                            MTP.flags.Val = 0;
                            MTP.flags.bits.ackReq = 1;

                            if(transceiver == 3){
                                #if defined(IEEE_802_15_4)
                                    MTP.altDestAddr = TRUE;
                                    MTP.altSrcAddr = TRUE;
                                    MTP.DestAddress = sourceShortAddress.v;
                                    MTP.DestPANID.Val = sourcePANID.Val;
                                #endif
                            }else{
                                TempNodeInfo->shortAddr.Val = sourceShortAddress.Val;
                                TempNodeInfo->PANID.Val = sourcePANID.Val;
                                if( (i = SearchForShortAddress(mb)) != 0xFF ){
                                    MTP.DestAddress = ConnectionTable[i].Address;
                                }
                            }

                            AuxMAC_SendPacket(&MTP, transceiver);
                        }
                    #else
                        // for end device, always send the message to its parent
                        MTP.flags.Val = 0;
                        MTP.flags.bits.ackReq = 1;

                        if(transceiver == 3){
                            #if defined(IEEE_802_15_4)
                                MTP.altDestAddr = TRUE;
                                MTP.altSrcAddr = TRUE;
                                MTP.DestAddress = ConnectionTable[*myParent].MIWI2400AltAddress.v;
                                MTP.DestPANID.Val = myPANID->Val;
                            #endif
                        }else{
                            MTP.DestAddress = ConnectionTable[*myParent].Address;
                        }

                        AuxMAC_SendPacket(&MTP, transceiver);
                    #endif
                    MIWI_rxMsg->flags.bits.ackReq = 1;
                }

ThisPacketIsForMe:
                //if the packet is a MiWi stack packet (but not an ACK report type
                if(MAC_rxPckt->Payload[MIWI_HEADER_LEN] == MIWI_STACK_REPORT_TYPE){
                    if(MAC_rxPckt->Payload[MIWI_HEADER_LEN+1] != ACK_REPORT_TYPE){
                        //determine which stack report it was
                        switch(MAC_rxPckt->Payload[MIWI_HEADER_LEN+1]){
                            #if defined(SUPPORT_EUI_ADDRESS_SEARCH)
                            #if defined(NWK_ROLE_COORDINATOR)
                            ////////////////////////////////////////////////////
                            case EUI_ADDRESS_SEARCH
                            ////////////////////////////////////////////////////
                            {
                                BYTE handle,j;
                                BOOL ItsMe;

                                if(sourceShortAddress.Val == myShortAddress->Val){
                                    if(sourcePANID.Val == myPANID->Val){
                                        //I started this message.  I don't need to re-broadcast it.
                                        break;
                                    }
                                }

                                //copy the address into tempLongAddress
                                ConsolePutROMString((ROM char*)"\r\nEUI address search request\r\n");
                                for(j = 0; j < MY_ADDRESS_LENGTH; j++){
                                    TempNodeInfo->longAddr[j] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+j];
                                }

                                ItsMe = TRUE;
                                for(j=0;j<MY_ADDRESS_LENGTH;j++){
                                    //see if the address in question is my own
                                    if(TempNodeInfo->longAddr[j] != myLongAddress[j]){
                                        ItsMe = FALSE;
                                        break;
                                    }
                                }

                                //load up the TX Buffer with the results
                                MiApp_FlushTx(mb);
                                MiApp_WriteData(MIWI_STACK_REPORT_TYPE, mb);      //Report Type
                                MiApp_WriteData(EUI_ADDRESS_SEARCH_RESPONSE, mb); //Report ID
                                for(j = 0; j < MY_ADDRESS_LENGTH; j++){
                                    MiApp_WriteData(TempNodeInfo->longAddr[j], mb);      //Data byte 0
                                }

                                //if it wasn't me
                                if(ItsMe == FALSE){
                                    //search the network table for the device
                                    handle = SearchForLongAddress(mb);

                                    //if we found the device
                                    if(handle!=0xFF){
                                        //check to see if the short address is valid for that device
                                        if(networkStatus[handle].bits.shortAddressValid){
                                            //I know the device in question so I will send back a response
                                            MiApp_WriteData(networkTable[handle].PANID.v[0], mb);
                                            MiApp_WriteData(networkTable[handle].PANID.v[1], mb);
                                            MiApp_WriteData(networkTable[handle].AltAddress.v[0], mb);
                                            MiApp_WriteData(networkTable[handle].AltAddress.v[1], mb);

                                            //if everything is cool then send the response back
                                            RouteMessage(sourcePANID, sourceShortAddress, FALSE, mb);
                                            //SendReportByShortAddress(sourcePANID,sourceShortAddress,FALSE);
                                        }
                                        else{
                                            //if we didn't have the short address then we can't
                                            //respond to this packet even though we know the long address
                                            ConsolePutROMString((ROM char*)"found long address but short address not valid\r\n");
                                            //clear out the data I already wrote
                                            AuxMAC_FlushTx(transceiver);
                                        }
                                    }
                                    else{
                                        //if we couldn't find the long address of the device then forget it
                                        ConsolePutROMString((ROM char*)"couldn't find the device");
                                        AuxMAC_FlushTx(transceiver);
                                    }
                                }
                                else{
                                    //The node in question is me
                                    MiApp_WriteData(myPANID.v[0], mb);
                                    MiApp_WriteData(myPANID.v[1], mb);
                                    MiApp_WriteData(myShortAddress.v[0], mb);
                                    MiApp_WriteData(myShortAddress.v[1], mb);

                                    //send a response back with my own information
                                    RouteMessage(sourcePANID, sourceShortAddress, FALSE, mb);
                                }
                            }
                            break;
                            #endif //COORDINATOR_CAPABLE
                            #endif //SUPPORT_EUI_ADDRESS_SEARCH

                            #if defined(SUPPORT_EUI_ADDRESS_SEARCH)
                            ////////////////////////////////////////////////////
                            case EUI_ADDRESS_SEARCH_RESPONSE:
                            ////////////////////////////////////////////////////
                            {
                                //got a response back from the IEEE address search
                                //record all of the information in the packet
                                //long address

                                msm->bits.EUISearching = 0;
                                for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                                    TempNodeInfo->longAddr[i] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+i];
                                }
                                //PANID and short address
                                tempPANID.v[0] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+MY_ADDRESS_LENGTH];
                                tempPANID.v[1] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+3+MY_ADDRESS_LENGTH];
                                TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+4+MY_ADDRESS_LENGTH];
                                TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+5+MY_ADDRESS_LENGTH];

                                if((SearchForLongAddress(mb) == 0xFF) && (SearchForShortAddress(mb) == 0xFF)){
                                    //create a new Status entry for the network table
                                    tempNodeStatus.Val = 0x8D;
                                    if(TempNodeInfo->shortAddr.v[0] & 0x80){
                                        tempNodeStatus.bits.RXOnWhenIdle = 0;
                                    }
                                    if((TempNodeInfo->shortAddr.Val == sourceShortAddress.Val)
                                        #if defined(IEEE_802_15_4)
                                            && (tempPANID.Val == sourcePANID.Val)
                                        #endif
                                       )
                                    {
                                        tempNodeStatus.bits.directConnection = 1;
                                    }
                                    //add the node to the network table
                                    i = AddNodeToNetworkTable();
                                    #if defined(ENABLE_NETWORK_FREEZER)
                                        if(i < 0xFF){
                                            nvmPutConnectionTableIndex(&(ConnectionTable[i]), i);
                                        }
                                    #endif
                                }
                            }
                            break;
                            #endif

                            #ifdef NWK_ROLE_COORDINATOR
                            ////////////////////////////////////////////////////
                            case OPEN_SOCKET_REQUEST:
                            ////////////////////////////////////////////////////
                            {
                                BYTE j;

                                if(OpenSocketInfo->status.bits.requestIsOpen == 0){
                                    //if there isn't a request already open
                                    //get the current time and the source address
                                    OpenSocketInfo->socketStart = MiWi_TickGet();
                                    OpenSocketInfo->ShortAddress1.v[0] = MAC_rxPckt->Payload[MIWI_HEADER_LEN-3];
                                    OpenSocketInfo->ShortAddress1.v[1] = MAC_rxPckt->Payload[MIWI_HEADER_LEN-2];

                                    //copy the long address
                                    for(j=0;j<MY_ADDRESS_LENGTH;j++){
                                        OpenSocketInfo->LongAddress1[j] = MAC_rxPckt->Payload[j+2+MIWI_HEADER_LEN];
                                    }
                                    //mark a request as open
                                    OpenSocketInfo->status.bits.requestIsOpen = 1;
                                    OpenSocketInfo->status.bits.matchFound = 0;

                                    #if ADDITIONAL_NODE_ID_SIZE > 0
                                        for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                                            OpenSocketInfo->AdditionalNodeID1[j] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+MY_ADDRESS_LENGTH+j];
                                        }
                                    #endif
                                }
                                else{
                                    //if there was already a request open

                                    if((MAC_rxPckt->Payload[MIWI_HEADER_LEN-3] == OpenSocketInfo->ShortAddress1.v[0]) &&
                                       (MAC_rxPckt->Payload[MIWI_HEADER_LEN-2] == OpenSocketInfo->ShortAddress1.v[1]))
                                    {
                                        //if we get a request from the same guy twice,
                                        //just throw away the second request
                                        //ConsolePutROMString((ROM char*)"got request from same guy twice\r\n");
                                    }
                                    else if(OpenSocketInfo->ShortAddress1.Val == 0x0000){
                                        //I am the PAN coordinator and I initiated the request so don't send back a 
                                        //response to myself. Copy the long and short address from the Rx Buffer
                                        for(j=0;j<MY_ADDRESS_LENGTH;j++){
                                            OpenSocketInfo->LongAddress2[j] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+j];      //Data byte
                                            TempNodeInfo->longAddr[j] = OpenSocketInfo->LongAddress2[j];
                                        }

                                        //save the important information about the node sending the request
                                        OpenSocketInfo->ShortAddress2.v[0] = MAC_rxPckt->Payload[MIWI_HEADER_LEN-3];
                                        OpenSocketInfo->ShortAddress2.v[1] = MAC_rxPckt->Payload[MIWI_HEADER_LEN-2];

                                        TempNodeInfo->shortAddr.v[0] = OpenSocketInfo->ShortAddress2.v[0];
                                        TempNodeInfo->shortAddr.v[1] = OpenSocketInfo->ShortAddress2.v[1];
                                        TempNodeInfo->PANID.Val = myPANID->Val;

                                        //look to see if that device exists in our network table

                                        //OpenSocketInfo->socketHandle = SearchForShortAddress(mb); //Juan: Original
                                        //Juan: Since now the node can be registered in connection table with another
                                        //MiWi interface, let's look it up for its long address, which is unique.
                                        OpenSocketInfo->socketHandle = SearchForLongAddress(mb);
                                        if(OpenSocketInfo->socketHandle == 0xFF){                       //Juan
                                            Printf("\r\nLongAddr not found.");                            //Juan
                                            OpenSocketInfo->socketHandle = SearchForShortAddress(mb);   //Juan
                                        }

                                        if(OpenSocketInfo->socketHandle == 0xFF){
                                            Printf("\r\nShortAddr not found.");                           //Juan
                                            //otherwise create it
                                            //Family, RxOnWHenIdle, Neighbor/Network, P2P, ShortVal, LongVal, Direct, Valid
                                            //Juan: original value for original struct: 0x8D
                                            //Equivalent new value 'CONNSTAT_DFLT_DC????'
                                            switch(transceiver){
                                                case 1:
                                                    TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_0434;
                                                    break;
                                                case 2:
                                                    TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_0868;
                                                    break;
                                                case 3:
                                                    TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_2400;
                                                    break;
                                                default:
                                                    return;
                                            }

                                            if (TempNodeInfo->shortAddr.v[0] & 0x80){
                                                TempNodeInfo->status.bits.RXOnWhenIdle = 0;
                                            }

                                            OpenSocketInfo->socketHandle = AddNodeToNetworkTable(mb);
                                            if(OpenSocketInfo->socketHandle < 0xFF){
                                                #if ADDITIONAL_NODE_ID_SIZE > 0
                                                    for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                                                        ConnectionTable[OpenSocketInfo->socketHandle].PeerInfo[j] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+MY_ADDRESS_LENGTH+j];
                                                    }
                                                #endif
                                                #if defined(ENABLE_NETWORK_FREEZER)
                                                    nvmPutConnectionTableIndex(&(ConnectionTable[openSocketInfo->socketHandle]), openSocketInfo->socketHandle);
                                                #endif
                                            }
                                        }

                                        OpenSocketInfo->status.bits.requestIsOpen = 1;
                                        OpenSocketInfo->status.bits.matchFound = 1;
                                    }
                                    else{
                                        //Juan: I'm the PAN Coordinator, but I haven't initiated the request
                                        //so I have to send a response.
                                        ConsolePutROMString((ROM char*)"sending out response\r\n");

                                        //we got a match so let's send out the response

                                        MiApp_FlushTx(mb);
                                        MiApp_WriteData(MIWI_STACK_REPORT_TYPE, mb);    //Report Type
                                        MiApp_WriteData(OPEN_SOCKET_RESPONSE, mb);      //Report ID

                                        //copy the long and short address from the Rx Buffer
                                        for(j=0;j<MY_ADDRESS_LENGTH;j++){
                                            OpenSocketInfo->LongAddress2[j] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+j];      //Data byte
                                            MiApp_WriteData(OpenSocketInfo->LongAddress2[j], mb);
                                        }

                                        OpenSocketInfo->ShortAddress2.v[0] = MAC_rxPckt->Payload[MIWI_HEADER_LEN-3];
                                        OpenSocketInfo->ShortAddress2.v[1] = MAC_rxPckt->Payload[MIWI_HEADER_LEN-2];
                                        /*#if ADDITIONAL_NODE_ID_SIZE > 0
                                            for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                                                OpenSocketInfo->AdditionalNodeID2[j] = MAC_rxPckt->Payload[MIWI_HEADER_LEN+10+j];
                                            }
                                        #endif
                                        */

                                        MiApp_WriteData(OpenSocketInfo->ShortAddress2.v[0], mb);
                                        MiApp_WriteData(OpenSocketInfo->ShortAddress2.v[1], mb);
                                        #if ADDITIONAL_NODE_ID_SIZE > 0
                                            for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                                                MiApp_WriteData(MAC_rxPckt->Payload[MIWI_HEADER_LEN+2+MY_ADDRESS_LENGTH+j], mb);
                                            }
                                        #endif

                                        MiApp_UnicastAddress(OpenSocketInfo->ShortAddress1.v, FALSE, FALSE, mb);
                                        OpenSocketInfo->status.bits.requestIsOpen = 1;
                                        OpenSocketInfo->status.bits.matchFound = 1;
                                    }
                                }
                                break;
                            }
                            #endif

                            ////////////////////////////////////////////////////
                            case OPEN_SOCKET_RESPONSE:
                            ////////////////////////////////////////////////////
                            {
                                BYTE j;

                                //save the long address of the device
                                for(j=0;j<MY_ADDRESS_LENGTH;j++){
                                    TempNodeInfo->longAddr[j] = MAC_rxPckt->Payload[j+13];
                                }

                                //create a new status for the device
                                switch(transceiver){
                                    case 1:
                                        TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_0434;
                                        break;
                                    case 2:
                                        TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_0868;
                                        break;
                                    case 3:
                                        TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_2400;
                                        break;
                                    default:
                                        return;
                                }
                                
                                //check the RxOnWhenIdle bit
                                if(MAC_rxPckt->Payload[13+MY_ADDRESS_LENGTH] & 0x80){
                                    TempNodeInfo->status.bits.RXOnWhenIdle = 0;
                                }

                                //copy the short address
                                TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->Payload[MY_ADDRESS_LENGTH+13];
                                TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->Payload[MY_ADDRESS_LENGTH+14];

                                //use my PANID since this version of MiWi only supports single PANIDs
                                TempNodeInfo->PANID.Val = myPANID->Val;

                                //see if this node already exists in my table

                                //OpenSocketInfo->socketHandle = SearchForShortAddress(mb); //Juan: Original
                                //Juan: Since now the node can be registered in connection table with another
                                //MiWi interface, let's look it up for its long address, which is unique.
                                OpenSocketInfo->socketHandle = SearchForLongAddress(mb);
                                if(OpenSocketInfo->socketHandle == 0xFF){                       //Juan
                                    Printf("\r\nLongAddr not found.");                            //Juan
                                    OpenSocketInfo->socketHandle = SearchForShortAddress(mb);   //Juan
                                }

                                if(OpenSocketInfo->socketHandle != 0xFF){
                                    Printf("\r\nShortAddr found.");                           //Juan
                                    //if it does then get the status of the node already in the table

                                    //JUAN: WE MUST UPDATE, not OVERWRITE, the connection status.
                                    TempNodeInfo->status.Val |= ConnectionTable[OpenSocketInfo->socketHandle].status.Val;   //Juan: |= instead of =
                                    TempNodeInfo->status.bits.longAddressValid = 1;
                                    switch (mb){
                                        case ISM_434:
                                            TempNodeInfo->status.bits.short0434AddrValid = 1;
                                            break;
                                        case ISM_868:
                                            TempNodeInfo->status.bits.short0868AddrValid = 1;
                                            break;
                                        case ISM_2G4:
                                            TempNodeInfo->status.bits.short2400AddrValid = 1;
                                            break;
                                        default:
                                            return;
                                    }
                                }
                                else{
                                    //Family, RxOnWHenIdle, Neighbor/Network, P2P, ShortVal, LongVal, Direct, Valid
                                    //tempNodeStatus.Val = 0x8D;    //Juan: original value.
                                    //Juan: new value depends on RI. Also flags have been rearranged.
                                    switch (mb){
                                        case ISM_434:
                                            TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_0434;
                                            break;
                                        case ISM_868:
                                            TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_0868;
                                            break;
                                        case ISM_2G4:
                                            TempNodeInfo->status.Val = CONNSTAT_DFLT_ON_2400;
                                            break;
                                        default:
                                            return;
                                    }
                                    if(TempNodeInfo->shortAddr.v[0] & 0x80){
                                        TempNodeInfo->status.bits.RXOnWhenIdle = 0;
                                    }
                                }
                                //update the network information
                                OpenSocketInfo->socketHandle = AddNodeToNetworkTable(mb);
                                if(OpenSocketInfo->socketHandle < 0xFF){
                                    #if ADDITIONAL_NODE_ID_SIZE > 0
                                        for(j = 0; j < ADDITIONAL_NODE_ID_SIZE; j++){
                                            ConnectionTable[OpenSocketInfo->socketHandle].PeerInfo[j] = MAC_rxPckt->Payload[MY_ADDRESS_LENGTH+4+MIWI_HEADER_LEN+j];
                                        }
                                    #endif
                                    #if defined(ENABLE_NETWORK_FREEZER)
                                        nvmPutConnectionTableIndex(&(ConnectionTable[OpenSocketInfo->socketHandle]), openSocketInfo.socketHandle);
                                    #endif
                                    OpenSocketInfo->status.bits.matchFound = 1;
                                }
                                else{
                                    OpenSocketInfo->status.bits.matchFound = 0;
                                }

                                //clear out out request
                                OpenSocketInfo->status.bits.requestIsOpen = 0;

                            }
                            break;


                            #if defined(ENABLE_FREQUENCY_AGILITY)
                            ////////////////////////////////////////////////////
                            case CHANNEL_HOPPING_REQUEST:
                            ////////////////////////////////////////////////////
                            {
                                if(MAC_rxPckt->Payload[MIWI_HEADER_LEN+2] != *currentChannel){
                                    break;
                                }
                                MiApp_SetChannel(MAC_rxPckt->Payload[MIWI_HEADER_LEN+3], mb);
                                Printf("\r\nHopping Channel to ");
                                PrintDec(*currentChannel - ChannelOffset);
                            }
                            break;
                            #endif

                            #if defined(NWK_ROLE_COORDINATOR) && defined(ENABLE_FREQUENCY_AGILITY)
                            ////////////////////////////////////////////////////
                            case RESYNCHRONIZATION_REQUEST:
                            ////////////////////////////////////////////////////
                            {
                                if(MAC_rxPckt->Payload[MIWI_HEADER_LEN+2] != *currentChannel){
                                    break;
                                }

                                *(pbuffer)      = defaultHops;
                                *(pbuffer + 1)  = 0x02;
                                *(pbuffer + 2)  = myPANID->v[0];
                                *(pbuffer + 3)  = myPANID->v[1];
                                *(pbuffer + 4)  = MAC_rxPckt->Payload[8];
                                *(pbuffer + 5)  = MAC_rxPckt->Payload[9];
                                *(pbuffer + 6)  = myPANID->v[0];
                                *(pbuffer + 7)  = myPANID->v[1];
                                *(pbuffer + 8)  = myShortAddress->v[0];
                                *(pbuffer + 9)  = myShortAddress->v[1];
                                *(pbuffer + 10) = *(MiWiSeqNum)++;
                                *(pbuffer + 11) = MIWI_STACK_REPORT_TYPE;
                                *(pbuffer + 12) = RESYNCHRONIZATION_RESPONSE;
                                *TxData = 13;

                                MTP.flags.Val = 0;
                                MTP.flags.bits.ackReq = 1;
                                //Juan: added transceiver condition.
                                if (transceiver == 3){
                                    #if defined(IEEE_802_15_4)
                                        MTP.altDestAddr = TRUE;
                                        MTP.altSrcAddr = TRUE;
                                        MTP.DestPANID.Val = myPANID->Val;
                                        MTP.DestAddress = &(MAC_rxPckt->Payload[8]);
                                    #endif
                                }else{
                                    TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->Payload[8];
                                    TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->Payload[9];
                                    i = SearchForShortAddress(mb);
                                    if(i == 0xFF){
                                        break;
                                    }
                                    MTP.DestAddress = ConnectionTable[i].Address;
                                }

                                AuxMAC_SendPacket(&MTP, transceiver);
                            }
                            break;
                            #endif

                            #if defined(ENABLE_FREQUENCY_AGILITY)
                            ////////////////////////////////////////////////////
                            case RESYNCHRONIZATION_RESPONSE:
                            ////////////////////////////////////////////////////
                            {
                                msm->bits.Resynning = 0;
                                break;
                            }
                            #endif

                            ////////////////////////////////////////////////////
                            default:
                            ////////////////////////////////////////////////////
                                //unknown stack request.  don't do anything
                                //just let this packet die
                                break;
                        }
                    }
                    else{
                        // in case receive acknowledgement
                        if(*AckSeqNum == MAC_rxPckt->Payload[MIWI_HEADER_LEN-1] &&
                            sourceShortAddress.Val == AckAddr->Val){

                            msm->bits.MiWiAckInProgress = 0;
                        }
                        break;
                    }
                }
                else{
                    //This data is for the user, pass it up to them
                    #if defined(ENABLE_SLEEP)
                        msm->bits.DataRequesting = 0;
                    #endif
                    MIWI_rxMsg->PayloadSize = MAC_rxPckt->PayloadLen - 11;
                    MIWI_rxMsg->Payload = &MAC_rxPckt->Payload[11];
                    MIWI_rxMsg->SourcePANID.Val = sourcePANID.Val;
                    if(MAC_rxPckt->Payload[8] == 0xFF && MAC_rxPckt->Payload[9] == 0xFF){
                        //Juan: if source short address is 0xFFFF

                        //Juan: added transceiver condition.
                        if(transceiver == 3){
                            #if defined(IEEE_802_15_4)
                                MIWI_rxMsg->flags.bits.altSrcAddr = MAC_rxPckt->altSourceAddress;
                                MIWI_rxMsg->SourceAddress = MAC_rxPckt->SourceAddress;
                            #endif
                        } else{
                            if(MAC_rxPckt->flags.bits.sourcePrsnt){
                                MIWI_rxMsg->SourceAddress = MAC_rxPckt->SourceAddress;
                            }
                            else{
                                MIWI_rxMsg->flags.bits.altSrcAddr = 1;
                                MIWI_rxMsg->SourceAddress = &(MAC_rxPckt->Payload[8]);
                            }
                        }
                    }
                    else{
                        MIWI_rxMsg->flags.bits.altSrcAddr = 1;
                        MIWI_rxMsg->SourceAddress = &(MAC_rxPckt->Payload[8]);
                    }
                    MIWI_rxMsg->flags.bits.srcPrsnt = 1;

                    if(MIWI_rxMsg->PayloadSize > 0){
                        msm->bits.RxHasUserData = 1;
                    }
                }
            }
            else{
                //if this packet wasn't for me
                #ifdef NWK_ROLE_COORDINATOR
                    //then if I am a coordinator pass it along to the
                    //next hop, decrementing the number of hops available
                    if(MAC_rxPckt->Payload[0] > 0){
                        MAC_rxPckt->Payload[0]--;      //decrement the hops counter
                        AuxMAC_FlushTx(transceiver);
                        for(i = 0; i < MAC_rxPckt->PayloadLen; i++){
                            MiApp_WriteData(MAC_rxPckt->Payload[i], mb);
                        }

                        if((destShortAddress.v[1] == myShortAddress->v[1]) &&
                           (*(pbuffer + 11) == MIWI_STACK_REPORT_TYPE) &&
                           (*(pbuffer + 12) == ACK_REPORT_TYPE)){

                            MTP.flags.Val = 0;
                            MTP.flags.bits.ackReq = 1;

                            //Juan: added transceiver condition.
                            if(transceiver == 3){
                                #if defined(IEEE_802_15_4)
                                    MTP.altDestAddr = TRUE;
                                    MTP.altSrcAddr = TRUE;
                                    MTP.DestAddress = destShortAddress.v;
                                    MTP.DestPANID.Val = destPANID.Val;
                                    AuxMAC_SendPacket(&MTP, transceiver);
                                #endif
                            }else{
                                TempNodeInfo->shortAddr.Val = destShortAddress.Val;
                                if((i = SearchForShortAddress(mb)) != 0xFF){
                                    MTP.DestAddress = ConnectionTable[i].Address;
                                    AuxMAC_SendPacket(&MTP, transceiver);
                                }
                            }
                        }
                        else{
                            RouteMessage(destPANID, destShortAddress, MAC_rxPckt->flags.bits.secEn, mb);
                        }
                    }
                #endif
            }
        }
        break;

        ////////////////////////////////////////////////////////////////////////
        case PACKET_TYPE_BEACON:                    //if it is a beacon packet
        ////////////////////////////////////////////////////////////////////////
        {
            Printf("\r\nI have received a beacon answer");
            BYTE rxIndex;
            //Juan: added transceiver condition.
            if(transceiver == 3){
                #if defined(IEEE_802_15_4)
                    rxIndex = 0;
                #endif
            }else{
                rxIndex = 4;
            }

            if (MAC_rxPckt->Payload[rxIndex+4] != MIWI_PROTOCOL_ID){
                #if defined(NWK_ROLE_END_DEVICE)
                    if(MAC_rxPckt->Payload[rxIndex+4] != MIWI_PRO_PROTOCOL_ID)
                #endif
                {
                    break;
                }
            }

            #ifdef NWK_ROLE_COORDINATOR
            {
                BYTE coordinatorNumber;
                //if I am a coordinator then get the coordinators number
                //Juan: Added transceiver condition.
                if(transceiver == 3){
                    #if defined(IEEE_802_15_4)
                        coordinatorNumber = MAC_rxPckt->SourceAddress[1];
                        Printf("\r\n\n\n COORDINATOR NUMBER");
                        PrintDec(coordinatorNumber);
                        Printf("\r\n");
                    #endif
                }else{
                    coordinatorNumber = MAC_rxPckt->Payload[3];
                }
                BYTE mask = 1<<(coordinatorNumber);

                //Make sure its a MiWi coordinator
                //Juan: modified to add transceiver condition.
                switch(mb){
                    case ISM_434:
                    case ISM_868:
                        if(MAC_rxPckt->Payload[2] == 0x00){
                            //if it is then mark this device as known
                            *knownCoordinators |= mask;
                        }
                        break;
                    case ISM_2G4:
                        #if defined(IEEE_802_15_4)
                            if(MAC_rxPckt->SourceAddress[0] == 0x00){
                                //if it is then mark this device as known
                                *knownCoordinators |= mask;
                            }
                        #endif
                        break;
                    default:
                        break;
                }

                RoutingTable[coordinatorNumber] = MAC_rxPckt->Payload[rxIndex+6];
                #if defined(ENABLE_NETWORK_FREEZER)
                    msm->bits.saveConnection = 1;
                #endif
            }
            #endif

            //#if !defined(IEEE_802_15_4) && defined(NWK_ROLE_COORDINATOR) //Juan: this was the original code.
            #if defined(NWK_ROLE_COORDINATOR)
            if (transceiver != 3)   //Juan: this is my modification.
            {
                BYTE entry;

                TempNodeInfo->PANID.v[0] = MAC_rxPckt->Payload[0];
                TempNodeInfo->PANID.v[1] = MAC_rxPckt->Payload[1];
                TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->Payload[2];
                TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->Payload[3];

                entry = SearchForShortAddress(mb);
                if(entry == 0xFF){
                    //this device doesn't already exist in our network table
                    //let's create a new entry for it.
                    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                        TempNodeInfo->longAddr[i] = MAC_rxPckt->SourceAddress[i];
                        //PrintChar(TempNodeInfo->longAddr[i]);
                    }
                    entry = SearchForLongAddress(mb);
                    if (entry == 0xFF){
                        entry = findNextNetworkEntry();
                    }
                }
                if (entry == 0xFF)
                {
                    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                        if(ConnectionTable[entry].Address[i] != MAC_rxPckt->SourceAddress[i]){
                            break;
                        }
                    }
                    if(i >= MY_ADDRESS_LENGTH){
                        entry = 0xFF;
                    }
                }

                if(entry < 0xFF){
                    //Juan: modified, rearranged.
                    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                        ConnectionTable[entry].Address[i] = MAC_rxPckt->SourceAddress[i];
                    }
                 
                    //mark this as a valid entry
                    //ConnectionTable[entry].status.Val = 0;   //Pisa el de 434!!  
                    ConnectionTable[entry].status.bits.RXOnWhenIdle = 1;
                    ConnectionTable[entry].status.bits.longAddressValid = 1;
                    ConnectionTable[entry].status.bits.isValid = 1;
                    switch (mb){
                        case ISM_434:
                            #if defined MIWI_0434_RI
                                ConnectionTable[entry].PAN0434ID.Val = TempNodeInfo->PANID.Val;
                                ConnectionTable[entry].MIWI0434AltAddress.Val = TempNodeInfo->shortAddr.Val;
                                ConnectionTable[entry].status.bits.direct0434Conn = 1;
                                ConnectionTable[entry].status.bits.short0434AddrValid = 1;
                            #endif
                            break;
                        case ISM_868:
                            #if defined MIWI_0868_RI
                                ConnectionTable[entry].PAN0868ID.Val = TempNodeInfo->PANID.Val;
                                ConnectionTable[entry].MIWI0868AltAddress.Val = TempNodeInfo->shortAddr.Val;
                                ConnectionTable[entry].status.bits.direct0868Conn = 1;
                                ConnectionTable[entry].status.bits.short0868AddrValid = 1;
                            #endif
                            break;
                        case ISM_2G4:
                            Printf("\r\nJuan: This should not happen.\r");
//                            #if defined MIWI_2400_RI
//                                ConnectionTable[entry].PAN2400ID.Val = TempNodeInfo->PANID.Val;
//                                ConnectionTable[entry].MIWI2400AltAddress.Val = TempNodeInfo->shortAddr.Val;
//                                ConnectionTable[entry].status.bits.direct2400Conn = 1;
//                                ConnectionTable[entry].status.bits.short2400AddrValid = 1;
//                            #endif
                            break;
                        default:
                            return;
                    }
                    
                    #if defined(ENABLE_NETWORK_FREEZER)
                        msm->bits.saveConnection = 1;
                    #endif
                }
            }
            #endif

            #if defined(ENABLE_NETWORK_FREEZER)
                if(msm->bits.saveConnection == 1){
                    nvmDelayTick = MiWi_TickGet();
                }
            #endif

            //if we are looking for a network
            if(msm->bits.searchingForNetwork){
                //Juan: added transceiver condition.
                if(transceiver == 3){
                    #if defined(IEEE_802_15_4)
                        TempNodeInfo->PANID.Val = MAC_rxPckt->SourcePANID.Val;
                        TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->SourceAddress[0];
                        TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->SourceAddress[1];
                    #endif
                }else {
                    TempNodeInfo->PANID.v[0] = MAC_rxPckt->Payload[0];
                    TempNodeInfo->PANID.v[1] = MAC_rxPckt->Payload[1];
                    TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->Payload[2];
                    TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->Payload[3];
                }

                //ignore all beacon networks
                if(MAC_rxPckt->Payload[rxIndex] == 0xFF){
                    MIWI_CAPACITY_INFO CapacityByte;
                    CapacityByte.Val = MAC_rxPckt->Payload[rxIndex+1];
                    if((ActiveScanResultIndex < ACTIVE_SCAN_RESULT_SIZE) && (CapacityByte.bits.ConnMode <= ENABLE_PREV_CONN)){

                        ActiveScanResults[ActiveScanResultIndex].Channel = *currentChannel;
                        ActiveScanResults[ActiveScanResultIndex].RSSIValue = MIWI_rxMsg->PacketRSSI;
                        ActiveScanResults[ActiveScanResultIndex].LQIValue = MIWI_rxMsg->PacketLQI;
                        ActiveScanResults[ActiveScanResultIndex].PANID.Val = TempNodeInfo->PANID.Val;

                        ActiveScanResults[ActiveScanResultIndex].Capability.Val = 0;
                        ActiveScanResults[ActiveScanResultIndex].Capability.bits.Direct = 1;
                        ActiveScanResults[ActiveScanResultIndex].Capability.bits.Role = CapacityByte.bits.Role;
                        if(CapacityByte.bits.Security){
                            ActiveScanResults[ActiveScanResultIndex].Capability.bits.SecurityEn = 1;
                        }
                        if(CapacityByte.bits.ConnMode <= ENABLE_PREV_CONN){
                            ActiveScanResults[ActiveScanResultIndex].Capability.bits.AllowJoin = 1;
                        }

                        if(transceiver == 3){
                            #if defined(IEEE_802_15_4)
                                ActiveScanResults[ActiveScanResultIndex].Address[0] = MIWI_rxMsg->SourceAddress[0];
                                ActiveScanResults[ActiveScanResultIndex].Address[1] = MIWI_rxMsg->SourceAddress[1];
                                ActiveScanResults[ActiveScanResultIndex].Capability.bits.altSrcAddr = 1;
                            #endif
                        }else{
                            for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                                ActiveScanResults[ActiveScanResultIndex].Address[i] = MIWI_rxMsg->SourceAddress[i];
                            }
                        }
                        #if ADDITIONAL_NODE_ID_SIZE > 0
                            for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                                ActiveScanResults[ActiveScanResultIndex].PeerInfo[i] = MAC_rxPckt->Payload[rxIndex+7+i];
                            }
                        #endif
                        ActiveScanResultIndex++;
                    }
                }
            }
        }
        break;

        ////////////////////////////////////////////////////////////////////////
        case PACKET_TYPE_COMMAND:                   //if it is a command packet
        ////////////////////////////////////////////////////////////////////////
HANDLE_COMMAND_PACKET:
            //figure out which command packet it is
            switch(MAC_rxPckt->Payload[0]){
                #ifdef NWK_ROLE_COORDINATOR
                ////////////////////////////////////////////////////////////////
                case MAC_COMMAND_ASSOCIATION_REQUEST:
                ////////////////////////////////////////////////////////////////
                {
                    BYTE handle;
                    BYTE associateStatus = ASSOCIATION_SUCCESSFUL;

                    if(ConnMode > ENABLE_PREV_CONN){
                        break;
                    }

                    //get the long address of the device
                    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                        TempNodeInfo->longAddr[i] = MAC_rxPckt->SourceAddress[i];
                    }

                    handle = SearchForLongAddress(mb);
                    if(handle == 0xFF){
                        if(ConnMode > ENABLE_ALL_CONN){
                            break;
                        }

                        //Juan: original tempNodeStatus = 0xB7;
                        switch(mb){
                            case ISM_434:
                                TempNodeInfo->status.Val = CONNSTAT_NW0434_ASREQ;
                                break;
                            case ISM_868:
                                TempNodeInfo->status.Val = CONNSTAT_NW0868_ASREQ;
                                break;
                            case ISM_2G4:
                                TempNodeInfo->status.Val = CONNSTAT_NW2400_ASREQ;
                                break;
                            default:
                                return;
                        }

                        TempNodeInfo->PANID.Val = myPANID->Val;

                        //add this node to the network table
                        handle = AddNodeToNetworkTable(mb);
                        if(handle == 0xFF){
                            associateStatus = ASSOCIATION_PAN_FULL;
                            TempNodeInfo->shortAddr.Val = 0xFFFE;
                            goto START_ASSOCIATION_RESPONSE;
                        }
                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutConnectionTableIndex(&(ConnectionTable[handle]), handle);
                        #endif
                    }
                    else{
                        switch(mb){
                            case ISM_434:
                                #if defined MIWI_0434_RI
                                    if(ConnectionTable[handle].status.bits.short0434AddrValid){
                                        TempNodeInfo->shortAddr.Val = ConnectionTable[handle].MIWI0434AltAddress.Val;
                                        goto START_ASSOCIATION_RESPONSE;
                                    }
                                    break;
                                #else
                                    return;
                                #endif
                            case ISM_868:
                                #if defined MIWI_0868_RI
                                    if(ConnectionTable[handle].status.bits.short0868AddrValid){
                                        TempNodeInfo->shortAddr.Val = ConnectionTable[handle].MIWI0868AltAddress.Val;
                                        goto START_ASSOCIATION_RESPONSE;
                                    }
                                    break;
                                #else
                                    return;
                                #endif
                            case ISM_2G4:
                                #if defined MIWI_2400_RI
                                    if(ConnectionTable[handle].status.bits.short2400AddrValid){
                                        TempNodeInfo->shortAddr.Val = ConnectionTable[handle].MIWI2400AltAddress.Val;
                                        goto START_ASSOCIATION_RESPONSE;
                                    }
                                    break;
                                #else
                                    return;
                                #endif
                            default:
                                return;
                        }
                    }

                    //if entry was successful then assign the device a short address
                    if(handle != 0xFF){
                        //set the short address as an end device address
                        TempNodeInfo->shortAddr.v[0] = handle + 1;
                        TempNodeInfo->shortAddr.v[1] = myShortAddress->v[1];
                        TempNodeInfo->PANID.Val = myPANID->Val;

                        //if I am the PAN coordinator
                        if(role == ROLE_PAN_COORDINATOR){
                            if(MAC_rxPckt->Payload[1] & 0x40){
                                //If this device is a potential coordinator
                                BYTE j;
                                WORD_VAL CoordAddress;
                                BYTE entry;

                                CoordAddress.Val = TempNodeInfo->shortAddr.Val;
                                TempNodeInfo->shortAddr.v[0] = 0;

                                //search to see if there is a coordinator address available
                                for(j=1;j<8;j++){
                                    
                                    
                                    
                                    TempNodeInfo->shortAddr.v[1] = j;
                                    entry = SearchForShortAddress(mb);

                                    if(entry == 0xFF){
                                        TempNodeInfo->shortAddr.v[0] = 0x00;
                                        TempNodeInfo->shortAddr.v[1] = j;
                                        *knownCoordinators |= (1<<j);
                                        //SendBeacon(mb);
                                        #if defined(ENABLE_NETWORK_FREEZER)
                                            nvmPutKnownCoordinators(&knownCoordinators);
                                        #endif
                                        break;
                                    }
                                }

                                if(j==8){
                                    TempNodeInfo->shortAddr.Val= CoordAddress.Val;
                                }
                            }
                        }

                        //create a new status entry for the device
                        //tempNodeStatus.Val = 0xBF;    //Juan: original value.

                        switch(mb){
                            case ISM_434:
                                TempNodeInfo->status.Val = CONNSTAT_NW0434_ASRESP;
                                break;
                            case ISM_868:
                                TempNodeInfo->status.Val = CONNSTAT_NW0868_ASRESP;
                                break;
                            case ISM_2G4:
                                TempNodeInfo->status.Val = CONNSTAT_NW2400_ASRESP;
                                break;
                            default:
                                return;
                        }

                        //mark if the Rx is on when idle
                        if((MAC_rxPckt->Payload[1] & 0x01) == 0){
                            TempNodeInfo->status.bits.RXOnWhenIdle = 0;
                            TempNodeInfo->shortAddr.v[0] |= 0x80;
                        }

                        //update the information in the network table
                        handle = AddNodeToNetworkTable(mb);

                        switch(mb){
                            case ISM_434:
                                ConnectionTable[handle].status.bits.Finish0434Join = 1;
                                break;
                            case ISM_868:
                                ConnectionTable[handle].status.bits.Finish0868Join = 1;
                                break;
                            case ISM_2G4:
                                ConnectionTable[handle].status.bits.Finish2400Join = 1;
                                break;
                            default:
                                return;
                        }

                        #if ADDITIONAL_NODE_ID_SIZE > 0
                            for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                                ConnectionTable[handle].PeerInfo[i] = MAC_rxPckt->Payload[2+i];
                            }
                        #endif

                        if(MiApp_CB_AllowConnection(handle) == FALSE){
                            ConnectionTable[handle].status.Val = 0;
                            associateStatus = ASSOCIATION_ACCESS_DENIED;
                        }

                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutConnectionTableIndex(&(ConnectionTable[handle]), handle);
                        #endif
                    }

START_ASSOCIATION_RESPONSE:
                    //send back the asociation response
                    *(pbuffer)     = MAC_COMMAND_ASSOCIATION_RESPONSE;
                    *(pbuffer + 1) = TempNodeInfo->shortAddr.v[0];
                    *(pbuffer + 2) = TempNodeInfo->shortAddr.v[1];
                    *(pbuffer + 3) = associateStatus;
                    #if ADDITIONAL_NODE_ID_SIZE > 0
                        for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                            *(pbuffer + 4 + i) = AdditionalNodeID[i];
                        }
                    #endif
                    *TxData = 4 + ADDITIONAL_NODE_ID_SIZE;

                    #if defined(ENABLE_SECURITY)
                        IncomingFrameCounter[handle].Val = 0;
                    #endif

                    #if defined(IEEE_802_15_4)
                        SendMACPacket(myPANID->v, ConnectionTable[handle].Address, PACKET_TYPE_COMMAND, 0, mb);
                    #else
                        SendMACPacket(ConnectionTable[handle].Address, PACKET_TYPE_COMMAND, mb);
                    #endif
                }
                break;
                #endif
                ////////////////////////////////////////////////////////////////
                case MAC_COMMAND_ASSOCIATION_RESPONSE:
                ////////////////////////////////////////////////////////////////
                {
                    if(MAC_rxPckt->Payload[3] == ASSOCIATION_SUCCESSFUL){
                        //The join was successful
                        //assign myself the correct short address
                        myShortAddress->v[0] = MAC_rxPckt->Payload[1];
                        myShortAddress->v[1] = MAC_rxPckt->Payload[2];

                        #ifdef NWK_ROLE_COORDINATOR
                            if(myShortAddress->v[0] == 0x00){
                                ConsolePutROMString((ROM char*)"\rI am a coordinator\r\n");
                                role = ROLE_COORDINATOR;
                                CapacityInfo->bits.Role = role;
                                *knownCoordinators |= ((1<<(myShortAddress->v[1] & 0x07)) + 1);
                                //I know the PAN coordinator and myself
                                //SendBeacon(mb);
                            }
                            else{
                                ConsolePutROMString((ROM char*)"\rI am an end device\r\n");
                                role = ROLE_FFD_END_DEVICE;
                                CapacityInfo->bits.Role = role;
                            }
                            #if defined(ENABLE_NVM)
                                nvmPutRole(&role);
                            #endif
                        #endif

                        #if defined MRF24J40
                            if(transceiver == 3){
                                //set the short address of the device
                                MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                            }
                        #endif

                        //mark as having joined a network
                        msm->bits.memberOfNetwork = 1;

                        //record the parents information
                        switch(mb){
                            case ISM_434:
                                TempNodeInfo->status.Val = CONNSTAT_NW0434_ASRESP;
                                break;
                            case ISM_868:
                                TempNodeInfo->status.Val = CONNSTAT_NW0868_ASRESP;
                                break;
                            case ISM_2G4:
                                TempNodeInfo->status.Val = CONNSTAT_NW2400_ASRESP;
                                break;
                            default:
                                return;
                        }

                        for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                            TempNodeInfo->longAddr[i] = MAC_rxPckt->SourceAddress[i];
                        }

                        if(myShortAddress->v[0] == 0x00){
                            //if I am a coordinator then my parent is the PAN coordinator
                            //and I am a router
                            TempNodeInfo->shortAddr.Val = 0x0000;
                        }
                        else{
                            TempNodeInfo->shortAddr.v[1] = myShortAddress->v[1];
                            TempNodeInfo->shortAddr.v[0] = 0x00;
                        }

                        TempNodeInfo->PANID.Val = myPANID->Val;

                        //and add the parents information to the network table
                        *myParent = AddNodeToNetworkTable(mb);

                        #if defined(ENABLE_SECURITY)
                            IncomingFrameCounter[*myParent].Val = 0;
                        #endif

                        #if ADDITIONAL_NODE_ID_SIZE > 0
                            for(i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++){
                            ConnectionTable[*myParent].PeerInfo[i] = MAC_rxPckt->Payload[4+i];
                        }
                        #endif

                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutMyPANID(myPANID->v);
                            nvmPutMyShortAddress(myShortAddress->v);
                            nvmPutMyParent(&myParent);
                            nvmPutConnectionTableIndex(&(ConnectionTable[*myParent]), *myParent);
                            #if defined(NWK_ROLE_COORDINATOR)
                                nvmPutKnownCoordinators(&knownCoordinators);
                            #endif
                        #endif

                        ConsolePutROMString((ROM char *)"\r\nJoined to network successfully\r\n");
                        
                        
                    }
                    else{
                        //Access denied, remove this as my parent
                        *myParent = 0xFF;
                        myPANID->Val = 0xFFFF;
                        TempNodeInfo->shortAddr.Val = 0xFFFF;

                        //Juan: added conditions.
                        #if defined IEEE_802_15_4
                            if (mb == ISM_2G4){
                                MiMAC_SetAltAddress(TempNodeInfo->shortAddr.v, myPANID->v);
                            }
                        #endif
                        
                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutMyParent(&myParent);
                            nvmPutMyPANID(myPANID->v);
                            nvmPutMyShortAddress(myShortAddress->v);
                        #endif
                        
                        ConsolePutROMString((ROM char *)"\r\nAccess denied\r\n");
                    }
                }
                break;

                #if !defined(TARGET_SMALL)
                ////////////////////////////////////////////////////////////////
                case MAC_COMMAND_DISASSOCIATION_NOTIFICATION:
                ////////////////////////////////////////////////////////////////
                {
                    BYTE cIndex;

                    #if defined(IEEE_802_15_4)
                        if(MAC_rxPckt->altSourceAddress){
                            TempNodeInfo->shortAddr.v[0] = MAC_rxPckt->SourceAddress[0];
                            TempNodeInfo->shortAddr.v[1] = MAC_rxPckt->SourceAddress[1];

                            if((cIndex = SearchForShortAddress(mb)) == 0xFF){
                                break;
                            }
                        }
                        else
                    #endif
                    {
                        for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                            TempNodeInfo->longAddr[i] = MAC_rxPckt->SourceAddress[i];
                        }
                        if((cIndex = SearchForLongAddress(mb)) == 0xFF){
                            break;
                        }
                    }

                    BYTE aux;
                    switch(mb){
                        case ISM_434:
                            aux = ConnectionTable[cIndex].status.bits.is0434Family;
                            break;
                        case ISM_868:
                            aux = ConnectionTable[cIndex].status.bits.is0868Family;
                            break;
                        case ISM_2G4:
                            aux = ConnectionTable[cIndex].status.bits.is2400Family;
                            break;
                        default:
                            return;
                    }
                    //Juan: I use an auxiliar variable for isFamily assertion.
                    if(aux){
                        #if defined(NWK_ROLE_COORDINATOR)
                            if(cIndex != *myParent){     // it is from my child
                                ConnectionTable[cIndex].status.Val = 0;
                                #if defined(ENABLE_NETWORK_FREEZER)
                                    nvmPutConnectionTableIndex(&(ConnectionTable[cIndex]), cIndex);
                                #endif
                            }
                            else{                       // it is from my parent
                                // notify my children
                                for(i = 0; i < CONNECTION_SIZE; i++){
                                    if((ConnectionTable[i].status.bits.isValid)
                                       && (aux != 0) && (i != *myParent )){

                                        WORD j;

                                        AuxMAC_FlushTx(transceiver);
                                        MiApp_WriteData(MAC_COMMAND_DISASSOCIATION_NOTIFICATION, mb);
                                        MTP.flags.Val = 0;
                                        MTP.flags.bits.ackReq = 1;
                                        MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;

                                        //Juan: original code was:
                                        //#if defined IEEE_802_15_4 => ISM_2G4
                                        //#else => ISM_434, ISM_868 #endif
                                        switch(mb){
                                            case ISM_434:
                                            case ISM_868:
                                                MTP.flags.bits.sourcePrsnt = 1;
                                                MTP.DestAddress = ConnectionTable[i].Address;
                                                break;
                                            case ISM_2G4:
                                                #if defined(IEEE_802_15_4) && defined MIWI_2400_RI
                                                    MTP.altDestAddr = TRUE;
                                                    MTP.altSrcAddr = TRUE;
                                                    MTP.DestPANID.Val = ConnectionTable[i].PAN2400ID.Val;
                                                    MTP.DestAddress = ConnectionTable[i].MIWI2400AltAddress.v;
                                                #endif
                                                break;
                                            default:
                                                return;
                                        }

                                        AuxMAC_SendPacket(&MTP, transceiver);
                                        for(j = 0; j < 0xFFF; j++) {}   // delay
                                    }
                                    ConnectionTable[i].status.Val = 0;
                                }
                                *myParent = 0xFF;
                                myShortAddress->Val = 0xFFFF;
                                myPANID->Val = 0xFFFF;

                                //Juan: added conditions.
                                #if defined IEEE_802_15_4
                                    if (mb == ISM_2G4){
                                        MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                                    }
                                #endif
                                
                                msm->bits.memberOfNetwork = 0;
                                #if defined(ENABLE_NETWORK_FREEZER)
                                    nvmPutMyParent(&myParent);
                                    nvmPutMyShortAddress(myShortAddress->v);
                                    nvmPutMyPANID(myPANID->v);
                                    nvmPutConnectionTable(ConnectionTable);
                                #endif
                            }
                        #else
                            // it is from my parent
                            for(i = 0; i < CONNECTION_SIZE; i++){
                                ConnectionTable[i].status.Val = 0;
                            }
                            *myParent = 0xFF;
                            myShortAddress->Val = 0xFFFF;
                            myPANID->Val = 0xFFFF;
                                                            //Juan: added conditions.
                            #if defined IEEE_802_15_4
                                if (mb == ISM_2G4){
                                    MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                                }
                            #endif

                            msm->bits.memberOfNetwork = 0;
                            #if defined(ENABLE_NETWORK_FREEZER)
                                nvmPutMyParent(&myParent);
                                nvmPutMyShortAddress(myShortAddress->v);
                                nvmPutMyPANID(myPANID->v);
                            #endif
                         #endif
                    }
                }
                break;
                #endif

                #if defined(NWK_ROLE_COORDINATOR) && defined(ENABLE_INDIRECT_MESSAGE)
                ////////////////////////////////////////////////////////////////
                case MAC_COMMAND_DATA_REQUEST:
                ////////////////////////////////////////////////////////////////
                {
                    Printf("\r\n Me piden datos \r\n");
                    BYTE handle;

                    #if defined(IEEE_802_15_4)
                        if(MAC_rxPckt->altSourceAddress){
                            SendIndirectPacket(NULL, MAC_rxPckt->SourceAddress, TRUE, mb);
                        }
                        else{
                            SendIndirectPacket(MAC_rxPckt->SourceAddress, NULL, FALSE, mb);
                        }
                    #else
                        SendIndirectPacket(MAC_rxPckt->SourceAddress, &(MAC_rxPckt->Payload[1]), TRUE, mb);
                    #endif

                    #if defined(ENABLE_ENHANCED_DATA_REQUEST)
                        #if defined(IEEE_802_15_4)
                            if(MAC_rxPckt->PayloadLen > 1){
                                goto HANDLE_DATA_PACKET;
                            }
                        #else
                            if(MAC_rxPckt->PayloadLen > 3){
                                for(i = 1; i < MAC_rxPckt->PayloadLen-2; i++){
                                    MAC_rxPckt->Payload[i] = MAC_rxPckt->Payload[i+2];
                                }
                                MAC_rxPckt->PayloadLen -= 2;
                                goto HANDLE_DATA_PACKET;
                            }
                        #endif
                    #endif

                    break;
                }
                #endif

                #ifdef NWK_ROLE_COORDINATOR
                ////////////////////////////////////////////////////////////////
                case MAC_COMMAND_BEACON_REQUEST:
                ////////////////////////////////////////////////////////////////
                {
                    if(ConnMode > ENABLE_ACTIVE_SCAN_RSP){
                        break;
                    }

                    //if we are a member of a network
                    if(msm->bits.memberOfNetwork){
                        if(MAC_rxPckt->Payload[1] != *currentChannel){
                            break;
                        }

                        //send out a beacon as long as we are not
                        //currently acting as an FFD end device
                        if(role != ROLE_FFD_END_DEVICE){
                            #if !defined(TARGET_SMALL)
                                BroadcastJitter(20);
                            #endif
                            SendBeacon(mb);
                        }
                    }
                }
                break;
                #endif


                #if defined(ENABLE_TIME_SYNC) && defined(ENABLE_SLEEP)
                ////////////////////////////////////////////////////////////////
                case MAC_COMMAND_TIME_SYNC_DATA_PACKET:
                case MAC_COMMAND_TIME_SYNC_COMMAND_PACKET:
                ////////////////////////////////////////////////////////////////
                {
                    WakeupTimes->v[0]  = MAC_rxPckt->Payload[1];
                    WakeupTimes->v[1]  = MAC_rxPckt->Payload[2];
                    CounterValue->v[0] = MAC_rxPckt->Payload[3];
                    CounterValue->v[1] = MAC_rxPckt->Payload[4];

                    if(MAC_rxPckt->PayloadLen > 5){
                        if(MAC_rxPckt->Payload[0] == MAC_COMMAND_TIME_SYNC_DATA_PACKET){
                            for(i = 0; i < MAC_rxPckt->PayloadLen-5; i++){
                                MAC_rxPckt->Payload[i] = MAC_rxPckt->Payload[5+i];
                            }
                            MAC_rxPckt->PayloadLen -= 5;

                            goto HANDLE_DATA_PACKET;
                        }
                        else{
                            for(i = 0; i < MAC_rxPckt->PayloadLen-5; i++){
                                MAC_rxPckt->Payload[i] = MAC_rxPckt->Payload[5+i];
                            }
                            MAC_rxPckt->PayloadLen -= 5;

                            goto HANDLE_COMMAND_PACKET;
                        }
                    }
                    else{
                        msm->bits.DataRequesting = 0;
                    }
                }
                break;
                #endif

                default:
                    break;
            }
            break;
        ////////////////////////////////////////////////////////////////////////
        default:
        ////////////////////////////////////////////////////////////////////////
           break;

    }

    if(msm->bits.RxHasUserData == 0){
        #if defined MRF49XA_1
            if(transceiver == 1){
                MiMAC_MRF49XA_DiscardPacket(1);
            }else
        #endif
        #if defined MRF49XA_2
            if(transceiver == 2){
                MiMAC_MRF49XA_DiscardPacket(2);
            }else
        #endif
        #if defined MRF24J40
            if(transceiver == 3){
                MiMAC_MRF24J40_DiscardPacket();
            }
        #else
            {}
        #endif
    }
}

    #ifdef NWK_ROLE_COORDINATOR
        /***********************************************************************
         * Function:    BOOL RouteMessage(WORD_VAL PANID, WORD_VAL ShortAddress,
         *                                BOOL SecEn)
         * PreCondition:Coordinator has joined the network
         * Input:       PANID        - PAN identifier of the destination node
         *              ShortAddress - Network short address of the destination
         *                             node
         *              SecEn        - Boolean to indicate if the message
         *                             payload needs to be secured
         * Output:      A boolean to indicate if routing successful
         * Side Effects:None
         * Overview:    This function is the backbone of MiWi routing mechanism.
         *              MiWi use this function to route the message across
         *              multiple hops to reach the destination node.
         **********************************************************************/
        BOOL RouteMessage(WORD_VAL PANID, WORD_VAL ShortAddress, BOOL SecEn,\
                          miwi_band mb){
            BYTE *pbuffer, *TxData, *currentChannel, *myParent;
            BYTE transceiver;
            WORD_VAL *myShortAddress, *myPANID;
            TEMP_NODE_INFO *TempNodeInfo;
            BYTE *knownCoordinators;
            BYTE *RouterFailures;
            BYTE *RoutingTable;
            switch (mb){
                case ISM_434:
                    #if !defined MIWI_0434_RI
                        Printf("Error: MiWi ISM 434 MHz band is not available.");
                        return FALSE;
                    #else
                        #if defined MRF49XA_1_IN_434
                            pbuffer = MRF49XA_1_TxBuffer;
                            TxData = & MRF49XA_1_TxData;
                            transceiver = 1;
                        #elif defined MRF49XA_2_IN_434
                            pbuffer = MRF49XA_2_TxBuffer;
                            TxData = & MRF49XA_2_TxData;
                            transceiver = 2;
                        #else   //Error
                            return FALSE;
                        #endif
                        currentChannel = &MIWI0434_currentChannel;
                        TempNodeInfo = & temp0434;
                        myShortAddress = & myShort0434Addr;
                        myPANID = & myPAN0434ID;
                        myParent = & my0434Parent;
                        knownCoordinators = & known0434Coordinators;
                        RouterFailures = & Router0434Failures[0];
                        RoutingTable = & Routing0434Table[0];
                        break;
                    #endif
                case ISM_868:
                    #if !defined MIWI_0868_RI
                        Printf("Error: MiWi ISM 868 MHz band is not available.");
                        return FALSE;
                    #else
                        #if defined MRF49XA_1_IN_868
                            pbuffer = MRF49XA_1_TxBuffer;
                            TxData = & MRF49XA_1_TxData;
                            transceiver = 1;
                        #elif defined MRF49XA_2_IN_868
                            pbuffer = MRF49XA_2_TxBuffer;
                            TxData = & MRF49XA_2_TxData;
                            transceiver = 2;
                        #else   //Error
                            return FALSE;
                        #endif
                        currentChannel = &MIWI0868_currentChannel;
                        TempNodeInfo = & temp0868;
                        myShortAddress = & myShort0868Addr;
                        myPANID = & myPAN0868ID;
                        myParent = & my0868Parent;
                        knownCoordinators = & known0868Coordinators;
                        RouterFailures = & Router0868Failures[0];
                        RoutingTable = & Routing0868Table[0];
                        break;
                    #endif
                case ISM_2G4:
                    #if !defined MIWI_2400_RI
                        Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                        return FALSE;
                    #else
                        pbuffer = MRF24J40_TxBuffer;
                        TxData = & MRF24J40_TxData;
                        transceiver = 3;
                        currentChannel = &MIWI2400_currentChannel;
                        TempNodeInfo = & temp2400;
                        myShortAddress = & myShort2400Addr;
                        myPANID = & myPAN2400ID;
                        myParent = & my2400Parent;
                        knownCoordinators = & known2400Coordinators;
                        RouterFailures = & Router2400Failures[0];
                        RoutingTable = & Routing2400Table[0];
                        break;
                    #endif
                default:
                    return FALSE;
            }

            //Juan: Coordinators shortAddr are 0x0?00. There are 8 coordinators
            //max, so ? takes values from 0 to 7. 0x0000 is for PANC.
            //Coordinators are in charge of routing.
            BYTE parentNode = (ShortAddress.v[1] & 0x07);   //Juan: 0x0? MS byte
            BYTE i;
            BOOL ok;
            //JUAN: I'VE MODIFIED THIS FUNCTION: REARRANGING, SAVING AN INDIRECT
            //PACKET AFTER SENDING THE PACKET...
            
//**********************************************//
// CASE ROUTING TO MY CHILD
//**********************************************//           
            
           
                
            if(parentNode == myShortAddress->v[1]){
                Printf("\r\nROUTING TO MY CHILD");
                // destination is my child
                //Juan:  THIS IS THE FOLLOWING ROUTING CASE:
                //      (SourceNode) -----> (ME - coord) -----> (DestNode)
                
                
                MTP.flags.Val = 0;
                MTP.flags.bits.ackReq = 1;
                MTP.flags.bits.secEn = SecEn;        
                TempNodeInfo->shortAddr.Val = ShortAddress.Val;
                //Juan: transceiver condition
                if(transceiver == 3){
                    #if defined(IEEE_802_15_4)
                        MTP.altDestAddr = TRUE;
                        MTP.altSrcAddr = TRUE;
                        MTP.DestAddress = TempNodeInfo->shortAddr.v;
                        MTP.DestPANID.Val = myPANID->Val;
                    #endif
                }else{
                    if((i = SearchForShortAddress(mb)) != 0xFF){
                        MTP.DestAddress = ConnectionTable[i].Address;
                    }
                    else{
                        // the children's info is not in the connection table
                        // should not happen
                        return FALSE;
                    }
                }

                ok = AuxMAC_SendPacket(&MTP, transceiver);

                #if defined(ENABLE_INDIRECT_MESSAGE)
                if (ok == FALSE){
                    //JUAN: MODIFIED BEHAVIOUR!! IF ENABLED, TRY TO SAVE AN
                    //INDIRECT PACKET FOR IT ONLY IF TRANSMISSION FAILED.
                    if(ShortAddress.v[0] > 0x80){
                        //Juan: DestNode is OFF when idle (sleeping device)

                        // this is a sleeping device, need indirect message

                      //Juan: modified. Different function declaration depending
                      //on whether 802_15_4 is defined or not, and different
                      //behaviour for subGHz transceivers.
                        i = 0xFF;                       //Invalid index
                        if(transceiver != 3){           //subGHz transceivers
                            TempNodeInfo->shortAddr.Val = ShortAddress.Val;
                            i = SearchForShortAddress(mb);
                        }
                        if (i == 0xFF)
                        #if defined(IEEE_802_15_4)
                            {
                                //Juan: MRF24J40 transceiver (2,4 GHz) or subGHz
                                //transceivers using short address if node was
                                //unregistered for this MiWi interface.
                                if (SaveIndirectMessage(FALSE, PANID, ShortAddress.v, TRUE, SecEn, mb))
                                    {
                                        Printf("\r\n\nIndirect packet saved.");
                                    }
                                    else{
                                        Printf("\r\n\nIndirect packet couldn't be saved.");
                                    }
                            }
                            //SubGHz transceivers using long address, but with
                            //802.15.4 format.
                            if (SaveIndirectMessage(FALSE, PANID, ConnectionTable[i].Address, FALSE, SecEn, mb))
                                {
                                        Printf("\r\n\nIndirect packet saved.");
                                    }
                                    else{
                                        Printf("\r\n\nIndirect packet couldn't be saved.");
                                    }
                        #else
                            {
                                //SubGHz transceivers using short address if
                                //node was unregistered for this MiWi interface.
                                if (SaveIndirectMessage(FALSE, ShortAddress.v, TRUE, SecEn, mb))
                                    {
                                        Printf("\r\n\nIndirect packet saved.");
                                    }
                                    else{
                                        Printf("\r\n\nIndirect packet couldn't be saved.");
                                    }
                            }
                            //SubGHz transceivers using long address.
                            if (SaveIndirectMessage(FALSE, ConnectionTable[i].Address, FALSE, SecEn, mb))
                                {
                                        Printf("\r\n\nIndirect packet saved.");
                                    }
                                    else{
                                        Printf("\r\n\nIndirect packet couldn't be saved.");
                                    }
                        #endif
                    }
                }
                #endif
                return ok;
                //True if packet was correctly sent. It has nothing to do with
                //saving the indirect packet result.
            }
            else {
            /*******************************************************************
             * Juan: NOT MY CHILD. THESE INCLUDE THE FOLLOWING ROUTING CASES:
             *
             *A) I KNOW THE "DESTINATION COORDINATOR"
             *   (SrcNode) --> (ME - coord) --> (DestCoord) --> (DestNode)
             * 
             *B) I KNOW A COORDINATOR WHICH KNOWS THE "DESTINATION COORDINATOR"
             *   The code related is the "ROUTE_THROUGH_NEIGHBOR" part.
             *   (SrcNode) --> (ME - coord) --> (KnownCoord) --> (DestCoord) -->
             *   --> (DestNode)
             * 
             *C) I DON'T KNOW HOW TO ROUTE THIS MESSAGE.
             *C1)IF I'M NOT THE PANC I WILL SEND IT TO THE PANC OR TO MY PARENT
             *   Generally speaking, code used is from "ROUTE_THROUGH_TREE" part
             *   (SrcNode) ---> (ME - coord) ---> (PANC) ---> ????
             *C2)I'M PANC AND I DON'T KNOW ALL THE COORDINATORS. I'M FUCKED xD
             *   An "emergency plan" is followed...
             *
             * D) "STRANGE CASES" like being an end device :S
             ******************************************************************/
                
//**********************************************//
// CASE A KNOW THE DESTINATION
//**********************************************//   
                if((*knownCoordinators & (1 << parentNode)) > 0){
                    
                    LED1 = 1;
                    
                    Printf("\r\n ****** CASE ROUTING A  ******\n");
                    //Juan: This is the routing case A)
                    if(RouterFailures[parentNode] >= MAX_ROUTING_FAILURE){
                        //Juan: But if the destination coordinator is having 
                        //problems (is no longer avilable or whatever) and
                        //routing failures exceed the limit I'll remove it from
                        //the my knownCoordinators status, send a beacon and 
                        //return a failure.
                        RouterFailures[parentNode] = 0;
                        *knownCoordinators &= ((1<<parentNode) ^ 0xFF);
                        RoutingTable[parentNode] = 0;
                        
                        TempNodeInfo->shortAddr.v[0] = 0;
                        TempNodeInfo->shortAddr.v[1] = parentNode;
                        i = SearchForShortAddress(mb);
                        
                        //Clara: It is necessary to remove it also from the
                        //connection table to keep it updated and afford new
                        //connections
                        switch(mb){
                            case ISM_434: 
                                ConnectionTable[i].MiWiInterfaces &= 0xF7;
                                ConnectionTable[i].status.bits.direct0434Conn = 0;
                                ConnectionTable[i].status.bits.short0434AddrValid = 0;
                                break;
                            case ISM_868: 
                                ConnectionTable[i].MiWiInterfaces &= 0xFD;
                                ConnectionTable[i].status.bits.direct0868Conn = 0;
                                ConnectionTable[i].status.bits.short0868AddrValid = 0;
                                break;
                            case ISM_2G4: 
                                ConnectionTable[i].MiWiInterfaces &= 0xFB;
                                ConnectionTable[i].status.bits.direct2400Conn = 0;
                                ConnectionTable[i].status.bits.short2400AddrValid = 0;
                                break;
                            default:
                                Printf("\r\n Shouldn't be here");
                                break;
                        }
                        //if there is no valid interface remove the entry
                        if (ConnectionTable[i].status.bits.direct0434Conn == 0 && 
                              ConnectionTable[i].status.bits.direct0868Conn == 0  && 
                              ConnectionTable[i].status.bits.direct2400Conn == 0){
                            ConnectionTable[i].status.bits.isValid = 0;
                        }
                            
                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutKnownCoordinators(&knownCoordinators);
                            nvmPutRoutingTable(RoutingTable);
                        #endif
                        SendBeacon(mb);
                        SWDelay(1000);
                        LED1 = 0;
                        return FALSE;
                    }
                    else{
                        //Juan: we still trust in the destination coordinator
                        MTP.flags.Val = 0;
                        MTP.flags.bits.ackReq = 1;
                        MTP.flags.bits.secEn = SecEn;
                        TempNodeInfo->shortAddr.v[0] = 0;
                        TempNodeInfo->shortAddr.v[1] = parentNode;

                        if (transceiver == 3){
                            #if defined(IEEE_802_15_4)
                                MTP.altDestAddr = TRUE;
                                MTP.altSrcAddr = TRUE;
                                MTP.DestAddress = TempNodeInfo->shortAddr.v;
                                MTP.DestPANID.Val = myPANID->Val;
                            #endif
                        }else{
                            if((i = SearchForShortAddress(mb)) != 0xFF){
                                MTP.DestAddress = ConnectionTable[i].Address;
                            }
                            else{
                                //Juan: highly impossible? Qué cojones es eso?xD

                                // highly impossible for none 15.4, where the
                                // access of Coordinators not through beacon
                                
                                SWDelay(1000);
                                LED1 = 0;
                                goto ROUTE_THROUGH_NEIGHBOR;
                            }
                        }

                        if(!(AuxMAC_SendPacket(&MTP, transceiver))){
                            RouterFailures[parentNode]++;
                            Printf("\r\n RouterFailures: ");
                            PrintDec(RouterFailures[parentNode]);
                            
                            SWDelay(1000);
                            LED1 = 0;
                            return FALSE;
                        }
                        else{
                            RouterFailures[parentNode] = 0;
                            SWDelay(1000);
                            LED1 = 0;
                            return TRUE;
                        }
                    }
                    LED1 = 0;
                }
//**********************************************//
// CASE B INTERMEDIUM
//**********************************************// 
                else{
                    
                    //Juan: Here comes the routing cases B), C)

ROUTE_THROUGH_NEIGHBOR:
                    //Juan: Let's check if we know a coordinator which knows the
                    //destination coordinator.
                    for(i=0; i<8; i++){
                        if((RoutingTable[i] & (1 << parentNode)) > 0){
                            Printf("\r\n ****** CASE ROUTING B  ******\n");
                            LED3 = 1;
                            //Juan: This is routing case B)
                            //(SourceNode) --> (ME - coord) --> (Known Coord) -->
                            //--> (Dest. Coord) --> (DestNode)

                            if(RouterFailures[i] >= MAX_ROUTING_FAILURE){
                                //Juan: As before, there is a chance of finding
                                //out that the knownCoordinator which knows the
                                //destination coordinator is no longer reliable.

                                RouterFailures[i] = 0;
                                *knownCoordinators &= ((1<<i) ^ 0xFF);
                                RoutingTable[i] = 0;
                                
                                //Clara: It is necessary to remove it also from the
                                //connection table to keep it updated and afford new
                                //connections
                        switch(mb){
                            case ISM_434: 
                                ConnectionTable[i].MiWiInterfaces &= 0xF7;
                                ConnectionTable[i].status.bits.direct0434Conn = 0;
                                ConnectionTable[i].status.bits.short0434AddrValid = 0;
                                break;
                            case ISM_868: 
                                ConnectionTable[i].MiWiInterfaces &= 0xFD;
                                ConnectionTable[i].status.bits.direct0868Conn = 0;
                                ConnectionTable[i].status.bits.short0868AddrValid = 0;
                                break;
                            case ISM_2G4: 
                                ConnectionTable[i].MiWiInterfaces &= 0xFB;
                                ConnectionTable[i].status.bits.direct2400Conn = 0;
                                ConnectionTable[i].status.bits.short2400AddrValid = 0;
                                break;
                            default:
                                Printf("\r\n Shouldn't be here");
                                break;
                        }
                        //if there is no valid interface remove the entry
                        if ((ConnectionTable[i].MiWiInterfaces & 0x07) == 0){
                            ConnectionTable[i].status.bits.isValid = 0;
                        }
                                
                                
                                #if defined(ENABLE_NETWORK_FREEZER)
                                    nvmPutKnownCoordinators(&knownCoordinators);
                                    nvmPutRoutingTable(RoutingTable);
                                #endif
                                if(role != ROLE_FFD_END_DEVICE){
                                    SendBeacon(mb);
                                }
                                else{
                                    // send out beacon request
                                    *TxData = 0;
                                    MiApp_WriteData(MAC_COMMAND_BEACON_REQUEST, mb);
                                    MiApp_WriteData(*currentChannel, mb);
                                    #if defined(IEEE_802_15_4)
                                        TempNodeInfo->PANID.Val = 0xFFFF;
                                        //Juan: no matter that subGHz transceivers use
                                        //this function instead of the right below it.
                                        //Parameters PANID and modeMask will be ignored.
                                        SendMACPacket(TempNodeInfo->PANID.v, NULL,\
                                                      PACKET_TYPE_COMMAND, 0, mb);
                                    #else
                                        SendMACPacket(NULL, PACKET_TYPE_COMMAND, mb);
                                    #endif
                                }
                                SWDelay(1000);
                                LED3 = 0;    
                                return FALSE;
                            }
                            else{
                                // we know the destination's neighbor directly
                                MTP.flags.Val = 0;
                                MTP.flags.bits.ackReq = 1;
                                MTP.flags.bits.secEn = SecEn;
                                TempNodeInfo->shortAddr.v[0] = 0;
                                TempNodeInfo->shortAddr.v[1] = i;

                                if(transceiver == 3){       //Juan: added
                                    #if defined(IEEE_802_15_4)
                                        MTP.altDestAddr = TRUE;
                                        MTP.altSrcAddr = TRUE;
                                        MTP.DestAddress = TempNodeInfo->shortAddr.v;
                                        MTP.DestPANID.Val = myPANID->Val;
                                    #endif
                                }else{
                                    if((i = SearchForShortAddress(mb)) != 0xFF){
                                        MTP.DestAddress = ConnectionTable[i].Address;
                                    }
                                    else{
                                        // highly impossible for none 15.4, where the
                                        // access of Coordinators not through beacon
                                        SWDelay(1000);
                                        LED3 = 0;
                                        goto ROUTE_THROUGH_TREE;
                                    }
                                }

                                if(!(AuxMAC_SendPacket(&MTP, transceiver))){
                                    RouterFailures[i]++;
                                    SWDelay(1000);
                                    LED3 = 0;
                                    return FALSE;
                                }
                                RouterFailures[i] = 0;
                                SWDelay(1000);
                                LED3 = 0; 
                                return TRUE;
                            }
                            SWDelay(1000);
                            LED3 = 0; 
                        }
                    }
                //Juan: Reaching this point means that routing case B) wasn't
                //possible or weird issues occured. Let's try other possibilities.
//**********************************************//
// CASE C
//**********************************************// 
ROUTE_THROUGH_TREE:
                    Printf("\r\n ****** CASE ROUTNG C  ******\n");
                    if(role != ROLE_PAN_COORDINATOR){
                        //Juan: not PANC. THIS IS ROUTING CASE C1)

                        MTP.flags.Val = 0;
                        MTP.flags.bits.ackReq = 1;
                        MTP.flags.bits.secEn = SecEn;

                        //if(role == ROLE_COORDINATOR){
                            if (transceiver == 3){
                                #if defined(IEEE_802_15_4)
                                    TempNodeInfo->shortAddr.Val = 0; //To the PANC.
                                #endif
                            } else{
                                //Clara: if the address of the parent is no available he sends it desesperately
                                // to another coord, just to keep some of the below code
                                if (!ConnectionTable[*myParent].status.bits.isValid){
                                    for(i=0; i<8; i++){
                                        if(knownCoordinators && (0x01 << i)){
                                        //I know coordinator "i" (i = 0 is PANC)
                                        break;
                                        }
                                    }
                                    if(i<8){
                                    //Set the coordinator "i" short address.
                                        TempNodeInfo->shortAddr.v[1] = i;
                                        TempNodeInfo->shortAddr.v[0] = 0;
                                    }else{
                                        TempNodeInfo->shortAddr.Val = 0;
                                    }
                                }else{
                                    for(i=0; i<MY_ADDRESS_LENGTH; i++){
                                        TempNodeInfo->longAddr[i] = ConnectionTable[*myParent].Address[i];
                                    }
                                }
                            }
                       // } else{
                            //Clara: This is the case when a NWK_ROLE_COORDINATOR join
                            // to another coordinator, so in spite of working as a coord
                            // he adquires the role ROLE_FFD_END_DEVICE
                            // So as an ED he sends the packet to his parent.
                            
                        /*    //Juan: Am I an end device? Only coordinators route
                            //messages. If I'm not coordinator, then I should be
                            //PANC... But I'm not, as I entered this part of the
                            //code. Why does this function handle the case of
                            //being an end device? ¬¬ THAT'S WHY I CONSIDER THIS
                            //CASE AS STRANGE - D)

                            //Juan: Send to any coordinator I know.
                            // (Me - end device) ---> (A coordinator)---> ???
                            for(i=0; i<8; i++){
                                if(knownCoordinators && (0x01 << i)){
                                    //Juan: I know coordinator "i" (i = 0 is PANC)
                                    break;
                                }
                            }
                            if(i<8){
                                //Juan: Set the coordinator "i" short address.
                                TempNodeInfo->shortAddr.v[1] = i;
                                TempNodeInfo->shortAddr.v[0] = 0;
                            }
                        
                            
                            else{
                                //Juan: I don't know any coordinators... Send
                                //the packet desperately to the PANC
                                // (Me - end device) --??--> (PANC)

                                // we have tried all possible way, just try to
                                // send to PANC as the last try
                                TempNodeInfo->shortAddr.Val = 0;
                            }
                        */
                            //#if !defined(IEEE_802_15_4)
                            //Juan: I comment this #if !defined IEEE_802_15_4...
                            //SubGHz transceivers have to execute this part so I
                            //add the transceiver condition.
                            if(transceiver != 3){
                                BYTE j;

                                if((j = SearchForShortAddress(mb)) < CONNECTION_SIZE){
                                    //Juan: short address stored in a valid connection slot.
                                    for(i=0; i<MY_ADDRESS_LENGTH; i++){
                                        TempNodeInfo->longAddr[i] = ConnectionTable[j].Address[i];
                                    }
                                }
                                else{
                                    //Juan: short address not found.
                                    //Send to my parent.
                                    for(i=0; i<MY_ADDRESS_LENGTH; i++){
                                        TempNodeInfo->longAddr[i] = ConnectionTable[*myParent].Address[i];
                                    }
                                }
                            //#endif            //Juan
                            }

                        //JUAN: Routing cases C1) and D): send the message.
                        if(transceiver == 3){    //Juan: added transceiver condition.
                            #if defined(IEEE_802_15_4)
                                MTP.altDestAddr = TRUE;
                                MTP.altSrcAddr = TRUE;
                                MTP.DestAddress = TempNodeInfo->shortAddr.v;
                                // send to the PAN Coordinator
                                MTP.DestPANID.Val = myPANID->Val;
                            #endif
                        }else{
                            MTP.DestAddress = ConnectionTable[*myParent].Address;
                        }

                        if(!(AuxMAC_SendPacket(&MTP, transceiver))){
                            RouterFailures[0]++;
                            return FALSE;
                        }
                        RouterFailures[0] = 0;
                        return TRUE;
                    }

                    else{
                        // Highly unlikely to get here, a PAN Coordinator should
                        // have all Coordinators on its neighbor table, here
                        // just as the backup plan for extreme case

                        //Juan: this is the "weird" C2) routing case.
                        MTP.flags.Val = 0;
                        MTP.flags.bits.ackReq = 1;
                        MTP.flags.bits.secEn = SecEn;

                        TempNodeInfo->shortAddr.v[1] = parentNode;
                        TempNodeInfo->shortAddr.v[0] = 0;

                        if(transceiver == 3){    //Juan: added as before.
                            #if defined(IEEE_802_15_4)
                                MTP.altDestAddr = TRUE;
                                MTP.altSrcAddr = TRUE;
                                MTP.DestAddress = TempNodeInfo->shortAddr.v;
                                MTP.DestPANID.Val = myPANID->Val;
                            #endif
                        }else{
                            MTP.DestAddress = ConnectionTable[*myParent].Address;
                        }

                        if(!(AuxMAC_SendPacket(&MTP, transceiver))){
                            RouterFailures[0]++;
                            return FALSE;
                        }
                        RouterFailures[0] = 0;
                        return TRUE;
                    }
                }
            }
        }//Juan: End of function.
    #endif


    /***************************************************************************
     * Function:    BOOL SendMACPacket(BYTE *PANID, BYTE *Address, 
     *                                 BYTE PacketType, BYTE ModeMask)
     * PreCondition:Transceiver has been initialized properly
     * Input:       PANID      - PAN identifier of the destination node.
     *                           (IEEE 802.15.4 only)
     *              Address    - Address of the destination node. Always long
     *                           address for non-IEEE 802.15.4, can be short or
     *                           long address for IEEE 802.15.4.
     *              PacketType - The packet type, passed to transceiver driver
     *                           directly
     *              ModeMask   - The source and destination address mode (short
     *                           or long) maks. (IEEE 802.15.4 only)
     * Output:      A boolean to indicate if packet sent successfully
     * Side Effects:None
     * Overview:    This function is a bridge from MiWi layer to transceiver MAC
     *              layer. It organizes the MiMAC layer parameters and return
     *              the status of MiMAC transmission status.
     **************************************************************************/
    #if defined(IEEE_802_15_4)
        BOOL SendMACPacket(BYTE *PANID, BYTE *Address, BYTE PacketType, \
                           BYTE ModeMask, miwi_band mb)
    #else
        BOOL SendMACPacket(BYTE *Address, BYTE PacketType, miwi_band mb)
    #endif
    {
        BYTE transceiver;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_434
                        transceiver = 1;
                    #elif defined MRF49XA_2_IN_434
                        transceiver = 2;
                    #else   //Error
                        return FALSE;
                    #endif
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_868
                        transceiver = 1;
                    #elif defined MRF49XA_2_IN_868
                        transceiver = 2;
                    #else   //Error
                        return FALSE;
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                    return FALSE;
                #else
                    transceiver = 3;
                    break;
                #endif
            default:
                return FALSE;
        }

        MTP.flags.Val = 0;
        
        MTP.flags.bits.packetType = PacketType;
        if(Address == NULL){
            MTP.flags.bits.broadcast = 1;
        }
        MTP.flags.bits.ackReq = (MTP.flags.bits.broadcast) ? 0:1;
        MTP.flags.bits.sourcePrsnt = 1;
        
        MTP.DestAddress = Address;

        if (transceiver == 3){          //Juan: added transceiver condition
            #if defined(IEEE_802_15_4)
                if((ModeMask & MSK_ALT_DST_ADDR) > 0){
                    MTP.altDestAddr = TRUE;
                }
                else{
                    MTP.altDestAddr = FALSE;
                }
                if((ModeMask & MSK_ALT_SRC_ADDR) > 0){
                    MTP.altSrcAddr = TRUE;
                }
                else{
                    MTP.altSrcAddr = FALSE;
                }
                MTP.DestPANID.v[0] = PANID[0];
                MTP.DestPANID.v[1] = PANID[1];
            #endif
        }

        if (AuxMAC_SendPacket(&MTP, transceiver)) {
            MiApp_FlushTx(mb);  //"Reset" buffer: Pointer to Payload Start
            return TRUE;
        }    
            
        return FALSE;
    }

    /***************************************************************************
     * Function:    void SendIndirectPacket(BYTE *Address, BYTE *AltAddress, 
     *                                      BOOL isAltAddress)
     * PreCondition:Node has joined the network
     * Input:       Address      - Pointer to the long address
     *              AltAddress   - Pointer to the short address
     *              isAltAddress - Boolean to indicate if use long/short address
     * Output:      None
     * Side Effects:None
     * Overview:    This function is used to send an indirect message to a
     *              sleeping device
     **************************************************************************/
    #if defined(NWK_ROLE_COORDINATOR) && defined(ENABLE_INDIRECT_MESSAGE)
        MIWI_TICK tmpTick;
        void SendIndirectPacket(BYTE *Address, BYTE *AltAddress, \
                                BOOL isAltAddress, miwi_band mb)
        {
            
            BYTE transceiver;
            BYTE *TxData;
            TEMP_NODE_INFO *TempNodeInfo;
            WORD_VAL *myPANID;
            switch (mb){
                case ISM_434:
                    #if !defined MIWI_0434_RI
                        Printf("Error: MiWi ISM 434 MHz band is not available.");
                        return;
                    #else
                        #if defined MRF49XA_1_IN_434
                            TxData = & MRF49XA_1_TxData;
                            transceiver = 1;
                        #elif defined MRF49XA_2_IN_434
                            TxData = & MRF49XA_2_TxData;
                            transceiver = 2;
                        #else   //Error
                            return;
                        #endif
                        TempNodeInfo = &temp0434;
                        myPANID = & myPAN0434ID;
                        break;
                    #endif
                case ISM_868:
                    #if !defined MIWI_0868_RI
                        Printf("Error: MiWi ISM 868 MHz band is not available.");
                        return;
                    #else
                        #if defined MRF49XA_1_IN_868
                            TxData = & MRF49XA_1_TxData;
                            transceiver = 1;
                        #elif defined MRF49XA_2_IN_868
                            TxData = & MRF49XA_2_TxData;
                            transceiver = 2;
                        #else   //Error
                            return;
                        #endif
                        TempNodeInfo = &temp0868;
                        myPANID = & myPAN0868ID;
                        break;
                    #endif
                case ISM_2G4:
                    #if !defined MIWI_2400_RI
                        Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                        return;
                    #else
                        TxData = & MRF24J40_TxData;
                        transceiver = 3;
                        TempNodeInfo = &temp2400;
                        myPANID = & myPAN2400ID;
                        break;
                    #endif
                default:
                    return;
            }
            Printf("\r\n SEND INDIRECT");
            BYTE i,j;
            BYTE index;
            BYTE packetType = PACKET_TYPE_DATA;
            #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                WORD_VAL tmpW;
            #endif
            
            *TxData = 0;
            #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                MiApp_WriteData(MAC_COMMAND_TIME_SYNC_DATA_PACKET, mb);
                packetType = PACKET_TYPE_COMMAND;
                tmpTick = MiWi_TickGet();
                if((tmpTick.Val - TimeSyncTick.Val) < ((ONE_SECOND) * RFD_WAKEUP_INTERVAL)){
                    tmpW.Val = (((ONE_SECOND) * RFD_WAKEUP_INTERVAL) - (tmpTick.Val - TimeSyncTick.Val) \
                                + (TimeSlotTick.Val * TimeSyncSlot)) / SYMBOLS_TO_TICKS((DWORD)0xFFFF * \
                                MICRO_SECOND_PER_COUNTER_TICK / 16);
                    MiApp_WriteData(tmpW.v[0], mb);
                    MiApp_WriteData(tmpW.v[1], mb);
                    
                    tmpW.Val = 0xFFFF - (WORD)((TICKS_TO_SYMBOLS(( ((ONE_SECOND) * RFD_WAKEUP_INTERVAL) \
                               - (tmpTick.Val - TimeSyncTick.Val) + (TimeSlotTick.Val * TimeSyncSlot) + \
                               TimeSlotTick.Val/2)) - ((DWORD)0xFFFF * MICRO_SECOND_PER_COUNTER_TICK / 16 \
                               * tmpW.Val) ) * 16 / MICRO_SECOND_PER_COUNTER_TICK);
                    if(TimeSyncSlot<TIME_SYNC_SLOTS){
                        TimeSyncSlot++;
                    }    
                    MiApp_WriteData(tmpW.v[0], mb);
                    MiApp_WriteData(tmpW.v[1], mb);
                }
                else{
                    MiApp_WriteData(0, mb);
                    MiApp_WriteData(0, mb);
                    MiApp_WriteData(0x5F, mb);
                    MiApp_WriteData(0xF0, mb);
                }        
            #endif
            
            if(isAltAddress){
                TempNodeInfo->shortAddr.v[0] = AltAddress[0];
                TempNodeInfo->shortAddr.v[1] = AltAddress[1];
                if((index = SearchForShortAddress(mb)) == 0xFF){
                    goto NO_INDIRECT_MESSAGE;
                }       
            }
            else{
                for(i=0; i<MY_ADDRESS_LENGTH; i++){
                    TempNodeInfo->longAddr[i] = Address[i];
                }
                if((index = SearchForLongAddress(mb)) == 0xFF){
                    goto NO_INDIRECT_MESSAGE;
                }        
            }

            WORD_VAL AuxAltAddr;
            switch(mb){
                case ISM_434:
                    #if defined MIWI_0434_RI
                        AuxAltAddr = ConnectionTable[index].MIWI0434AltAddress;
                    #endif
                case ISM_868:
                    #if defined MIWI_0868_RI
                        AuxAltAddr = ConnectionTable[index].MIWI0868AltAddress;
                    #endif
                case ISM_2G4:
                    #if defined MIWI_2400_RI
                        AuxAltAddr = ConnectionTable[index].MIWI2400AltAddress;
                    #endif
                default:
                    return;
            }

            //Juan: By reaching this point, we've found the destination node in
            //the connection table and AuxAltAddr has the destination node's
            //short address at the requested mb.
            
            for(j=0; j<INDIRECT_MESSAGE_SIZE; j++){
                        if (indirectMessages[i].flags.bits.isValid) Printf ("\r\n Valid ind");
                        else Printf ("\r\n no valir");
                    }
            Printf ("\r\nafter");
            
            
            for(i=0; i<INDIRECT_MESSAGE_SIZE; i++){
                //if(indirectMessages[i].flags.bits.isValid){
                //Juan: Original code above. I add the miwi band condition to
                //avoid dealing with inappropriate messages.
                if((indirectMessages[i].flags.bits.isValid) && (indirectMessages[i].MiWiFreqBand == mb)){
                    if ((indirectMessages[i].flags.bits.isBroadcast == 0)       &&
                        (indirectMessages[i].flags.bits.isAltAddr)              &&
                        (AuxAltAddr.v[0] == indirectMessages[i].DestAddress[0]) &&
                        (AuxAltAddr.v[1] == indirectMessages[i].DestAddress[1])){

                        for(j=0; j<indirectMessages[i].PayLoadSize; j++){
                            MiApp_WriteData(indirectMessages[i].PayLoad[j], mb);
                        }

                        if (transceiver == 3){
                            #if defined(IEEE_802_15_4) && defined MRF24J40
                                if(indirectMessages[i].flags.bits.isCommand){
                                    #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                                        MRF24J40_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                    #endif
                                    SendMACPacket(myPANID->v, AuxAltAddr.v, \
                                                  PACKET_TYPE_COMMAND, MSK_ALT_DST_ADDR | MSK_ALT_SRC_ADDR, mb);
                                }
                                else{
                                    MTP.flags.Val = 0;
                                    MTP.flags.bits.packetType = packetType;
                                    MTP.flags.bits.ackReq = 1;
                                    MTP.flags.bits.secEn = indirectMessages[i].flags.bits.isSecured;
                                    MTP.DestAddress = AuxAltAddr.v;
                                    MTP.altDestAddr = TRUE;
                                    MTP.altSrcAddr = TRUE;
                                    MTP.DestPANID.Val = indirectMessages[i].DestPANID.Val;

                                    AuxMAC_SendPacket(&MTP, transceiver);

                                //SendMACPacket(myPANID.v, AltAddress, PACKET_TYPE_DATA, MSK_ALT_DST_ADDR | MSK_ALT_SRC_ADDR);
                                }
                            #endif
                        }
                        else{
                            MTP.flags.Val = 0;
                            MTP.flags.bits.packetType = packetType;
                            MTP.flags.bits.ackReq = 1;
                            MTP.flags.bits.secEn = indirectMessages[i].flags.bits.isSecured;
                            MTP.DestAddress = ConnectionTable[index].Address;
                            if(indirectMessages[i].flags.bits.isCommand){
                                MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                                MTP.flags.bits.sourcePrsnt = 1;
                                #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                                    #if defined MRF49XA_1
                                        if(transceiver == 1){
                                            MRF49XA_1_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                    #if defined MRF49XA_2
                                        if(transceiver == 2){
                                            MRF49XA_2_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                #endif
                            }
                            AuxMAC_SendPacket(&MTP, transceiver);
                            Printf("\r\n 1");
                            indirectMessages[i].flags.bits.isValid = 0;
                        }
                        
                        indirectMessages[i].flags.bits.isValid = 0;
                        return;
                    }
    
                    if((indirectMessages[i].flags.bits.isBroadcast == 0) &&
                       (indirectMessages[i].flags.bits.isAltAddr == 0) &&
                       isSameAddress(ConnectionTable[index].Address, indirectMessages[i].DestAddress)){

                        for(j=0; j<indirectMessages[i].PayLoadSize; j++){
                            MiApp_WriteData(indirectMessages[i].PayLoad[j], mb);
                        }

                        if(transceiver == 3){   //Juan: added transceiver condition
                            #if defined(IEEE_802_15_4) && defined MRF24J40
                                if(indirectMessages[i].flags.bits.isCommand){
                                    #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                                        MRF24J40_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                    #endif
                                    SendMACPacket(myPANID->v, ConnectionTable[index].Address, PACKET_TYPE_COMMAND, 0, mb);
                                    indirectMessages[i].flags.bits.isValid = 0;
                                }
                                else{
                                    MTP.flags.Val = 0;
                                    MTP.flags.bits.packetType = packetType;
                                    MTP.flags.bits.ackReq = 1;
                                    MTP.flags.bits.secEn = indirectMessages[i].flags.bits.isSecured;
                                    MTP.DestAddress = ConnectionTable[index].Address;
                                    MTP.altDestAddr = FALSE;
                                    MTP.altSrcAddr = TRUE;
                                    MTP.DestPANID.Val = indirectMessages[i].DestPANID.Val;

                                    AuxMAC_SendPacket(&MTP, transceiver);

                                    //SendMACPacket(myPANID.v, Address, PACKET_TYPE_DATA, 0);
                                }
                            #endif
                        }
                        else{
                            MTP.flags.Val = 0;
                            MTP.flags.bits.packetType = packetType;
                            MTP.flags.bits.ackReq = 1;
                            MTP.flags.bits.secEn = indirectMessages[i].flags.bits.isSecured;
                            MTP.DestAddress = ConnectionTable[index].Address;
                            if(indirectMessages[i].flags.bits.isCommand){
                                MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                                #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                                    #if defined MRF49XA_1
                                        if(transceiver == 1){
                                            MRF49XA_1_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                    #if defined MRF49XA_2
                                        if(transceiver == 2){
                                            MRF49XA_2_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                #endif
                                MTP.flags.bits.sourcePrsnt = 1;
                            }
                            Printf("\r\n 2");
                            AuxMAC_SendPacket(&MTP, transceiver);
                            indirectMessages[i].flags.bits.isValid = 0;
                        }
                        
                        indirectMessages[i].flags.bits.isValid = 0;
                        
                    for(j=0; j<INDIRECT_MESSAGE_SIZE; j++){
                        if (indirectMessages[i].flags.bits.isValid) Printf ("\r\n Valid ind");
                        else Printf ("\r\n no valir");
                    }
                        
                        return;
                    }
                }
            }
            
            
            for(i=0; i<INDIRECT_MESSAGE_SIZE; i++){
                //if(indirectMessages[i].flags.bits.isValid){
                //Juan: Original code above. I add the miwi band condition to
                //avoid dealing with inappropriate messages.
                if((indirectMessages[i].flags.bits.isValid) && (indirectMessages[i].MiWiFreqBand == mb)){
                    if(indirectMessages[i].flags.bits.isBroadcast){
                        for(j=0; j<indirectMessages[i].PayLoadSize; j++){
                            MiApp_WriteData(indirectMessages[i].PayLoad[j], mb);
                        }
                        if(transceiver == 3){    //Juan: added transceiver condition
                            #if defined(IEEE_802_15_4)
                                MTP.flags.Val = 0;
                                MTP.flags.bits.packetType = packetType;
                                if(indirectMessages[i].flags.bits.isCommand){
                                    MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                                    #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                                        MRF24J40_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                    #endif
                                }
                                MTP.flags.bits.ackReq = 1;
                                MTP.flags.bits.sourcePrsnt = 1;
                                MTP.flags.bits.secEn = indirectMessages[i].flags.bits.isSecured;
                                MTP.altSrcAddr = TRUE;
                                if(isAltAddress){
                                    MTP.altDestAddr = TRUE;
                                    MTP.DestAddress = AuxAltAddr.v;
                                }
                                else{
                                    MTP.altDestAddr = FALSE;
                                    MTP.DestAddress = ConnectionTable[index].Address;
                                }
                                MTP.DestPANID.Val = indirectMessages[i].DestPANID.Val;

                                AuxMAC_SendPacket(&MTP, transceiver);
                                indirectMessages[i].flags.bits.isValid = 0;
                            #endif
                        }
                        else{
                            MTP.flags.Val = 0;
                            MTP.flags.bits.packetType = packetType;
                            MTP.flags.bits.ackReq = 1;
                            MTP.flags.bits.secEn = indirectMessages[i].flags.bits.isSecured;
                            if(indirectMessages[i].flags.bits.isCommand){
                                MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                                #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP)
                                    #if defined MRF49XA_1
                                        if(transceiver == 1){
                                            MRF49XA_1_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                    #if defined MRF49XA_2
                                        if(transceiver == 2){
                                            MRF49XA_2_TxBuffer[0] = MAC_COMMAND_TIME_SYNC_COMMAND_PACKET;
                                        }
                                    #endif
                                #endif
                            }
                            MTP.DestAddress = ConnectionTable[index].Address;
Printf("\r\n 3");
                            AuxMAC_SendPacket(&MTP, transceiver);
                            indirectMessages[i].flags.bits.isValid = 0;

                        }
                        return;   
                    }
                }
            }

NO_INDIRECT_MESSAGE:            
            // no indirect message found
            if (transceiver == 3){
                #if defined(IEEE_802_15_4)
                    if(isAltAddress){
                        SendMACPacket(myPANID->v, AltAddress, packetType, MSK_ALT_DST_ADDR | MSK_ALT_SRC_ADDR, mb);
                    }
                    else{
                        SendMACPacket(myPANID->v, Address, packetType, MSK_ALT_SRC_ADDR, mb);
                    }
                #endif
            }else{
                MTP.flags.Val = 0;
                MTP.flags.bits.packetType = packetType;
                MTP.flags.bits.ackReq = 1;
                MTP.DestAddress = Address;

                AuxMAC_SendPacket(&MTP, transceiver);
            }
        }
    #endif
    
    
    /***************************************************************************
     * Function:    BYTE findNextNetworkEntry(void)
     * PreCondition:None
     * Input:       None
     * Output:      BYTE 0xFF if there is no room in the network table otherwise
     *              this function returns the index of the first blank entry in
     *              the table
     * Side Effects:None
     * Overview:    This function is used to determine the next available slot
     *              in the network table. This function can be used to manually
     *              create entries in the networkTable
     **************************************************************************/
    BYTE findNextNetworkEntry(void){
        BYTE i;
        
        for(i=0; i<CONNECTION_SIZE; i++){
            if(ConnectionTable[i].status.bits.isValid == 0){
                return i;
            }
        }
        return 0xFF;
    }
    
    /***************************************************************************
     * Function:    void DumpNetworkTable(void)
     * PreCondition:None
     * Input:       None
     * Output:      None
     * Side Effects:None
     * Overview:    This function dumps the contents of many tables to the
     *              console. This is a debugging/helper function only and is not
     *              used by the stack
     **************************************************************************/
    #if defined(ENABLE_DUMP)
    void DumpConnection(BYTE index){
        BYTE i, j,  k;
        WORD auxiliar;

        if(index == 0xFF){
            Printf("\r\n\nLong Address: 0x");
            for(i=0; i<MY_ADDRESS_LENGTH; i++){
                PrintChar(myLongAddress[MY_ADDRESS_LENGTH-1-i]);
            }
            #if defined MIWI_0434_RI
                Printf("\r\nMiWi at 434 MHz.  Channel: ");
                PrintDec(MIWI0434_currentChannel - MIWI0434ConfChannelOffset);
                Printf("  Short Addr: ");
                PrintChar(myShort0434Addr.v[1]);
                PrintChar(myShort0434Addr.v[0]);
                Printf("  PANID: ");
                PrintChar(myPAN0434ID.v[1]);
                PrintChar(myPAN0434ID.v[0]);
            #endif
            #if defined MIWI_0868_RI
                Printf("\r\nMiWi at 868 MHz.  Channel: ");
                PrintDec(MIWI0868_currentChannel - MIWI0868ConfChannelOffset);
                Printf("  Short Addr: ");
                PrintChar(myShort0868Addr.v[1]);
                PrintChar(myShort0868Addr.v[0]);
                Printf("  PANID: ");
                PrintChar(myPAN0868ID.v[1]);
                PrintChar(myPAN0868ID.v[0]);
            #endif
            #if defined MIWI_2400_RI
                Printf("\r\nMiWi at 2,4 GHz.  Channel: ");
                PrintDec(MIWI2400_currentChannel - MIWI2400ConfChannelOffset);
                Printf("  Short Addr: ");
                PrintChar(myShort2400Addr.v[1]);
                PrintChar(myShort2400Addr.v[0]);
                Printf("  PANID: ");
                PrintChar(myPAN2400ID.v[1]);
                PrintChar(myPAN2400ID.v[0]);
            #endif
        }
        
       
        //Juan: print table key.
        ConsolePutROMString((ROM char*)
            "\r\nAvailable nodes           |------ 434MHz ------|------ 868MHz ------|------ 2,4GHz ------|\r\n"
                "Index RX LONG_ADDR        | Con DC  ADDR PANID | Con DC  ADDR PANID | Con DC  ADDR PANID | PeerInfo\r\n");

        //Juan: print requested connection
        BOOL lastTurn;
        if(index < CONNECTION_SIZE){
            lastTurn = TRUE;
            i = index;
        }else{
            //0xFF or invalid index prints all the table
            lastTurn = FALSE;
            i = 0;
        }
        
        do{
            //Printing loop. It tries to print at least one connection.
            if(ConnectionTable[i].status.bits.isValid){
                PrintChar(i);   //Juan: Prints connection index or "handle"
                Printf("    ");

                if(ConnectionTable[i].status.bits.RXOnWhenIdle == 1){
                    ConsolePut('Y');
                } else{
                    ConsolePut('N');
                }
                ConsolePut(' ');
                ConsolePut(' ');

                if(ConnectionTable[i].status.bits.longAddressValid){
                    for(j = 0; j < MY_ADDRESS_LENGTH; j++){
                        PrintChar(ConnectionTable[i].Address[MY_ADDRESS_LENGTH-1-j]);
                    }
                }
                else{
                    for(j = 0; j < MY_ADDRESS_LENGTH; j++){
                        ConsolePut(' ');
                        ConsolePut(' ');
                    }
                }
                ConsolePut(' ');
                ConsolePut('|');
                ConsolePut(' ');

                WORD_VAL shortAddr;    //Broadcast addr (invalid)
                WORD_VAL PANID;
                for (j=0; j<3; j++){
                    switch (j){
                        case 0:
                            #if defined MIWI_0434_RI
                                shortAddr.Val = ConnectionTable[i].MIWI0434AltAddress.Val;
                                PANID.Val = ConnectionTable[i].PAN0434ID.Val;
                            #else
                                shortAddr.Val = 0xFFFF; //Broadcast addr (invalid)
                                PANID.Val = 0xFFFF;
                            #endif
                            break;
                        case 1:
                            #if defined MIWI_0868_RI
                                shortAddr.Val = ConnectionTable[i].MIWI0868AltAddress.Val;
                                PANID.Val = ConnectionTable[i].PAN0868ID.Val;
                            #else
                                shortAddr.Val = 0xFFFF; //Broadcast addr (invalid)
                                PANID.Val = 0xFFFF;
                            #endif
                            break;
                        case 2:
                            #if defined MIWI_2400_RI
                                shortAddr.Val = ConnectionTable[i].MIWI2400AltAddress.Val;
                                PANID.Val = ConnectionTable[i].PAN2400ID.Val;
                            #else
                                shortAddr.Val = 0xFFFF; //Broadcast addr (invalid)
                                PANID.Val = 0xFFFF;
                            #endif
                            break;
                        default:
                            return;
                    }

                    if (ConnectionTable[i].MiWiInterfaces & (0x01 << j)){
                        //Juan: This part has been added. It begins checking
                        //MIWI_0434_RI flag and ends up checking MIWI_2400_RI.
                        Printf("YES ");
                    } else{
                        Printf("NO  ");
                    }
                    
                    if(ConnectionTable[i].status.Val & (0x0040 << (4*j))){

                        //Juan: This part has been added. It begins checking
                        //direct0434Connection status flag and ends up
                        //checking direct2400Connection status flag.
                        Printf("YES ");
                    } else{
                        Printf("NO  ");
                    }

                    if(ConnectionTable[i].status.Val & (0x0010 << (4*j))){
                        //Juan: This part has been added. It begins checking
                        //short0434AddrValid status flag and ends up checking
                        //short2400AddrValid status flag.
                        PrintChar(shortAddr.v[1]);
                        PrintChar(shortAddr.v[0]);
                        ConsolePut(' ');
                        PrintChar(PANID.v[1]);
                        PrintChar(PANID.v[0]);
                        ConsolePut(' ');
                        ConsolePut(' ');
                        ConsolePut('|');
                        ConsolePut(' ');
                    } else{
                        Printf("           | ");
                    }
                }
                #if ADDITIONAL_NODE_ID_SIZE > 0
                    for(j=0; j<ADDITIONAL_NODE_ID_SIZE; j++){
                        PrintChar(ConnectionTable[i].PeerInfo[j]);
                    }
                #endif
                ConsolePut('\r');
                ConsolePut('\n');
            }
            i++;
            //Printf("\r\n\nValor de índice: ");
            //PrintChar(i);
            //Printf("\n\n");
            if(i >= CONNECTION_SIZE){
                lastTurn = TRUE;
            }
        }while(!lastTurn);


        #ifdef NWK_ROLE_COORDINATOR
            BYTE knownCoordinators;
            Printf("\r\n                    434MHz  868MHz  2.4GHz \r\n Known Coordinators: ");
            #ifdef MIWI_0434_RI
                PrintChar(known0434Coordinators);
            #else
                ConsolePut(' ');
                ConsolePut(' ');
            #endif
                Printf("      ");
            #ifdef MIWI_0868_RI
                PrintChar(known0868Coordinators);
            #else
                ConsolePut(' ');
                ConsolePut(' ');
            #endif
                Printf("      ");
            #ifdef MIWI_2400_RI
                PrintChar(known2400Coordinators);
            #else
                ConsolePut(' ');
                ConsolePut(' ');
            #endif

            ConsolePutROMString((ROM char*)"\r\r\nRouting Table\r\n"
                        "434 MHz-------| 868 MHz-------| 2,4 GHz-------|\r"
                        "Coord NextHop | Coord NextHop | Coord NextHop |\r");
            for(i=0; i<8; i++){
                PrintChar(i);
                Printf("    ");
                #ifdef MIWI_0434_RI
                    PrintChar(Routing0434Table[i]);
                #else
                    ConsolePut(' ');
                    ConsolePut(' ');
                #endif
                Printf("      | ");
                PrintChar(i);
                Printf("    ");
                #ifdef MIWI_0868_RI
                    PrintChar(Routing0868Table[i]);
                #else
                    ConsolePut(' ');
                    ConsolePut(' ');
                #endif
                Printf("      | ");
                PrintChar(i);
                Printf("    ");
                #ifdef MIWI_2400_RI
                    PrintChar(Routing2400Table[i]);
                #endif
                Printf("      |\r\n");
            }
        #endif
    }
    #else
    void DumpNetworkTable(void){}
    #endif
    
    /***************************************************************************
     * Function:    void DiscoverNodeByEUI(void)
     * PreCondition:tempLongAddress[0-7] need to be set to the address that
     *              needs to be discovered
     * Input:       None
     * Output:      None
     * Side Effects:None
     * Overview:    This function constructs and sends out the from to discover
     *              a device by their EUI address
     **************************************************************************/
    #if defined(SUPPORT_EUI_ADDRESS_SEARCH)
    void DiscoverNodeByEUI(miwi_band mb){       //Juan: review before enabling!!

        BYTE *TxData;
        BYTE *pbuffer;
        MIWI_STATE_MACHINE *msm;
        BYTE transceiver;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return;
                #else
                    #if defined MRF49XA_1_IN_434
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                        msm = &MRF49XA_1_MiWiStateMachine;
                        transceiver = 1:
                    #elif defined MRF49XA_2_IN_434
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                        msm = &MRF49XA_2_MiWiStateMachine;
                        transceiver = 2;
                    #else   //Error
                        return;
                    #endif
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return;
                #else
                    #if defined MRF49XA_1_IN_868
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                        msm = &MRF49XA_1_MiWiStateMachine;
                        transceiver = 1:
                    #elif defined MRF49XA_2_IN_868
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                        msm = &MRF49XA_2_MiWiStateMachine;
                        transceiver = 2:
                    #else   //Error
                        return;
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                    return;
                #else
                    TxData = & MRF24J40_TxData;
                    pbuffer = MRF24J40_TxBuffer;
                    msm = &MRF24J40_MiWiStateMachine;
                    transceiver = 3;
                    break;
                #endif
            default:
                return;
        }

        BYTE i;
        
        msm->bits.EUISearching = 1;
        #ifndef NWK_ROLE_COORDINATOR
            //if we a member of the network
            if(msm->bits.memberOfNetwork == 1){
    
                *(pbuffer)    = defaultHops;
                *(pbuffer+1)  = 0x06;                       //Frame Control
                *(pbuffer+2)  = 0xFF;                       //dest PANID LSB
                *(pbuffer+3)  = 0xFF;                       //dest PANID MSB
                *(pbuffer+4)  = 0xFF;                       //dest address LSB
                *(pbuffer+5)  = 0xFF;                       //dest address MSB
                *(pbuffer+6)  = myPANID.v[0];               //source PANID LSB
                *(pbuffer+7)  = myPANID.v[1];               //source PANID MSB
                *(pbuffer+8)  = myShortAddress.v[0];        //source address LSB
                *(pbuffer+9)  = myShortAddress.v[1];        //source address MSB
                *(pbuffer+10) = *(MiWiSeqNum)++;               //seq num
                *(pbuffer+11) = MIWI_STACK_REPORT_TYPE;     //Report Type
                *(pbuffer+12) = EUI_ADDRESS_SEARCH_REQUEST; //Report ID
                *TxData = 13;
                
                for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                    MiApp_WriteData(TempNodeInfo->longAddr[i], mb);     //Data byte 0
                }

                MTP.flags.Val = 0;
                MTP.flags.bits.ackReq = 1;
                MTP.flags.bits.sourcePrsnt = 1;
                
                #if defined(IEEE_802_15_4)
                    MTP.DestAddress = ConnectionTable[myParent].AltAddress.v;
                    MTP.DestPANID.Val = ConnectionTable[myParent].PANID.Val;
                    MTP.altDestAddr = TRUE;
                    MTP.altSrcAddr = TRUE;
                #else
                    MTP.DestAddress = ConnectionTable[myParent].LongAddress;
                #endif
                
                AuxMAC_SendPacket(&MTP, transceiver);
            }
        #else
            //if I am a coordinator capable device
            if(msm->bits.memberOfNetwork == 1){

                *(pbuffer)    = defaultHops;
                *(pbuffer+1)  = 0x06;                       //Frame Control
                *(pbuffer+2)  = 0xFF;                       //dest PANID LSB
                *(pbuffer+3)  = 0xFF;                       //dest PANID MSB
                *(pbuffer+4)  = 0xFF;                       //dest address LSB
                *(pbuffer+5)  = 0xFF;                       //dest address MSB
                *(pbuffer+6)  = myPANID.v[0];               //source PANID LSB
                *(pbuffer+7)  = myPANID.v[1];               //source PANID MSB
                *(pbuffer+8)  = myShortAddress.v[0];        //source address LSB
                *(pbuffer+9)  = myShortAddress.v[1];        //source address MSB
                *(pbuffer+10) = *(MiWiSeqNum)++;               //seq num
                *(pbuffer+11) = MIWI_STACK_REPORT_TYPE;     //Report Type
                *(pbuffer+12) = EUI_ADDRESS_SEARCH_REQUEST; //Report ID
                *TxData = 13;

                for(i=0; i<MY_ADDRESS_LENGTH; i++){
                    MiApp_WriteData(TempNodeInfo->longAddr[i], mb);      //Data byte 0
                }


                MTP.flags.Val = 0;
                MTP.flags.bits.broadcast = 1;
                
                #if defined(IEEE_802_15_4)
                    MTP.DestAddress = NULL;
                    MTP.altDestAddr = TRUE;
                    MTP.altSrcAddr = TRUE;
                    MTP.DestPANID.Val = myPANID.Val;
                #else
                    MTP.DestAddress = NULL;
                #endif
                
                AuxMAC_SendPacket(&MTP, transceiver);
            }
        #endif
    }
    #endif

    /***************************************************************************
     * Function:    void OpenSocket(void)
     * PreCondition:None
     * Input:       BYTE socketType - either CLUSTER_SOCKET for a cluster socket
     *              or P2P_SOCKET for a P2P socket.
     * Output:      None
     * Side Effects:This will send out a packet trying to create a socket
     * Overview:    This will send out a packet to the PAN coordinator that is 
     *              trying to link this device to another device. The socket
     *              operation is complete when OpenSocketComplete() returns TRUE
     *              The status of the operation is retreived through the
     *              OpenSocketSuccessful() function. If it returns TRUE then
     *              OpenSocketHandle() returns the handle of the created socket.
     *              This value is valid until openSocket() is called again.
     *              If OpenSocketComplete() returns FALSE then the scan is still
     *              in progress.
     **************************************************************************/
    void OpenSocket(miwi_band mb){
        BYTE *pbuffer;
        OPEN_SOCKET *openSocketInfo;
        TEMP_NODE_INFO *TempNodeInfo;
        WORD_VAL StatusVal, *myShortAddress, *myPANID;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return;
                #else
                    openSocketInfo = & Socket0434Info;
                    TempNodeInfo = & temp0434;
                    StatusVal.Val = CONNSTAT_DFLT_OFF_0434;
                    myShortAddress = &myShort0434Addr;
                    myPANID = & myPAN0434ID;
                    #if defined MRF49XA_1_IN_434
                        pbuffer = MRF49XA_1_TxBuffer;
                    #elif defined MRF49XA_2_IN_434
                        pbuffer = MRF49XA_2_TxBuffer;
                    #else   //Error
                        return;
                    #endif
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return;
                #else
                    openSocketInfo = & Socket0868Info;
                    TempNodeInfo = & temp0868;
                    StatusVal.Val = CONNSTAT_DFLT_OFF_0868;
                    myShortAddress = &myShort0868Addr;
                    myPANID = & myPAN0868ID;
                    #if defined MRF49XA_1_IN_868
                        pbuffer = MRF49XA_1_TxBuffer;
                    #elif defined MRF49XA_2_IN_868
                        pbuffer = MRF49XA_2_TxBuffer;
                    #else   //Error
                        return;
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                    return;
                #else
                    openSocketInfo = & Socket2400Info;
                    TempNodeInfo = & temp2400;
                    StatusVal.Val = CONNSTAT_DFLT_OFF_2400;
                    myShortAddress = &myShort2400Addr;
                    myPANID = & myPAN2400ID;
                    pbuffer = MRF24J40_TxBuffer;
                    break;
                #endif
            default:
                return;
        }

        BYTE i;
        
        openSocketInfo->status.bits.matchFound = 0;
        
        #ifdef NWK_ROLE_COORDINATOR        
            //If I am the PAN coordinator
            if(role == ROLE_PAN_COORDINATOR){
                if(openSocketInfo->status.bits.requestIsOpen == 0){
                    //if there isn't a request currently open
                    //I am the PAN coordinator, there is no reason to send a request out
                    openSocketInfo->socketStart = MiWi_TickGet();
                    openSocketInfo->ShortAddress1.Val = 0x0000;
                    for(i=0; i<MY_ADDRESS_LENGTH; i++){
                        openSocketInfo->LongAddress1[i] = myLongAddress[i];
                    }
                    openSocketInfo->status.bits.requestIsOpen = 1;
                    openSocketInfo->status.bits.matchFound = 0;
                    #if ADDITIONAL_NODE_ID_SIZE > 0
                        for(i=0; i<ADDITIONAL_NODE_ID_SIZE; i++){
                            openSocketInfo->AdditionalNodeID1[i] = AdditionalNodeID[i];
                        }
                    #endif
                    //Juan: By this point, the request at the current interface
                    //wasn't open, I've opened a socket and I don't have to send
                    //the request to the PAN Coordinator because I'm the PANC!
                    //Then goes to return.
                }
                else{
                    //Juan: The request at the current radio interface was found
                    //open so I'm the second peer to invoke it...
                    if(openSocketInfo->ShortAddress1.Val == 0x0000){
                        //I am the PAN coordinator and I don't want to talk to
                        //myself so lets forget we even tried
                        return;
                    }
                    //Juan: if the first peer's short address is different than
                    //mine I will send an open_socket_response packet.
                    if(openSocketInfo->ShortAddress1.Val != myShortAddress->Val){
                        MiApp_FlushTx(mb);
                        MiApp_WriteData(MIWI_STACK_REPORT_TYPE, mb); //Report Type
                        MiApp_WriteData(OPEN_SOCKET_RESPONSE, mb);   //Report ID
                        for(i=0; i<MY_ADDRESS_LENGTH; i++){
                            MiApp_WriteData(myLongAddress[i], mb);
                        }
                        MiApp_WriteData(myShortAddress->v[0], mb);
                        MiApp_WriteData(myShortAddress->v[1], mb);
                        #if ADDITIONAL_NODE_ID_SIZE > 0
                            for(i=0; i<ADDITIONAL_NODE_ID_SIZE; i++){
                                MiApp_WriteData(AdditionalNodeID[i], mb);
                            }
                        #endif
                        //Juan: Now I have prepared the response and written my
                        //info I will add the first peer to my connection table.
                        for(i=0; i<MY_ADDRESS_LENGTH; i++){
                            TempNodeInfo->longAddr[i] = openSocketInfo->LongAddress1[i];
                        }
                        TempNodeInfo->shortAddr.Val = openSocketInfo->ShortAddress1.Val;

                        openSocketInfo->socketHandle = SearchForShortAddress(mb);

                        if(openSocketInfo->socketHandle != 0xFF){
                            TempNodeInfo->status.Val = ConnectionTable[openSocketInfo->socketHandle].status.Val;
                        }
                        else{
                            //Family, RxOnWHenIdle, Neighbor/Network, P2P, ShortVal, LongVal, Direct, Valid
                            //Juan: Original value for original status structure (the same as in adapted P2P)
                            //tempNodeStatus.Val = 0x8C;

                            //Juan: I load a value initialised at the beginning of this function with the same
                            //common flags as the original but only with the flags of the current interface.
                            TempNodeInfo->status.Val = StatusVal.Val;
                            if((TempNodeInfo->shortAddr.v[0] & 0x80) == 0){
                                TempNodeInfo->status.bits.RXOnWhenIdle = 1;
                            }
                        }

                        TempNodeInfo->PANID.Val = myPANID->Val;
                        openSocketInfo->socketHandle = AddNodeToNetworkTable(mb);

                        #if ADDITIONAL_NODE_ID_SIZE > 0
                            for(i=0; i<ADDITIONAL_NODE_ID_SIZE; i++){
                                ConnectionTable[openSocketInfo->socketHandle].PeerInfo[i] = openSocketInfo->AdditionalNodeID1[i];
                            }
                        #endif

                        //RouteMessage(myPANID,openSocketInfo.ShortAddress1,FALSE);
                        MiApp_UnicastAddress(openSocketInfo->ShortAddress1.v, FALSE, FALSE, mb);
                        openSocketInfo->status.bits.requestIsOpen = 0;
                        openSocketInfo->status.bits.matchFound = 1;

                        #if defined(ENABLE_NETWORK_FREEZER)
                            //THIS IS NOT ADAPTED
                            nvmPutConnectionTableIndex(&(ConnectionTable[openSocketInfo->socketHandle]), openSocketInfo->socketHandle);
                        #endif
                    }
                }
                return;
            }
            else{
                //take a record of when you started to send the socket request
                //and send it to the PAN coordinator
                openSocketInfo->socketStart = MiWi_TickGet();
                //Juan: Execution continues below!
            }
        
        #else
            openSocketInfo->socketStart = MiWi_TickGet();
        #endif

        //Juan: I'm not the PAN Coordinator, so I'll send an open_socket_request
        //to PANC (whose short address is 0x0000).
        TempNodeInfo->shortAddr.Val = 0x0000;

        MiApp_FlushTx(mb);
        MiApp_WriteData(MIWI_STACK_REPORT_TYPE, mb);    //Report Type
        MiApp_WriteData(OPEN_SOCKET_REQUEST, mb);       //Report ID
        for(i=0; i<MY_ADDRESS_LENGTH; i++){
            MiApp_WriteData(myLongAddress[i], mb);
        }
        #if ADDITIONAL_NODE_ID_SIZE > 0
            for(i=0; i<ADDITIONAL_NODE_ID_SIZE; i++){
                MiApp_WriteData(AdditionalNodeID[i], mb);
            }
        #endif
        
        MiApp_UnicastAddress(TempNodeInfo->shortAddr.v, FALSE, FALSE, mb);
        
        openSocketInfo->status.bits.requestIsOpen = 1;
    }

    #if defined(NWK_ROLE_COORDINATOR) && defined(ENABLE_INDIRECT_MESSAGE)
    /***************************************************************************
     * Function:    IndirectPacket(BOOL Broadcast, WORD_VAL DestinationPANID,
     *                             BYTE *DestinationAddress, BOOL isCommand,
     *                             BOOL SecurityEnabled)
     * Overview:    This function store the indirect message for node that turns
     *              off radio when idle
     * PreCondition:None
     * Input:       Broadcast          - Boolean to indicate if the indirect
     *                                   message a broadcast message
     *              DestinationPANID   - The PAN Identifier of the destination
     *                                   node
     *              DestinationAddress - The pointer to the destination long
     *                                   address
     *              isCommand          - The boolean to indicate if the packet
     *                                   is command
     *              SecurityEnabled    - The boolean to indicate if the packet
     *                                   needs encryption
     * Output:      boolean to indicate if operation successful
     * Side Effects:An indirect message stored and waiting to deliever to
     *              sleeping device. An indirect message timer has started to
     *              expire the indirect message in case RFD does not acquire
     *              data in predefined interval
     **************************************************************************/
    #if defined(IEEE_802_15_4)
        BOOL SaveIndirectMessage(INPUT BOOL Broadcast, 
                                 INPUT WORD_VAL DestinationPANID, 
                                 INPUT BYTE *DestinationAddress, 
                                 INPUT BOOL isAltAddress, 
                                 INPUT BOOL SecurityEnabled,
                                 INPUT miwi_band mb)
    #else
        BOOL SaveIndirectMessage(INPUT BOOL Broadcast, 
                                 INPUT BYTE *DestinationAddress, 
                                 INPUT BOOL isAltAddress, 
                                 INPUT BOOL SecurityEnabled,
                                 INPUT miwi_band mb)
    #endif    
    {
        BYTE *TxData;
        BYTE *pbuffer;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_434
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                    #elif defined MRF49XA_2_IN_434
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                    #else   //Error
                        return FALSE;
                    #endif
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_868
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                    #elif defined MRF49XA_2_IN_868
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                    #else   //Error
                        return FALSE;
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                    return FALSE;
                #else
                    TxData = & MRF24J40_TxData;
                    pbuffer = MRF24J40_TxBuffer;
                    break;
                #endif
            default:
                return FALSE;
        }

        BYTE i, j;
        for(i=0; i<INDIRECT_MESSAGE_SIZE; i++){
            if (indirectMessages[i].flags.bits.isValid == 0){
                indirectMessages[i].flags.Val = 0;
                indirectMessages[i].flags.bits.isBroadcast = Broadcast;
                indirectMessages[i].flags.bits.isSecured = SecurityEnabled;
                indirectMessages[i].flags.bits.isValid = 1;
                indirectMessages[i].flags.bits.isAltAddr = isAltAddress;

                if (mb == ISM_2G4){             //Added mb condition
                    #if defined(IEEE_802_15_4)
                        if(isAltAddress == FALSE){
                            if(Broadcast == FALSE){
                                for(j=0; j<MY_ADDRESS_LENGTH; j++){
                                    indirectMessages[i].DestAddress[j] = DestinationAddress[j];
                                }
                            }
                        }
                        else{
                            if(Broadcast == FALSE){
                                indirectMessages[i].DestAddress[0] = DestinationAddress[0];
                                indirectMessages[i].DestAddress[1] = DestinationAddress[1];
                            }
                        }
                        indirectMessages[i].DestPANID.Val = DestinationPANID.Val;
                    #endif
                }else{
                    if(Broadcast == FALSE){
                        if(isAltAddress){
                            indirectMessages[i].DestAddress[0] = DestinationAddress[0];
                            indirectMessages[i].DestAddress[1] = DestinationAddress[1]; 
                        }
                        else{
                            for(j=0; j<MY_ADDRESS_LENGTH; j++){
                                indirectMessages[i].DestAddress[j] = DestinationAddress[j];
                            }
                        }     
                    }
                }
                
                #if defined(ENABLE_SECURITY) && defined(ENABLE_NETWORK_FREEZER) && !defined(TARGET_SMALL)
                    if(SecurityEnabled){  
                        if(isAltAddress){
                            TempNodeInfo->shortAddr.v[0] = DestinationAddress[0];
                            TempNodeInfo->shortAddr.v[1] = DestinationAddress[1];
                            j = SearchForShortAddress();
                        }
                        else{
                            for(j=0; j<MY_ADDRESS_LENGTH; j++){
                                TempNodeInfo->longAddr[j] = DestinationAddress[j];
                            }
                            j = SearchForLongAddress(mb);
                        }
                    }
                #endif
                indirectMessages[i].PayLoadSize = *TxData;
                for(j = 0; j < *TxData; j++){
                    indirectMessages[i].PayLoad[j] = *(pbuffer+j);
                }
                //Juan: Register also which miwi band is using this indirect
                //message slot for sending it with the same miwi band in the
                //future.
                indirectMessages[i].MiWiFreqBand = mb;

                indirectMessages[i].TickStart = MiWi_TickGet();
                return TRUE;
            }
        }
        return FALSE;
    }
    #endif
 

    #if defined(NWK_ROLE_COORDINATOR)
        /***********************************************************************
         * Function:        void SendBeacon(void)
         * PreCondition:    Coordinator has joined the network
         * Input:           None
         * Output:          None
         * Side Effects:    None
         * Overview:        This function sends a beacon frame.
         **********************************************************************/
        void SendBeacon(miwi_band mb){
            WORD_VAL *myPANID, *myShortAddress;
            MIWI_CAPACITY_INFO *MiWiCapacityInfo;
            BYTE knownCoordinators;
            Printf("\r\n I send a beacon");
            switch (mb){
                case ISM_434:
                    #if !defined MIWI_0434_RI
                        Printf("Error: MiWi ISM 434 MHz band is not available.");
                        return;
                    #else
                        myPANID = & myPAN0434ID;
                        myShortAddress = & myShort0434Addr;
                        MiWiCapacityInfo = & MIWI0434CapacityInfo;
                        knownCoordinators = known0434Coordinators;
                        #if defined MRF49XA_1_IN_434
                            AuxMAC_FlushTx(1);
                        #elif defined MRF49XA_2_IN_434
                            AuxMAC_FlushTx(2);
                        #else   //Error
                            return;
                        #endif
                        break;
                    #endif
                case ISM_868:
                    #if !defined MIWI_0868_RI
                        Printf("Error: MiWi ISM 868 MHz band is not available.");
                        return;
                    #else
                        myPANID = & myPAN0868ID;
                        myShortAddress = & myShort0868Addr;
                        MiWiCapacityInfo = & MIWI0868CapacityInfo;
                        knownCoordinators = known0868Coordinators;
                        #if defined MRF49XA_1_IN_868
                            AuxMAC_FlushTx(1);
                        #elif defined MRF49XA_2_IN_868
                            AuxMAC_FlushTx(2);
                        #else   //Error
                            return;
                        #endif
                        break;
                    #endif
                case ISM_2G4:
                    #if !defined MIWI_2400_RI
                        Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                        return;
                    #else
                        myPANID = & myPAN2400ID;
                        myShortAddress = & myShort2400Addr;
                        MiWiCapacityInfo = & MIWI2400CapacityInfo;
                        knownCoordinators = known2400Coordinators;
                        AuxMAC_FlushTx(3);
                        break;
                    #endif
                default:
                    return;
            }           
            BYTE i;
            if(mb != ISM_2G4){
                //Juan: added mb condition. #if commented for enabling subGHz
                //transceivers execute this part.
                //#if !defined(IEEE_802_15_4)
                    MiApp_WriteData(myPANID->v[0], mb);
                    MiApp_WriteData(myPANID->v[1], mb);
                    MiApp_WriteData(myShortAddress->v[0], mb);
                    MiApp_WriteData(myShortAddress->v[1], mb);
                //#endif
            }   //Juan: no else. No matter if 802.15.4 is not defined, the code
                //below is executed by all transceivers.
            MiApp_WriteData(0xFF, mb);  //superframe specification (BO = 0xF, SO = 0xF)
            MiApp_WriteData(MiWiCapacityInfo->Val, mb);
            MiApp_WriteData(0x00, mb);              // GTS
            MiApp_WriteData(0x00, mb);              // Pending addresses
            MiApp_WriteData(MIWI_PROTOCOL_ID, mb);  // Protocol ID
            MiApp_WriteData(MIWI_VERSION_NUM, mb);  // Version Number
            MiApp_WriteData(knownCoordinators, mb);
            #if ADDITIONAL_NODE_ID_SIZE > 0 
                for(i=0; i<ADDITIONAL_NODE_ID_SIZE; i++){
                    MiApp_WriteData(AdditionalNodeID[i], mb);
                }
            #endif
           
            #if defined(IEEE_802_15_4)
                //Juan: Packet type reserve is the same than packet type beacon.
                SendMACPacket(myPANID->v, NULL, PACKET_TYPE_RESERVE, MSK_ALT_SRC_ADDR, mb);
            #else
                SendMACPacket(NULL, PACKET_TYPE_RESERVE, mb);
            #endif   
        }    
    #endif

    /***************************************************************************
     * Function:    BYTE SearchForShortAddress(void)
     * PreCondition:tempShortAddress and tempPANID are set to the device that
     *              you are looking for
     * Input:       BYTE entryType - this is set to NEIGHBOR if you want to find
     *              a Node on the network. This value is set to NETWORK if you
     *              are looking for a specific network and not a node on the
     *              current operating network
     * Output:      BYTE - the index of the network table entry of the requested
     *              device. 0xFF indicates that the requested device doesn't
     *              exist in the network table
     * Side Effects:None
     * Overview:    This function looks up the index of a node or network in the
     *              network table by short address.
     **************************************************************************/
    BYTE SearchForShortAddress(miwi_band mb){
        BYTE i;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return 0xFF;        //Invalid index
                #else
                    for(i=0; i<CONNECTION_SIZE; i++){
                        if(ConnectionTable[i].status.bits.isValid && \
                           ConnectionTable[i].status.bits.short0434AddrValid){
                            if(ConnectionTable[i].MIWI0434AltAddress.Val == \
                               temp0434.shortAddr.Val){
                                return i;
                            }
                        }
                    }
                    return 0xFF;        //Invalid index. Not found.
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return 0xFF;        //Invalid index
                #else
                    for(i=0; i<CONNECTION_SIZE; i++){
                        if(ConnectionTable[i].status.bits.isValid && \
                           ConnectionTable[i].status.bits.short0868AddrValid){
                            if(ConnectionTable[i].MIWI0868AltAddress.Val == \
                               temp0868.shortAddr.Val){
                                return i;
                            }
                        }
                    }
                    return 0xFF;        //Invalid index. Not found.
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz band is not available.");
                    return 0xFF;        //Invalid index
                #else
                    for(i=0; i<CONNECTION_SIZE; i++){
                        if(ConnectionTable[i].status.bits.isValid && \
                           ConnectionTable[i].status.bits.short2400AddrValid){
                            if(ConnectionTable[i].MIWI2400AltAddress.Val == \
                               temp2400.shortAddr.Val){
                                return i;
                            }
                        }
                    }
                    return 0xFF;        //Invalid index. Not found.
                #endif
            default:
                return 0xFF;        //Invalid index
        }
    }

    /***************************************************************************
     * Function:    BYTE SearchForLongAddress(void)
     * PreCondition:tempLongAddress is set to the device that you are looking
     *              for
     * Input:       None
     * Output:      BYTE - the index of the network table entry of the requested
     *                     device. 0xFF indicates that the requested device
     *                     doesn't exist in the network table
     * Side Effects:None
     * Overview:    This function looks up the index of a node or network in the
     *              network table by long address.
     **************************************************************************/
    BYTE SearchForLongAddress(miwi_band mb){
        BYTE i,j;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return 0xFF;        //Invalid index
                #else
                    for(i=0; i<CONNECTION_SIZE; i++){
                        if(ConnectionTable[i].status.bits.isValid && \
                           ConnectionTable[i].status.bits.longAddressValid){
                            for(j=0; j<MY_ADDRESS_LENGTH; j++){
                                if(ConnectionTable[i].Address[j] != temp0434.longAddr[j]){
                                    break;              //Continue for next 'i'
                                }
                                if(j == MY_ADDRESS_LENGTH-1)        //Match
                                    return i;
                            }
                        }
                    }
                    return 0xFF;        //Invalid index. Not found.
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return 0xFF;        //Invalid index
                #else
                    for(i=0; i<CONNECTION_SIZE; i++){
                        if(ConnectionTable[i].status.bits.isValid && \
                           ConnectionTable[i].status.bits.longAddressValid){
                            for(j=0; j<MY_ADDRESS_LENGTH; j++){
                                if(ConnectionTable[i].Address[j] != temp0868.longAddr[j]){
                                    break;              //Continue for next 'i'
                                }
                                if(j == MY_ADDRESS_LENGTH-1)        //Match
                                    return i;
                            }
                        }
                    }
                    return 0xFF;        //Invalid index. Not found.
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz band is not available.");
                    return 0xFF;        //Invalid index
                #else
                    for(i=0; i<CONNECTION_SIZE; i++){
                        if(ConnectionTable[i].status.bits.isValid && \
                           ConnectionTable[i].status.bits.longAddressValid){
                            for(j=0; j<MY_ADDRESS_LENGTH; j++){
                                if(ConnectionTable[i].Address[j] != temp2400.longAddr[j]){
                                    break;              //Continue for next 'i'
                                }
                                if(j == MY_ADDRESS_LENGTH-1)        //Match
                                    return i;
                            }
                        }
                    }
                    return 0xFF;        //Invalid index. Not found.
                #endif
            default:
                return 0xFF;        //Invalid index
        }
    }
    
    /***************************************************************************
     * Function:    BYTE AddNodeToNetworkTable(void)
     * PreCondition:tempLongAddress, tempShortAddress, tempPANID, and 
     *              tempNodeStatus are set to the correct values.
     *              If tempNodeStatus.bits.longAddressValid is set then
     *              tempLongAddress needs to be set. 
     *              If tempNodeStatus.bits.shortAddressValid is set then
     *              tempShortAddress and tempPANID need to be set.
     * Input:       None
     * Output:      BYTE - the index of the network table entry where the device
     *                     was inserted. 0xFF indicates that the requested
     *                     device couldn't be inserted into the table
     * Side Effects:Network table is updated with the devices info
     * Overview:    This function is used to insert new device into the network
     *              table (or update already existing entries)
     **************************************************************************/
    //Juan: This function is similar to addConnection in P2P... ¬¬
    //Handle <-> ConnectionIndex; NetworkTable <-> ConnectionTable
    //Instead of checking received_message addr, it checks long and short addr
    //stored in global variables tempLongAddr and tempShortAddr. These variables
    //have been put together with tempNodeStatus in a struct named TempNodeInfo.
    //Field names have been shortened. There is one TempNodeInfo per interface.
    BYTE AddNodeToNetworkTable(miwi_band mb){
        BYTE handle = 0xFF;             //Juan: by now, invalid connection index
        BYTE RI_MASK;
        BOOL shortAddrValid;
        TEMP_NODE_INFO *tempNodeInfo;
        if(mb == ISM_434){
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return handle;
            #else
                RI_MASK = MIWI_0434_RI_MASK;
                tempNodeInfo = & temp0434;
                shortAddrValid = temp0434.status.bits.short0434AddrValid;
            #endif
        } else if(mb == ISM_868){
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return handle;
            #else
                RI_MASK = MIWI_0868_RI_MASK;
                tempNodeInfo = & temp0868;
                shortAddrValid = temp0868.status.bits.short0868AddrValid;
            #endif
        } else if(mb == ISM_2G4){
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return handle;
            #else
                RI_MASK = MIWI_2400_RI_MASK;
                tempNodeInfo = & temp2400;
                shortAddrValid = temp2400.status.bits.short2400AddrValid;
            #endif
        }
        else {return handle;}    //Error

        if(tempNodeInfo->status.bits.longAddressValid){
            //Juan: Check if a connection slot has the same long address (node)
            handle = SearchForLongAddress(mb);
            
        }
        else{
            if(shortAddrValid){
                //Juan: Check if a connection slot has the same short address
                handle = SearchForShortAddress(mb);
            }
        }
                                       
        if(handle==0xFF){
            //Juan: Node not found. Register the node in a free connection slot.
            handle = findNextNetworkEntry();
        }

        //Juan: By reaching this point, if handle is 0xFF the node is not
        //registered but we don't have enough space in ConnectionTable to store
        //it. Otherwise, we may update the node information in the table.
        if(handle != 0xFF){ 
            //isFamily = 1, RxOnWhenIdle = 1, Is a neighbor and not a network,
            //not a P2P connection, short and long addresses valid as well 
            //as the entire entry
            
            //If it is not family then update the node information

            #if defined(ENABLE_SECURITY)
                IncomingFrameCounter[handle].Val = 0;
            #endif
 
            //Juan: added the second condition for not overwriting the long addr
            if((tempNodeInfo->status.bits.longAddressValid) && (ConnectionTable[handle].status.bits.longAddressValid == 0)){
                BYTE i;
                for(i = 0; i < MY_ADDRESS_LENGTH; i++){
                    ConnectionTable[handle].Address[i] = tempNodeInfo->longAddr[i];
                }
            }

            //Juan: modified because of the new arrangement of status structure
            //and the new flags included. Let's write the short address related
            //to the current interface only.

            //Common flags
            ConnectionTable[handle].status.bits.RXOnWhenIdle = tempNodeInfo->status.bits.RXOnWhenIdle;
            ConnectionTable[handle].status.bits.longAddressValid = tempNodeInfo->status.bits.longAddressValid;
            ConnectionTable[handle].status.bits.isValid = 1;

            //RI flags
            switch (mb){
                case ISM_434:
                    #if defined MIWI_0434_RI
                        ConnectionTable[handle].PAN0434ID.Val = tempNodeInfo->PANID.Val;
                        //434 flags and 434 short address.
                        ConnectionTable[handle].status.byte[0] |= (tempNodeInfo->status.byte[0] & 0xF0);
                        if(shortAddrValid){
                            ConnectionTable[handle].MIWI0434AltAddress.Val = tempNodeInfo->shortAddr.Val;
                        }
                    #endif
                    break;
                case ISM_868:
                    #if defined MIWI_0868_RI
                        ConnectionTable[handle].PAN0868ID.Val = tempNodeInfo->PANID.Val;
                        //868 flags and 868 short address.
                        ConnectionTable[handle].status.byte[1] |= (tempNodeInfo->status.byte[1] & 0x0F);
                        if(shortAddrValid){
                            ConnectionTable[handle].MIWI0868AltAddress.Val = tempNodeInfo->shortAddr.Val;
                        }
                    #endif
                    break;
                case ISM_2G4:
                    #if defined MIWI_2400_RI
                        ConnectionTable[handle].PAN2400ID.Val = tempNodeInfo->PANID.Val;
                        //2400 flags and 2400 short address.
                        ConnectionTable[handle].status.byte[1] |= (tempNodeInfo->status.byte[1] & 0xF0);
                        if(shortAddrValid){
                            ConnectionTable[handle].MIWI2400AltAddress.Val = tempNodeInfo->shortAddr.Val;
                        }
                    #endif
                    break;
                default:
                    return 0xFF;
            }
            //Juan: added.
            ConnectionTable[handle].MiWiInterfaces |= RI_MASK;
        }

        return handle;
    }

    /***************************************************************************
     * FUnction:    BOOL isSameAddress(BYTE *Address1, BYTE *Address2)
     * Overview:    This function compares two long addresses and returns the
     *              boolean to indicate if they are the same
     * PreCondition:None
     * Input:       Address1 - Pointer to the first long address to be compared
     *              Address2 - Pointer to the second long address to be compared
     * Output:      If the two address are the same
     * Side Effects:
     **************************************************************************/
    BOOL isSameAddress(INPUT BYTE *Address1, INPUT BYTE *Address2){
        BYTE i;
        
        for(i=0; i<MY_ADDRESS_LENGTH; i++){
            if(Address1[i] != Address2[i]){
                return FALSE;
            }
        }
        return TRUE;
    }

    #if defined(ENABLE_SLEEP)
    /**************************************************************************
     * FUnction:    BOOL CheckForData(void)
     * Overview:    This function sends out a Data Request to the peer device of
     *              the first P2P connection.
     * PreCondition:Transceiver is initialized and fully waken up
     * Input:       None
     * Output:      None
     * Side Effects:The MiWi stack is waiting for the response from the peer
     *              device. A data request timer has been started. In case there
     *              is no response from the peer device, the data request will
     *              time-out itself
     **************************************************************************/
     BOOL CheckForData(INPUT miwi_band mb){
        BYTE tmpTxData, transceiver;
        BYTE myParent, i, aux;
        BYTE *TxData;
        BYTE *pbuffer;
        MIWI_TICK *drtimer;
        MIWI_STATE_MACHINE *msm;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_434
                        tmpTxData = MRF49XA_1_TxData;
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                        msm = &MRF49XA_1_MiWiStateMachine;
                        drtimer = &MRF49XA_1_DataReqTimer;
                        transceiver = 1;
                        aux = 0;
                    #elif defined MRF49XA_2_IN_434
                        tmpTxData = MRF49XA_2_TxData;
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                        msm = &MRF49XA_2_MiWiStateMachine;
                        drtimer = &MRF49XA_2_DataReqTimer;
                        transceiver = 2;
                        aux = 0;
                    #else   //Error
                        return FALSE;
                    #endif
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return FALSE;
                #else
                    #if defined MRF49XA_1_IN_868
                        tmpTxData = MRF49XA_1_TxData;
                        TxData = & MRF49XA_1_TxData;
                        pbuffer = MRF49XA_1_TxBuffer;
                        msm = &MRF49XA_1_MiWiStateMachine;
                        drtimer = &MRF49XA_1_DataReqTimer;
                        transceiver = 1;
                        aux = 1;
                        myParent = & my0868Parent;
                    #elif defined MRF49XA_2_IN_868
                        tmpTxData = MRF49XA_2_TxData;
                        TxData = & MRF49XA_2_TxData;
                        pbuffer = MRF49XA_2_TxBuffer;
                        msm = &MRF49XA_2_MiWiStateMachine;
                        drtimer = &MRF49XA_2_DataReqTimer;
                        transceiver = 2;
                        aux = 1;
                    #else   //Error
                        return FALSE;
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                    return FALSE;
                #else
                    tmpTxData = MRF24J40_TxData;
                    TxData = & MRF24J40_TxData;
                    pbuffer = MRF24J40_TxBuffer;
                    msm = &MRF24J40_MiWiStateMachine;
                    drtimer = & MRF24J40_DataReqTimer;
                    transceiver = 3;
                    aux = 3;
                    break;
                #endif
            default:
                return FALSE;
        }
        BYTE firstByte = *pbuffer;
        
        AuxMAC_FlushTx(transceiver);
        MiApp_WriteData(MAC_COMMAND_DATA_REQUEST, mb);
        
        for (i=0; i<CONNECTION_SIZE; i++){
            if (ConnectionTable[i].status.bits.isValid && (ConnectionTable[i].MiWiInterfaces & (0x01 << (aux))) ){
                myParent = i;  
            }
        }
        
        #if defined(ENABLE_ENHANCED_DATA_REQUEST)
            if(tmpTxData > PAYLOAD_START){
                *TxData = tmpTxData;
                
                MTP.flags.Val = 0;#endif
                MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                MTP.flags.bits.ackReq = 1;
                MTP.flags.bits.sourcePrsnt = 1;
                if((*(pbuffer+1) & 0x01) > 0){
                    MTP.flags.bits.secEn = 1;
                    *(pbuffer+1) &= 0xFE;
                }

                if(transceiver==3)      //Juan
                    #if defined(IEEE_802_15_4)
                        MTP.altDestAddr = TRUE;
                        MTP.altSrcAddr = TRUE;

                        MTP.DestPANID.Val = ConnectionTable[myParent].PANID.Val;
                        MTP.DestAddress = ConnectionTable[myParent].AltAddress.v;
                    #endif
                }else{
                         
                    BYTE i;
                    for(i=tmpTxData; i>1; i--){
                        *(pbuffer+i+1) = *(pbuffer+i-1);
                    }
                    *TxData += 2;
                       
                    *(pbuffer+1) = myShortAddress.v[0];
                    *(pbuffer+2) = myShortAddress.v[1];
                    MTP.DestAddress = ConnectionTable[myParent].Address;
                }

                if(AuxMAC_SendPacket(&MTP, transceiver)){
                    msm->bits.DataRequesting = 1;
                    *drtimer = MiWi_TickGet();
                    return TRUE;
                }
                return FALSE;    
            }    
        #endif


     
        WORD_VAL AuxPANID, AuxAltAddress;
        switch (mb){
            case ISM_434:
                #if defined MIWI_0434_RI
                    #if defined(IEEE_802_15_4)
                        AuxPANID.Val = ConnectionTable[myParent].PAN0434ID.Val;
                        AuxAltAddress.Val = ConnectionTable[myParent].MIWI0434AltAddress.Val;
                        break;
                    #else
                        AuxAltAddress.Val = myShort0434Addr.Val;
                    #endif
                #endif
            case ISM_868:
                #if defined MIWI_0868_RI
                    #if defined(IEEE_802_15_4)
                        AuxPANID.Val = ConnectionTable[myParent].PAN0868ID.Val;
                        AuxAltAddress.Val = ConnectionTable[myParent].MIWI0868AltAddress.Val;
                        break;
                    #else
                        AuxAltAddress.Val = myShort0868Addr.Val;
                    #endif
                #endif
            case ISM_2G4:
                #if defined MIWI_2400_RI
                    #if defined(IEEE_802_15_4)
                        AuxPANID.Val = ConnectionTable[myParent].PAN2400ID.Val;
                        AuxAltAddress.Val = ConnectionTable[myParent].MIWI2400AltAddress.Val;
                        break;
                    #else
                        AuxAltAddress.Val = myShort2400Addr.Val;
                    #endif
                #endif
            default:
                return FALSE;
        }
        


        #if defined(IEEE_802_15_4)
        
            if(SendMACPacket(AuxPANID.v, AuxAltAddress.v, PACKET_TYPE_COMMAND, MSK_ALT_DST_ADDR|MSK_ALT_SRC_ADDR, mb))
        #else
        MiApp_WriteData(AuxAltAddress.v[0], mb);
        MiApp_WriteData(AuxAltAddress.v[1], mb);

        if(SendMACPacket(ConnectionTable[myParent].Address, PACKET_TYPE_COMMAND, mb))
        #endif
        {
            msm->bits.DataRequesting = 1;
            *drtimer = MiWi_TickGet();
            *TxData = tmpTxData;
            #if defined(ENABLE_TIME_SYNC)
                #if defined(__18CXX)
                    TMR3H = 0;
                    TMR3L = 0;
                #elif defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
                    PR1 = 0xFFFF;
                    TMR1 = 0;
                #elif defined(__PIC32MX__)
                    PR1 = 0xFFFF;
                    while(T1CONbits.TWIP) ;
                    TMR1 = 0;
                #endif
            #endif
            return TRUE;
        }
        *TxData = tmpTxData;
        return FALSE;
    }
    #endif

/*******************************************************************************
 ******************************************************************************/
BOOL MiApp_ProtocolInit(BOOL bNetworkFreezer){
    MACINIT_PARAM MIP;
    BYTE i;

    #if defined(ENABLE_NVM)
        #if defined(ENABLE_NVM_MAC)
            if(MY_ADDRESS_LENGTH > 6){
                for(i = 0; i < 3; i++){
                    EEPROMRead(&(myLongAddress[MY_ADDRESS_LENGTH-1-i]), EEPROM_MAC_ADDR+i, 1);
                }
                myLongAddress[4] = 0xFF;
                if(MY_ADDRESS_LENGTH > 7){
                    myLongAddress[3] = 0xFE;
                }
                for(i=0; i<3; i++){
                    EEPROMRead(&(myLongAddress[2-i]), EEPROM_MAC_ADDR+3+i, 1);
                }
            }
            else{
                for(i=0; i<MY_ADDRESS_LENGTH; i++){
                    EEPROMRead(&(myLongAddress[MY_ADDRESS_LENGTH-1-i]), EEPROM_MAC_ADDR+i, 1);
                }
            }
        #endif
    #endif

    #if defined(ENABLE_NETWORK_FREEZER)
        NVMInit();
    #endif

    #if defined MIWI_2400_RI
        myShort2400Addr.Val = 0xFFFF;
        myPAN2400ID.Val = MY_PAN_ID;
        Socket2400Info.status.Val = 0;
        MIWI2400CapacityInfo.Val = 0;
    #endif
    #if defined MIWI_0868_RI
        myShort0868Addr.Val = 0xFFFF;
        myPAN0868ID.Val = MY_PAN_ID + 1;
        Socket0868Info.status.Val = 0;
        MIWI0868CapacityInfo.Val = 0;
    #endif
    #if defined MIWI_0434_RI
        myShort0434Addr.Val = 0xFFFF;
        myPAN0434ID.Val = MY_PAN_ID + 2;
        Socket0434Info.status.Val = 0;
        MIWI0434CapacityInfo.Val = 0;
    #endif

    //MiMAC_SetAltAddress(myShortAddress.v, myPANID.v);

    //clear the network table
    for(i=0; i<CONNECTION_SIZE; i++){
        ConnectionTable[i].status.Val = 0;
    }

    #ifdef NWK_ROLE_COORDINATOR
        for(i=0; i<8; i++){
            #ifdef MIWI_0434_RI
                Routing0434Table[i] = 0;
                Router0434Failures[i] = 0;
                known0434Coordinators = 0;
            #endif
            #ifdef MIWI_0868_RI
                Routing0868Table[i] = 0;
                Router0868Failures[i] = 0;
                known0868Coordinators = 0;
            #endif
            #ifdef MIWI_2400_RI
                Routing2400Table[i] = 0;
                Router2400Failures[i] = 0;
                known2400Coordinators = 0;
            #endif
        }
        role = ROLE_FFD_END_DEVICE;
    #endif

    #if defined MRF49XA_1
        MRF49XA_1_MiWiStateMachine.Val = 0;
        MRF49XA_1_TxData = 0;
    #endif
    #if defined MRF49XA_2
        MRF49XA_2_MiWiStateMachine.Val = 0;
        MRF49XA_2_TxData = 0;
    #endif
    #if defined MRF24J40
        MRF24J40_MiWiStateMachine.Val = 0;
        MRF24J40_TxData = 0;
    #endif

    InitSymbolTimer();      //Initialises MiWiTick - Timers 2 & 3 (32 bit mode)

    #if defined ENABLE_INDIRECT_MESSAGE && defined NWK_ROLE_CORDINTATOR
        for(i=0; i<INDIRECT_MESSAGE_SIZE; i++){
            indirectMessages[i].flags.Val = 0;
        }
    #endif

    #if defined(ENABLE_SLEEP) && defined(ENABLE_BROADCAST_TO_SLEEP_DEVICE)
        for(i=0; i<BROADCAST_RECORD_SIZE; i++){
            BroadcastRecords[i].RxCounter = 0;
        }
    #endif

    #if defined(ENABLE_SECURITY)
        for(i=0; i<CONNECTION_SIZE; i++){
            IncomingFrameCounter[i].Val = 0;
        }
    #endif

    #if defined(ENABLE_NETWORK_FREEZER)
        // WARNING NVM NOT ADAPTED!!! Needs modifying nvm functions (which
        // radio interface is involved)
        if(bNetworkFreezer){
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

            //Juan: get short address, connmode, PANID, myParent... of every
            //radio interface... REQUIRES NVM ADAPTATION
            nvmGetMyShort0Address(myShortAddress.v);
            if(myShortAddress.Val == 0xFFFF){
                return FALSE;
            }

            nvmGetMyPANID(myPANID.v);
            nvmGetConnMode(&ConnMode);
            MiWiCapacityInfo.bits.ConnMode = ConnMode;
            nvmGetConnectionTable(ConnectionTable);
            nvmGetMyShortAddress(myShortAddress.v);
            nvmGetMyParent(&myParent);
            #if defined(NWK_ROLE_COORDINATOR)
                nvmGetRoutingTable(RoutingTable);
                nvmGetKnownCoordinators(&knownCoordinators);
                nvmGetRole(&role);
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

            Printf(" PANID:");
            PrintChar(myPANID.v[1]);
            PrintChar(myPANID.v[0]);
            Printf("ShortAddr:");
            PrintChar(myShortAddress.v[1]);
            PrintChar(myShortAddress.v[0]);

            MiMAC_SetAltAddress(myShortAddress.v, myPANID.v);
            #if defined MRF49XA_1
                MRF49XA_1_MiWiStateMachine.bits.memberOfNetwork = 1;
            #endif
            #if defined MRF49XA_2
                MRF49XA_2_MiWiStateMachine.bits.memberOfNetwork = 1;
            #endif
            #if defined MRF24J40
                MRF24J40_MiWiStateMachine.bits.memberOfNetwork = 1;
            #endif
        }
        else{
            nvmPutMyPANID(myPANID.v);
            #if defined MIWI_0434_RI
                nvmPutCurrentChannel(&MIWI0434_currentChannel);
            #endif
            #if defined MIWI_0868_RI
                nvmPutCurrentChannel(&MIWI0868_currentChannel);
            #endif
            #if defined MIWI_2400_RI
                nvmPutCurrentChannel(&MIWI2400_currentChannel);
            #endif
            nvmPutConnMode(&ConnMode);
            nvmPutConnectionTable(ConnectionTable);

            nvmPutMyShortAddress(myShortAddress.v);
            nvmPutMyParent(&myParent);
            #if defined(NWK_ROLE_COORDINATOR)
                nvmPutRoutingTable(RoutingTable);
                nvmPutKnownCoordinators(&knownCoordinators);
                nvmPutRole(&role);
            #endif
        }
    #endif

    MIP.actionFlags.Val = 0;
    MIP.actionFlags.bits.PAddrLength = MY_ADDRESS_LENGTH;
    MIP.actionFlags.bits.CCAEnable = 1;
    MIP.actionFlags.bits.NetworkFreezer = bNetworkFreezer;
    MIP.PAddress = myLongAddress;

    #if defined MRF49XA_1
        MiMAC_MRF49XA_Init(MIP, 1);
        #if defined MRF49XA_1_IN_434
            MiApp_SetChannel(MIWI0434_currentChannel + MIWI0434ConfChannelOffset, ISM_434);
        #elif defined MRF49XA_1_IN_868
            MiApp_SetChannel(MIWI0868_currentChannel + MIWI0868ConfChannelOffset, ISM_868);
        #endif
    #endif
    #if defined MRF49XA_2
        MiMAC_MRF49XA_Init(MIP, 2);
        #if defined MRF49XA_2_IN_434
            MiApp_SetChannel(MIWI0434_currentChannel + MIWI0434ConfChannelOffset, ISM_434);
        #elif defined MRF49XA_2_IN_868
            MiApp_SetChannel(MIWI0868_currentChannel + MIWI0868ConfChannelOffset, ISM_868);
        #endif
    #endif
    #if defined MRF24J40
        MiMAC_MRF24J40_Init(MIP);
        MiApp_SetChannel(MIWI2400_currentChannel + MIWI2400ConfChannelOffset, ISM_2G4);
    #endif

    //Juan: added the 2nd condition for only setting 2,4GHz shortAddr and PANID.
    #if defined(IEEE_802_15_4) && defined MIWI_2400_RI
        // Microchip LSI Stack - Only MRF24J40 supports Alt. Address in MAC
        // layer.
        MiMAC_SetAltAddress(myShort2400Addr.v, myPAN2400ID.v);
    #endif


    MIWI_CAPACITY_INFO MiWiCapacityInfo;
    MiWiCapacityInfo.Val = 0;
    #if !defined(ENABLE_SLEEP)
        MiWiCapacityInfo.bits.Sleep = 1;
    #endif
    #if defined(ENABLE_SECURITY)
        MiWiCapacityInfo.bits.Security = 1;
    #endif
    #ifdef NWK_ROLE_COORDINATOR
        MiWiCapacityInfo.bits.CoordCap = 1;
        MiWiCapacityInfo.bits.Role = role;
    #endif
    #if defined MIWI_0434_RI
        MIWI0434CapacityInfo.Val = MiWiCapacityInfo.Val | (MIWI0434_ConnMode << 4);
    #endif
    #if defined MIWI_0868_RI
        MIWI0868CapacityInfo.Val = MiWiCapacityInfo.Val | (MIWI0868_ConnMode << 4);
    #endif
    #if defined MIWI_2400_RI
        MIWI2400CapacityInfo.Val = MiWiCapacityInfo.Val | (MIWI2400_ConnMode << 4);
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

    BYTE kindOfRandom = TMRL;
    #if defined MIWI_0434_RI
        MIWI0434SeqNum = kindOfRandom;
    #endif
    #if defined MIWI_0868_RI
        MIWI0868SeqNum = kindOfRandom << 2;
    #endif
    #if defined MIWI_2400_RI
        MIWI2400SeqNum = kindOfRandom << 4;
    #endif

    #if defined MRF24J40
        MRF24J40_IF = 0;
        MRF24J40_IE = 1;
    #endif
    #if defined MRF49XA_1
        MRF49XA_1_IF = 0;
        MRF49XA_1_IE = 1;
    #endif
    #if defined MRF49XA_2
        MRF49XA_2_IF = 0;
        MRF49XA_2_IE = 1;
    #endif

    return TRUE;
}
    
    #if defined(ENABLE_SLEEP)
    /***************************************************************************
     * Function:    BYTE    MiApp_TransceiverPowerState(BYTE Mode)
     * Summary:     This function put the RF transceiver into different power
     *              state. i.e. Put the RF transceiver into sleep or wake it up.
     * Description: This is the primary user interface functions for the
     *              application layer to put RF transceiver into sleep or wake
     *              it up. This function is only available to those wireless
     *              nodes that may have to disable the transceiver to save
     *              battery power.
     * PreCondition:Protocol initialization has been done. 
     * Parameters:  BYTE Mode - The mode of power state for the RF transceiver
     *                          to be set. The possible power states are the
     *                          following:
     *                  * POWER_STATE_SLEEP     The deep sleep mode for RF
     *                                          transceiver
     *                  * POWER_STATE_WAKEUP    Wake up state, or operating
     *                                          state for RF transceiver
     *                  * POWER_STATE_WAKEUP_DR Put device into wakeup mode and 
     *                                          then transmit a data request to
     *                                          the device's associated device
     * Returns:     The status of the operation. The possible status are:
     *                  * SUCCESS           Operation successful
     *                  * ERR_TRX_FAIL      Transceiver fails to go to sleep or
     *                                      wake up
     *                  * ERR_TX_FAIL       Transmission of Data Request command
     *                                      failed. Only available if the input
     *                                      mode is POWER_STATE_WAKEUP_DR.
     *                  * ERR_RX_FAIL       Failed to receive any response to
     *                                      Data Request command. Only available
     *                                      if input is POWER_STATE_WAKEUP_DR.
     *                  * ERR_INVLAID_INPUT Invalid input mode.
     * Example:
     *      <code>
     *      // put RF transceiver into sleep
     *      MiApp_TransceiverPowerState(POWER_STATE_SLEEP;
     *      // Put the MCU into sleep
     *      Sleep();    
     *      // wakes up the MCU by WDT, external interrupt or any other means
     *      // make sure that RF transceiver to wake up and send out Data Request
     *      MiApp_TransceiverPowerState(POWER_STATE_WAKEUP_DR);
     *      </code>
     * Remarks:     None
     **************************************************************************/
    BYTE MiApp_TransceiverPowerState(BYTE Mode, miwi_band mb){
        MIWI_STATE_MACHINE *msm;
        BYTE transceiver;
        switch (mb){
            case ISM_434:
                #if !defined MIWI_0434_RI
                    Printf("Error: MiWi ISM 434 MHz band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    #if defined MRF49XA_1_IN_434
                        msm = &MRF49XA_1_MiWiStateMachine;
                        transceiver = 1;
                    #elif defined MRF49XA_2_IN_434
                        msm = &MRF49XA_2_MiWiStateMachine;
                        transceiver = 2;
                    #else   //Error
                        return ERR_INVALID_INPUT;
                    #endif
                    break;
                #endif
            case ISM_868:
                #if !defined MIWI_0868_RI
                    Printf("Error: MiWi ISM 868 MHz band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    #if defined MRF49XA_1_IN_868
                        msm = &MRF49XA_1_MiWiStateMachine;
                        transceiver = 1;
                    #elif defined MRF49XA_2_IN_868
                        msm = &MRF49XA_2_MiWiStateMachine;
                        transceiver = 2;
                    #else   //Error
                        return ERR_INVALID_INPUT;
                    #endif
                    break;
                #endif
            case ISM_2G4:
                #if !defined MIWI_2400_RI
                    Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                    return ERR_INVALID_INPUT;
                #else
                    msm = &MRF24J40_MiWiStateMachine;
                    transceiver = 3;
                    break;
                #endif
            default:
                return ERR_INVALID_INPUT;
        }

        BYTE status;
        BOOL ok;

        switch(Mode){
            case POWER_STATE_SLEEP:
                {
                    #if defined(ENABLE_NETWORK_FREEZER)
                        if(msm->bits.saveConnection){
                            nvmPutConnectionTable(ConnectionTable);
                            msm->bits.saveConnection = 0;
                        }
                    #endif
                    #if defined MRF49XA_1
                        if(transceiver == 1){
                            ok = MiMAC_MRF49XA_PowerState(POWER_STATE_DEEP_SLEEP, 1);
                        }else
                    #endif
                    #if defined MRF49XA_2
                        if( transceiver ==2){
                            ok = MiMAC_MRF49XA_PowerState(POWER_STATE_DEEP_SLEEP, 2);
                        }else
                    #endif
                    #if defined MRF24J40
                        if(transceiver ==3){
                            ok = MiMAC_MRF24J40_PowerState(POWER_STATE_DEEP_SLEEP);
                        }
                    #else
                        {}
                    #endif
                    if(ok){
                        msm->bits.Sleeping = 1;
                        return SUCCESS;
                    }
                    return ERR_TRX_FAIL;
                }
                
            case POWER_STATE_WAKEUP:
                {
                    #if defined MRF49XA_1
                        if(transceiver == 1){
                            ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 1);
                        }else
                    #endif
                    #if defined MRF49XA_2
                        if( transceiver ==2){
                            ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 2);
                        }else
                    #endif
                    #if defined MRF24J40
                        if(transceiver == 3){
                            ok = MiMAC_MRF24J40_PowerState(POWER_STATE_OPERATE);
                        }
                    #else
                        {}
                    #endif
                    if(ok){
                        msm->bits.Sleeping = 0;
                        return SUCCESS;
                    }
                    return ERR_TRX_FAIL;
                }
               
            case POWER_STATE_WAKEUP_DR:
                {
                    #if defined MRF49XA_1
                        if(transceiver == 1){
                            ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 1);
                        }else
                    #endif
                    #if defined MRF49XA_2
                        if( transceiver ==2){
                            ok = MiMAC_MRF49XA_PowerState(POWER_STATE_OPERATE, 2);
                        }else
                    #endif
                    #if defined MRF24J40
                        if(transceiver ==3){
                            ok = MiMAC_MRF24J40_PowerState(POWER_STATE_OPERATE);
                        }
                    #else
                        {}
                    #endif

                    if(ok==FALSE){
                        return ERR_TRX_FAIL;
                    } 
                    msm->bits.Sleeping = 0;
                    if(CheckForData(mb)==FALSE){
                        Printf("\r\n NO DATA FOR ME :,(");
                        return ERR_TX_FAIL;
                    }
                    while(msm->bits.DataRequesting){
                        MiWiTasks();
                    }
                    return SUCCESS;
                }
                
             default:
                break;
        }
        return ERR_INVALID_INPUT;    
    }    
    #endif
    
    
BOOL MiApp_SetChannel(BYTE channel, miwi_band mb){
    switch (mb){
        case ISM_434:
            #ifndef MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 1))
                #elif defined MRF49XA_2_IN_434
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 2))
                #else
                    return FALSE;       //Error
                #endif
                    {
                        MIWI0434_currentChannel = channel;
                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutCurrentChannel(&MIWI0434_currentChannel);
                        #endif
                        return TRUE;
                    }
                    return FALSE;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 1))
                #elif defined MRF49XA_2_IN_868
                    if(MiMAC_MRF49XA_SetChannel(channel, 0, 2))
                #else
                    return FALSE;       //Error
                #endif
                    {
                        MIWI0868_currentChannel = channel;
                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutCurrentChannel(&MIWI0868_currentChannel);
                        #endif
                        return TRUE;
                    }
                    return FALSE;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return FALSE;
            #else
                if(MiMAC_MRF24J40_SetChannel(channel, 0)){
                    MIWI2400_currentChannel = channel;
                    #if defined(ENABLE_NETWORK_FREEZER)
                        nvmPutCurrentChannel(&MIWI2400_currentChannel);
                    #endif
                    return TRUE;
                }
                return FALSE;
            #endif
        default:
            return FALSE;   //Error.
    }
}    
    
/*******************************************************************************
 * Function:    void MiApp_ConnectionMode(BYTE Mode)
 * Summary:     This function set the current connection mode.
 * Description: This is the primary user interface function for the application
 *              layer to configure the way that the host device accept
 *              connection request.
 * PreCondition:Protocol initialization has been done.
 * Parameters:  BYTE Mode - The mode to accept connection request. The privilege
 *                          for those modes decreases gradually as defined. The
 *                          higher privilege mode has all the rights of the
 *                          lower privilege modes. The possible modes are:
 *                  * ENABLE_ALL_CONN        Enable response to all connection
 *                                           request
 *                  * ENABLE_PREV_CONN       Enable response to connection
 *                                           request from device already in the
 *                                           connection table.
 *                  * ENABLE_ACTIVE_SCAN_RSP Enable response to active scan only
 *                  * DISABLE_ALL_CONN       Disable response to connection
 *                                           request, including an acitve scan
 *                                           request.
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
                MIWI0434CapacityInfo.bits.ConnMode = Mode;
                break;
            #endif
        case ISM_868:
            #ifndef MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return;
            #else
                MIWI0868_ConnMode = Mode;
                MIWI0868CapacityInfo.bits.ConnMode = Mode;
                break;
            #endif
        case ISM_2G4:
            #ifndef MIWI_2400_RI
                Printf("Error: MiWi ISM 2,4 GHz band is not available.");
                return;
            #else
                MIWI2400_ConnMode = Mode;
                MIWI2400CapacityInfo.bits.ConnMode = Mode;
                break;
            #endif
        default:
            return;   //Error
    }

    #if defined(ENABLE_NETWORK_FREEZER)
        nvmPutConnMode(&ConnMode);      //Adapt functions... Then save new mode
    #endif
}    

/*******************************************************************************
 * Function:    BYTE MiApp_SearchConnection(BYTE ScanDuartion, DWORD ChannelMap)
 * Summary:     This function perform an active scan to locate operating PANs in
 *              the neighborhood.
 * Description: This is the primary user interface function for the application
 *              layer to perform an active scan. After this function call, all
 *              active scan response will be stored in the global variable
 *              ActiveScanResults in the format of structure ACTIVE_SCAN_RESULT.
 *              The return value indicates the total number of valid active scan
 *              response in the active scan result array.
 * PreCondition:Protocol initialization has been done.
 * Parameters:  BYTE ScanDuration - The maximum time to perform scan on single
 *                                  channel. The value is from 5 to 14. The real
 *                                  time to perform scan can be calculated in
 *                                  following formula from IEEE 802.15.4
 *                                  specification:
 *                                  960 * (2^ScanDuration + 1) * 10^(-6) second
 *              DWORD ChannelMap  - The bit map of channels to perform noise
 *                                  scan. The 32-bit double word parameter use
 *                                  one bit to represent corresponding channels
 *                                  from 0 to 31. For instance, 0x00000003
 *                                  represent to scan channel 0 and channel 1.
 * Returns:     The number of valid active scan response stored in the global
 *              variable ActiveScanResults.
 * Example:
 *      <code>
 *      // Perform an active scan on all possible channels
 *      NumOfActiveScanResponse = MiApp_SearchConnection(10, 0xFFFFFFFF);
 *      </code>
 * Remarks:     None
 ******************************************************************************/
BYTE MiApp_SearchConnection(INPUT BYTE ScanDuration, INPUT DWORD ChannelMap,\
                            INPUT miwi_band mb){
    BYTE backupChannel, ChannelOffset;
    DWORD FullChannelMap;
    BYTE *currentChannel;
    BYTE *TxData;
    MIWI_STATE_MACHINE *msm;

    ActiveScanResultIndex = 0;          //Juan: PANs found... by now 0.
    if(mb == ISM_434){
        #ifndef MIWI_0434_RI
            Printf("Error: MiWi ISM 434 MHz band is not available.");
            return ActiveScanResultIndex;
        #else
            #if defined MRF49XA_1_IN_434
                FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                msm = & MRF49XA_1_MiWiStateMachine;
                TxData = & MRF49XA_1_TxData;
            #elif defined MRF49XA_2_IN_434
                FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                msm = & MRF49XA_2_MiWiStateMachine;
                TxData = & MRF49XA_2_TxData;
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
                msm = & MRF49XA_1_MiWiStateMachine;
                TxData = & MRF49XA_1_TxData;
            #elif defined MRF49XA_2_IN_868
                FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                msm = & MRF49XA_2_MiWiStateMachine;
                TxData = & MRF49XA_2_TxData;
            #else
                return ActiveScanResultIndex; //Error
            #endif
            ChannelOffset = MIWI0868ConfChannelOffset;
            backupChannel = MIWI0868_currentChannel;
            currentChannel = & MIWI0868_currentChannel;
        #endif
    } else if(mb == ISM_2G4){
        #ifndef MIWI_2400_RI
            Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
            return ActiveScanResultIndex;
        #else
            ChannelOffset = MIWI2400ConfChannelOffset;
            backupChannel = MIWI2400_currentChannel;
            FullChannelMap = MRF24J40_FULL_CHANNEL_MAP;
            currentChannel = & MIWI2400_currentChannel;
            msm = & MRF24J40_MiWiStateMachine;
            TxData = & MRF24J40_TxData;
        #endif
    }
    else {return ActiveScanResultIndex;}    //Error. Search aborted.

    BYTE i;
    DWORD channelMask = 0x00000001;  
    MIWI_TICK t1, t2;
        
    for(i=0; i<ACTIVE_SCAN_RESULT_SIZE; i++){
        //Juan: Node can store info about [ACTIVE_SCAN_RESULTS_SIZE] PANs
        ActiveScanResults[i].Channel = 0xFF;
        //Juan: By now, operating channels are set to invalid.
    }

    msm->bits.searchingForNetwork = 1;

    //Juan: Now the search for PANs begins...
    i = 0;
    WORD_VAL AuxPANID;
    while(i<32){
        if(ChannelMap & FullChannelMap & (channelMask << i)){
            Printf("\r\nScan Channel ");
            PrintDec(i - ChannelOffset);
            /* choose appropriate channel */
            MiApp_SetChannel(i, mb);
 
            *TxData = 0;
            MiApp_WriteData(MAC_COMMAND_BEACON_REQUEST, mb);
            MiApp_WriteData(*currentChannel, mb);

            #if defined(IEEE_802_15_4)
                //tempPANID.Val = 0xFFFF;           //Juan: Original
                AuxPANID.Val = 0xFFFF;          
                //JUAN: It doesn't matter that subGHz transceivers get into this
                //part, because PANID argument will be ignored for them.

                SendMACPacket(AuxPANID.v, NULL, PACKET_TYPE_COMMAND, 0, mb);
            #else
                SendMACPacket(NULL, PACKET_TYPE_COMMAND, mb);
            #endif
            
            t1 = MiWi_TickGet();
            while(1){
                if(MiApp_MessageAvailable(mb)){
                    //Juan: MiWiTasks has been called and commands received have
                    //been processed... ActiveScanResults may have been updated!
                    MiApp_DiscardMessage(mb);
                }
                //MiWiTasks();
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > ((DWORD)(ScanTime[ScanDuration]))){
                    //if scan time exceed scan duration, prepare to scan the next channel
                    break;
                }
            }          
        }  
        i++;
    }
    
    MiApp_SetChannel(backupChannel, mb);
    msm->bits.searchingForNetwork = 0;

    return ActiveScanResultIndex;
}

    
/*******************************************************************************
 * Function:    BYTE MiApp_EstablishConnection(BYTE ActiveScanIndex, BYTE Mode)
 * Summary:     This function establish a connection with one or more nodes in
 *              an existing PAN.
 * Description: This is the primary user interface function for the application 
 *              layer to start communication with an existing PAN. For P2P
 *              protocol, this function call can establish one or more
 *              connections. For network protocol, this function can be used to
 *              join the network, or establish a virtual socket connection with
 *              a node out of the radio range. There are multiple ways to
 *              establish connection(s), all depends on the input parameters.
 * PreCondition:Protocol initialization has been done. If only to establish 
 *              connection with a predefined device, an active scan must be
 *              performed before and valid active scan result has been saved.
 * Parameters:  BYTE ActiveScanIndex - The index of the target device in the
 *                                     ActiveScanResults array, if a predefined
 *                                     device is targeted. If the value of
 *                                     ActiveScanIndex is 0xFF, the protocol
 *                                     stack will try to establish a connection
 *                                     with any device.
 *              BYTE Mode            - The mode to establish a connection. This
 *                                     parameter is generally valid in a network
 *                                     protocol. The possible modes are:
 *                  * CONN_MODE_DIRECT   Establish a connection without radio
 *                                       range.
 *                  * CONN_MODE_INDIRECT Establish a virtual connection with a 
 *                                       device that may be in or out of the
 *                                       radio range. This mode sometimes is
 *                                       called cluster socket, which is only
 *                                       valid for network protocol. The PAN
 *                                       Coordinator will be involved to
 *                                       establish a virtual indirect socket
 *                                       connection.
 * Returns:     The index of the peer device on the connection table.
 * Example:
 *      <code>
 *      // Establish one or more connections with any device
 *      PeerIndex = MiApp_EstablishConnection(0xFF, CONN_MODE_DIRECT);
 *      </code>
 * Remarks:     If more than one connections have been established through this
 *              function call, the return value points to the index of one of
 *              the peer devices.
 ******************************************************************************/  
BYTE MiApp_EstablishConnection(INPUT BYTE ActiveScanIndex, INPUT BYTE Mode,\
                               INPUT miwi_band mb){
    BYTE *currentChannel;
    BYTE *TxData, *ConnMode, *myParent;
    WORD_VAL *myShortAddress, *myPANID;
    MIWI_STATE_MACHINE *msm;
    MIWI_CAPACITY_INFO MiWiCapacityInfo;
    OPEN_SOCKET *openSocketInfo;
    TEMP_NODE_INFO *TempNodeInfo;
    BYTE prueba, auxiliar, k;
    
    
    if(mb == ISM_434){
        #ifndef MIWI_0434_RI
            Printf("Error: MiWi ISM 434 MHz band is not available.");
            return 0xFF;
        #else
            currentChannel = & MIWI0434_currentChannel;
            ConnMode = & MIWI0434_ConnMode;
            myParent = & my0434Parent;
            myShortAddress = & myShort0434Addr;
            myPANID = & myPAN0434ID;
            openSocketInfo = & Socket0434Info;
            TempNodeInfo = & temp0434;
            MiWiCapacityInfo = MIWI0434CapacityInfo;
            #if defined MRF49XA_1_IN_434
                msm = & MRF49XA_1_MiWiStateMachine;
                TxData = & MRF49XA_1_TxData;
            #elif defined MRF49XA_2_IN_434
                msm = & MRF49XA_2_MiWiStateMachine;
                TxData = & MRF49XA_2_TxData;
            #else   //Error
                return 0xFF;
            #endif
        #endif
    } else if(mb == ISM_868){
        #ifndef MIWI_0868_RI
            Printf("Error: MiWi ISM 868 MHz band is not available.");
            return 0xFF;
        #else
            currentChannel = & MIWI0868_currentChannel;
            ConnMode = & MIWI0868_ConnMode;
            myParent = & my0868Parent;
            myShortAddress = & myShort0868Addr;
            myPANID = & myPAN0868ID;
            openSocketInfo = & Socket0868Info;
            TempNodeInfo = & temp0868;
            MiWiCapacityInfo = MIWI0868CapacityInfo;
            #if defined MRF49XA_1_IN_868
                msm = & MRF49XA_1_MiWiStateMachine;
                TxData = & MRF49XA_1_TxData;
            #elif defined MRF49XA_2_IN_868
                msm = & MRF49XA_2_MiWiStateMachine;
                TxData = & MRF49XA_2_TxData;
            #else
                return 0xFF;    //Error
            #endif
        #endif
    } else if(mb == ISM_2G4){
        #ifndef MIWI_2400_RI
            Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
            return FALSE;
        #else
            msm = & MRF24J40_MiWiStateMachine;
            currentChannel = & MIWI2400_currentChannel;
            ConnMode = & MIWI2400_ConnMode;
            myParent = & my2400Parent;
            myShortAddress = & myShort2400Addr;
            myPANID = & myPAN2400ID;
            openSocketInfo = & Socket2400Info;
            TempNodeInfo = & temp2400;
            MiWiCapacityInfo = MIWI2400CapacityInfo;
            TxData = & MRF24J40_TxData;
        #endif
    }
    else {return FALSE;}    //Error

    BYTE retry = CONNECTION_RETRY_TIMES;
    BYTE i;
    MIWI_TICK t1, t2;
    
    if(Mode == CONN_MODE_INDIRECT){
        #if defined(ENABLE_SLEEP)
            t1 = MiWi_TickGet();
        #endif
        OpenSocket(mb);
        while(openSocketInfo->status.bits.requestIsOpen){
            if(MiApp_MessageAvailable(mb)){
                MiApp_DiscardMessage(mb);
            }
            //MiWiTasks();
            #if defined(ENABLE_SLEEP) && defined(NWK_ROLE_END_DEVICE)
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > OPEN_SOCKET_POLL_INTERVAL){
                    CheckForData(mb);
                    t1.Val = t2.Val;
                }
            #endif
        }
        if(openSocketInfo->status.bits.matchFound){
            return openSocketInfo->socketHandle;
        }
        return 0xFF;
    }
    else if(Mode == CONN_MODE_DIRECT){
        //Juan: 0xFF => Establish as much connections as you can with any node.
        if(ActiveScanIndex == 0xFF){
            while(MiApp_SearchConnection(10, (((DWORD)0x00000001) << *currentChannel), mb) == 0){
                if(--retry == 0){
                    return 0xFF;
                }
            }
            ActiveScanIndex = 0;    //Juan: First PAN found has ActiveScanIndex 0
        }
      
        
        // CLARA
        // La petición se hace a partir de la información de los beacons recibidos. En la interfaz
        // de 2.4GHz estos usan la dirección corta del coordinador, luego no conocemos la dirección 
        // larga y por tanto no podemos calcular myParent y quedar a la espera del bit Join.
        // Es por ello que se hace la solicitud y se sale. Conoce el resultado de la conexión cuando
        // recibe el comando MAC_COMMAND_ASSOCIATIO_RESPONSE
        //
        // En cuanto a las frecuencias de 434 y 868, no se toca la tabla, ya que eso se hace también 
        // cuando recibe el comando anterior, así es innecsario replicar el acceso a la tabla.
        
        
        
    if (mb != ISM_2G4){
        TempNodeInfo->PANID.Val = ActiveScanResults[ActiveScanIndex].PANID.Val;
            for(i=0; i<MY_ADDRESS_LENGTH; i++){
                TempNodeInfo->longAddr[i] = ActiveScanResults[ActiveScanIndex].Address[i];
        }    
        *myParent = SearchForLongAddress(mb);

        //Juan: By reaching this point, myParent contains the connection index
        //of another device. If 0xFF, the device wasn't found in the conn. table
        //so it is treated as a new device...
        if(*myParent == 0xFF){
            if((*myParent = findNextNetworkEntry()) == 0xFF){
                //Juan: No free space in conn. table to register myParent
                return 0xFF;
            }
        }
        
        for(i=0; i<MY_ADDRESS_LENGTH; i++){
                        TempNodeInfo->longAddr[i] = ActiveScanResults[ActiveScanIndex].Address[i];
        }
    }

        WORD_VAL AuxPANID;
        AuxPANID.Val = ActiveScanResults[ActiveScanIndex].PANID.Val;
        BYTE nibbleOffset;
        switch(mb){
            case ISM_434:
                #if defined MIWI_0434_RI
                    nibbleOffset = 1;            //Juan: Aux
                #endif
                break;
            case ISM_868:
                #if defined MIWI_0868_RI
                    nibbleOffset = 2;            //Juan: Aux                    
                #endif
                break;
            case ISM_2G4:
                #if defined MIWI_2400_RI && defined(IEEE_802_15_4)
                    nibbleOffset = 3;           //Juan: Aux
                #endif
                break;
            default:
                return 0xFF;
        }

        //Juan: Changing to the found PAN operating channel.
        MiApp_SetChannel(ActiveScanResults[ActiveScanIndex].Channel, mb);
        
        /* Program the PANID to the attempted network */
        myPANID->Val = AuxPANID.Val;
        TempNodeInfo->shortAddr.Val = 0xFFFF;
        
        //Juan: added conditions.
        #if defined IEEE_802_15_4
            if (mb == ISM_2G4){
                MiMAC_SetAltAddress(TempNodeInfo->shortAddr.v, myPANID->v);
            }
        #endif
        
        //Juan: Trying to join the network. Sends an association request with my
        //MiWiCapacityInfo attached...
        *TxData = 0;
        MiApp_WriteData(MAC_COMMAND_ASSOCIATION_REQUEST, mb);
        MiApp_WriteData(MiWiCapacityInfo.Val, mb);

        #if ADDITIONAL_NODE_ID_SIZE > 0
            for(i=0; i<ADDITIONAL_NODE_ID_SIZE; i++){
                MiApp_WriteData(AdditionalNodeID[i], mb);
            }
        #endif
       
        
        
        #if defined(IEEE_802_15_4)
            if (mb == ISM_2G4){
                #if defined MIWI_2400_RI
                
                    SendMACPacket(AuxPANID.v, TempNodeInfo->longAddr, \
                                  PACKET_TYPE_COMMAND, MSK_ALT_DST_ADDR, mb);
                #endif
            }else{
                //Juan: 4th argument (modemask) will be ignored if mb != ISM_2G4
                SendMACPacket(AuxPANID.v, TempNodeInfo->longAddr, PACKET_TYPE_COMMAND, 0, mb);
            }
        #else
            SendMACPacket(ConnectionTable[*myParent].Address, PACKET_TYPE_COMMAND, mb);
        #endif
           
        t1 = MiWi_TickGet();
        //JUAN: A tricky way for checking Finish????Join
        
        
        if (mb != ISM_2G4){
            while(!(ConnectionTable[*myParent].status.Val & (0x0008 << 4*nibbleOffset))){

                if(MiApp_MessageAvailable(mb)){
                    //Juan: MiWiTasks has been called for dealing with responses.
                    MiApp_DiscardMessage(mb);
                }
                //MiWiTasks();
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > 5*ONE_SECOND){
                    return 0xFF;
                }

            }
        
        
        #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && defined(ENABLE_INDIRECT_MESSAGE)
            TimeSyncTick = MiWi_TickGet();
        #endif
        return *myParent; //Juan: Returns conn. Index of myParent.
        }
        else{            
            return 0;
        } 
    }
    return 0xFF;                //Juan: returns invalid conn. index... 
}

/*******************************************************************************
 ******************************************************************************/
BYTE MiApp_MessageAvailable(miwi_band mb){
    MiWiTasks();
    BYTE mbWithData = 0x00;
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return mbWithData;
            #else
                #if defined MRF49XA_1_IN_434
                    if (MRF49XA_1_MiWiStateMachine.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_434
                    if (MRF49XA_2_MiWiStateMachine.bits.RxHasUserData){
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
                    if (MRF49XA_1_MiWiStateMachine.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_868
                    if (MRF49XA_2_MiWiStateMachine.bits.RxHasUserData){
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
                if (MRF24J40_MiWiStateMachine.bits.RxHasUserData){
                    mbWithData |= MIWI_2400_RI_MASK;
                }
                return mbWithData;
            #endif
        case ALL_ISM:
            #if defined MIWI_0434_RI
                #if defined MRF49XA_1_IN_434
                    if (MRF49XA_1_MiWiStateMachine.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_434
                    if (MRF49XA_2_MiWiStateMachine.bits.RxHasUserData){
                        mbWithData |= MIWI_0434_RI_MASK;
                    }
                #endif
            #endif
            #if defined MIWI_0868_RI
                #if defined MRF49XA_1_IN_868
                    if (MRF49XA_1_MiWiStateMachine.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #elif defined MRF49XA_2_IN_868
                    if (MRF49XA_2_MiWiStateMachine.bits.RxHasUserData){
                        mbWithData |= MIWI_0868_RI_MASK;
                    }
                #endif
            #endif
            #if defined MIWI_2400_RI
                if (MRF24J40_MiWiStateMachine.bits.RxHasUserData){
                    mbWithData |= MIWI_2400_RI_MASK;
                }
            #endif
            return mbWithData;
        default:
            return mbWithData;
    }
}

/*******************************************************************************
 ******************************************************************************/
void MiApp_DiscardMessage(miwi_band mb){
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_434
                    MRF49XA_1_MiWiStateMachine.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(1);
                    return;
                #elif defined MRF49XA_2_IN_434
                    MRF49XA_2_MiWiStateMachine.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(2);
                    return;
                #else   //Error
                    return;
                #endif
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return;
            #else
                #if defined MRF49XA_1_IN_868
                    MRF49XA_1_MiWiStateMachine.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(1);
                    return;
                #elif defined MRF49XA_2_IN_868
                    MRF49XA_2_MiWiStateMachine.bits.RxHasUserData = 0;
                    MiMAC_MRF49XA_DiscardPacket(2);
                    return;
                #else   //Error
                    return;
                #endif
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return;
            #else
                MRF24J40_MiWiStateMachine.bits.RxHasUserData = 0;
                MiMAC_MRF24J40_DiscardPacket();
                return;
            #endif
        default:
            return;
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
 * Returns:     A boolean to indicates if the broadcast procedure is succcessful
 * Example:
 *      <code>
 *      // Secure and then broadcast the message stored in TxBuffer
 *      MiApp_BroadcastPacket(TRUE);
 *      </code>
 * Remarks:     None
 ******************************************************************************/ 
BOOL MiApp_BroadcastPacket(INPUT BOOL SecEn, INPUT miwi_band mb){
    BYTE *pbuffer, *MiWiSeqNum;
    MIWI_STATE_MACHINE *msm;
    BYTE transceiver;
    WORD_VAL *myShortAddress, *myPANID;
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                myShortAddress = &myShort0434Addr;
                myPANID = & myPAN0434ID;
                MiWiSeqNum = & MIWI0434SeqNum;
                #if defined MRF49XA_1_IN_434
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    transceiver = 1;
                #elif defined MRF49XA_2_IN_434
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;
                    transceiver = 2;
                #else   //Error
                    return FALSE;
                #endif
                break;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                myShortAddress = & myShort0868Addr;
                myPANID = & myPAN0868ID;
                MiWiSeqNum = & MIWI0868SeqNum;
                #if defined MRF49XA_1_IN_868
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    transceiver = 1;
                #elif defined MRF49XA_2_IN_868
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;
                    transceiver = 2;
                #else   //Error
                    return FALSE;
                #endif
                break;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return FALSE;
            #else
                myShortAddress = & myShort2400Addr;
                myPANID = & myPAN2400ID;
                MiWiSeqNum = & MIWI2400SeqNum;
                pbuffer = MRF24J40_TxBuffer;
                msm = & MRF24J40_MiWiStateMachine;
                transceiver = 3;
                break;
            #endif
        default:
            return FALSE;
    }
    BOOL ok;

    *(pbuffer)    = defaultHops;
    *(pbuffer+1)  = 0x02;
    *(pbuffer+2)  = myPANID->v[0];
    *(pbuffer+3)  = myPANID->v[1];
    *(pbuffer+4)  = 0xFF;
    *(pbuffer+5)  = 0xFF;
    *(pbuffer+6)  = myPANID->v[0];
    *(pbuffer+7)  = myPANID->v[1];
    *(pbuffer+8)  = myShortAddress->v[0];
    *(pbuffer+9)  = myShortAddress->v[1];
    *(pbuffer+10) = *(MiWiSeqNum)++;
  
    MTP.flags.Val = 0;
    MTP.flags.bits.broadcast = 1;
    MTP.flags.bits.secEn = SecEn;

    if (transceiver == 3){
        #if defined(IEEE_802_15_4)
            MTP.altSrcAddr = TRUE;
            MTP.DestPANID.Val = myPANID->Val;
        #endif
    }

    ok = AuxMAC_SendPacket(&MTP, transceiver);

    #if defined(ENABLE_INDIRECT_MESSAGE) && defined(NWK_ROLE_COORDINATOR)
    if(ok == FALSE){
        //JUAN: MODIFIED BEHAVIOUR. First send the packet. If failure, then save
        //indirect message. Always, return trnasmission result.
        BYTE i;
        for(i = 0; i<CONNECTION_SIZE; i++){
            if (ConnectionTable[i].status.bits.isValid && \
                ConnectionTable[i].status.bits.RXOnWhenIdle == 0){
                break;
            }     
        }
        
        if (i < CONNECTION_SIZE){
            #if defined(IEEE_802_15_4)
                if (SaveIndirectMessage(TRUE, *myPANID, NULL, FALSE, SecEn, mb))
            #else
                if (SaveIndirectMessage(TRUE, NULL, FALSE, SecEn, mb))
            #endif
{
                                        Printf("\r\n\nIndirect packet saved.");
                                    }
                                    else{
                                        Printf("\r\n\nIndirect packet couldn't be saved.");
                                    }
        }
    }
    #endif
        
    #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
        if(msm->bits.Sleeping){
            if(SecEn){
                *(pbuffer+1) |= 1;
            }    
            return TRUE;
        }    
    #endif

    return ok;
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
 *              BOOL SecEn           - The boolean indicates if the application
 *                                     payload needs to be secured before
 *                                     transmission.
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

    if(ConnectionTable[ConnectionIndex].status.bits.isValid == 0){
        return FALSE;
    }
    
    BYTE *pbuffer, *MiWiSeqNum, *AckSeqNum, *myParent;
    WORD_VAL *AckAddr, *myShortAddress, *myPANID;
    WORD_VAL AuxAltAddr, AuxPANID;
    MIWI_STATE_MACHINE *msm;
    BYTE transceiver;
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    transceiver = 1;
                #elif defined MRF49XA_2_IN_434
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;
                    transceiver = 2;
                #else   //Error
                    return FALSE;
                #endif
                MiWiSeqNum = & MIWI0434SeqNum;
                AckSeqNum = & Ack0434SeqNum;
                AckAddr = & Ack0434Addr;
                myShortAddress = & myShort0434Addr;
                myPANID = & myPAN0434ID;
                myParent = & my0434Parent;

                AuxAltAddr.Val = ConnectionTable[ConnectionIndex].MIWI0434AltAddress.Val;
                AuxPANID = ConnectionTable[ConnectionIndex].PAN0434ID;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    transceiver = 1;
                #elif defined MRF49XA_2_IN_868
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;
                    transceiver = 2;
                #else   //Error
                    return FALSE;
                #endif
                MiWiSeqNum = & MIWI0868SeqNum;
                AckSeqNum = & Ack0868SeqNum;
                AckAddr = & Ack0868Addr;
                myShortAddress = & myShort0868Addr;
                myPANID = & myPAN0868ID;
                myParent = & my0868Parent;

                AuxAltAddr.Val = ConnectionTable[ConnectionIndex].MIWI0868AltAddress.Val;
                AuxPANID = ConnectionTable[ConnectionIndex].PAN0868ID;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return FALSE;
            #else
                pbuffer = MRF24J40_TxBuffer;
                msm = & MRF24J40_MiWiStateMachine;
                transceiver = 3;
                MiWiSeqNum = & MIWI2400SeqNum;
                AckSeqNum = & Ack2400SeqNum;
                AckAddr = & Ack2400Addr;
                myShortAddress = & myShort2400Addr;
                myPANID = & myPAN2400ID;
                myParent = & my2400Parent;

                AuxAltAddr.Val = ConnectionTable[ConnectionIndex].MIWI2400AltAddress.Val;
                AuxPANID = ConnectionTable[ConnectionIndex].PAN2400ID;
            #endif
        default:
            return FALSE;
    }

    BYTE MiWiFrameControl;

    if(MiWiAckRequired && *(pbuffer+MIWI_HEADER_LEN)){
        msm->bits.MiWiAckInProgress = 1;
        *AckSeqNum = *MiWiSeqNum;
        AckAddr->Val = AuxAltAddr.Val;
        MiWiFrameControl = 0x06;
    }    
    else{
        MiWiFrameControl = 0x02;
        msm->bits.MiWiAckInProgress = 0;
    }
        
    *(pbuffer)    = defaultHops;            //number of hops
    *(pbuffer+1)  = MiWiFrameControl;       //Frame Control
    *(pbuffer+2)  = AuxPANID.v[0];
    *(pbuffer+3)  = AuxPANID.v[1];
    *(pbuffer+4)  = AuxAltAddr.v[0];
    *(pbuffer+5)  = AuxAltAddr.v[1];
    *(pbuffer+6)  = myPANID->v[0];           //source PANID LSB
    *(pbuffer+7)  = myPANID->v[1];           //source PANID MSB
    *(pbuffer+8)  = myShortAddress->v[0];    //source address LSB
    *(pbuffer+9)  = myShortAddress->v[1];    //source address MSB
    *(pbuffer+10) = *(MiWiSeqNum)++;	    //sequence number
    
    #if defined(NWK_ROLE_COORDINATOR)
        if(RouteMessage(AuxPANID, AuxAltAddr, SecEn, mb) == FALSE){
            msm->bits.MiWiAckInProgress = 0;
            return FALSE;
        }
        else if(msm->bits.MiWiAckInProgress){
            MIWI_TICK t1, t2;
            t1 = MiWi_TickGet();
            while(1){
                if(MiApp_MessageAvailable(mb)){
                    MiApp_DiscardMessage(mb);
                }
                //MiWiTasks();
                if(msm->bits.MiWiAckInProgress == 0){
                    return TRUE;
                }    
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > MIWI_ACK_TIMEOUT){
                    msm->bits.MiWiAckInProgress = 0;
                    return FALSE;
                }    
            }    
        }
        return TRUE;
    #else
        // for end device, always send the message to its parent
        #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
            if(msm->bits.Sleeping){
                if(SecEn){
                    *(pbuffer+1) |= 1;
                }  
                return TRUE;
            }    
        #endif
        
        MTP.flags.Val = 0;
        MTP.flags.bits.ackReq = 1;
        MTP.flags.bits.secEn = SecEn;

        //JUAN: Modified.
        if(transceiver == 3){
            #if defined(IEEE_802_15_4) && defined MIWI_2400_RI
                MTP.altDestAddr = TRUE;
                MTP.altSrcAddr = TRUE;
                MTP.DestAddress = ConnectionTable[*myParent].MIWI2400AltAddress.v;
                MTP.DestPANID.Val = myPANID->Val;
            #endif
        }else{
            MTP.DestAddress = ConnectionTable[*myParent].Address;
        }

        if(!(AuxMAC_SendPacket(&MTP, transceiver))){
            msm->bits.MiWiAckInProgress = 0;
            return FALSE;
        }    
        else if(msm->bits.MiWiAckInProgress){
            MIWI_TICK t1, t2;
            t1 = MiWi_TickGet();
            while(1){
                if(MiApp_MessageAvailable(mb)){
                    MiApp_DiscardMessage(mb);
                }
                //MiWiTasks();
                if(msm->bits.MiWiAckInProgress == 0){
                    return TRUE;
                }    
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > MIWI_ACK_TIMEOUT){
                    msm->bits.MiWiAckInProgress = 0;
                    return FALSE;
                }    
            }    
        }
        return TRUE; 
    #endif
}                        

/*******************************************************************************
 ******************************************************************************/
BOOL MiApp_UnicastAddress(BYTE *DestAddress, BOOL PermanentAddr, BOOL SecEn, \
                          miwi_band mb){
    BYTE *pbuffer, *MiWiSeqNum, *AckSeqNum, *myParent;
    WORD_VAL *AckAddr, *myShortAddress, *myPANID;
    MIWI_STATE_MACHINE *msm;
    BYTE transceiver;
    TEMP_NODE_INFO *TempNodeInfo;
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    transceiver = 1;
                #elif defined MRF49XA_2_IN_434
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;
                    transceiver = 2;
                #else   //Error
                    return FALSE;
                #endif
                MiWiSeqNum = & MIWI0434SeqNum;
                AckSeqNum = & Ack0434SeqNum;
                AckAddr = & Ack0434Addr;
                myShortAddress = & myShort0434Addr;
                myPANID = & myPAN0434ID;
                myParent = & my0434Parent;
                TempNodeInfo = & temp0434;
            #endif
            break;
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    pbuffer = MRF49XA_1_TxBuffer;
                    msm = & MRF49XA_1_MiWiStateMachine;
                    transceiver = 1;
                #elif defined MRF49XA_2_IN_868
                    pbuffer = MRF49XA_2_TxBuffer;
                    msm = & MRF49XA_2_MiWiStateMachine;
                    transceiver = 2;
                #else   //Error
                    return FALSE;
                #endif
                MiWiSeqNum = & MIWI0868SeqNum;
                AckSeqNum = & Ack0868SeqNum;
                AckAddr = & Ack0868Addr;
                myShortAddress = & myShort0868Addr;
                myPANID = & myPAN0868ID;
                myParent = & my0868Parent;
                TempNodeInfo = & temp0868;
            #endif
            break;
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return FALSE;
            #else
                pbuffer = MRF24J40_TxBuffer;
                msm = & MRF24J40_MiWiStateMachine;
                transceiver = 3;
                MiWiSeqNum = & MIWI2400SeqNum;
                AckSeqNum = & Ack2400SeqNum;
                AckAddr = & Ack2400Addr;
                myShortAddress = & myShort2400Addr;
                myPANID = & myPAN2400ID;
                myParent = & my2400Parent;
                TempNodeInfo = & temp2400;
            #endif
            break;
        default:
            return FALSE;
    }

    BYTE handle;
    BYTE i;
    BYTE MiWiFrameControl;

    
    if(MiWiAckRequired && *(pbuffer + MIWI_HEADER_LEN)){
        msm->bits.MiWiAckInProgress = 1;
        *AckSeqNum = *MiWiSeqNum;
        MiWiFrameControl = 0x06;
    } 
    else{
        MiWiFrameControl = 0x02;
        msm->bits.MiWiAckInProgress = 0;
    }
           
    if(PermanentAddr){
        for(i=0; i<MY_ADDRESS_LENGTH; i++){
            TempNodeInfo->longAddr[i] = DestAddress[i];
        }
        if((handle = SearchForLongAddress(mb)) == 0xFF){

DIRECT_LONG_ADDRESS:
            *(pbuffer)    = defaultHops;            //number of hops
            *(pbuffer+1)  = 0x02;		    //Frame Control
            *(pbuffer+2)  = 0xFF;
            *(pbuffer+3)  = 0xFF;
            *(pbuffer+4)  = 0xFF;
            *(pbuffer+5)  = 0xFF;
            *(pbuffer+6)  = myPANID->v[0];           //source PANID LSB
            *(pbuffer+7)  = myPANID->v[1];           //source PANID MSB
            *(pbuffer+8)  = myShortAddress->v[0];    //source address LSB
            *(pbuffer+9)  = myShortAddress->v[1];    //source address MSB
            *(pbuffer+10) = *(MiWiSeqNum)++;	    //sequence number

            #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                if(msm->bits.Sleeping){
                    if(SecEn){
                        *(pbuffer+1) |= 1;
                    }  
                    return TRUE;
                }    
            #endif
            
            MTP.flags.Val = 0;
            MTP.flags.bits.ackReq = 1;
            MTP.flags.bits.secEn = SecEn;   
            MTP.DestAddress = DestAddress;

            if(transceiver == 3){               //Juan: added if.
                #if defined(IEEE_802_15_4)
                    MTP.altDestAddr = FALSE;
                    MTP.altSrcAddr = TRUE;
                    MTP.DestPANID.Val = myPANID->Val;
                #endif
            }

            msm->bits.MiWiAckInProgress = 0;
            
            return AuxMAC_SendPacket(&MTP, transceiver);
        }
        else{

            //Juan: modified.
            WORD_VAL AuxAltAddr, AuxPANID;
            switch(mb){
                case ISM_434:
                    #if defined MIWI_0434_RI
                        if(ConnectionTable[handle].status.bits.short0434AddrValid == 0){
                            goto DIRECT_LONG_ADDRESS;
                        }
                        AuxAltAddr.Val = ConnectionTable[handle].MIWI0434AltAddress.Val;
                        AuxPANID.Val = ConnectionTable[handle].PAN0434ID.Val;
                    #endif
                    break;
                case ISM_868:
                    #if defined MIWI_0868_RI
                        if(ConnectionTable[handle].status.bits.short0868AddrValid == 0){
                            goto DIRECT_LONG_ADDRESS;
                        }
                        AuxAltAddr.Val = ConnectionTable[handle].MIWI0868AltAddress.Val;
                        AuxPANID.Val = ConnectionTable[handle].PAN0868ID.Val;
                    #endif
                    break;
                case ISM_2G4:
                    #if defined MIWI_2400_RI
                        if(ConnectionTable[handle].status.bits.short2400AddrValid == 0){
                            goto DIRECT_LONG_ADDRESS;
                        }
                        AuxAltAddr.Val = ConnectionTable[handle].MIWI2400AltAddress.Val;
                        AuxPANID.Val = ConnectionTable[handle].PAN2400ID.Val;
                    #endif
                    break;
                default:
                    return FALSE;
            }
            
            if(MiWiAckRequired){
                AckAddr->Val = AuxAltAddr.Val;
            }    
            
            *(pbuffer)    = MAX_HOPS;               //number of hops
            *(pbuffer+1)  = MiWiFrameControl;       //Frame Control
            *(pbuffer+2)  = AuxPANID.v[0];
            *(pbuffer+3)  = AuxPANID.v[1];
            *(pbuffer+4)  = AuxAltAddr.v[0];
            *(pbuffer+5)  = AuxAltAddr.v[1];
            *(pbuffer+6)  = myPANID->v[0];           //source PANID LSB
            *(pbuffer+7)  = myPANID->v[1];           //source PANID MSB
            *(pbuffer+8)  = myShortAddress->v[0];    //source address LSB
            *(pbuffer+9)  = myShortAddress->v[1];    //source address MSB
            *(pbuffer+10) = *(MiWiSeqNum)++;	    //sequence number

            #if defined(NWK_ROLE_COORDINATOR)
                if(RouteMessage(AuxPANID, AuxAltAddr, SecEn, mb) == FALSE){
                    msm->bits.MiWiAckInProgress = 0;
                    return FALSE;
                }
                else if(msm->bits.MiWiAckInProgress){
                    MIWI_TICK t1, t2;
                    t1 = MiWi_TickGet();
                    while(1){
                        if(MiApp_MessageAvailable(mb)){
                            MiApp_DiscardMessage(mb);
                        }                        
                        //MiWiTasks();
                        if(msm->bits.MiWiAckInProgress == 0){
                            return TRUE;
                        }    
                        t2 = MiWi_TickGet();
                        if(MiWi_TickGetDiff(t2, t1) > MIWI_ACK_TIMEOUT){
                            msm->bits.MiWiAckInProgress = 0;
                            return FALSE;
                        }    
                    }    
                }
                return TRUE;
            #else
                // for end device, always send data to its parent
                
                #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
                    if(msm->bits.Sleeping){
                        if(SecEn){
                            *(pbuffer+1) |= 1;
                        }  
                        return TRUE;
                    }    
                #endif
                
                MTP.flags.Val = 0;
                MTP.flags.bits.ackReq = 1;
                MTP.flags.bits.secEn = SecEn;   

                if(transceiver == 3){               //JUAN: Modified.
                    #if defined(IEEE_802_15_4) && defined MIWI_2400_RI
                        MTP.DestPANID.Val = ConnectionTable[*myParent].PAN2400ID.Val;
                        MTP.altDestAddr = TRUE;
                        MTP.altSrcAddr = TRUE;
                        MTP.DestAddress = ConnectionTable[*myParent].MIWI2400AltAddress.v;
                    #endif
                }else{
                    MTP.DestAddress = ConnectionTable[*myParent].Address;
                }
                Printf("\r\n SENDING TO DADDY");
                if(!(AuxMAC_SendPacket(&MTP, transceiver))){
                    msm->bits.MiWiAckInProgress = 0;
                    return FALSE;
                }    
                else if(msm->bits.MiWiAckInProgress){
                    MIWI_TICK t1, t2;
                    t1 = MiWi_TickGet();
                    while(1){
                        if(MiApp_MessageAvailable(mb)){
                            MiApp_DiscardMessage(mb);
                        }                        
                        //MiWiTasks();
                        if(msm->bits.MiWiAckInProgress == 0){
                            return TRUE;
                        }    
                        t2 = MiWi_TickGet();
                        if(MiWi_TickGetDiff(t2, t1) > MIWI_ACK_TIMEOUT){
                            msm->bits.MiWiAckInProgress = 0;
                            return FALSE;
                        }    
                    }    
                }
                return TRUE;
            #endif
        }
    }

    *(pbuffer)    = MAX_HOPS;               //number of hops
    *(pbuffer+1)  = MiWiFrameControl;	    //Frame Control
    *(pbuffer+2)  = myPANID->v[0];
    *(pbuffer+3)  = myPANID->v[1];
    *(pbuffer+4)  = DestAddress[0];
    *(pbuffer+5)  = DestAddress[1];
    *(pbuffer+6)  = myPANID->v[0];           //source PANID LSB
    *(pbuffer+7)  = myPANID->v[1];           //source PANID MSB
    *(pbuffer+8)  = myShortAddress->v[0];    //source address LSB
    *(pbuffer+9)  = myShortAddress->v[1];    //source address MSB
    *(pbuffer+10) = *(MiWiSeqNum)++;	    //sequence number
    
    AckAddr->v[0] = DestAddress[0];
    AckAddr->v[1] = DestAddress[1];

    #if defined(NWK_ROLE_COORDINATOR)
        TempNodeInfo->shortAddr.v[0] = DestAddress[0];
        TempNodeInfo->shortAddr.v[1] = DestAddress[1];
        if(RouteMessage(*myPANID, TempNodeInfo->shortAddr, SecEn, mb) == FALSE){
            msm->bits.MiWiAckInProgress = 0;
            return FALSE;
        }
        else if(msm->bits.MiWiAckInProgress){
            MIWI_TICK t1, t2;
            t1 = MiWi_TickGet();
            while(1){
                if(MiApp_MessageAvailable(mb)){
                    MiApp_DiscardMessage(mb);
                }
                if(msm->bits.MiWiAckInProgress == 0){
                    return TRUE;
                }    
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > MIWI_ACK_TIMEOUT){
                    msm->bits.MiWiAckInProgress = 0;
                    return FALSE;
                }    
            }    
        }
        return TRUE;
    #else
        // for end device, always send data to its parent
        
        #if defined(ENABLE_ENHANCED_DATA_REQUEST) && defined(ENABLE_SLEEP)
            if(msm->bits.Sleeping){
                if(SecEn){
                    *(pbuffer+1) |= 1;
                }  
                return TRUE;
            }    
        #endif
        
        MTP.flags.Val = 0;
        MTP.flags.bits.ackReq = 1;
        MTP.flags.bits.secEn = SecEn;   

        if (transceiver == 3) {     //Juan: modified
            #if defined(IEEE_802_15_4) && defined MIWI_2400_RI
                MTP.DestPANID.Val = ConnectionTable[*myParent].PAN2400ID.Val;
                MTP.altDestAddr = TRUE;
                MTP.altSrcAddr = TRUE;
                MTP.DestAddress = ConnectionTable[*myParent].MIWI2400AltAddress.v;
            #endif
        }else{
            MTP.DestAddress = ConnectionTable[*myParent].Address;
        }
        Printf("\r\n SENDING DADDY CON DIR CORTA");
        if(!AuxMAC_SendPacket(&MTP, transceiver)){
            msm->bits.MiWiAckInProgress = 0;
            return FALSE;
        }    
        else if(msm->bits.MiWiAckInProgress){
            MIWI_TICK t1, t2;
            t1 = MiWi_TickGet();
            while(1){
                if(MiApp_MessageAvailable(mb)){
                    MiApp_DiscardMessage(mb);
                }
                //MiWiTasks();
                if(msm->bits.MiWiAckInProgress == 0){
                    return TRUE;
                }    
                t2 = MiWi_TickGet();
                if(MiWi_TickGetDiff(t2, t1) > MIWI_ACK_TIMEOUT){
                    msm->bits.MiWiAckInProgress = 0;
                    return FALSE;
                }    
            }    
        }
        return TRUE;
    #endif
}

/*******************************************************************************
 ******************************************************************************/
BOOL MiApp_StartConnection(BYTE Mode, BYTE ScanDuration, DWORD ChannelMap, \
                           miwi_band mb){
    BYTE ChannelOffset;
    MIWI_STATE_MACHINE *msm;
    BYTE *myParent, *knownCoordinators;
    WORD_VAL *myShortAddress, *myPANID;
    MIWI_CAPACITY_INFO MiWiCapacityInfo;
    switch (mb){
        case ISM_434:
            #if !defined MIWI_0434_RI
                Printf("Error: MiWi ISM 434 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_434
                    msm = & MRF49XA_1_MiWiStateMachine;
                #elif defined MRF49XA_2_IN_434
                    msm = & MRF49XA_2_MiWiStateMachine;
                #else   //Error
                    return FALSE;
                #endif     
                ChannelOffset = MIWI0434ConfChannelOffset;
                myParent = & my0434Parent;
                #if defined NWK_ROLE_COORDINATOR
                    knownCoordinators = & known0434Coordinators;
                #endif
                myShortAddress = & myShort0434Addr;
                myPANID = & myPAN0434ID;
                MiWiCapacityInfo = MIWI0434CapacityInfo;
                break;
            #endif
        case ISM_868:
            #if !defined MIWI_0868_RI
                Printf("Error: MiWi ISM 868 MHz band is not available.");
                return FALSE;
            #else
                #if defined MRF49XA_1_IN_868
                    msm = & MRF49XA_1_MiWiStateMachine;
                #elif defined MRF49XA_2_IN_868
                    msm = & MRF49XA_2_MiWiStateMachine;
                #else   //Error
                    return FALSE;
                #endif
                ChannelOffset = MIWI0868ConfChannelOffset;
                myParent = & my0868Parent;
                #if defined NWK_ROLE_COORDINATOR
                    knownCoordinators = & known0868Coordinators;
                #endif
                myShortAddress = & myShort0868Addr;
                myPANID = & myPAN0868ID;
                MiWiCapacityInfo = MIWI0868CapacityInfo;
                break;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return FALSE;
            #else
                msm = & MRF24J40_MiWiStateMachine;
                ChannelOffset = MIWI2400ConfChannelOffset;
                myParent = & my2400Parent;
                #if defined NWK_ROLE_COORDINATOR
                    knownCoordinators = & known2400Coordinators;
                #endif
                myShortAddress = & myShort2400Addr;
                myPANID = & myPAN2400ID;
                MiWiCapacityInfo = MIWI2400CapacityInfo;
                break;
            #endif
        default:
            return FALSE;
    }

    switch(Mode) {
        #if defined (NWK_ROLE_COORDINATOR)
            case START_CONN_DIRECT:
                myShortAddress->Val = 0;
                *myParent = 0xFF;
                #if MY_PAN_ID == 0xFFFF
                    myPANID->v[0] = TMRL;
                    myPANID->v[1] = TMRL+0x51;
                #else
                    myPANID->Val = MY_PAN_ID;
                #endif

                //Juan: added conditions.
                #if defined IEEE_802_15_4
                    if (mb == ISM_2G4){
                        MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                    }
                #endif
                
                msm->bits.memberOfNetwork = 1;
                role = ROLE_PAN_COORDINATOR;
                MiWiCapacityInfo.bits.Role = role;
                *knownCoordinators = 0x01;   //I know myself
                #if defined(ENABLE_NETWORK_FREEZER)
                    nvmPutMyShortAddress(myShortAddress->v);
                    nvmPutMyPANID(myPANID->v);
                    nvmPutRole(&role);
                    nvmPutKnownCoordinators(&knownCoordinators);
                #endif
                #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && defined(ENABLE_INDIRECT_MESSAGE)
                    TimeSyncTick = MiWi_TickGet();
                #endif
                return TRUE;
        #endif
            
        case START_CONN_ENERGY_SCN:
            #if defined(ENABLE_ED_SCAN) && defined(NWK_ROLE_COORDINATOR)
            {
                BYTE channel;
                BYTE RSSIValue;
                
                channel = MiApp_NoiseDetection(ChannelMap, ScanDuration, NOISE_DETECT_ENERGY, &RSSIValue, mb);
                MiApp_SetChannel(channel, mb);
                Printf("\r\nStart Wireless Communication on Channel ");
                PrintDec(channel- ChannelOffset);
                Printf("\r\n");
                msm->bits.memberOfNetwork = 1;
                myShortAddress->Val = 0;
                *myParent = 0xFF;
                #if MY_PAN_ID == 0xFFFF
                    myPANID->v[0] = TMRL;
                    myPANID->v[1] = TMRL+0x51;
                #else
                    myPANID->Val = MY_PAN_ID;
                #endif

                //Juan: added conditions.
                #if defined IEEE_802_15_4
                    if (mb == ISM_2G4){
                        MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                    }
                #endif

                role = ROLE_PAN_COORDINATOR;
                MiWiCapacityInfo.bits.Role = role;
                *knownCoordinators = 0x01;   //I know myself
                #if defined(ENABLE_NETWORK_FREEZER)
                    nvmPutMyShortAddress(myShortAddress->v);
                    nvmPutMyPANID(myPANID->v);
                    nvmPutRole(&role);
                    nvmPutKnownCoordinators(&knownCoordinators);
                #endif
                #if defined(ENABLE_TIME_SYNC) && !defined(ENABLE_SLEEP) && defined(ENABLE_INDIRECT_MESSAGE)
                    TimeSyncTick = MiWi_TickGet();
                #endif
                return TRUE;
            }
            #else
                return FALSE;
            #endif
            
        case START_CONN_CS_SCN:
            // Carrier sense scan is not supported for current available transceivers
            return FALSE;                
        default:
            break;
    }    
    return FALSE;
}


    #ifdef ENABLE_ED_SCAN
        /***********************************************************************
         * Function:    MiApp_NoiseDetection(DWORD ChannelMap, BYTE ScanDuration,
         *                                  BYTE DetectionMode, BYTE *NoiseLevel)
         * Summary:     This function perform a noise scan and returns the
         *              channel with least noise
         * Description: This is the primary user interface functions for the 
         *              application layer to perform noise detection on multiple
         *              channels.
         * PreCondition:Protocol initialization has been done. 
         * Parameters:  DWORD ChannelMap   - The bit map of channels to perform
         *                                   noise scan. The 32-bit double word
         *                                   parameter use one bit to represent
         *                                   corresponding channels from 0 to 31
         *                                   For instance, 0x00000003 represent
         *                                   to scan channel 0 and channel 1.
         *              BYTE ScanDuration  - The maximum time to perform scan on
         *                                   single channel. The value is from 5
         *                                   to 14. The real time to perform
         *                                   scan can be calculated in following
         *                                   formula from IEEE 802.15.4
         *                                   specification
         *                                   960*(2^ScanDuration+1)*10^(-6) sec
         *              BYTE DetectionMode - The noise detection mode to perform
         *                                   the scan. The two possible scan
         *                                   modes are
         *                  * NOISE_DETECT_ENERGY Energy detection mode
         *                  * NOISE_DETECT_CS     Carrier sense detection mode
         *              BYTE *NoiseLevel   - The noise level at the channel with
         *                                   least noise level
         * Returns:     The channel that has the lowest noise level
         * Example:
         *      <code>
         *      BYTE NoiseLevel;
         *      OptimalChannel = MiApp_NoiseDetection(0xFFFFFFFF, 10,
         *                                    NOISE_DETECT_ENERGY, &NoiseLevel);
         *      </code>
         * Remarks:     None
         **********************************************************************/
        BYTE MiApp_NoiseDetection(INPUT DWORD ChannelMap, INPUT BYTE ScanDuration,\
                                  INPUT BYTE DetectionMode, OUTPUT BYTE *RSSIValue, \
                                  INPUT miwi_band mb){
            
            if(DetectionMode != NOISE_DETECT_ENERGY){
                return 0xFF;
            }         
            //Carrier sense detection not supported by MIWI STACK...

            BYTE i, OptimalChannel, ChannelOffset;
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
                            break;
                        #elif defined MRF49XA_2_IN_434
                            FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                            ChannelOffset = MIWI0434ConfChannelOffset;
                            break;
                        #else
                            //Error
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
                            break;
                        #elif defined MRF49XA_2_IN_868
                            FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                            ChannelOffset = MIWI0868ConfChannelOffset;
                            break;
                        #else
                            //Error
                        #endif
                    #endif
                case ISM_2G4:
                    #if !defined MIWI_2400_RI
                        Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                        return 0xFF;
                    #else
                        ChannelOffset = MIWI2400ConfChannelOffset;
                        FullChannelMap = MRF24J40_FULL_CHANNEL_MAP;
                        break;
                    #endif
                default:
                    return 0xFF;
            }

            BYTE minRSSI = 0xFF;
            DWORD channelMask = 0x00000001;   
            ConsolePutROMString("\r\nEnergy Scan Results:");
            i = 0;
            while(i<32){
                if(ChannelMap & FullChannelMap & (channelMask << i)){
                    BYTE RSSIcheck;
                    BYTE maxRSSI = 0;
                    BYTE j, k;
    
                    /* choose appropriate channel */
                    MiApp_SetChannel(i, mb);
                    
                    t2 = MiWi_TickGet();
                    
                    while(1){
                        if(mb == ISM_434){
                            #if defined MRF49XA_1_IN_434
                                RSSIcheck = MiMAC_MRF49XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, 1);
                            #elif defined MRF49XA_2_IN_434
                                RSSIcheck = MiMAC_MRF49XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, 2);
                            #endif
                        } else if(mb == ISM_868){
                            #if defined MRF49XA_1_IN_868
                                RSSIcheck = MiMAC_MRF49XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, 1);
                            #elif defined MRF49XA_2_IN_868
                                RSSIcheck = MiMAC_MRF49XA_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT, 2);
                            #endif
                        } else if(mb == ISM_2G4){
                            #if defined MRF24J40
                                RSSIcheck = MiMAC_MRF24J40_ChannelAssessment(CHANNEL_ASSESSMENT_ENERGY_DETECT);
                            #endif
                        }
                        if(RSSIcheck > maxRSSI){
                            maxRSSI = RSSIcheck;
                        }                
                        
                        t1 = MiWi_TickGet();
                        if(MiWi_TickGetDiff(t1, t2) > ((DWORD)(ScanTime[ScanDuration]))){
                            // if scan time exceed scan duration, prepare to scan the next channel
                            break;
                        }
                    } 
                    
                    Printf("\r\nChannel ");
                    PrintDec(i - ChannelOffset);
                    Printf(": ");
                    j = maxRSSI/5;
                    for(k=0; k<j; k++){
                        ConsolePut('-');
                    }
                    Printf(" ");
                    PrintChar(maxRSSI);

                    //Refresh scan variables in NodeStatus
                    if(mb == ISM_434){
                        #if defined MIWI_0434_RI
                            NodeStatus.scanMIWI0434[i-ChannelOffset] = maxRSSI;
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


    #if defined(ENABLE_FREQUENCY_AGILITY)
        /***********************************************************************
         * Function:    BOOL MiApp_ResyncConnection(BYTE ConnectionIndex, \
         *                                          DWORD ChannelMap)
         * Summary:     This function tries to resynchronize the lost connection
         *              with peers, probably due to channel hopping
         * Description: This is the primary user interface function for the 
         *              application to resynchronize a lost connection. For a
         *              RFD device that goes to sleep periodically, it may not
         *              receive the channel hopping command that is sent when it
         *              is sleep. The sleeping RFD device depends on this
         *              function to hop to the channel that the rest of the PAN
         *              has jumped to. This function call is usually triggered
         *              by continously communication failure with the peers.
         * PreCondition:Transceiver has been initialized
         * Parameters:  DWORD ChannelMap - The bit map of channels to perform 
         *                                 noise scan. The 32-bit double word
         *                                 parameter use one bit to represent
         *                                 corresponding channels from 0 to 31.
         *                                 For instance, 0x00000003 represent to
         *                                 scan channel 0 and channel 1.
         * Returns:     a boolean to indicate if resynchronization of connection
         *              is successful
         * Example:
         *      <code>
         *      // Sleeping RFD device resync with its associated device, 
         *      // usually the first peer in the connection table
         *      MiApp_ResyncConnection(0, 0xFFFFFFFF);
         *      </code>
         * Remark:      If operation is successful, the wireless node will be
         *              hopped to the channel that the rest of the PAN is
         *              operating on.
         **********************************************************************/ 
        BOOL MiApp_ResyncConnection(INPUT BYTE ConnectionIndex, \
                                    INPUT DWORD ChannelMap, INPUT miwi_band mb){

            DWORD FullChannelMap;
            BYTE backupChannel, ChannelOffset;
            BYTE *currentChannel;
            MIWI_STATE_MACHINE *msm;
            BOOL aux;
            switch (mb){
                case ISM_434:
                    #if !defined MIWI_0434_RI
                        Printf("Error: MiWi ISM 434 MHz band is not available.");
                        return FALSE;
                    #else
                        #if defined MRF49XA_1_IN_434
                            FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                            msm = & MRF49XA_1_MiWiStateMachine;
                            ChannelOffset = 0;
                        #elif defined MRF49XA_2_IN_434
                            FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                            msm = & MRF49XA_2_MiWiStateMachine;
                            ChannelOffset = 0;
                        #else
                            return FALSE;   //Error
                        #endif
                        backupChannel = MIWI0434_currentChannel;
                        currentChannel = & MIWI0434_currentChannel;
                        break;
                    #endif
                case ISM_868:
                    #if !defined MIWI_0868_RI
                        Printf("Error: MiWi ISM 868 MHz band is not available.");
                        return FALSE;
                    #else
                        #if defined MRF49XA_1_IN_868
                            FullChannelMap = MRF49XA_1_FULL_CHANNEL_MAP;
                            msm = & MRF49XA_1_MiWiStateMachine;
                            ChannelOffset = 0;
                        #elif defined MRF49XA_2_IN_868
                            FullChannelMap = MRF49XA_2_FULL_CHANNEL_MAP;
                            msm = & MRF49XA_2_MiWiStateMachine;
                            ChannelOffset = 0;
                        #else
                            return FALSE;   //Error
                        #endif
                        backupChannel = MIWI0868_currentChannel;
                        currentChannel = & MIWI0868_currentChannel;
                        break;
                    #endif
                case ISM_2G4:
                    #if !defined MIWI_2400_RI
                        Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                        return FALSE;
                    #else
                        FullChannelMap = MRF24J40_FULL_CHANNEL_MAP;
                        msm = & MRF24J40_MiWiStateMachine;
                        ChannelOffset = MIWI2400ConfChannelOffset;
                        backupChannel = MIWI2400_currentChannel;
                        currentChannel = & MIWI2400_currentChannel;
                        break;
                    #endif
                default:
                    return FALSE;
            }

            BYTE i;
            BYTE j;
            MIWI_TICK t1, t2;
            
            t1 = MiWi_TickGet();
            msm->bits.Resynning = 1;
            for(i=0; i<RESYNC_TIMES; i++){
                DWORD ChannelMask = 0x00000001;
                
                j = 0;
                while(msm->bits.Resynning){
                    t2 = MiWi_TickGet();
                    
                    if(MiWi_TickGetDiff(t2, t1) > SCAN_DURATION_9){
                        t1.Val = t2.Val;
                        
                        if(j>31){
                            break;
                        }
                        while((ChannelMap & FullChannelMap & (ChannelMask << j)) == 0){
                            if(++j>31){
                                goto GetOutOfLoop;
                            }
                        }
                        
                        Printf("\r\nChecking Channel ");
                        PrintDec(j- ChannelOffset);
                        MiApp_SetChannel(j, mb);
                        j++;

                        MiApp_FlushTx(mb);
                        MiApp_WriteData(MIWI_STACK_REPORT_TYPE, mb);
                        MiApp_WriteData(RESYNCHRONIZATION_REQUEST, mb);
                        MiApp_WriteData(*currentChannel, mb);
                        MiApp_UnicastConnection(ConnectionIndex, FALSE, mb);
                    }
                    
                    if(MiApp_MessageAvailable(mb)){
                        MiApp_DiscardMessage(mb);
                    }
                    //MiWiTasks();
                }
                if(msm->bits.Resynning == 0){
                    Printf("\r\nResynchronized Connection to Channel ");
                    PrintDec(*currentChannel);
                    Printf("\r\n");
                    return TRUE;
                }
GetOutOfLoop:
                MacroNop();         
            }
            
            MiApp_SetChannel(backupChannel, mb);
            msm->bits.Resynning = 0;
            return FALSE;
        }

        #if defined(NWK_ROLE_COORDINATOR)
            /*******************************************************************
             * Function:    void StartChannelHopping(BYTE OptimalChannel)
             * Overview:    This function broadcast the channel hopping command
             *              and after that, change operating channel to the 
             *              input optimal channel     
             * PreCondition:Transceiver has been initialized
             * Input:       OptimalChannel - The channel to hop to
             * Output:      None
             * Side Effects:The operating channel for current device will change
             *              to the specified channel
             ******************************************************************/
            void StartChannelHopping(INPUT BYTE OptimalChannel, \
                                     INPUT miwi_band mb){
                BYTE *currentChannel;
                BYTE *pbuffer;
                BYTE transceiver;
                WORD_VAL *myShortAddress, *myPANID;
                BYTE *MiWiSeqNum;
                switch (mb){
                    case ISM_434:
                        #if !defined MIWI_0434_RI
                            Printf("Error: MiWi ISM 434 MHz band is not available.");
                            return;
                        #else
                            #if defined MRF49XA_1_IN_434
                                pbuffer = MRF49XA_1_TxBuffer;
                                transceiver = 1;
                            #elif defined MRF49XA_2_IN_434
                                pbuffer = MRF49XA_2_TxBuffer;
                                transceiver = 2;
                            #else
                                return;   //Error
                            #endif
                            currentChannel = & MIWI0434_currentChannel;
                            myShortAddress = & myShort0434Addr;
                            myPANID = & myPAN0434ID;
                            MiWiSeqNum = & MIWI0434SeqNum;
                            break;
                        #endif
                    case ISM_868:
                        #if !defined MIWI_0868_RI
                            Printf("Error: MiWi ISM 868 MHz band is not available.");
                            return;
                        #else
                            #if defined MRF49XA_1_IN_868
                                pbuffer = MRF49XA_1_TxBuffer;
                                transceiver = 1;
                            #elif defined MRF49XA_2_IN_868
                                pbuffer = MRF49XA_2_TxBuffer;
                                transceiver = 2;
                            #else
                                return;   //Error
                            #endif
                            currentChannel = & MIWI0868_currentChannel;
                            myShortAddress = & myShort0868Addr;
                            myPANID = & myPAN0868ID;
                            MiWiSeqNum = & MIWI0868SeqNum;
                            break;
                        #endif
                    case ISM_2G4:
                        #if !defined MIWI_2400_RI
                            Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                            return;
                        #else
                            currentChannel = & MIWI2400_currentChannel;
                            pbuffer = MRF24J40_TxBuffer;
                            transceiver = 3;
                            myShortAddress = & myShort2400Addr;
                            myPANID = & myPAN2400ID;
                            MiWiSeqNum = & MIWI2400SeqNum;
                            break;
                        #endif
                    default:
                        return;
                }

                BYTE i;
                MIWI_TICK t1, t2;
                
                for(i=0; i<FA_BROADCAST_TIME; i++){
                    t1 = MiWi_TickGet();
                    while(1){
                        t2 = MiWi_TickGet();
                        if(MiWi_TickGetDiff(t2, t1) > SCAN_DURATION_9){
                            *(pbuffer)    = defaultHops;
                            *(pbuffer+1)  = 0x02;
                            *(pbuffer+2)  = myPANID->v[0];
                            *(pbuffer+3)  = myPANID->v[1];
                            *(pbuffer+4)  = 0xFF;
                            *(pbuffer+5)  = 0xFF;
                            *(pbuffer+6)  = myPANID->v[0];
                            *(pbuffer+7)  = myPANID->v[1];
                            *(pbuffer+8)  = myShortAddress->v[0];
                            *(pbuffer+9)  = myShortAddress->v[1];
                            *(pbuffer+10) = *(MiWiSeqNum)++;
                            *(pbuffer+11) = MIWI_STACK_REPORT_TYPE;
                            *(pbuffer+12) = CHANNEL_HOPPING_REQUEST;
                            *(pbuffer+13) = *currentChannel;
                            *(pbuffer+14) = OptimalChannel;
                            
                            MTP.flags.Val = 0;
                            MTP.flags.bits.broadcast = 1;           
                            #if defined(IEEE_802_15_4)
                            if (transceiver == 3){
                                MTP.altSrcAddr = TRUE;
                                MTP.DestPANID.Val = myPANID->Val;
                            }
                            #endif    
                            
                            AuxMAC_SendPacket(&MTP, transceiver);
                            break;
                        }
                    }
                }
                MiApp_SetChannel(OptimalChannel, mb);
            }

            /*******************************************************************
             ******************************************************************/
            BOOL MiApp_InitChannelHopping(INPUT DWORD ChannelMap, \
                                          INPUT miwi_band mb){
                BYTE *currentChannel;
                BYTE ChannelOffset, ConnMode;
                switch (mb){
                    case ISM_434:
                        #if !defined MIWI_0434_RI
                            Printf("Error: MiWi ISM 434 MHz band is not available.");
                            return FALSE;
                        #else
                            currentChannel = & MIWI0434_currentChannel;
                            ConnMode = MIWI0434_ConnMode;
                            ChannelOffset = MIWI0434ConfChannelOffset;
                            break;
                        #endif
                    case ISM_868:
                        #if !defined MIWI_0868_RI
                            Printf("Error: MiWi ISM 868 MHz band is not available.");
                            return FALSE;
                        #else
                            currentChannel = & MIWI0868_currentChannel;
                            ChannelOffset = MIWI0868ConfChannelOffset;
                            ConnMode = MIWI0868_ConnMode;
                            break;
                        #endif
                    case ISM_2G4:
                        #if !defined MIWI_2400_RI
                            Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                            return FALSE;
                        #else
                            currentChannel = & MIWI2400_currentChannel;
                            ChannelOffset = MIWI2400ConfChannelOffset;
                            ConnMode = MIWI2400_ConnMode;
                            break;
                        #endif
                    default:
                        return FALSE;
                }

                BYTE RSSIValue;
                BYTE backupChannel = *currentChannel;
                BYTE backupConnMode = ConnMode;
                BYTE optimalChannel;
                
                MiApp_ConnectionMode(DISABLE_ALL_CONN, mb);
                optimalChannel = MiApp_NoiseDetection(ChannelMap, 10, NOISE_DETECT_ENERGY, &RSSIValue, mb);
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
                #else
                    //Error
                #endif
                return;
            #endif
        case ISM_2G4:
            #if !defined MIWI_2400_RI
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
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
                Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                return;
            #else
                MRF24J40_TxBuffer[MRF24J40_TxData++] = data;
                return;
            #endif
        default:
            return;
    }
}

    #if !defined(TARGET_SMALL)
        /***********************************************************************
         * Function:    void MiApp_RemoveConnection(BYTE ConnectionIndex)
         * Summary:     This function remove connection(s) in connection table
         * Description: This is the primary user interface function to 
         *              disconnect connection(s). For a P2P protocol, it simply
         *              remove the connection. For a network protocol, if the
         *              device referred by the input parameter is the parent of
         *              the device calling this function, the calling device
         *              will get out of network along with its children. If the
         *              device referred by the input parameter is children of
         *              the device calling this function, the target device will
         *              get out of network.
         * PreCondition:Transceiver has been initialized. Node has establish one
         *              or more MiApp_WriteDataconnections
         * Parameters:  BYTE ConnectionIndex - The index of the connection in
         *                                    the connection table to be removed
         * Returns:     None
         * Example:
         *      <code>
         *      MiApp_RemoveConnection(0x00);
         *      </code>
         * Remarks:     None
         **********************************************************************/
        void MiApp_RemoveConnection(INPUT BYTE ConnectionIndex, \
                                    INPUT miwi_band mb){
            BYTE *currentChannel, *TxData, *myParent;
            MIWI_STATE_MACHINE *msm;
            BYTE transceiver;
            WORD isFamilyMask;
            WORD_VAL *myPANID, *myShortAddress;
            switch (mb){
                case ISM_434:
                    #if !defined MIWI_0434_RI
                        Printf("Error: MiWi ISM 434 MHz band is not available.");
                        return;
                    #else
                        #if defined MRF49XA_1_IN_434
                            msm = & MRF49XA_1_MiWiStateMachine;
                            TxData = & MRF49XA_1_TxData;
                            transceiver = 1;
                        #elif defined MRF49XA_2_IN_434
                            msm = & MRF49XA_2_MiWiStateMachine;
                            TxData = & MRF49XA_2_TxData;
                            transceiver = 2;
                        #else
                            return;   //Error
                        #endif
                        currentChannel = & MIWI0434_currentChannel;
                        myParent = & my0434Parent;
                        myPANID = & myPAN0434ID;
                        myShortAddress = & myShort0434Addr;
                        isFamilyMask = 0x0020;
                        break;
                    #endif
                case ISM_868:
                    #if !defined MIWI_0868_RI
                        Printf("Error: MiWi ISM 868 MHz band is not available.");
                        return;
                    #else
                        #if defined MRF49XA_1_IN_868
                            msm = & MRF49XA_1_MiWiStateMachine;
                            TxData = & MRF49XA_1_TxData;
                            transceiver = 1;
                        #elif defined MRF49XA_2_IN_868
                            msm = & MRF49XA_2_MiWiStateMachine;
                            TxData = & MRF49XA_2_TxData;
                            transceiver = 2;
                        #else
                            return;   //Error
                        #endif
                        currentChannel = & MIWI0868_currentChannel;
                        myParent = & my0868Parent;
                        myPANID = & myPAN0868ID;
                        myShortAddress = & myShort0868Addr;
                        isFamilyMask = 0x0200;
                        break;
                    #endif
                case ISM_2G4:
                    #if !defined MIWI_2400_RI
                        Printf("Error: MiWi at 2,4 GHz 802.15.4 band is not available.");
                        return;
                    #else
                        msm = & MRF24J40_MiWiStateMachine;
                        TxData = & MRF24J40_TxData;
                        transceiver = 3;
                        currentChannel = & MIWI2400_currentChannel;
                        myParent = & my2400Parent;
                        myPANID = & myPAN2400ID;
                        myShortAddress = & myShort2400Addr;
                        isFamilyMask = 0x2000;
                        break;
                    #endif
                default:
                    return;
            }

            BYTE i;
            WORD j;
                                
            if(ConnectionIndex == 0xFF){    //Juan: REMOVE ALL CONNECTIONS
                for(i=0; i<CONNECTION_SIZE; i++){
                    if(ConnectionTable[i].status.bits.isValid){
                        //Juan: modified for matching is????Family Flag.
                        if(ConnectionTable[i].status.Val & isFamilyMask){
                            *TxData = 0;
                            MiApp_WriteData(MAC_COMMAND_DISASSOCIATION_NOTIFICATION, mb);
                            MTP.flags.Val = 0;
                            MTP.flags.bits.ackReq = 1;
                            MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                            //Juan: modified. Added transceiver, RI condition
                            if (transceiver == 3){
                                #if defined(IEEE_802_15_4) && defined MIWI_2400_RI
                                    MTP.altDestAddr = TRUE;
                                    MTP.altSrcAddr = TRUE;
                                    MTP.DestPANID.Val = ConnectionTable[i].PAN2400ID.Val;
                                    MTP.DestAddress = ConnectionTable[i].MIWI2400AltAddress.v;
                                #endif
                            }else{
                                MTP.flags.bits.sourcePrsnt = 1;
                                MTP.DestAddress = ConnectionTable[i].Address;
                            }

                            AuxMAC_SendPacket(&MTP, transceiver);
                            for(j = 0; j < 0xFFF; j++) {}
                        }
                    }
                    ConnectionTable[i].status.Val = 0;
                } 
                //Juan: invalid parent, shortAddr, PANID...
                *myParent = 0xFF;
                myShortAddress->Val = 0xFFFF;
                myPANID->Val = 0xFFFF;

                //Juan: added conditions.
                #if defined IEEE_802_15_4
                    if (mb == ISM_2G4){
                        MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                    }
                #endif
                
                msm->bits.memberOfNetwork = 0;
                #if defined(ENABLE_NETWORK_FREEZER)
                    nvmPutMyShortAddress(myShortAddress->v);
                    nvmPutMyPANID(myPANID->v);
                    nvmPutMyParent(&myParent);
                    nvmPutConnectionTable(ConnectionTable);
                #endif
            }
            //Juan: REMOVE A SINGLE CONNECTION.
            else if(ConnectionTable[ConnectionIndex].status.bits.isValid){

                if((ConnectionTable[ConnectionIndex].status.Val & isFamilyMask) && \
                   (ConnectionIndex == *myParent)){ 
                    //parent
                
                    // first notify my parent
                    *TxData = 0;
                    MiApp_WriteData(MAC_COMMAND_DISASSOCIATION_NOTIFICATION, mb);
                    MTP.flags.Val = 0;
                    MTP.flags.bits.ackReq = 1;
                    MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                    if(transceiver == 3){
                        #if defined(IEEE_802_15_4)
                            MTP.altDestAddr = TRUE;
                            MTP.altSrcAddr = TRUE;
                            MTP.DestPANID.Val = ConnectionTable[ConnectionIndex].PAN2400ID.Val;
                            MTP.DestAddress = ConnectionTable[ConnectionIndex].MIWI2400AltAddress.v;
                        #endif
                    }else {
                        MTP.flags.bits.sourcePrsnt = 1;
                        MTP.DestAddress = ConnectionTable[ConnectionIndex].Address;
                    }

                    AuxMAC_SendPacket(&MTP, transceiver);
                    for(j= 0; j<0xFFF; j++) {}
                   
                    for(i=0; i<CONNECTION_SIZE; i++){
                        #if defined(NWK_ROLE_COORDINATOR)
                            if((ConnectionTable[i].status.bits.isValid)      &&\
                               (ConnectionTable[i].status.Val & isFamilyMask)&&\
                               (*myParent != i)){

                                // children
                                if (transceiver == 3){
                                    #if defined(IEEE_802_15_4)
                                        MTP.DestAddress = ConnectionTable[i].MIWI2400AltAddress.v;
                                        MTP.DestPANID.Val = ConnectionTable[i].PAN2400ID.Val;
                                    #endif
                                }else{
                                    MTP.DestAddress = ConnectionTable[i].Address;
                                }

                                AuxMAC_SendPacket(&MTP, transceiver);
                                for(j = 0; j < 0xFFF; j++) {}      
                            }
                        #endif
                        ConnectionTable[i].status.Val = 0;
                    }

                    // get myself out of network
                    *myParent = 0xFF;
                    myShortAddress->Val = 0xFFFF;
                    myPANID->Val = 0xFFFF;

                    //Juan: added conditions.
                    #if defined IEEE_802_15_4
                        if (mb == ISM_2G4){
                            MiMAC_SetAltAddress(myShortAddress->v, myPANID->v);
                        }
                    #endif
                    
                    msm->bits.memberOfNetwork = 0;
                    #if defined(ENABLE_NETWORK_FREEZER)
                        nvmPutMyShortAddress(myShortAddress->v);
                        nvmPutMyPANID(myPANID->v);
                        nvmPutMyParent(&myParent);
                        nvmPutConnectionTable(ConnectionTable);
                    #endif
                }
                #if defined(NWK_ROLE_COORDINATOR)                
                    else if((ConnectionTable[ConnectionIndex].status.Val & isFamilyMask) && \
                            (ConnectionIndex != *myParent)){
                        // child
                    
                        *TxData = 0;
                        MiApp_WriteData(MAC_COMMAND_DISASSOCIATION_NOTIFICATION, mb);
                        MTP.flags.Val = 0;
                        MTP.flags.bits.ackReq = 1;
                        MTP.flags.bits.packetType = PACKET_TYPE_COMMAND;
                        if (transceiver == 3){
                            #if defined(IEEE_802_15_4)
                                MTP.altDestAddr = TRUE;
                                MTP.altSrcAddr = TRUE;
                                MTP.DestPANID.Val = ConnectionTable[ConnectionIndex].PAN2400ID.Val;
                                MTP.DestAddress = ConnectionTable[ConnectionIndex].MIWI2400AltAddress.v;
                            #endif
                        }else{
                            MTP.flags.bits.sourcePrsnt = 1;
                            MTP.DestAddress = ConnectionTable[ConnectionIndex].Address;
                        }

                        AuxMAC_SendPacket(&MTP, transceiver);
                        ConnectionTable[ConnectionIndex].status.Val = 0;
                        #if defined(ENABLE_NETWORK_FREEZER)
                            nvmPutConnectionTableIndex(&(ConnectionTable[ConnectionIndex]), ConnectionIndex);
                        #endif
                    }
                #endif
            }
        }
    #endif


// LSI-CWSN new functions
static BOOL AuxMAC_SendPacket(MAC_TRANS_PARAM *MTP, BYTE transceiver){
    BOOL ok;
    switch (transceiver){
        case 1:
            #if defined MRF49XA_1
                ok = MiMAC_MRF49XA_SendPacket(*(MTP), MRF49XA_1_TxBuffer, MRF49XA_1_TxData, 1);
                return ok;
            #else
                return FALSE;   // ERROR here! transceiver has an invalid value
            #endif
        case 2:
            #if defined MRF49XA_2
                ok = MiMAC_MRF49XA_SendPacket(*(MTP), MRF49XA_2_TxBuffer, MRF49XA_2_TxData, 2);
                return ok;
            #else
                return FALSE;   // ERROR here! transceiver has an invalid value
            #endif
        case 3:
            #if defined MRF24J40
                ok = MiMAC_MRF24J40_SendPacket(*(MTP), MRF24J40_TxBuffer, MRF24J40_TxData);
                return ok;
            #else
                return FALSE;   // ERROR here! transceiver has an invalid value
            #endif
        default:
            return FALSE;       //Wrong transceiver!
    }
}

static void AuxMAC_FlushTx(BYTE transceiver){
    switch (transceiver){
        case 1:
            #if defined MRF49XA_1
                MRF49XA_1_TxData = 0;
            #endif
            return;
        case 2:
            #if defined MRF49XA_2
                MRF49XA_2_TxData = 0;
            #endif
            return;
        case 3:
            #if defined MRF24J40
                MRF24J40_TxData = 0;
            #endif
            return;
        default:
            return;       //Wrong transceiver!
    }
}

#else
    // define a bogus variable to bypass limitation of C18 compiler not able to compile an empty file
    extern char bogusVar;
#endif

