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
FINAL_IMAGE=${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=main.c lcd.c adc.c speech_processing.c kiss_fft130/kiss_fft.c kiss_fft130/tools/kiss_fftr.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/main.o ${OBJECTDIR}/lcd.o ${OBJECTDIR}/adc.o ${OBJECTDIR}/speech_processing.o ${OBJECTDIR}/kiss_fft130/kiss_fft.o ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o
POSSIBLE_DEPFILES=${OBJECTDIR}/main.o.d ${OBJECTDIR}/lcd.o.d ${OBJECTDIR}/adc.o.d ${OBJECTDIR}/speech_processing.o.d ${OBJECTDIR}/kiss_fft130/kiss_fft.o.d ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/main.o ${OBJECTDIR}/lcd.o ${OBJECTDIR}/adc.o ${OBJECTDIR}/speech_processing.o ${OBJECTDIR}/kiss_fft130/kiss_fft.o ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o

# Source Files
SOURCEFILES=main.c lcd.c adc.c speech_processing.c kiss_fft130/kiss_fft.c kiss_fft130/tools/kiss_fftr.c

# Pack Options 
PACK_COMPILER_OPTIONS=-I "${DFP_DIR}/include"
PACK_COMMON_OPTIONS=-B "${DFP_DIR}/gcc/dev/atmega32"



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=ATmega32
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
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/47ead31d8a9821ddb4e7e353a5d61639879d9b82 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/main.o.d" -MT "${OBJECTDIR}/main.o.d" -MT ${OBJECTDIR}/main.o  -o ${OBJECTDIR}/main.o main.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/lcd.o: lcd.c  .generated_files/flags/default/959b1dd90b6152e7bac66bed0bbaffbd475d59bb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/lcd.o.d 
	@${RM} ${OBJECTDIR}/lcd.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/lcd.o.d" -MT "${OBJECTDIR}/lcd.o.d" -MT ${OBJECTDIR}/lcd.o  -o ${OBJECTDIR}/lcd.o lcd.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/adc.o: adc.c  .generated_files/flags/default/94593676f68a67352e064fa140329ba1c7312136 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/adc.o.d 
	@${RM} ${OBJECTDIR}/adc.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/adc.o.d" -MT "${OBJECTDIR}/adc.o.d" -MT ${OBJECTDIR}/adc.o  -o ${OBJECTDIR}/adc.o adc.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/speech_processing.o: speech_processing.c  .generated_files/flags/default/1e3ccabdff33a1fb508f0fba87273e2364e80af3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/speech_processing.o.d 
	@${RM} ${OBJECTDIR}/speech_processing.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/speech_processing.o.d" -MT "${OBJECTDIR}/speech_processing.o.d" -MT ${OBJECTDIR}/speech_processing.o  -o ${OBJECTDIR}/speech_processing.o speech_processing.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/kiss_fft130/kiss_fft.o: kiss_fft130/kiss_fft.c  .generated_files/flags/default/e4506504e0a42bd87c0393f8a50ca98b39ca2bc4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/kiss_fft130" 
	@${RM} ${OBJECTDIR}/kiss_fft130/kiss_fft.o.d 
	@${RM} ${OBJECTDIR}/kiss_fft130/kiss_fft.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/kiss_fft130/kiss_fft.o.d" -MT "${OBJECTDIR}/kiss_fft130/kiss_fft.o.d" -MT ${OBJECTDIR}/kiss_fft130/kiss_fft.o  -o ${OBJECTDIR}/kiss_fft130/kiss_fft.o kiss_fft130/kiss_fft.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o: kiss_fft130/tools/kiss_fftr.c  .generated_files/flags/default/18ef6361b8aba0a7f34ce69f509726368875b2f2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/kiss_fft130/tools" 
	@${RM} ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d 
	@${RM} ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d" -MT "${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d" -MT ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o  -o ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o kiss_fft130/tools/kiss_fftr.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/cf981e5aa345f4727d5c820b001a988f866b8ada .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/main.o.d" -MT "${OBJECTDIR}/main.o.d" -MT ${OBJECTDIR}/main.o  -o ${OBJECTDIR}/main.o main.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/lcd.o: lcd.c  .generated_files/flags/default/ec14d985117e03f39c33a2a78fe3e99c8c8a5329 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/lcd.o.d 
	@${RM} ${OBJECTDIR}/lcd.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/lcd.o.d" -MT "${OBJECTDIR}/lcd.o.d" -MT ${OBJECTDIR}/lcd.o  -o ${OBJECTDIR}/lcd.o lcd.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/adc.o: adc.c  .generated_files/flags/default/907e2318874d981586eb18de20d26e0dd01c79b3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/adc.o.d 
	@${RM} ${OBJECTDIR}/adc.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/adc.o.d" -MT "${OBJECTDIR}/adc.o.d" -MT ${OBJECTDIR}/adc.o  -o ${OBJECTDIR}/adc.o adc.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/speech_processing.o: speech_processing.c  .generated_files/flags/default/ec13bca2d0a5c8b43d49bfe2ccce021df929142c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/speech_processing.o.d 
	@${RM} ${OBJECTDIR}/speech_processing.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/speech_processing.o.d" -MT "${OBJECTDIR}/speech_processing.o.d" -MT ${OBJECTDIR}/speech_processing.o  -o ${OBJECTDIR}/speech_processing.o speech_processing.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/kiss_fft130/kiss_fft.o: kiss_fft130/kiss_fft.c  .generated_files/flags/default/8075875ed6e1687f79176ccae4200200c0a1701a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/kiss_fft130" 
	@${RM} ${OBJECTDIR}/kiss_fft130/kiss_fft.o.d 
	@${RM} ${OBJECTDIR}/kiss_fft130/kiss_fft.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/kiss_fft130/kiss_fft.o.d" -MT "${OBJECTDIR}/kiss_fft130/kiss_fft.o.d" -MT ${OBJECTDIR}/kiss_fft130/kiss_fft.o  -o ${OBJECTDIR}/kiss_fft130/kiss_fft.o kiss_fft130/kiss_fft.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o: kiss_fft130/tools/kiss_fftr.c  .generated_files/flags/default/a886f8c9a9ccc91427fe04f72fa1db625c101e3a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/kiss_fft130/tools" 
	@${RM} ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d 
	@${RM} ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega32 ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -I"kiss_fft130" -mcall-prologues -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d" -MT "${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o.d" -MT ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o  -o ${OBJECTDIR}/kiss_fft130/tools/kiss_fftr.o kiss_fft130/tools/kiss_fftr.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mmcu=atmega32 ${PACK_COMMON_OPTIONS}  -D__MPLAB_DEBUGGER_SIMULATOR=1 -gdwarf-2 -D__$(MP_PROCESSOR_OPTION)__  -Wl,-u,vfprintf -lprintf_flt -lm -Wl,-Map="${DISTDIR}\SpeechRecog.X.${IMAGE_TYPE}.map"    -o ${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1 -Wl,--gc-sections -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	
	
	
	
	
	
else
${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mmcu=atmega32 ${PACK_COMMON_OPTIONS}  -D__$(MP_PROCESSOR_OPTION)__  -Wl,-u,vfprintf -lprintf_flt -lm -Wl,-Map="${DISTDIR}\SpeechRecog.X.${IMAGE_TYPE}.map"    -o ${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION) -Wl,--gc-sections -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	${MP_CC_DIR}\\avr-objcopy -O ihex "${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}" "${DISTDIR}/SpeechRecog.X.${IMAGE_TYPE}.hex"
	
	
	
	
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
