function(target_set_debug_flags TARGET ENABLED)
	if (NOT ${ENABLED})
		message(STATUS "Warnings Disabled for: ${TARGET}")
		return()
	endif()

	set(GCC_FLAGSS
		-g)
	
	#detect current compiler and accordingly set the warnings
	if(CMAKE_C_COMPILER_ID MATCHES "GNU")
		set(CMAKE_BUILD_TYPE Debug)
	endif()

	target_compile_options(${TARGET} PRIVATE ${WARNINGS})
	message(STATUS ${WARNINGS})

endfunction()
