cmake_minimum_required(VERSION 3.28)

project(
    language

    VERSION 0.0.1
    LANGUAGES C
    HOMEPAGE_URL "https://github.com/cally72jhb/"
    DESCRIPTION "language - an embedded programming language I am working on"
)

set(CPACK_PACKAGE_NAME "language")
set(CPACK_PACKAGE_VENDOR "cally72jhb")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "language - an embedded programming language I am working on")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "language")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

###########################
## Features (TRUE/FALSE) ##
###########################

set(PLATFORM_WIN64 TRUE)
set(PLATFORM_UNIX FALSE)

##################
## Source Files ##
##################

link_directories(${PROJECT_SOURCE_DIR}/platform)
link_directories(${PROJECT_SOURCE_DIR}/src)

file(
    GLOB_RECURSE
    SRC_FILES

    # platform

    ${PROJECT_SOURCE_DIR}/platform/platform.h

    # headers (for ide clarity)

    src/*.h

    # src

        src/main.c

        # src/common

        src/common/debug.c
        src/common/error_codes.c

            # common/data/color

            src/common/data/color/color.c
            src/common/data/color/color_registry.c

            # common/data/string

            src/common/data/string/hash.c
            src/common/data/string/string.c

            # src/common/math

            src/common/bit_utils.c

                # common/math/mat

                src/common/math/mat/mat4.c

                # common/math/primitives

                src/common/math/primitives/f32_math.c
                src/common/math/primitives/f64_math.c
                src/common/math/primitives/iint_math.c
                src/common/math/primitives/uint_math.c

                # common/math/vec

                src/common/math/vec/vec2d.c
                src/common/math/vec/vec2f.c
                src/common/math/vec/vec2i.c
                src/common/math/vec/vec3d.c
                src/common/math/vec/vec3f.c
                src/common/math/vec/vec3i.c
                src/common/math/vec/vec4f.c

            #src/common/memory

            src/common/memory/heap_manager.c
            src/common/memory/memory.c

        #src/language

        src/language/wave_common.c
        src/language/wave_opcodes.c

            #src/language/compiler

            src/language/compiler/compiler.c
            src/language/compiler/disassembler.c
            src/language/compiler/optimizer.c
            src/language/compiler/parser.c
            src/language/compiler/tokenizer.c
            src/language/compiler/type_resolver.c

                #src/language/compiler/data

                src/language/compiler/data/wave_ast.c
                src/language/compiler/data/wave_precedence.c
                src/language/compiler/data/wave_type.c

            #src/language/func

            src/language/func/wave_debug.c
            src/language/func/wave_math.c

            #src/language/runtime

            src/language/runtime/wave_vm.c
            src/language/runtime/wave_vm_container.c
)

################
## Executable ##
################

if (PLATFORM_WIN64)
    add_executable(
        language
        WIN32

        ${PROJECT_SOURCE_DIR}/platform/win64console.c
        ${SRC_FILES}
    )

    target_link_libraries(language PUBLIC Dwmapi.dll)
    target_link_libraries(language PUBLIC Uxtheme.dll)

    target_compile_definitions(language PUBLIC PROGRAM_PLATFORM_WIN64)
elseif (PLATFORM_UNIX)
    add_executable(
        language

        ${PROJECT_SOURCE_DIR}/src/platform/unix.c
        ${SRC_FILES}
    )

    target_compile_definitions(language PUBLIC PROGRAM_PLATFORM_UNIX)
endif()

target_include_directories(language PRIVATE ${PROJECT_SOURCE_DIR}/platform)
target_include_directories(language PRIVATE ${PROJECT_SOURCE_DIR}/src)

######################
## Standard Library ##
######################

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static") # statically link stdlib

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
#list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES stdc)

# For File Path Macro

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_LENGTH)
target_compile_definitions(language PUBLIC SOURCE_PATH_LENGTH=${SOURCE_PATH_LENGTH})

###############
## Libraries ##
###############

# hashing : https://github.com/Nicoshev/rapidhash/

target_include_directories(language PRIVATE ${PROJECT_SOURCE_DIR}/include/hash)
