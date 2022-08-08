
cmake_minimum_required(VERSION 3.22)

set(target catVulkanRender)

project(${target})

set(header_files
	commandAllocator.h
	descriptorAllocator.h
	imgui_impl_vulkan.h
	load_png.h
	pipelineKey.h
	simplevulkan.h
	uniformDataKey.h
	vulkanRender.h
	xxh3.h
	xxhash.h
)

set(src_files
	xxhash.c
	commandAllocator.cpp
	descriptorAllocator.cpp
	imgui_impl_vulkan.cpp
	load_png.cpp
	pipelineKey.cpp
	simplevulkan.cpp
	uniformDataKey.cpp
	vulkanRender.cpp
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${header_files})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${src_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_library(${target} STATIC ${src_files} ${header_files})

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(ARCH 64) 
endif()

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../lib${ARCH})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../lib${ARCH})
set_target_properties(${target} PROPERTIES DEBUG_POSTFIX _d)

#../;../free/;../free/imgui/;../free/vulkan/Include

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../
	${PROJECT_SOURCE_DIR}/../free/
	${PROJECT_SOURCE_DIR}/../free/imgui/
	${PROJECT_SOURCE_DIR}/../free/vulkan/Include/
	)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DGFX_ENABLE_IMAGE)



