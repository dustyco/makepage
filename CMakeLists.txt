

cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME makepage)

# Set version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
set(VERSION_EXTRA "")
if (VERSION_EXTRA)
	set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_EXTRA}")
else ()
	set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif ()
MESSAGE(STATUS "*** Building ${PROJECT_NAME}-${VERSION_STRING} ***")

# Generate config header:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

project(${PROJECT_NAME} CXX)


# Build in GNU profiling
# Other compiler id's: Clang, GNU, Intel, MSVC
if (PROFILE)
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		message("Building in C GNU profiling")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
	endif ()
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		message("Building in CXX GNU profiling")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
	endif ()
endif ()


# Subdirectories can create source file groups
# Usage: add_sources([group name] [source file name]...)
#        After, call propagate_groups() once in each directory
#        The variable [group name]_SRC will contain the source file paths
macro(add_sources)
	set(_arguments ${ARGN})
	list(GET _arguments 0 _group)
	list(REMOVE_AT _arguments 0)
	list(APPEND _groups ${_group})
	file(RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
	foreach(_src ${_arguments})
		if(_relPath)
			list(APPEND ${_group}_SRC "${_relPath}/${_src}")
		else()
			list(APPEND ${_group}_SRC "${_src}")
		endif()
	endforeach()
endmacro()

# Subdirectories can create external library groups
# Usage: require_libraries([group name] [library name]...)
#        After, call propagate_groups() once in each directory
#        The variable [group name]_LIB will contain the library names
macro (require_libraries)
	set(_arguments ${ARGN})
	list(GET _arguments 0 _group)
	list(REMOVE_AT _arguments 0)
	list(APPEND _groups ${_group})
	foreach(_lib ${_arguments})
		list(APPEND ${_group}_LIB "${_lib}")
	endforeach()
endmacro()

# Call this in each subdirectory AFTER all group related calls
macro(propagate_groups)
	file(RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
	if(_relPath)
		set(_groups ${_groups} PARENT_SCOPE)
		foreach(_group ${_groups})
			set(${_group}_SRC ${${_group}_SRC} PARENT_SCOPE)
			set(${_group}_LIB ${${_group}_LIB} PARENT_SCOPE)
		endforeach()
	endif()
endmacro()


# Some build type stuff #######################################################
if(CMAKE_BUILD_TYPE STREQUAL "")
	# CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
	# differentiation between debug and release builds.
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()


# Find Boost ##################################################################
#if (WIN32 OR APPLE)
#	set(Boost_USE_STATIC_LIBS TRUE)
#else ()
#	# Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit
#	set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
#endif ()
set(Boost_USE_STATIC_LIBS TRUE)

# Decide where to look
set(BOOST_ROOT_ENV $ENV{BOOST_ROOT})
if (BOOST_ROOT_ENV)
	# Get boost from a specifically defined folder
	set(BOOST_ROOT $ENV{BOOST_ROOT})
	message("BOOST_ROOT: " ${BOOST_ROOT})
elseif (WIN32)
	# Get boost from the ogre sdk
	set(BOOST_ROOT $ENV{OGRE_HOME}/boost)
	message("BOOST_ROOT: " ${BOOST_ROOT})
endif ()

# Specify which linkable libraries we should find
set(BOOST_COMPONENTS
	thread
	system
	filesystem
)

# Find it
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
if (NOT Boost_FOUND)
	# Try again with the other type of libs
	message("Boost wasn't found, trying again with the other link type")
	set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
	find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
	if (NOT Boost_FOUND)
		message(SEND_ERROR "Failed to find Boost.")
	endif ()
endif ()

# Set up referencing of Boost
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_ALL_NO_LIB)



# Work through the source tree
add_subdirectory(src)



# Game client executable
set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})
add_executable(${EXECUTABLE_NAME} ${makepage_SRC})
target_link_libraries(${EXECUTABLE_NAME} ${makepage_LIB})


install(TARGETS
	${EXECUTABLE_NAME}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)


