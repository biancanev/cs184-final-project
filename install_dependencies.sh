#!/bin/bash

# Exit on any error
set -e

echo "====================================================="
echo "NPR_Renderer Dependency and ImGui Installer"
echo "====================================================="

# Check if running as root or with sudo
if [ "$EUID" -eq 0 ]; then
  echo "Please do not run this script as root or with sudo."
  echo "The script will ask for privileges when needed."
  exit 1
fi

# Function to install system dependencies
install_system_dependencies() {
  echo "Installing system dependencies..."
  
  # Check which package manager to use
  if [ -f /etc/debian_version ]; then
    echo "Detected Debian/Ubuntu-based system"
    
    # Update package lists
    sudo apt-get update
    
    # Install essential build tools
    sudo apt-get install -y build-essential cmake git
    
    # Install GLFW dependencies
    sudo apt-get install -y libglfw3 libglfw3-dev
    
    # Install GLEW
    sudo apt-get install -y libglew-dev
    
    # Install GLM
    sudo apt-get install -y libglm-dev
    
    # Install Assimp
    sudo apt-get install -y libassimp-dev
    
    # Install ImGui dependencies (X11, GL)
    sudo apt-get install -y libx11-dev libxrandr-dev libxinerama-dev \
                            libxcursor-dev libxi-dev
    
  # Check if running on a Red Hat/Fedora based system
  elif [ -f /etc/redhat-release ]; then
    echo "Detected Red Hat/Fedora-based system"
    
    # Install essential build tools
    sudo dnf install -y gcc gcc-c++ cmake git
    
    # Install GLFW dependencies
    sudo dnf install -y glfw glfw-devel
    
    # Install GLEW
    sudo dnf install -y glew glew-devel
    
    # Install GLM
    sudo dnf install -y glm-devel
    
    # Install Assimp
    sudo dnf install -y assimp assimp-devel
    
    # Install ImGui dependencies
    sudo dnf install -y libX11-devel libXrandr-devel libXinerama-devel \
                        libXcursor-devel libXi-devel mesa-libGL-devel
    
  # Check if running on Arch Linux
  elif [ -f /etc/arch-release ]; then
    echo "Detected Arch Linux-based system"
    
    # Install essential build tools
    sudo pacman -S --needed base-devel cmake git
    
    # Install GLFW
    sudo pacman -S --needed glfw-x11
    
    # Install GLEW
    sudo pacman -S --needed glew
    
    # Install GLM
    sudo pacman -S --needed glm
    
    # Install Assimp
    sudo pacman -S --needed assimp

  # Check if running on macOS (using Homebrew)
  elif [ "$(uname)" == "Darwin" ]; then
    echo "Detected macOS"
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
      echo "Homebrew not found. Installing Homebrew..."
      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    # Install essential build tools
    brew install cmake
    
    # Install GLFW
    brew install glfw
    
    # Install GLEW
    brew install glew
    
    # Install GLM
    brew install glm
    
    # Install Assimp
    brew install assimp
    
  else
    echo "Unsupported operating system. Please install dependencies manually."
    exit 1
  fi
  
  echo "System dependencies installed successfully!"
}

# Function to install ImGui docking branch
install_imgui_docking() {
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
  
  # Clone ImGui docking branch as a submodule
  echo "Cloning ImGui docking branch as a Git submodule..."
  cd "${PROJECT_DIR}"
  
  # Check if this is a git repository
  if [ ! -d .git ]; then
    echo "Not a git repository. Initializing git repository..."
    git init
  fi
  
  # Add the submodule
  git submodule add -b docking https://github.com/ocornut/imgui.git "${IMGUI_DIR}" || {
    echo "Failed to add submodule. Trying with regular git clone instead..."
    rm -rf "${IMGUI_DIR}"
    git clone -b docking https://github.com/ocornut/imgui.git "${IMGUI_DIR}"
  }
  
  if [ -d "${IMGUI_DIR}" ]; then
    echo "Successfully installed ImGui docking branch!"
    echo "Verifying installation..."
    
    # Verify that the docking branch was correctly cloned
    cd "${IMGUI_DIR}"
    CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
    
    if [ "${CURRENT_BRANCH}" = "docking" ]; then
      echo "✓ ImGui is correctly on the docking branch."
      
      # Check for docking feature in imgui.h
      if grep -q "ImGuiConfigFlags_DockingEnable" "${IMGUI_DIR}/imgui.h"; then
        echo "✓ ImGuiConfigFlags_DockingEnable flag found in imgui.h"
      else
        echo "⚠ Warning: ImGuiConfigFlags_DockingEnable flag not found in imgui.h"
        echo "  This may indicate an issue with the ImGui version."
      fi
    else
      echo "⚠ Warning: ImGui is not on the docking branch. Current branch: ${CURRENT_BRANCH}"
    fi
  else
    echo "Failed to install ImGui. Please check your internet connection and Git configuration."
    exit 1
  fi
}

