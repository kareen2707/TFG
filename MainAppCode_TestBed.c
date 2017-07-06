/*******************************************************************************
 * File:   Main.c
 * Author: Karen Flores Yanez - B105 Electronic Systems Lab - UPM
 *
 * File Description: Finite State Machine to check all node`s hardware
                    functionalities.
 * Change History:
 * Rev   Date         Description
 * 0.0  17/04/2017
 ******************************************************************************/

#include "NodeHAL.h"
#include "WirelessProtocols/Console.h"    //Pruebas
#include "HardwareProfile.h"
#include "fsm.h"
#include "node_states.h"

#define S2_BUTTON 1
#define S3_BUTTON 2
/*#define LED1 1
#define LED2 2
#define LED3 3*/

/*---------------------------Private variables------------------------------*/
BYTE SMS[18] = "Programa de prueba";
BYTE TestAddress[8];
BYTE data1, data2, data3;
/*--------------------------------------------------------------------------*/

/*----------------------------Private Defines-------------------------------*/
#define CCLK   (80000000L)    // system clock
#define PBCLK   (CCLK / 4)    // peripheral bus clock
#define SAMPLE_RATE       5000
#define CCLK_US  (PBCLK/1000000L)   //used for microsecond delay 

#if defined MIWI_2400_RI
    radioInterface ri3 = MIWI_2400;
    BYTE NumChannels3 = MIWI2400NumChannels; // 16 channels
    BYTE powerStep3 = 8;
    BYTE ri_RI_MASK3 = MIWI_2400_RI_MASK; // 0x04
#endif
#if defined MIWI_0868_RI
    radioInterface ri2 = MIWI_0868;
    BYTE NumChannels2 = MIWI0868NumChannels;
    BYTE powerStep2 = 32;
    BYTE ri_RI_MASK2 = MIWI_0868_RI_MASK; //0x02
#endif
#if defined MIWI_0434_RI
    radioInterface ri = MIWI_0434;
    BYTE NumChannels = MIWI0434NumChannels;
    BYTE powerStep = 32;
    BYTE ri_RI_MASK = MIWI_0434_RI_MASK;  //0x01

    radioInterface ri1 = MIWI_0434;
    BYTE NumChannels1 = MIWI0434NumChannels;
    BYTE powerStep1 = 32;
    BYTE ri_RI_MASK1 = MIWI_0434_RI_MASK;
#endif
/*------------------------------------------------------------------------*/
    
/*--------------------Private Prototypes----------------------------------*/
    void InitiAppVariables();
    void PrintTestAddress(BYTE AddrMode, BYTE* Address);
    int send_data(radioInterface ri, BYTE* message ,BYTE *Address);
    int receive_data(BYTE i, radioInterface ri, BYTE ri_mask, BYTE storeHere );
    void time_delay_us(int delay);
/*------------------------------------------------------------------------*/
void InitAppVariables(){
    //Si las hubiera...
}

void PrintTestAddress(BYTE AddrMode, BYTE* Address){
    BYTE i;
    BYTE j = 0;
    Printf("\r\nAddrMode: ");
    switch(AddrMode){
        case LONG_MIWI_ADDRMODE: // 0x10
            Printf("LONG_ADDR    Address: ");
            j = MY_ADDRESS_LENGTH;
            break;
        case SHORT_MIWI_ADDRMODE: // 0x11
            Printf("SHORT_ADDR   Address: ");
            j = 2;
            break;
        default:
            Printf("ERROR");
            break;
    }
    for(i=0; i<j; i++){
        PrintChar(*(Address+j-1-i));
    }
}

int send_data(radioInterface ri, BYTE* message ,BYTE *Address){
    BYTE i=0;
    BYTE j;
    int result=0;
    while(i < sizeof(message)){
        j = PutTXData(ri, message[i]);
        if (j){
            Printf("\r\n Estado, ENVIO, Error buffer writing:\n");
            PrintChar(j);
            DiscardTXData(ri);
            break;
        }
        else {
            i++;
        }
    }

    i = SendPckt(ri, 7, Address);
    if (i){
        Printf("\r\n ENVIO, Failure ocurred!");
    }
    else{
        result = 1;
        Printf("\r\n ENVIO, Mensaje Enviado!");
    }
    return result;
}

int receive_data(BYTE i, radioInterface ri, BYTE ri_mask,  BYTE storeHere ){
    int result=0;
    if (i & ri_mask){
        result = 1;
        Printf("\r\n RECIBIENDO, Mensaje Recibido!\n ");
        while(GetPayloadToRead(ri) >0){
            GetRXData(ri, &storeHere);
            ConsolePut(storeHere);
            }
        }
    return result;
}

void time_delay_us (int delay) {
    unsigned int startTime = ReadCoreTimer();     
    unsigned int delayCount = delay * CCLK_US; 
    while ( ReadCoreTimer() - startTime < delayCount);  
 } 

/*--------------------------FSM functions-----------------------------------*/
static void leds_off(fsm_t* this){
    SwitchOffLed(LED1);
    SwitchOffLed(LED2);
    SwitchOffLed(LED3);
    Printf("\r\n Estado actual: ENVIANDO/ RECIBIENDO");
    Printf("\r\n Siguiente estado: ESPERA");
    Printf("\r\n Si pulsas S2 pasas a ENVIANDO");
    Printf("\r\n Si pulsas S3 pasas a RECIBIENDO");
    Printf("\r\n Funcion ejecutándose: apagar_leds");
    Printf("\r\n ");
}
 
