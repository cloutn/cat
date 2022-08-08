
cmake_minimum_required(VERSION 3.22)

set(target cat)

project(${target})

set(header_files
	animation.h
	animationChannel.h
	bufferRefCounter.h
	camera.h
	cgltf_util.h
	def.h
	env.h
	gltf_raw_render.h
	gltf_type.h
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
	shaderCache.h
	shaderMacro.h
	skin.h
	string.h
	textureFile.h
	transform.h
	typedef.h
	uiRenderOpenGL.h
	wstring.h)

set(src_files
	cgltf.c
	cgltf_util.c
	animation.cpp
	animationChannel.cpp
	bufferRefCounter.cpp
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
	shaderCache.cpp
	skin.cpp
	string.cpp
	transform.cpp
	uiRenderOpenGL.cpp
	wstring.cpp)

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
	)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DOPENGL_ES -DGFX_ENABLE_IMAGE)


