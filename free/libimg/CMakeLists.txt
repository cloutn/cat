
cmake_minimum_required(VERSION 3.22)

set(target libimg)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

project(${target})

set(src_files
	image.cpp
	)

set(header_files
	image.h
	base.h
	)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${src_files})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${header_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_library(${target} STATIC ${src_files} ${header_files})

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(ARCH 64) 
endif()

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})
set_target_properties(${target} PROPERTIES DEBUG_POSTFIX _d)

#message(STATUS "config = ${CMAKE_BUILD_TYPE}")
target_include_directories(
	${target}
	PRIVATE
	${PROJECT_SOURCE_DIR}/../../free/
	${PROJECT_SOURCE_DIR}/../../free/jpeg_turbo/ 
	${PROJECT_SOURCE_DIR}/../../free/gles/qualcomm/include
)

add_definitions(-DUNICODE -D_UNICODE)

add_definitions(
-DOPENGL_ES
-DGFX_ENABLE_IMAGE
-DKTX_OPENGL_ES3
#-DSCL_ENABLE
)

if(WIN32)
  add_definitions("/wd4819")
endif(WIN32)


