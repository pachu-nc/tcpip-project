
# Create a Static Library named Library with the Source files as my_lib.c

set(LIB_SRC
	"linked_list.c")

add_library(${LIB_NAME} STATIC ${LIB_SRC})

#include the incude directory

target_include_directories(${LIB_NAME} PUBLIC "../include")

#Enable Warnings

if(${ENABLE_WARNINGS})
	target_set_warnings(${LIB_NAME}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)
endif()
