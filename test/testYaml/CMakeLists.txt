
cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set(target testYaml)

project(${target})


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64) 
	set(arch64 1)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/scl ${PROJECT_SOURCE_DIR}/../../free/scl/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../catbase ${PROJECT_SOURCE_DIR}/../../catbase/build${ARCH})
add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/rapidyaml ${PROJECT_SOURCE_DIR}/../../free/rapidyaml/build${ARCH})


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

#add_dependencies(${target} scl gfx imgui jpeg_turbo libktx libpng libtga zlib spirv_cross cat catvulkan)
add_dependencies(${target} scl catbase rapidyaml)

#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin${ARCH})
#set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin${ARCH})
if(WIN32)
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin${ARCH})
else()
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../../
	${PROJECT_SOURCE_DIR}/../../catbase/
	${PROJECT_SOURCE_DIR}/../../free/
	${PROJECT_SOURCE_DIR}/../../free/rapidyaml/
	)

# 跨平台库文件配置
if(WIN32)
	set(debug_links 	
		debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/scl_d.lib
		debug ${PROJECT_SOURCE_DIR}/../../catbase/lib${ARCH}/catbase_d.lib
		debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/rapidyaml_d.lib
	)
	
	set(release_links 	
		optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/scl.lib
		optimized ${PROJECT_SOURCE_DIR}/../../catbase/lib${ARCH}/catbase.lib
		optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/rapidyaml.lib
	)
else() # Linux/Unix systems
	set(debug_links 	
		debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libscl.a
		debug ${PROJECT_SOURCE_DIR}/../../catbase/lib${ARCH}/libcatbase_d.a
		debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/librapidyaml_d.a
	)
	
	set(release_links 	
		optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/libscl.a
		optimized ${PROJECT_SOURCE_DIR}/../../catbase/lib${ARCH}/libcatbase.a
		optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/librapidyaml.a
	)
endif()


set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})

target_link_libraries(${target} 
	${debug_links}
	${release_links})

# Linux系统库依�?
if(UNIX AND NOT APPLE)
	target_link_libraries(${target} pthread dl)
elseif(APPLE)
	target_link_libraries(${target} pthread dl)
endif()

# 平台特定设置
if(WIN32)
	#add_definitions("/wd4819")
	add_definitions("/source-charset:utf-8")
endif()

add_definitions(-DUNICODE -D_UNICODE)
#add_definitions(-DGFX_WIN32)


