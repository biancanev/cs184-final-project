cmake_minimum_required(VERSION 3.10)
project(NPR_Renderer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OS-specific configurations
if(APPLE)
    # macOS specific settings
    set(GLEW_INCLUDE_DIR "/opt/homebrew/include")
    set(GLEW_LIBRARY "/opt/homebrew/lib/libGLEW.dylib")
    
    set(GLFW_INCLUDE_DIR "/opt/homebrew/include")
    set(GLFW_LIBRARY "/opt/homebrew/lib/libglfw.dylib")
    
    set(ASSIMP_INCLUDE_DIR "/opt/homebrew/include")
    set(ASSIMP_LIBRARY "/opt/homebrew/lib/libassimp.dylib")
elseif(WIN32 OR MINGW)
    # Windows specific settings using MSYS2 packages
    set(GLFW_INCLUDE_DIR "C:/msys64/mingw64/include")
    set(GLFW_LIBRARY "C:/msys64/mingw64/lib/libglfw3.dll.a")
    
    set(GLEW_INCLUDE_DIR "C:/msys64/mingw64/include")
    set(GLEW_LIBRARY "C:/msys64/mingw64/lib/libglew32.dll.a")
    
    set(ASSIMP_INCLUDE_DIR "C:/msys64/mingw64/include")
    set(ASSIMP_LIBRARY "C:/msys64/mingw64/lib/libassimp.dll.a")
    
    # GLM is header-only, already installed
    set(GLM_INCLUDE_DIR "C:/msys64/mingw64/include")
    set(GLM_INCLUDE_DIRS "C:/msys64/mingw64/include")
    set(GLM_FOUND TRUE)
else()
    # Linux will use find_package
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(assimp REQUIRED)
    find_package(glm REQUIRED)
endif()

# Find required packages for all platforms
find_package(OpenGL REQUIRED)
# Only find GLM on non-Windows platforms
if(NOT WIN32 AND NOT MINGW) 
    find_package(glm REQUIRED)
endif()

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# Platform-specific include directories
if(APPLE)
    include_directories(
        ${GLEW_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
    )
elseif(WIN32 OR MINGW)
    include_directories(
        ${GLEW_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
    )
else()
    include_directories(
        ${GLEW_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
    )
endif()

# Source files
file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})

# ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Link libraries - platform specific
if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARY}
        ${ASSIMP_LIBRARY}
        ${GLEW_LIBRARY}
        ${CMAKE_DL_LIBS}
    )
elseif(WIN32 OR MINGW)
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARY}
        ${ASSIMP_LIBRARY}
        ${GLEW_LIBRARY}
        ${CMAKE_DL_LIBS}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        glfw
        ${ASSIMP_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${CMAKE_DL_LIBS}
    )
endif()