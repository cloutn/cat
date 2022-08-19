
cmake_minimum_required(VERSION 3.22)

project(gfx)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

file(GLOB SRC_FILES  *.h *.cpp)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SRC_FILES})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_library(gfx STATIC ${SRC_FILES})

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(ARCH 64) 
endif()


set_target_properties(gfx PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})
set_target_properties(gfx PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})

#message(STATUS "config = ${CMAKE_BUILD_TYPE}")

target_include_directories(
	gfx 
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../../free 
	${PROJECT_SOURCE_DIR}/../../free/jpeg_turbo/ 
	${PROJECT_SOURCE_DIR}/../../free/gles/qualcomm/include)


add_definitions(-DUNICODE -D_UNICODE)

add_definitions(
-DOPENGL_ES
-DGFX_ENABLE_IMAGE
-DKTX_OPENGL_ES3
-DSCL_ENABLE
)

