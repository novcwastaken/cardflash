cmake_minimum_required(VERSION 3.16)
project(Cardflash CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR external/imgui)
add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

file(GLOB_RECURSE CARDFLASH_SOURCES CONFIGURE_DEPENDS src/*.cc)
add_executable(Cardflash ${CARDFLASH_SOURCES})
target_include_directories(Cardflash PRIVATE src)
target_link_libraries(Cardflash PRIVATE imgui)
