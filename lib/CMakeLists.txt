
# Create a Static Library named Library with the Source files as my_lib.c

set(LIB_SRC
	"linked_list.c")

set(DLL_LIB_SRC
	"dll.c")

set(GRAPH_LIB_SRC
	"graph.c"
	"net.c")

add_library(${LIB_NAME} STATIC ${LIB_SRC})
add_library(${DLL_LIB_NAME} STATIC ${DLL_LIB_SRC})
add_library(${GRAPH_LIB_NAME} STATIC ${GRAPH_LIB_SRC})

#include the incude directory

target_include_directories(${LIB_NAME} PUBLIC "../include")
target_include_directories(${DLL_LIB_NAME} PUBLIC "../include")
target_include_directories(${GRAPH_LIB_NAME} PUBLIC "../include")

target_link_libraries(${GRAPH_LIB_NAME} PRIVATE ${DLL_LIB_NAME})
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

	target_set_warnings(${GRAPH_LIB_NAME}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)

endif()
