
cmake_minimum_required(VERSION 3.22)

set(target scl)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

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
	pstring.h
	ptr.h
	pwstring.h
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
	wstring.h
	zip_int.h)

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
	pstring.cpp
	pwstring.cpp
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

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH})
set_target_properties(${target} PROPERTIES DEBUG_POSTFIX _d)

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../../free/)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DSCL_FORCE_SLEEP)


if(WIN32)
  add_definitions("/wd4819")
endif(WIN32)


