# Non-Photorealistic Renderer

## 0.0 Instaling Dependencies and Building the Project

First clone the repository by running 
```shell
git clone https://github.com/biancanev/cs184-final-project.git
```

### 0.1 MacOS/Linux(Debian)
Run the `install_dependencies.sh` script to install all necessary dependencies for building. Then run the following commands:
```shell
mkdir build
cd build
cmake ..
make
```

The program should now be properly built. Run `./NPR_Renderer` to launch the GUI.

### 0.2 Windows
Installing dependencies on Windows is a bit more of an involved process. If you wish to use your own compilers or toolchains feel free to edit `CMakeLists.txt`

1. **Install MSYS2** at [https://www.msys2.org/](https://www.msys2.org/). This is the easiest way to have a central environment for all of our tools. Make sure to get any updates with the `pacman -Syu` command.

2. **Get the MinGW toolchain** by running `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`. If prompted type `y` to install the necessary packages.

3. **Install the necessary libaries** using the following commands:
```bash
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glm
pacman -S mingw-w64-x86_64-assimp
```

4. **Optional: set your environment variables** by adding directories such as `C:/msys64/mingw64/bin` to your PATH environment variables may come in useful, especially if you are planning on adding more libraries to the code. Make sure to update `CMakeLists.txt` for proper compilation.

5. **Build the program** by running the following commands
```shell
mkdir build
cd build
windows_build
```

Congratulations! The renderer should be fully built on your Windows machine! in the `build` folder run the `NPR_Renderer` executable to launch the GUI.

## 1.0 GUI Controls
| Hotkey | Function |
| --- | --- |
| `1 - 9` | Load specified shader(See section 2 for corresponding shader) |
| `Esc` | Quit program |
| `Left Click` | Camera orbit |
| `Ctrl + Left Click` | Camera Pan |

## 2.0 Shading Types:

### 2.1 Standard/Specular(Blinn-Phong) Shading
The default shading method is Blinn-Phong shading

### 2.2 Cel/Toon Shading

### 2.3 Watercolor Shading

### 2.4 Sketch Shading