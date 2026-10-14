cmake_minimum_required(VERSION 3.5)
project(PointCloudViewer LANGUAGES C CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()
 
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
  GIT_SHALLOW    TRUE)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
  GIT_SHALLOW    TRUE)
 
set(GLAD_PROFILE   "core"   CACHE STRING "" FORCE)
set(GLAD_API       "gl=3.3" CACHE STRING "" FORCE)
set(GLAD_GENERATOR "c"      CACHE STRING "" FORCE)
FetchContent_Declare(glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
  GIT_SHALLOW    TRUE)

FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.9
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  _no_cmake_here)
 
FetchContent_MakeAvailable(glfw glm imgui)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MakeAvailable(glad)
unset(CMAKE_POLICY_VERSION_MINIMUM)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

add_executable(pcviewer
  src/main.cpp
  src/ply_loader.cpp
  src/renderer.cpp
  # add later: src/ply_loader.cpp src/renderer.cpp src/camera.cpp ...
)
target_link_libraries(pcviewer PRIVATE glfw glad glm::glm imgui)
 
if(MSVC)
  target_compile_options(pcviewer PRIVATE /W4)
else()
  target_compile_options(pcviewer PRIVATE -Wall -Wextra)
endif()