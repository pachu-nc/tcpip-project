
# Create a Static Library named Library with the Source files as my_lib.c

set(GLUE_DIR_LIB_SRC
	"glthread.c")

add_library(${GLUE_DIR_LIB} STATIC ${GLUE_DIR_LIB_SRC})

#include the incude directory

target_include_directories(${SUB_DIR_LIB} PUBLIC "../../include")

#Enable Warnings

if(${ENABLE_WARNINGS})
	target_set_warnings(${LIB_NAME}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)

	target_set_warnings(${DLL_LIB_NAME}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)

endif()
