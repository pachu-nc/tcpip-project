
# Create a Static Library named Library with the Source files as my_lib.c

set(CLI_LIB_SRC
	"clicbext.c"
	"clistd.c"
	"cmd_hier.c"
	"parser.c"
	"serialize.c"
	"string_util.c"
	)


add_subdirectory(ut)
add_subdirectory(gluethread)
#AUX_SOURCE_DIRECTORY(ut/utinfra SUB_DIR)
add_library(${LIB_CLI} STATIC ${CLI_LIB_SRC})

#include the incude directory

target_include_directories(${LIB_CLI} PUBLIC "../include")

#target_link_libraries(${LIB_CLI} PRIVATE ${LB_NAME} ${DLL_LIB_NAME} ${LIB_CLI})
#Enable Warnings

if(${ENABLE_WARNINGS})
	target_set_warnings(${LIB_CLI}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)
endif()
