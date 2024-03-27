#create an executable with the name hello_world and source file hello_world.c

set(EXEC_SRC
	"tcpip.c"
	"nwcli.c")


set(PKT_GEN_SRC
	"pkt_gen.c")


add_executable(${EXECUTABLE} ${EXEC_SRC})
add_executable(${PKT_GEN} ${PKT_GEN_SRC})


target_include_directories(${EXECUTABLE} PUBLIC "../include")
target_include_directories(${PKT_GEN} PUBLIC "../include")

#link the Lib named Library with the executable hello_world making it publioc
# Executable hello_world depends on Library

target_link_libraries(${EXECUTABLE} PRIVATE ${GRAPH_LIB_NAME} ${LAYER2_LIB_NAME} ${LAYER3_LIB_NAME} ${LAYER5_LIB_NAME} ${SPF_LIB_NAME} ${TCP_STACK_LIB_NAME} ${DLL_LIB_NAME} ${LIB_CLI} ${SUB_DIR_LIB})
target_link_libraries(${PKT_GEN} PRIVATE ${GRAPH_LIB_NAME} ${LAYER2_LIB_NAME} ${LAYER3_LIB_NAME} ${LAYER5_LIB_NAME} ${DLL_LIB_NAME} ${LIB_CLI} ${SUB_DIR_LIB})
#target_link_libraries(${EXECUTABLE} PRIVATE ${GRAPH_LIB_NAME} ${LIB_CLI} ${SUB_DIR_LIB} ${GLUE_DIR_LIB} )

#Enable Warnings
if(${ENABLE_WARNINGS})
	target_set_warnings(${EXECUTABLE}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
		)


	target_set_warnings(${PKT_GEN}
		${ENABLE_WARNINGS}
		${ENABLE_WARNINGS_AS_ERRORS}
	)

endif()
