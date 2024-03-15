function(add_sanitizer_flags)
	if (NOT ${ENABLE_SANITIZE_ADDR})
		message(STATUS "Sanitizer Deactivated")
		return()
	endif()

	set(GCC_WARNINGS
		-Wall
		-Wextra
		-Wpedantic)

	#detect current compiler and accordingly set the warnings
	if(CMAKE_C_COMPILER_ID MATCHES "GNU")
		add_compile_options("-fno-omit-frame-pointer")
		add_link_options("-fno-omit-frame-pointer")
	endif()
	
	if(${ENABLE_SANITIZE_ADDR})
		add_compile_options("-fsanitize=address")
		add_link_options("-fsanitize=address")
	endif()

endfunction()
