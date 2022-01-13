#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/Client.o \
	${OBJECTDIR}/src/ConfUtils.o \
	${OBJECTDIR}/src/Log.o \
	${OBJECTDIR}/src/TrafficSource.o \
	${OBJECTDIR}/src/Vatic.o \
	${OBJECTDIR}/src/client_program.o \
	${OBJECTDIR}/src/sandbox.o \
	${OBJECTDIR}/src/server_program.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vatic

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vatic: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vatic ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/Client.o: src/Client.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Client.o src/Client.cpp

${OBJECTDIR}/src/ConfUtils.o: src/ConfUtils.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ConfUtils.o src/ConfUtils.cpp

${OBJECTDIR}/src/Log.o: src/Log.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Log.o src/Log.cpp

${OBJECTDIR}/src/TrafficSource.o: src/TrafficSource.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/TrafficSource.o src/TrafficSource.cpp

${OBJECTDIR}/src/Vatic.o: src/Vatic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Vatic.o src/Vatic.cpp

${OBJECTDIR}/src/client_program.o: src/client_program.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/client_program.o src/client_program.cpp

${OBJECTDIR}/src/sandbox.o: src/sandbox.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/sandbox.o src/sandbox.cpp

${OBJECTDIR}/src/server_program.o: src/server_program.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/server_program.o src/server_program.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
