
# Create a Static Library named Library with the Source files as my_lib.c

set(LAYER5_LIB_SRC
	"layer5.c"
	"ping.c")


add_library(${LAYER5_LIB_NAME} STATIC ${LAYER5_LIB_SRC})

#include the incude directory

target_include_directories(${LAYER5_LIB_NAME} PUBLIC "../include")

#target_link_libraries(${GRAPH_LIB_NAME} PRIVATE ${DLL_LIB_NAME})
#Enable Warnings

if(${ENABLE_WARNINGS})
	
	target_set_warnings(${LAYER5_LIB_NAME}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)

endif()
