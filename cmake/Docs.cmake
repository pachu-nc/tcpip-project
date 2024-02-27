#find doxygen on the system

find_package(Doxygen)

if (DOXYGEN_FOUND)
	#for defining a utility target which we can execute something in the console
	add_custom_target(
		docs
		#DOXYGEN_EXECUTABLE variable is set by find_package operation
		COMMAND ${DOXYGEN_EXECUTABLE}
		#CMAKE_SOURCE_DIR is the absolute path to the main CMakeLists.txt file./root dir
		#CMAKE_BINARY_DIR is the path to the build directory
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
		COMMENT "Generating HTML Documentation with Doxygen"
		)
endif()
