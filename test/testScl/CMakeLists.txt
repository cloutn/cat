
cmake_minimum_required(VERSION 3.22)


set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set(target testScl)

project(${target})


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH 64) 
	set(arch64 1)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/scl ${PROJECT_SOURCE_DIR}/../../free/scl/build${ARCH})
#add_subdirectory(${PROJECT_SOURCE_DIR}/../../cat ${PROJECT_SOURCE_DIR}/../../cat/build${ARCH})
#add_subdirectory(${PROJECT_SOURCE_DIR}/../../free/rapidyaml ${PROJECT_SOURCE_DIR}/../../free/rapidyaml/build${ARCH})


set(header_files
	testArray.h
	testBigInt.h
	testBitset.h
	testFile.h
	testHashTable.h
	testHeap.h
	testIniFile.h
	testList.h
	testLog.h
	testMemory.h
	testPath.h
	testPString.h
	testPWString.h
	testRingBuffer.h
	testRingQueue.h
	testStack.h
	testString.h
	testStringFunction.h
	testThread.h
	testTime.h
	testTree.h
	testWString.h
	testTypeTraits.h
)

set(src_files
	main.cpp
	testArray.cpp
	testBigInt.cpp
	testBitset.cpp
	testFile.cpp
	testHashTable.cpp
	testHeap.cpp
	testIniFile.cpp
	testList.cpp
	testLog.cpp
	testMemory.cpp
	testPath.cpp
	testPString.cpp
	testPWString.cpp
	testRingBuffer.cpp
	testRingQueue.cpp
	testStack.cpp
	testString.cpp
	testStringFunction.cpp
	testThread.cpp
	testTime.cpp
	testTree.cpp
	testWString.cpp
	testTypeTraits.cpp
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${header_files})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${src_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_executable(${target} ${src_files} ${header_files})

#add_dependencies(${target} scl gfx imgui jpeg_turbo libktx libpng libtga zlib spirv_cross cat catVulkanRender)
add_dependencies(${target} scl)

set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin${ARCH})
set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin${ARCH})

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR}/../../
	${PROJECT_SOURCE_DIR}/../../free/
	#${PROJECT_SOURCE_DIR}/../../free/rapidyaml/
	)

set(debug_links 	
	debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/scl_d.lib
	#debug ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/rapidyaml_d.lib
)

set(release_links 	
	optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/scl.lib
	#optimized ${PROJECT_SOURCE_DIR}/../../lib${ARCH}/cat.lib
	#optimized ${PROJECT_SOURCE_DIR}/../../free/lib${ARCH}/rapidyaml.lib
)


set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})

target_link_libraries(${target} 
	${debug_links}
	${release_links})

if(WIN32)
	#add_definitions("/wd4819")
	add_definitions("/source-charset:utf-8")
endif(WIN32)

add_definitions(-DUNICODE -D_UNICODE)
#add_definitions(-DGFX_WIN32)


