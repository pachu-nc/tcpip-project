function(target_set_warnings TARGET ENABLED ENABLED_AS_ERRORS)
	if (NOT ${ENABLED})
		message(STATUS "Warnings Disabled for: ${TARGET}")
		return()
	endif()

	set(GCC_WARNINGS
		-Wall
		-Wextra
		-Wpedantic)

	if(${ENABLED_AS_ERRORS})
		#append -Werror to already preset warning flags
		set(GCC_WARNINGS ${GCC_WARNINGS} -Werror)
	endif()
	
	#detect current compiler and accordingly set the warnings
	if(CMAKE_C_COMPILER_ID MATCHES "GNU")
		set(WARNINGS ${GCC_WARNINGS})
	endif()

	target_compile_options(${TARGET} PRIVATE ${WARNINGS})
	message(STATUS ${WARNINGS})

endfunction()