static int s2_pressed(fsm_t* this){
    if(s2Pressed()){
        Printf("\r\n S2 pulsado");
        return 1;
    }
    return 0;
}

static void sendSMS (fsm_t* this){
    TestAddress[1] = 0x00;
    TestAddress[0] = 0x00;
    Printf("\r\n Estado origen: ESPERANDO");
    Printf("\r\n Siguiente estado: ENVIANDO");
    Printf("\r\n Funcion ejecutándose: enviar_sms");
    Printf("\r\n ");
    if(send_data(ri, SMS, TestAddress)){
       SwitchOnLed(LED1);
    }
    if(send_data(ri2, SMS, TestAddress)){
       SwitchOnLed(LED2);
    }
    if(send_data(ri3, SMS, TestAddress)){
       SwitchOnLed(LED3);
    }
    Printf("\r\n ");
}

static int s3_pressed(fsm_t* this){
    if(s3Pressed()){
        Printf("\r\n S3 pulsado");
        return 1;
    }
    return 0;
}

static void receiveSMS(fsm_t* this){
    Printf("\r\n Estado origen: ESPERANDO");
    Printf("\r\n Siguiente estado: RECIBIENDO");
    Printf("\r\n Funcion ejecutándose: recibir_sms");
    Printf("\r\n ");
    BYTE i = WhichRIHasData();
    if(receive_data(i, ri1, ri_RI_MASK1, data1)){
        Printf("\r\n Por 434 MHz");
        SwitchOnLed(LED1);
    }
    if(receive_data(i, ri2, ri_RI_MASK2, data2)){
        Printf("\r\n Por 868 MHz");
        SwitchOnLed(LED2);
    }
    if(receive_data(i, ri3, ri_RI_MASK3, data3)){
        Printf("\r\n Por 2,4 GHz");
        SwitchOnLed(LED3);
    }
}

/* ---------TEST-BED FSM TRANSITIONS-----------------------------------*/
static fsm_trans_t testBedMachine[] = {
	{NODE_IDLE, s2_pressed , NODE_SEND,	sendSMS},
    {NODE_SEND,	s2_pressed,	NODE_IDLE,	leds_off},
    {NODE_IDLE,	s3_pressed,	NODE_RECEIVE, receiveSMS},
    {NODE_RECEIVE,	s3_pressed,	NODE_IDLE,	leds_off},
	{-1,NULL,-1,NULL}
};
int mainApp(void) {

    InitAppVariables();
    InitNode();
    fsm_t* testBed_fsm = fsm_new (testBedMachine);
    
/*    
    BYTE SMS2[7] = "Que tal";
    BYTE SMS3[10] = "Bien soy 2";
    BYTE SMS5[12] = "Bien soy 3 3";
    BYTE SMS4[9] = "Secreto 1";
    BYTE SMS6[9] = "Secreto 2";

    BYTE *beatles = "When I find myself in times of trouble, Mother Mary comes to me,"
                    " speaking words of wisdom, let it be. And in my hour of darkness"
                    " she is standing right in front of me, speaking words of wisdom,"
                    "let it be";

    BYTE *LoremIpsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                       "Praesent fermentum dolor vitae nunc dignissim at sollicitudin "
                       "enim ullamcorper. Pellentesque eu nulla ut enim varius aliquet"
                       " ut quis sapien. Aliquam sodales felis id nisi sagittis "
                       "vulputate volutpat.";*/
    
    while(1){
        fsm_fire(testBed_fsm);
        time_delay_us(500000);

}
    return 0;
}

// Exception handler:
static enum {
    EXCEP_IRQ = 0,                  // interrupt
    EXCEP_AdEL = 4,                 // address error exception (load or ifetch)
    EXCEP_AdES,                     // address error exception (store)
    EXCEP_IBE,                      // bus error (ifetch)
    EXCEP_DBE,                      // bus error (load/store)
    EXCEP_Sys,                      // syscall
    EXCEP_Bp,                       // breakpoint
    EXCEP_RI,                       // reserved instruction
    EXCEP_CpU,                      // coprocessor unusable
    EXCEP_Overflow,                 // arithmetic overflow
    EXCEP_Trap,                     // trap (possible divide by zero)
    EXCEP_IS1 = 16,                 // implementation specfic 1
    EXCEP_CEU,                      // CorExtend Unuseable
    EXCEP_C2E                       // coprocessor 2
} _excep_code;

static unsigned int _epc_code;
static unsigned int _excep_addr;

// this function overrides the normal _weak_ generic handler
void _general_exception_handler(void){
    Printf("\r\n\nGeneral Exception Handler...\r\r");

    asm volatile("mfc0 %0,$13" : "=r" (_excep_code));
    asm volatile("mfc0 %0,$14" : "=r" (_excep_addr));

    _excep_code = (_excep_code & 0x0000007C) >> 2;
    while (1) {
        // Examine _excep_code to identify the type of exception
        // Examine _excep_addr to find the address that caused the exception
        Nop();
        Nop();
        Nop();
    }
}//End of exception handler
