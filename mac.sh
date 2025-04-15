#!/bin/bash

# Exit on any error
set -e

echo "====================================================="
echo "NPR_Renderer macOS Dependency Installer (GLEW Fix)"
echo "====================================================="

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
  echo "Homebrew not found. Installing Homebrew..."
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  
  # Add Homebrew to PATH
  if [ -f ~/.zshrc ]; then
    echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zshrc
    eval "$(/opt/homebrew/bin/brew shellenv)"
  elif [ -f ~/.bash_profile ]; then
    echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.bash_profile
    eval "$(/opt/homebrew/bin/brew shellenv)"
  fi
  
  echo "Homebrew installed and configured."
else
  echo "Homebrew is already installed."
fi

# Check if we're on Apple Silicon or Intel Mac
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
  echo "Detected Apple Silicon (M1/M2) Mac"
  HOMEBREW_PREFIX="/opt/homebrew"
else
  echo "Detected Intel Mac"
  HOMEBREW_PREFIX="/usr/local"
fi

# Install system dependencies
echo "Installing system dependencies..."
brew update

# Install essential build tools
brew install cmake

# Install GLFW
brew install glfw

# Install GLEW (this is where the issue is)
brew install glew

# Install GLM
brew install glm

# Install Assimp
brew install assimp

echo "Dependencies installed with Homebrew."

# Fix GLEW include path issue
echo "Checking and fixing GLEW include path..."

# Check if the GLEW header exists in expected location
if [ ! -f "${HOMEBREW_PREFIX}/include/GL/glew.h" ]; then
  echo "GLEW header not found in ${HOMEBREW_PREFIX}/include/GL/"
  
  # Check where GLEW headers actually are
  GLEW_HEADER_PATH=$(find ${HOMEBREW_PREFIX}/Cellar/glew -name "glew.h" | head -n 1)
  
  if [ -n "$GLEW_HEADER_PATH" ]; then
    echo "Found GLEW header at: $GLEW_HEADER_PATH"
    
    # Create the GL directory if it doesn't exist
    mkdir -p "${HOMEBREW_PREFIX}/include/GL"
    
    # Create symbolic link to the header
    echo "Creating symbolic link from $GLEW_HEADER_PATH to ${HOMEBREW_PREFIX}/include/GL/glew.h"
    ln -sf "$GLEW_HEADER_PATH" "${HOMEBREW_PREFIX}/include/GL/glew.h"
    
    echo "GLEW include path fixed!"
  else
    echo "Could not find GLEW header in Homebrew Cellar. This is unusual."
    echo "Attempting to reinstall GLEW..."
    brew reinstall glew
    
    # Check again after reinstall
    GLEW_HEADER_PATH=$(find ${HOMEBREW_PREFIX}/Cellar/glew -name "glew.h" | head -n 1)
    
    if [ -n "$GLEW_HEADER_PATH" ]; then
      echo "Found GLEW header at: $GLEW_HEADER_PATH after reinstall"
      
      # Create the GL directory if it doesn't exist
      mkdir -p "${HOMEBREW_PREFIX}/include/GL"
      
      # Create symbolic link to the header
      echo "Creating symbolic link from $GLEW_HEADER_PATH to ${HOMEBREW_PREFIX}/include/GL/glew.h"
      ln -sf "$GLEW_HEADER_PATH" "${HOMEBREW_PREFIX}/include/GL/glew.h"
      
      echo "GLEW include path fixed!"
    else
      echo "ERROR: Still could not find GLEW header after reinstall."
    fi
  fi
else
  echo "GLEW header already exists at ${HOMEBREW_PREFIX}/include/GL/glew.h"
fi

# Set up ImGui docking branch
echo "Setting up ImGui docking branch..."

# Set up directories
PROJECT_DIR=$(pwd)
EXTERNAL_DIR="${PROJECT_DIR}/external"
IMGUI_DIR="${EXTERNAL_DIR}/imgui"

echo "Project directory: ${PROJECT_DIR}"

