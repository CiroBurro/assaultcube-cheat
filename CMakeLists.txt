# CMakeList.txt : CMake project for cheat, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ac_cheat")

# Add source to this project's executable.
add_executable (ac_cheat
    "main.cpp"
    "imgui_backend.cpp"
    "headers/imgui_backend.h"
    "headers/utils.h"
    "headers/process.h"
    "headers/memory.h"
    "headers/offsets.h"
    "headers/esp.h"
    
    "esp.cpp"
    )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ac_cheat PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.5
)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)

set_property(TARGET imgui PROPERTY CXX_STANDARD 20)
set_property(TARGET imgui PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends)

target_include_directories(ac_cheat PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(ac_cheat PRIVATE
    imgui
    d3d11
    dxgi
    d3dcompiler
    user32
)