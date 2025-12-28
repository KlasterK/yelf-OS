cmake_minimum_required(VERSION 3.20)
project(yeself LANGUAGES C CXX ASM_NASM)

set(CROSS C:/msys64/usr/local/i686-elf/bin/i686-elf)
set(CMAKE_C_COMPILER ${CROSS}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS}-g++)
set(CMAKE_ASM_NASM_COMPILER C:/msys64/mingw64/bin/nasm.exe)
set(CMAKE_LINKER ${CROSS}-ld)

set(CMAKE_CXX_FLAGS "-ffreestanding -nostdlib -fno-exceptions -fno-rtti")
set(CMAKE_ASM_NASM_FLAGS "-f elf32")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)

add_executable(yeself boot.s com.s kernel.cpp)
set_target_properties(yeself PROPERTIES
    LINK_FLAGS "-T link.ld -nostdlib"
    SUFFIX ".elf"
    WIN32_EXECUTABLE OFF
    MSVC_RUNTIME_LIBRARY ""
    LINK_LIBRARIES ""
)

# add_custom_command(TARGET yeself POST_BUILD
#     COMMAND ${CROSS}-ld
#         -T ${CMAKE_SOURCE_DIR}/link.ld
#         -nostdlib
#         --oformat elf32-i386
#         $<TARGET_OBJECTS:yeself>
#         -o ${CMAKE_BINARY_DIR}/yeself.elf
#     COMMENT "Linking kernel with custom LD command"
#     VERBATIM
# )

set_source_files_properties(boot.s com.s PROPERTIES LANGUAGE ASM_NASM)