# Create external directory if it doesn't exist
mkdir -p "${EXTERNAL_DIR}"
echo "Created external directory: ${EXTERNAL_DIR}"

# Remove existing ImGui if it exists
if [ -d "${IMGUI_DIR}" ]; then
  echo "Found existing ImGui directory. Removing..."
  
  # Check if it's a Git repository
  if [ -d "${IMGUI_DIR}/.git" ]; then
    # Check if it's a submodule
    if grep -q "${IMGUI_DIR}" "${PROJECT_DIR}/.gitmodules" 2>/dev/null; then
      echo "ImGui is a Git submodule. Removing properly..."
      cd "${PROJECT_DIR}"
      git submodule deinit -f external/imgui || true
      git rm -f external/imgui || true
      rm -rf ".git/modules/external/imgui" || true
    else
      echo "ImGui is a regular Git repository (not a submodule). Removing..."
      rm -rf "${IMGUI_DIR}"
    fi
  else
    echo "ImGui is a regular directory. Removing..."
    rm -rf "${IMGUI_DIR}"
  fi
  
  echo "Removed existing ImGui directory."
fi

# Clone ImGui docking branch
echo "Cloning ImGui docking branch..."
cd "${PROJECT_DIR}"

# Check if this is a git repository
if [ ! -d .git ]; then
  echo "Not a git repository. Initializing git repository..."
  git init
fi

# Try to add as submodule, if that fails use regular clone
git submodule add -b docking https://github.com/ocornut/imgui.git "${IMGUI_DIR}" || {
  echo "Failed to add submodule. Trying with regular git clone instead..."
  rm -rf "${IMGUI_DIR}"
  git clone -b docking https://github.com/ocornut/imgui.git "${IMGUI_DIR}"
}

# Create modified CMakeLists.txt for macOS
echo "Creating macOS-specific CMakeLists.txt example..."

cat > "${PROJECT_DIR}/macos_cmake_example.txt" << 'EOL'
cmake_minimum_required(VERSION 3.10)
project(NPR_Renderer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS specific settings
if(APPLE)
    # Silence deprecated warnings for OpenGL on macOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")
    
    # Check if we're on Apple Silicon
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        message(STATUS "Configuring for Apple Silicon (M1/M2)")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        message(STATUS "Configuring for Intel Mac")
        set(HOMEBREW_PREFIX "/usr/local")
    endif()
    
    # Add Homebrew include paths explicitly
    include_directories(
        ${HOMEBREW_PREFIX}/include
        ${HOMEBREW_PREFIX}/include/GL
    )
    
    # Add Homebrew lib paths explicitly
    link_directories(
        ${HOMEBREW_PREFIX}/lib
    )
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
# Modified for macOS - don't rely on pkg-config for GLEW
if(APPLE)
    # On macOS, set GLEW variables manually
    set(GLEW_INCLUDE_DIRS ${HOMEBREW_PREFIX}/include)
    set(GLEW_LIBRARIES ${HOMEBREW_PREFIX}/lib/libGLEW.dylib)
else()
    find_package(GLEW REQUIRED)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# Source files
file(GLOB SOURCES "src/*.cpp" "src/*.c")

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

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
    ${ASSIMP_LIBRARIES}
)

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
EOL

echo "====================================================="
echo "Installation complete!"
echo ""
echo "IMPORTANT: A macOS-specific CMakeLists.txt example has been created at:"
echo "${PROJECT_DIR}/macos_cmake_example.txt"
echo ""
echo "If you continue to have issues with GLEW include paths, please:"
echo "1. Replace your current CMakeLists.txt with this example"
echo "2. Add the following to your source files that include GLEW:"
echo ""
echo "#ifdef __APPLE__"
echo "#include <OpenGL/gl.h>"
echo "#include <OpenGL/glu.h>"
echo "#include <GLEW/glew.h>  // or try just <glew.h>"
echo "#else"
echo "#include <GL/glew.h>"
echo "#endif"
echo ""
echo "Build with: mkdir -p build && cd build && cmake .. && make"
echo "====================================================="

exit 0