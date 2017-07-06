#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED="CWSN LSI Node/Common/TimeDelay.c" "CWSN LSI Node/Common/timer.c" "CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.c" "CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.c" "CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.c" "CWSN LSI Node/Transceivers/crc.c" "CWSN LSI Node/Transceivers/security.c" "CWSN LSI Node/USB/usb_descriptors.c" ../libs/Microchip/USB/usb_device.c "../libs/Microchip/USB/CDC Device Driver/usb_function_cdc.c" "CWSN LSI Node/USB/usb_callback_functions.c" "CWSN LSI Node/WirelessProtocols/MiWi/MiWi.c" "CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.c" "CWSN LSI Node/WirelessProtocols/P2P/P2P.c" "CWSN LSI Node/WirelessProtocols/SymbolTime.c" "CWSN LSI Node/WirelessProtocols/NVM.c" "CWSN LSI Node/WirelessProtocols/EEPROM.c" "CWSN LSI Node/WirelessProtocols/LCDBlocking.c" "CWSN LSI Node/WirelessProtocols/MSPI.c" "CWSN LSI Node/WirelessProtocols/Console.c" "CWSN LSI Node/HardwareProfile.c" "CWSN LSI Node/NodeHAL.c" fsm.c MainAppCode_TestBed.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED="${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o" "${OBJECTDIR}/CWSN LSI Node/Common/timer.o" "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o" "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o" "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o" "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o" "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o" "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o" ${OBJECTDIR}/_ext/967747917/usb_device.o ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o" "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o" "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o" ${OBJECTDIR}/fsm.o ${OBJECTDIR}/MainAppCode_TestBed.o
POSSIBLE_DEPFILES="${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o.d" "${OBJECTDIR}/CWSN LSI Node/Common/timer.o.d" "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o.d" "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o.d" "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o.d" "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o.d" "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o.d" "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o.d" ${OBJECTDIR}/_ext/967747917/usb_device.o.d ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o.d" "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o.d" "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o.d" "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o.d" ${OBJECTDIR}/fsm.o.d ${OBJECTDIR}/MainAppCode_TestBed.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/CWSN\ LSI\ Node/Common/TimeDelay.o ${OBJECTDIR}/CWSN\ LSI\ Node/Common/timer.o ${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF24J40/MRF24J40.o ${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF49XA/MRF49XA.o ${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF89XA/MRF89XA.o ${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/crc.o ${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/security.o ${OBJECTDIR}/CWSN\ LSI\ Node/USB/usb_descriptors.o ${OBJECTDIR}/_ext/967747917/usb_device.o ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o ${OBJECTDIR}/CWSN\ LSI\ Node/USB/usb_callback_functions.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MiWi/MiWi.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MiWiPRO/MiWiPRO.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/P2P/P2P.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/SymbolTime.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/NVM.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/EEPROM.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/LCDBlocking.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MSPI.o ${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/Console.o ${OBJECTDIR}/CWSN\ LSI\ Node/HardwareProfile.o ${OBJECTDIR}/CWSN\ LSI\ Node/NodeHAL.o ${OBJECTDIR}/fsm.o ${OBJECTDIR}/MainAppCode_TestBed.o

