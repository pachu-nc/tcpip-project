cmake_minimum_required(VERSION 3.22)

project(tcpip
	VERSION 1.0.0 
	LANGUAGES C CXX)

#Override some default CMake variables

#set(CMAKE_C_STANDARD 		99)
#set(CMAKE_C_STANDARD_REQUIRED	ON) # can be set to OFF also
#set(CMAKE_C_EXTENSIONS		OFF)

## Defining Variables

#set(LIB_NAME linked_list)
set(GRAPH_LIB_NAME graph)
set(LIB_CLI cli)
set(DLL_LIB_NAME glthread)
set(SUB_DIR_LIB ut_parser)
set(GLUE_DIR_LIB gluethread)
set(LAYER2_LIB_NAME layer2)
set(LAYER3_LIB_NAME layer3)
set(LAYER5_LIB_NAME ping)
set(EXECUTABLE tcpip_appl)


#set cmake module path variable

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

#Add cmake submodule file
#include(arm32-cross-toolchain)
include(Docs)

#add_custom_target()

#call the function in AddGitSubmodule

#Enable Warnings

option(ENABLE_WARNINGS "Whether to Enable Warnings" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Whether to Enable Warnings as errors" OFF)
option(ENABLE_SANITIZE_ADDR "Enable Address Sanitizer .. " ON)
option(ENABLE_DEBUG_FLAGS "Whether to Enable Warnings" ON)

if(ENABLE_WARNINGS)
	include(Warnings)
	message(STATUS "Included Warnings")
endif()

if(ENABLE_SANITIZE_ADDR)
        include(Sanitizer)
        add_sanitizer_flags()
	message(STATUS "Included Address Sanitizer Flag")
endif()

if(ENABLE_CROSS_COMPILER)
	include(arm32-cross-toolchain)
	message(STATUS "Included armtool-chain")
endif()

if(ENABLE_DEBUG_FLAGS)
	#include(Debugflags)
	set(CMAKE_BUILD_TYPE Debug)
	message(STATUS "Included Debug Flags")
endif()


add_subdirectory(lib)
add_subdirectory(CommandParser1)
add_subdirectory(app)
add_subdirectory(Layer2)
add_subdirectory(Layer3)
add_subdirectory(Layer5)
