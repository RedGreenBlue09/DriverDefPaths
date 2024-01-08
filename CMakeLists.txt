cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0069 NEW) 
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

macro(set_build_dir_default TARGET_NAME_)
	set_target_properties(${TARGET_NAME_}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${TARGET_NAME_}/Binary"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${TARGET_NAME_}/Library"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${TARGET_NAME_}/Binary"
	)
endmacro()

project(DriverDefPaths LANGUAGES C)

set(EXPAT_BUILD_DOCS OFF)
set(EXPAT_BUILD_EXAMPLES OFF)
set(EXPAT_BUILD_TESTS OFF)
set(EXPAT_BUILD_TOOLS OFF)
set(EXPAT_SHARED_LIBS OFF)
add_subdirectory("${DriverDefPaths_SOURCE_DIR}/Library/libexpat/expat")
set_target_properties(expat PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
set_build_dir_default(expat)

set(HEADER_FILES
)
set(SOURCE_FILES
	"${DriverDefPaths_SOURCE_DIR}/Source/Main.c"
)
add_executable(DriverDefPaths
	${HEADER_FILES}
	${SOURCE_FILES}
)
target_include_directories(DriverDefPaths PRIVATE
	${EXPAT_INCLUDE_DIRS}
	${DriverDefPaths_SOURCE_DIR}/Include
)
target_link_libraries(DriverDefPaths PUBLIC expat)
set_target_properties(DriverDefPaths PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
set_build_dir_default(DriverDefPaths)