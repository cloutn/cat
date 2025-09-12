cmake_minimum_required(VERSION 3.22)

set(target catui)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${target})

set(header_files
	cat/imgui_impl_win32.h
	cat/imguiex.h
	cat/ImGuizmo.h
)

set(src_files
	cat/imgui_impl_win32.cpp
	cat/imguiex.cpp
	cat/ImGuizmo.cpp
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
	${PROJECT_SOURCE_DIR}/../catui/
	${PROJECT_SOURCE_DIR}/../catBase/
	${PROJECT_SOURCE_DIR}/../free/imgui/
	${PROJECT_SOURCE_DIR}/../free/
	)

add_definitions(-DOPENGL_ES -DGFX_ENABLE_IMAGE)

if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
  #add_definitions("/wd4819") 
  add_definitions("/source-charset:utf-8")
endif(WIN32)