# Source Files
SOURCEFILES=CWSN LSI Node/Common/TimeDelay.c CWSN LSI Node/Common/timer.c CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.c CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.c CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.c CWSN LSI Node/Transceivers/crc.c CWSN LSI Node/Transceivers/security.c CWSN LSI Node/USB/usb_descriptors.c ../libs/Microchip/USB/usb_device.c ../libs/Microchip/USB/CDC Device Driver/usb_function_cdc.c CWSN LSI Node/USB/usb_callback_functions.c CWSN LSI Node/WirelessProtocols/MiWi/MiWi.c CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.c CWSN LSI Node/WirelessProtocols/P2P/P2P.c CWSN LSI Node/WirelessProtocols/SymbolTime.c CWSN LSI Node/WirelessProtocols/NVM.c CWSN LSI Node/WirelessProtocols/EEPROM.c CWSN LSI Node/WirelessProtocols/LCDBlocking.c CWSN LSI Node/WirelessProtocols/MSPI.c CWSN LSI Node/WirelessProtocols/Console.c CWSN LSI Node/HardwareProfile.c CWSN LSI Node/NodeHAL.c fsm.c MainAppCode_TestBed.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX675F256L
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/CWSN\ LSI\ Node/Common/TimeDelay.o: CWSN\ LSI\ Node/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Common" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o" "CWSN LSI Node/Common/TimeDelay.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Common/timer.o: CWSN\ LSI\ Node/Common/timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Common" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/timer.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/timer.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Common/timer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Common/timer.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Common/timer.o" "CWSN LSI Node/Common/timer.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF24J40/MRF24J40.o: CWSN\ LSI\ Node/Transceivers/MRF24J40/MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o" "CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF49XA/MRF49XA.o: CWSN\ LSI\ Node/Transceivers/MRF49XA/MRF49XA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o" "CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF89XA/MRF89XA.o: CWSN\ LSI\ Node/Transceivers/MRF89XA/MRF89XA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o" "CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/crc.o: CWSN\ LSI\ Node/Transceivers/crc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o" "CWSN LSI Node/Transceivers/crc.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/security.o: CWSN\ LSI\ Node/Transceivers/security.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o" "CWSN LSI Node/Transceivers/security.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/USB/usb_descriptors.o: CWSN\ LSI\ Node/USB/usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/USB" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o.d" -o "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o" "CWSN LSI Node/USB/usb_descriptors.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/967747917/usb_device.o: ../libs/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/967747917" 
	@${RM} ${OBJECTDIR}/_ext/967747917/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/967747917/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/967747917/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/_ext/967747917/usb_device.o.d" -o ${OBJECTDIR}/_ext/967747917/usb_device.o ../libs/Microchip/USB/usb_device.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o: ../libs/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1104080584" 
	@${RM} ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d" -o ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o "../libs/Microchip/USB/CDC Device Driver/usb_function_cdc.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/USB/usb_callback_functions.o: CWSN\ LSI\ Node/USB/usb_callback_functions.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/USB" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o.d" -o "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o" "CWSN LSI Node/USB/usb_callback_functions.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MiWi/MiWi.o: CWSN\ LSI\ Node/WirelessProtocols/MiWi/MiWi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o" "CWSN LSI Node/WirelessProtocols/MiWi/MiWi.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MiWiPRO/MiWiPRO.o: CWSN\ LSI\ Node/WirelessProtocols/MiWiPRO/MiWiPRO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o" "CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/P2P/P2P.o: CWSN\ LSI\ Node/WirelessProtocols/P2P/P2P.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o" "CWSN LSI Node/WirelessProtocols/P2P/P2P.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/SymbolTime.o: CWSN\ LSI\ Node/WirelessProtocols/SymbolTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o" "CWSN LSI Node/WirelessProtocols/SymbolTime.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/NVM.o: CWSN\ LSI\ Node/WirelessProtocols/NVM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o" "CWSN LSI Node/WirelessProtocols/NVM.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/EEPROM.o: CWSN\ LSI\ Node/WirelessProtocols/EEPROM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o" "CWSN LSI Node/WirelessProtocols/EEPROM.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/LCDBlocking.o: CWSN\ LSI\ Node/WirelessProtocols/LCDBlocking.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o" "CWSN LSI Node/WirelessProtocols/LCDBlocking.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MSPI.o: CWSN\ LSI\ Node/WirelessProtocols/MSPI.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o" "CWSN LSI Node/WirelessProtocols/MSPI.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/Console.o: CWSN\ LSI\ Node/WirelessProtocols/Console.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o" "CWSN LSI Node/WirelessProtocols/Console.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/HardwareProfile.o: CWSN\ LSI\ Node/HardwareProfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o.d" -o "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o" "CWSN LSI Node/HardwareProfile.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/NodeHAL.o: CWSN\ LSI\ Node/NodeHAL.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o.d" -o "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o" "CWSN LSI Node/NodeHAL.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/fsm.o: fsm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fsm.o.d 
	@${RM} ${OBJECTDIR}/fsm.o 
	@${FIXDEPS} "${OBJECTDIR}/fsm.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/fsm.o.d" -o ${OBJECTDIR}/fsm.o fsm.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/MainAppCode_TestBed.o: MainAppCode_TestBed.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MainAppCode_TestBed.o.d 
	@${RM} ${OBJECTDIR}/MainAppCode_TestBed.o 
	@${FIXDEPS} "${OBJECTDIR}/MainAppCode_TestBed.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/MainAppCode_TestBed.o.d" -o ${OBJECTDIR}/MainAppCode_TestBed.o MainAppCode_TestBed.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/CWSN\ LSI\ Node/Common/TimeDelay.o: CWSN\ LSI\ Node/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Common" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Common/TimeDelay.o" "CWSN LSI Node/Common/TimeDelay.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Common/timer.o: CWSN\ LSI\ Node/Common/timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Common" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/timer.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Common/timer.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Common/timer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Common/timer.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Common/timer.o" "CWSN LSI Node/Common/timer.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF24J40/MRF24J40.o: CWSN\ LSI\ Node/Transceivers/MRF24J40/MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.o" "CWSN LSI Node/Transceivers/MRF24J40/MRF24J40.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF49XA/MRF49XA.o: CWSN\ LSI\ Node/Transceivers/MRF49XA/MRF49XA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.o" "CWSN LSI Node/Transceivers/MRF49XA/MRF49XA.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/MRF89XA/MRF89XA.o: CWSN\ LSI\ Node/Transceivers/MRF89XA/MRF89XA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.o" "CWSN LSI Node/Transceivers/MRF89XA/MRF89XA.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/crc.o: CWSN\ LSI\ Node/Transceivers/crc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/crc.o" "CWSN LSI Node/Transceivers/crc.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/Transceivers/security.o: CWSN\ LSI\ Node/Transceivers/security.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/Transceivers" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o.d" -o "${OBJECTDIR}/CWSN LSI Node/Transceivers/security.o" "CWSN LSI Node/Transceivers/security.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/USB/usb_descriptors.o: CWSN\ LSI\ Node/USB/usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/USB" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o.d" -o "${OBJECTDIR}/CWSN LSI Node/USB/usb_descriptors.o" "CWSN LSI Node/USB/usb_descriptors.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/967747917/usb_device.o: ../libs/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/967747917" 
	@${RM} ${OBJECTDIR}/_ext/967747917/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/967747917/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/967747917/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/_ext/967747917/usb_device.o.d" -o ${OBJECTDIR}/_ext/967747917/usb_device.o ../libs/Microchip/USB/usb_device.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o: ../libs/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1104080584" 
	@${RM} ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o.d" -o ${OBJECTDIR}/_ext/1104080584/usb_function_cdc.o "../libs/Microchip/USB/CDC Device Driver/usb_function_cdc.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/USB/usb_callback_functions.o: CWSN\ LSI\ Node/USB/usb_callback_functions.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/USB" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o.d" -o "${OBJECTDIR}/CWSN LSI Node/USB/usb_callback_functions.o" "CWSN LSI Node/USB/usb_callback_functions.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MiWi/MiWi.o: CWSN\ LSI\ Node/WirelessProtocols/MiWi/MiWi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWi/MiWi.o" "CWSN LSI Node/WirelessProtocols/MiWi/MiWi.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MiWiPRO/MiWiPRO.o: CWSN\ LSI\ Node/WirelessProtocols/MiWiPRO/MiWiPRO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.o" "CWSN LSI Node/WirelessProtocols/MiWiPRO/MiWiPRO.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/P2P/P2P.o: CWSN\ LSI\ Node/WirelessProtocols/P2P/P2P.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/P2P/P2P.o" "CWSN LSI Node/WirelessProtocols/P2P/P2P.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/SymbolTime.o: CWSN\ LSI\ Node/WirelessProtocols/SymbolTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/SymbolTime.o" "CWSN LSI Node/WirelessProtocols/SymbolTime.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/NVM.o: CWSN\ LSI\ Node/WirelessProtocols/NVM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/NVM.o" "CWSN LSI Node/WirelessProtocols/NVM.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/EEPROM.o: CWSN\ LSI\ Node/WirelessProtocols/EEPROM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/EEPROM.o" "CWSN LSI Node/WirelessProtocols/EEPROM.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/LCDBlocking.o: CWSN\ LSI\ Node/WirelessProtocols/LCDBlocking.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/LCDBlocking.o" "CWSN LSI Node/WirelessProtocols/LCDBlocking.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/MSPI.o: CWSN\ LSI\ Node/WirelessProtocols/MSPI.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/MSPI.o" "CWSN LSI Node/WirelessProtocols/MSPI.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/WirelessProtocols/Console.o: CWSN\ LSI\ Node/WirelessProtocols/Console.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o.d" -o "${OBJECTDIR}/CWSN LSI Node/WirelessProtocols/Console.o" "CWSN LSI Node/WirelessProtocols/Console.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/HardwareProfile.o: CWSN\ LSI\ Node/HardwareProfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o.d" -o "${OBJECTDIR}/CWSN LSI Node/HardwareProfile.o" "CWSN LSI Node/HardwareProfile.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/CWSN\ LSI\ Node/NodeHAL.o: CWSN\ LSI\ Node/NodeHAL.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/CWSN LSI Node" 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o".d 
	@${RM} "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o" 
	@${FIXDEPS} "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o.d" -o "${OBJECTDIR}/CWSN LSI Node/NodeHAL.o" "CWSN LSI Node/NodeHAL.c"    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/fsm.o: fsm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fsm.o.d 
	@${RM} ${OBJECTDIR}/fsm.o 
	@${FIXDEPS} "${OBJECTDIR}/fsm.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/fsm.o.d" -o ${OBJECTDIR}/fsm.o fsm.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/MainAppCode_TestBed.o: MainAppCode_TestBed.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/MainAppCode_TestBed.o.d 
	@${RM} ${OBJECTDIR}/MainAppCode_TestBed.o 
	@${FIXDEPS} "${OBJECTDIR}/MainAppCode_TestBed.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -D"_SUPPRESS_PLIB_WARNING" -D"_DISABLE_OPENADC10_CONFIGPORT_WARNING" -I"CWSN LSI Node/Include" -I"../../../../../../../../../../../Program Files (x86)/Microchip/xc32/v1.40/pic32mx/include" -I"../../../../../Archivos de programa/Microchip/xc32/v1.40/pic32mx/include" -I"../libs/Microchip/Include" -MMD -MF "${OBJECTDIR}/MainAppCode_TestBed.o.d" -o ${OBJECTDIR}/MainAppCode_TestBed.o MainAppCode_TestBed.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_ICD3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC02000:0x1FC02FEF -mreserve=boot@0x1FC02000:0x1FC024FF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1,--defsym=_min_heap_size=200,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=200,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/NODE_FW_clara.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
