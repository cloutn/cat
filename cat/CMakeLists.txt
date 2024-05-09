
cmake_minimum_required(VERSION 3.22)

set(target cat)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

project(${target})

set(header_files
	animation.h
	animationChannel.h
	camera.h
	cgltf_util.h
	def.h
	env.h
	gltf_raw_render.h
	grid.h
	IFileProvider.h
	IRender.h
	keyFrame.h
	material.h
	mesh.h
	names.h
	object.h
	objectIDMap.h
	primitive.h
	resources.h
	scene.h
	shader.h
	shader_gles.h
	shaderCache.h
	shaderMacro.h
	skin.h
	string.h
	textureFile.h
	transform.h
	uiRenderOpenGL.h
	wstring.h
	yaml.h
	win32Window.h
	eglWindow.h
	color.h
	vertex.h
	terrain.h
)

set(src_files
	cgltf.c
	cgltf_util.c
	animation.cpp
	animationChannel.cpp
	camera.cpp
	env.cpp
	gltf_raw_render.cpp
	grid.cpp
	keyFrame.cpp
	material.cpp
	mesh.cpp
	names.cpp
	object.cpp
	primitive.cpp
	resources.cpp
	scene.cpp
	shader.cpp
	shader_gles.cpp
	shaderCache.cpp
	shaderMacro.cpp
	skin.cpp
	string.cpp
	transform.cpp
	uiRenderOpenGL.cpp
	wstring.cpp
	yaml.cpp
	win32Window.cpp
	eglWindow.cpp
	color.cpp
	vertex.cpp
	terrain.cpp
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

#..\free\;..\free\gles\qualcomm\include\;..\free\vulkan\Include;..\

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../
	${PROJECT_SOURCE_DIR}/../free/gles/qualcomm/include/
	${PROJECT_SOURCE_DIR}/../free/
	${PROJECT_SOURCE_DIR}/../free/vulkan/Include/
	${PROJECT_SOURCE_DIR}/../free/rapidyaml/
	)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DOPENGL_ES -DGFX_ENABLE_IMAGE)


if(WIN32)
  #add_definitions("/wd4819") 
  add_definitions("/source-charset:utf-8")
endif(WIN32)


