#create an executable with the name hello_world and source file hello_world.c

set(EXEC_SRC
	"tcpip.c")

add_executable(${EXECUTABLE} ${EXEC_SRC})


target_include_directories(${EXECUTABLE} PUBLIC "../include")

#link the Lib named Library with the executable hello_world making it publioc
# Executable hello_world depends on Library

target_link_libraries(${EXECUTABLE} PRIVATE ${LIB_NAME})

#Enable Warnings
if(${ENABLE_WARNINGS})
	target_set_warnings(${EXECUTABLE}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)
endif()
