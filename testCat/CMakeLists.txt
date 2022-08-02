
cmake_minimum_required(VERSION 3.22)

set(target testCat)

project(${target})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(arch64 1)
endif()
  #else()
  #  set(arch64 False)
  #endif()
if (arch64)
	message(STATUS "is arch64")
else()
	message(STATUS "is arch32")
endif()

#message(STATUS "cmake build type = ${platform} , ${CMAKE_CL_64}")

set(header_files
	client.h
	imgui_impl_win32.h
)

set(src_files
	client.cpp
	imgui_impl_win32.cpp
	main.cpp
)

#set(link_dir_debug
#	${PROJECT_SOURCE_DIR}/../lib64
#	${PROJECT_SOURCE_DIR}/../free/lib64
#	${PROJECT_SOURCE_DIR}/../free/gles/qualcomm/lib64
#	${PROJECT_SOURCE_DIR}/../free/vld_runtime/lib/Win64/
#	${PROJECT_SOURCE_DIR}/../free/vulkan/Lib
#)
#
#link_directories(
#	$<$<CONFIG:Debug>:${link_dir_debug}>
#	$<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/../free>
#	)

add_executable(${target} ${src_files} ${header_files})

if (arch64)
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin64)
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin64)
else()
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
endif()

#..\;..\free;..\free\imgui;..\free\gles\qualcomm\include;..\free\vulkan\Include

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../
	${PROJECT_SOURCE_DIR}/../free/
	${PROJECT_SOURCE_DIR}/../free/imgui/
	${PROJECT_SOURCE_DIR}/../free/gles/qualcomm/include/
	${PROJECT_SOURCE_DIR}/../free/vulkan/Include/
	)

set(debug_links 	
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/scl_d.lib
	debug ${PROJECT_SOURCE_DIR}/../lib64/cat_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/gfx_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/imgui_d.lib
	debug ${PROJECT_SOURCE_DIR}/../lib64/catVulkanRender_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/vulkan/Lib/vulkan-1.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/shaderc_combined.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/spirv_cross_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/gles/qualcomm/lib64/libEGL.lib
	debug ${PROJECT_SOURCE_DIR}/../free/gles/qualcomm/lib64/libGLESv2.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/jpeg_turbo_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/libktx_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/libpng_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/libtga_d.lib
	debug ${PROJECT_SOURCE_DIR}/../free/lib64/zlib_d.lib
)

set(release_links 	
	optimized scl.lib
	optimized cat.lib
	optimized gfx.lib
	optimized imgui.lib
	optimized catVulkanRender.lib
	optimized vulkan-1.lib
	optimized shaderc_combined.lib
	optimized spirv_cross.lib
	optimized libEGL.lib
	optimized libGLESv2.lib
	optimized jpeg_turbo.lib
	optimized libktx.lib
	optimized libpng.lib
	optimized libtga.lib
	optimized zlib.lib)


#target_link_libraries(${target} 
#	PUBLIC
#	$<$<CONFIG:Debug>:"${debug_links}">
#	$<$<CONFIG:Release>:"${release_links}">
#)

#$set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/mybin")
#set_property(TARGET ${target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})

target_link_libraries(${target} 
	${debug_links}
	${release_links})
#target_link_libraries(${target} )

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DGFX_WIN32)


