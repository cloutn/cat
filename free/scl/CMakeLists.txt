
cmake_minimum_required(VERSION 3.22)

set(target scl)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(${target})

set(header_files
	allocator.h
	allocator_thread.h
	alloc_pool.h
	android.h
	array.h
	assert.h
	backtrace.h
	big_int.h
	bitdef.h
	bitset.h
	buffer.h
	directory.h
	dump.h
	encoding.h
	file.h
	file_mapping.h
	file_reader.h
	frame_lock.h
	hash_table.h
	heap.h
	heap_ptr.h
	ini_file.h
	list.h
	log.h
	log_define.h
	log_file.h
	math.h
	matrix.h
	memory_barrier.h
	page_array.h
	pair.h
	path.h
	point.h
	pool.h
	ptr.h
	quaternion.h
	rect.h
	ring_buffer.h
	ring_queue.h
	stack.h
	string.h
	stringdef.h
	task_thread.h
	task_thread_pool.h
	thread.h
	thread_allocator.h
	thread_log.h
	time.h
	tree.h
	type.h
	user_type.h
	varray.h
	vbitset.h
	vector.h
	vring_queue.h
	zip_int.h
	function.h
	plane.h
	type_traits.h)

set(src_files
	android.cpp
	assert.cpp
	backtrace.cpp
	buffer.cpp
	directory.cpp
	dump.cpp
	file.cpp
	file_mapping.cpp
	file_reader.cpp
	frame_lock.cpp
	ini_file.cpp
	log.cpp
	log_file.cpp
	math.cpp
	matrix.cpp
	path.cpp
	quaternion.cpp
	rect.cpp
	ring_buffer.cpp
	stringdef.cpp
	task_thread.cpp
	task_thread_pool.cpp
	thread.cpp
	thread_log.cpp
	time.cpp
	vbitset.cpp
	vector.cpp)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${header_files})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${src_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_library(${target} STATIC ${src_files} ${header_files})

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(ARCH 64) 
endif()

# 设置库输出目�?
set(LIB_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})

# 输出调试信息
message(STATUS "SCL Library output directory: ${LIB_OUTPUT_DIR}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# 确保输出目录存在
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_OUTPUT_DIR})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_OUTPUT_DIR})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})

# 在Windows上使用_d后缀区分Debug版本，Linux上通常不区�?
set_target_properties(${target} PROPERTIES DEBUG_POSTFIX _d)

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../../free/)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DSCL_FORCE_SLEEP)

# 平台特定定义
if(WIN32)
  #add_definitions("/wd4819") 
  add_definitions("/source-charset:utf-8")
elseif(UNIX AND NOT APPLE)
  add_definitions(-DSCL_LINUX)
elseif(APPLE)
  add_definitions(-DSCL_APPLE)
endif()

# 添加自定义命令，确保库文件被复制到正确位�?
add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    $<TARGET_FILE:${target}> ${LIB_OUTPUT_DIR}/$<TARGET_FILE_NAME:${target}>
    COMMENT "Copying ${target} to ${LIB_OUTPUT_DIR}"
)


