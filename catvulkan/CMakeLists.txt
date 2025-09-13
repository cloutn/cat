
cmake_minimum_required(VERSION 3.22)

set(target catvulkan)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${target})

set(header_files
	cat/commandAllocator.h
	cat/descriptorAllocator.h
	cat/imgui_impl_vulkan.h
	cat/load_png.h
	cat/pipelineKey.h
	cat/simplevulkan.h
	cat/descriptorDataKey.h
	cat/vulkanRender.h
	cat/deviceInfo.h
)

set(src_files
	cat/commandAllocator.cpp
	cat/descriptorAllocator.cpp
	cat/imgui_impl_vulkan.cpp
	cat/load_png.cpp
	cat/pipelineKey.cpp
	cat/simplevulkan.cpp
	cat/descriptorDataKey.cpp
	cat/vulkanRender.cpp
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

#../;../free/;../free/imgui/;../free/vulkan/Include

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/../
	${PROJECT_SOURCE_DIR}/../catbase/
	${PROJECT_SOURCE_DIR}/../free/
	${PROJECT_SOURCE_DIR}/../free/imgui/
	${PROJECT_SOURCE_DIR}/../free/vulkan/Include/
	)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DGFX_ENABLE_IMAGE)

if(WIN32)
  #add_definitions("/wd4819") 
  add_definitions("/source-charset:utf-8")
endif(WIN32)


