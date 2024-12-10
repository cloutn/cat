
cmake_minimum_required(VERSION 3.22)

set(target rapidyaml)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

project(${target})

set(src_files
        ryml.hpp
        ryml_std.hpp
        c4/yml/detail/checks.hpp
        c4/yml/detail/parser_dbg.hpp
        c4/yml/detail/print.hpp
        c4/yml/detail/stack.hpp
        c4/yml/common.hpp
        c4/yml/common.cpp
        c4/yml/emit.def.hpp
        c4/yml/emit.hpp
        c4/yml/event_handler_stack.hpp
        c4/yml/event_handler_tree.hpp
        c4/yml/filter_processor.hpp
        c4/yml/fwd.hpp
        c4/yml/export.hpp
        c4/yml/node.hpp
        c4/yml/node.cpp
        c4/yml/node_type.hpp
        c4/yml/node_type.cpp
        c4/yml/parser_state.hpp
        c4/yml/parse.hpp
        c4/yml/parse.cpp
        c4/yml/parse_engine.hpp
        c4/yml/parse_engine.def.hpp
        c4/yml/preprocess.hpp
        c4/yml/preprocess.cpp
        c4/yml/reference_resolver.hpp
        c4/yml/reference_resolver.cpp
        c4/yml/std/map.hpp
        c4/yml/std/std.hpp
        c4/yml/std/string.hpp
        c4/yml/std/vector.hpp
        c4/yml/tag.hpp
        c4/yml/tag.cpp
        c4/yml/tree.hpp
        c4/yml/tree.cpp
        c4/yml/version.hpp
        c4/yml/version.cpp
        c4/yml/writer.hpp
        c4/yml/yml.hpp
        ryml.natvis
    c4/allocator.hpp
    c4/base64.hpp
    c4/base64.cpp
    c4/blob.hpp
    c4/bitmask.hpp
    c4/charconv.hpp
    c4/c4_pop.hpp
    c4/c4_push.hpp
    c4/char_traits.cpp
    c4/char_traits.hpp
    c4/common.hpp
    c4/compiler.hpp
    c4/config.hpp
    c4/cpu.hpp
    c4/ctor_dtor.hpp
    c4/dump.hpp
    c4/enum.hpp
    c4/error.cpp
    c4/error.hpp
    c4/export.hpp
    c4/format.hpp
    c4/format.cpp
    c4/gcc-4.8.hpp
    c4/hash.hpp
    c4/language.hpp
    c4/language.cpp
    c4/memory_resource.cpp
    c4/memory_resource.hpp
    c4/memory_util.cpp
    c4/memory_util.hpp
    c4/platform.hpp
    c4/preprocessor.hpp
    c4/restrict.hpp
    c4/span.hpp
    c4/std/std.hpp
    c4/std/std_fwd.hpp
    c4/std/string.hpp
    c4/std/string_fwd.hpp
    c4/std/string_view.hpp
    c4/std/tuple.hpp
    c4/std/vector.hpp
    c4/std/vector_fwd.hpp
    c4/substr.hpp
    c4/substr_fwd.hpp
    c4/szconv.hpp
    c4/type_name.hpp
    c4/types.hpp
    c4/unrestrict.hpp
    c4/utf.hpp
    c4/utf.cpp
    c4/windows.hpp
    c4/windows_pop.hpp
    c4/windows_push.hpp
    c4/c4core.natvis
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${src_files})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")

add_library(${target} STATIC ${src_files})

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(ARCH 64) 
endif()

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../lib${ARCH})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../lib${ARCH})
set_target_properties(${target} PROPERTIES DEBUG_POSTFIX _d)

target_include_directories(
	${target}
	PRIVATE 
	${PROJECT_SOURCE_DIR})

add_definitions(-DUNICODE -D_UNICODE)

if(WIN32)
  #add_definitions("/wd4819") 
  add_definitions("/source-charset:utf-8")
endif(WIN32)