# Function to update CMakeLists.txt
update_cmake_lists() {
  echo "Checking CMakeLists.txt for ImGui configuration..."
  
  if [ ! -f CMakeLists.txt ]; then
    echo "CMakeLists.txt not found in current directory!"
    return
  fi
  
  # Check if ImGui is already included in CMakeLists.txt
  if grep -q "imgui_impl_" CMakeLists.txt; then
    echo "ImGui already appears to be configured in CMakeLists.txt"
    echo "Please verify the configuration manually."
    return
  fi
  
  echo "Would you like to update your CMakeLists.txt to include ImGui? (y/n)"
  read -p "> " confirm
  
  if [ "$confirm" != "y" ]; then
    echo "Skipping CMakeLists.txt update."
    return
  fi
  
  # Create backup
  cp CMakeLists.txt CMakeLists.txt.bak
  echo "Created backup: CMakeLists.txt.bak"
  
  # Add ImGui include directories
  sed -i '/include_directories/a\    ${CMAKE_SOURCE_DIR}/external/imgui\n    ${CMAKE_SOURCE_DIR}/external/imgui/backends' CMakeLists.txt
  
  # Add ImGui source files before add_executable
  imgui_sources='# ImGui source files\nset(IMGUI_SOURCES\n    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp\n    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp\n    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp\n    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp\n    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp\n    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp\n    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp\n)\n'
  
  # Find the line with add_executable and add IMGUI_SOURCES
  if grep -q "add_executable" CMakeLists.txt; then
    # Add ImGui sources before add_executable
    sed -i "/add_executable/i\\$imgui_sources" CMakeLists.txt
    
    # Update add_executable to include IMGUI_SOURCES
    sed -i 's/add_executable([^)]*)/& ${IMGUI_SOURCES}/' CMakeLists.txt
    
    echo "Updated CMakeLists.txt to include ImGui"
  else
    echo "Could not find add_executable in CMakeLists.txt"
    echo "Please update CMakeLists.txt manually to include ImGui"
  fi
}

# Main script execution
echo "This script will install all required dependencies for NPR_Renderer"
echo "including the ImGui docking branch."
echo "Continue? (y/n)"

read -p "> " confirm
if [ "$confirm" != "y" ]; then
  echo "Installation cancelled."
  exit 0
fi

# Install system dependencies
install_system_dependencies

# Install ImGui docking branch
install_imgui_docking

# Update CMakeLists.txt
update_cmake_lists

# Return to the project directory
cd "$(pwd)"

echo "====================================================="
echo "Installation complete!"
echo ""
echo "To use ImGui docking branch in your code, include the following initialization:"
echo ""
echo "// Setup ImGui context"
echo "IMGUI_CHECKVERSION();"
echo "ImGui::CreateContext();"
echo "ImGuiIO& io = ImGui::GetIO();"
echo "io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking"
echo ""
echo "// Setup Platform/Renderer backends"
echo "ImGui_ImplGlfw_InitForOpenGL(window, true);"
echo "ImGui_ImplOpenGL3_Init();"
echo ""
echo "You can now build your project with CMake:"
echo "mkdir -p build && cd build && cmake .. && make"
echo "====================================================="

exit 0