cmake_minimum_required(VERSION 3.15)

project(PET2
    VERSION 0.1
    DESCRIPTION "Prefix Embedded Tree 2.0"
    LANGUAGES CXX
)

# ========================
# C++ 标准
# ========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ========================
# MSVC / 非 MSVC 编译选项
# ========================
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# ========================
# 头文件路径
# ========================
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# ========================
# 源文件
# ========================
set(PET2_SOURCES
    src/pet.cpp
    "src/petnode.cpp"
    src/block.cpp
    "src/auxo.cpp"
    "tests/block_test.cpp"
)

# ========================
# 核心库
# ========================
add_library(pet2 STATIC
    ${PET2_SOURCES}
 "include/block.h" "tests/block_test.h")

target_include_directories(pet2 PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# ========================
# 可执行程序
# ========================
add_executable(pet2_main
    main.cpp
 "include/block.h" "tests/block_test.h")

target_link_libraries(pet2_main
    pet2
)

# ========================
# Debug 宏
# ========================
target_compile_definitions(pet2 PRIVATE
    $<$<CONFIG:Debug>:PET_DEBUG>
)

# ========================
# VS 输出目录（可选，整洁）
# ========================
set_target_properties(pet2_main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${PROJECT_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin
)
