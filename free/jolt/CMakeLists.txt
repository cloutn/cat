
cmake_minimum_required(VERSION 3.22)

set(target jolt)
project(${target})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------
# Jolt 编译选项：锁定 cat 期望的 ABI
# 任何选项变化都会改变 ABI，必须和 testCat 链接侧严格一致。
# ----------------------------------------------------------------------

# MSVC 运行时库必须用 DLL 版（/MD），与 cat 其他库一致
# Jolt 默认 USE_STATIC_MSVC_RUNTIME_LIBRARY=ON 会用 /MT，会导致 LNK2005
set(USE_STATIC_MSVC_RUNTIME_LIBRARY      OFF CACHE BOOL ""   FORCE)

# 精度 / 异常 / RTTI / 共享库
set(DOUBLE_PRECISION                     OFF CACHE BOOL ""   FORCE)
set(CPP_EXCEPTIONS_ENABLED               OFF CACHE BOOL ""   FORCE)
set(CPP_RTTI_ENABLED                     OFF CACHE BOOL ""   FORCE)
set(BUILD_SHARED_LIBS                    OFF CACHE BOOL ""   FORCE)

# 调试与断言
set(USE_ASSERTS                          ON  CACHE BOOL ""   FORCE)
set(GENERATE_DEBUG_SYMBOLS               ON  CACHE BOOL ""   FORCE)
set(FLOATING_POINT_EXCEPTIONS_ENABLED    OFF CACHE BOOL ""   FORCE)

# Layer 位宽
set(OBJECT_LAYER_BITS                    32  CACHE STRING "" FORCE)

# SIMD（PC x64 主线）
set(USE_SSE4_1                           ON  CACHE BOOL ""   FORCE)
set(USE_SSE4_2                           ON  CACHE BOOL ""   FORCE)
set(USE_AVX                              ON  CACHE BOOL ""   FORCE)
set(USE_AVX2                             ON  CACHE BOOL ""   FORCE)
set(USE_AVX512                           OFF CACHE BOOL ""   FORCE)
set(USE_LZCNT                            ON  CACHE BOOL ""   FORCE)
set(USE_TZCNT                            ON  CACHE BOOL ""   FORCE)
set(USE_F16C                             ON  CACHE BOOL ""   FORCE)
set(USE_FMADD                            ON  CACHE BOOL ""   FORCE)

# DebugRenderer / Profiler
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE  ON  CACHE BOOL ""   FORCE)
set(DEBUG_RENDERER_IN_DISTRIBUTION       OFF CACHE BOOL ""   FORCE)
set(PROFILER_IN_DEBUG_AND_RELEASE        OFF CACHE BOOL ""   FORCE)
set(PROFILER_IN_DISTRIBUTION             OFF CACHE BOOL ""   FORCE)

# 容器 / 序列化
set(USE_STD_VECTOR                       OFF CACHE BOOL ""   FORCE)
set(ENABLE_OBJECT_STREAM                 ON  CACHE BOOL ""   FORCE)
set(DISABLE_CUSTOM_ALLOCATOR             OFF CACHE BOOL ""   FORCE)

# 编译质量与确定性
set(OVERRIDE_CXX_FLAGS                   OFF CACHE BOOL ""   FORCE)
set(ENABLE_ALL_WARNINGS                  OFF CACHE BOOL ""   FORCE)
set(CROSS_PLATFORM_DETERMINISTIC         OFF CACHE BOOL ""   FORCE)
set(INTERPROCEDURAL_OPTIMIZATION         OFF CACHE BOOL ""   FORCE)

# 安装与统计：不需要
set(ENABLE_INSTALL                       OFF CACHE BOOL ""   FORCE)
set(TRACK_BROADPHASE_STATS               OFF CACHE BOOL ""   FORCE)
set(TRACK_NARROWPHASE_STATS              OFF CACHE BOOL ""   FORCE)

# ----------------------------------------------------------------------
# 让上游 Build/CMakeLists.txt 接管 600+ 文件清单
# 上游 Jolt/Jolt.cmake 中创建的 target 名是 "Jolt"（大写）
# ----------------------------------------------------------------------
set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Build)

# ----------------------------------------------------------------------
# 把上游 Jolt target 输出对齐到 cat 风格
#   - 文件名：jolt.lib / jolt_d.lib（OUTPUT_NAME=jolt + DEBUG_POSTFIX=_d）
#   - 输出目录：free/lib64/
# ----------------------------------------------------------------------
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH 64)
endif()

set_target_properties(Jolt PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/../lib${ARCH}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../lib${ARCH}
    OUTPUT_NAME                      jolt
    DEBUG_POSTFIX                    _d
)

# 让 cat 上层只用 #include <Jolt/Physics/PhysicsSystem.h> 风格
target_include_directories(Jolt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DUNICODE -D_UNICODE)
