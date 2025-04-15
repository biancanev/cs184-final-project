#!/bin/bash

# Exit on any error
set -e

echo "Installing dependencies for NPR_Renderer..."
echo "This script requires sudo privileges to install packages."

# Check if running on a Debian/Ubuntu-based system
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

echo "All dependencies installed successfully!"
echo "You can now build your project with CMake:"
echo "mkdir -p build && cd build && cmake .. && make"

exit 0