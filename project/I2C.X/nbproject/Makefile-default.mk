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
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1445274692/I2C.o ${OBJECTDIR}/_ext/1445274692/Barometer.o ${OBJECTDIR}/_ext/1445274692/Serial.o ${OBJECTDIR}/_ext/1445274692/Timer.o ${OBJECTDIR}/_ext/1445274692/Board.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1445274692/I2C.o.d ${OBJECTDIR}/_ext/1445274692/Barometer.o.d ${OBJECTDIR}/_ext/1445274692/Serial.o.d ${OBJECTDIR}/_ext/1445274692/Timer.o.d ${OBJECTDIR}/_ext/1445274692/Board.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1445274692/I2C.o ${OBJECTDIR}/_ext/1445274692/Barometer.o ${OBJECTDIR}/_ext/1445274692/Serial.o ${OBJECTDIR}/_ext/1445274692/Timer.o ${OBJECTDIR}/_ext/1445274692/Board.o


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
	${MAKE} ${MAKE_OPTIONS} -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX320F128H
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
${OBJECTDIR}/_ext/1445274692/I2C.o: ../../src/I2C.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/I2C.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/I2C.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/I2C.o.d" -o ${OBJECTDIR}/_ext/1445274692/I2C.o ../../src/I2C.c   
	
${OBJECTDIR}/_ext/1445274692/Barometer.o: ../../src/Barometer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Barometer.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Barometer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Barometer.o.d" -o ${OBJECTDIR}/_ext/1445274692/Barometer.o ../../src/Barometer.c   
	
${OBJECTDIR}/_ext/1445274692/Serial.o: ../../src/Serial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Serial.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Serial.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Serial.o.d" -o ${OBJECTDIR}/_ext/1445274692/Serial.o ../../src/Serial.c   
	
${OBJECTDIR}/_ext/1445274692/Timer.o: ../../src/Timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Timer.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Timer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Timer.o.d" -o ${OBJECTDIR}/_ext/1445274692/Timer.o ../../src/Timer.c   
	
${OBJECTDIR}/_ext/1445274692/Board.o: ../../src/Board.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Board.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Board.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Board.o.d" -o ${OBJECTDIR}/_ext/1445274692/Board.o ../../src/Board.c   
	
else
${OBJECTDIR}/_ext/1445274692/I2C.o: ../../src/I2C.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/I2C.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/I2C.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/I2C.o.d" -o ${OBJECTDIR}/_ext/1445274692/I2C.o ../../src/I2C.c   
	
${OBJECTDIR}/_ext/1445274692/Barometer.o: ../../src/Barometer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Barometer.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Barometer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Barometer.o.d" -o ${OBJECTDIR}/_ext/1445274692/Barometer.o ../../src/Barometer.c   
	
${OBJECTDIR}/_ext/1445274692/Serial.o: ../../src/Serial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Serial.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Serial.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Serial.o.d" -o ${OBJECTDIR}/_ext/1445274692/Serial.o ../../src/Serial.c   
	
${OBJECTDIR}/_ext/1445274692/Timer.o: ../../src/Timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Timer.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Timer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Timer.o.d" -o ${OBJECTDIR}/_ext/1445274692/Timer.o ../../src/Timer.c   
	
${OBJECTDIR}/_ext/1445274692/Board.o: ../../src/Board.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/Board.o.d 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/Board.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I"." -I"C:/Users/dagoodma/Documents/SDP/include" -I"C:/Users/Shehadeh/Documents/GitHub/sdp/include" -I"/Users/ddeo/Documents/sdp/include" -MMD -MF "${OBJECTDIR}/_ext/1445274692/Board.o.d" -o ${OBJECTDIR}/_ext/1445274692/Board.o ../../src/Board.c   
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/I2C.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
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
