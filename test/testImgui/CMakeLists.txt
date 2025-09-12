
cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set(target testImgui)

project(${target})


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64) 
	set(arch64 1)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/scl ${PROJECT_SOURCE_DIR}/../../free/scl/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/imgui ${PROJECT_SOURCE_DIR}/../../free/imgui/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/libimg ${PROJECT_SOURCE_DIR}/../../free/libimg/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/spirv_cross ${PROJECT_SOURCE_DIR}/../../free/spirv_cross/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/jpeg_turbo ${PROJECT_SOURCE_DIR}/../../free/jpeg_turbo/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/libktx ${PROJECT_SOURCE_DIR}/../../free/libktx/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/libpng ${PROJECT_SOURCE_DIR}/../../free/libpng/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/libtga ${PROJECT_SOURCE_DIR}/../../free/libtga/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/zlib ${PROJECT_SOURCE_DIR}/../../free/zlib/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../catui ${PROJECT_SOURCE_DIR}/../../catui/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../catvulkan ${PROJECT_SOURCE_DIR}/../../catvulkan/build${ARCH})


set(header_files

)

set(src_files
	main.cpp
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${header_files})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${src_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_executable(${target} ${src_files} ${header_files})

add_dependencies(${target} scl imgui libimg spirv_cross jpeg_turbo libktx libpng libtga zlib catui catvulkan)

set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin${ARCH})
set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin${ARCH})

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../../
	${PROJECT_SOURCE_DIR}/../../catvulkan
	${PROJECT_SOURCE_DIR}/../../catui
	${PROJECT_SOURCE_DIR}/../../catui/cat
	${PROJECT_SOURCE_DIR}/../../free/
	${PROJECT_SOURCE_DIR}/../../free/imgui/
	${PROJECT_SOURCE_DIR}/../../free/vulkan/Include/
	)

set(debug_links 	
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/scl_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/imgui_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libimg_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/spirv_cross_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/jpeg_turbo_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libktx_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libpng_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libtga_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/zlib_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../catui/lib${ARCH}/catui_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../lib${ARCH}/catvulkan_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/vulkan/Lib${ARCH}/vulkan-1.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/shaderc_combined_d.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/gles/qualcomm/lib${ARCH}/libEGL.lib
	debug ${PROJECT_SOURCE_DIR}/../../free/gles/qualcomm/lib${ARCH}/libGLESv2.lib
	debug opengl32.lib
)

set(release_links 	
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/scl.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/imgui.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libimg.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/spirv_cross.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/jpeg_turbo.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libktx.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libpng.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libtga.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/zlib.lib
	optimized ${PROJECT_SOURCE_DIR}/../../catui/lib${ARCH}/catui.lib
	optimized ${PROJECT_SOURCE_DIR}/../../lib${ARCH}/catvulkan.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/vulkan/Lib${ARCH}/vulkan-1.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/shaderc_combined.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/gles/qualcomm/lib${ARCH}/libEGL.lib
	optimized ${PROJECT_SOURCE_DIR}/../../free/gles/qualcomm/lib${ARCH}/libGLESv2.lib
	optimized opengl32.lib
)


set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})

target_link_libraries(${target} 
	${debug_links}
	${release_links})


# 平台特定设置
if(WIN32)
	#add_definitions("/wd4819")
	add_definitions("/source-charset:utf-8")
endif()

add_definitions(-DUNICODE -D_UNICODE)


