cmake_minimum_required(VERSION 3.22)

set(target catbase)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${target})

set(header_files
	cat/yaml.h
	cat/def.h
	cat/string.h
	cat/IFileProvider.h
	cat/IRender.h
	cat/color.h
	cat/box.h
	cat/transform.h
	cat/vertex.h
	cat/xxh3.h
	cat/xxhash.h
)

set(src_files
	cat/yaml.cpp
	cat/color.cpp
	cat/box.cpp
	cat/transform.cpp
	cat/vertex.cpp
	cat/xxhash.c
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${header_files})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${src_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_library(${target} STATIC ${src_files} ${header_files})

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(ARCH 64) 
endif()

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/lib${ARCH})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/lib${ARCH})
set_target_properties(${target} PROPERTIES DEBUG_POSTFIX _d)

target_include_directories(
	${target}
	PRIVATE
	${PROJECT_SOURCE_DIR}/../
	${PROJECT_SOURCE_DIR}/../catbase/
	${PROJECT_SOURCE_DIR}/../free/
	${PROJECT_SOURCE_DIR}/../free/rapidyaml/
	)

add_definitions(-DOPENGL_ES -DGFX_ENABLE_IMAGE)

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
  #add_definitions("/wd4819") 
  add_definitions("/source-charset:utf-8")
endif(WIN32)

