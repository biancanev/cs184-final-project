#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int SLIDER_WIDTH = 200;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseButtonPressed = false;
float rotationAngle = 0.0f;
bool fixed_lighting = false;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Shader index
int currentShader = 0;
std::vector<Shader> shaders;

// ImGui Variables
bool showDemoWindow = false;
bool showControlPanel = true;
bool showToolPanel = true;

ImVec4 objectColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
float ambientStrength = 0.1f;
float specularStrength = 0.5f;
float shininess = 32.0f;
float lightPosX = 1.2f;
float lightPosY = 1.0f;
float lightPosZ = 2.0f;

std::string modelPath = "";
bool modelLoaded = false;

enum Camera_Mode {
    CAMERA_PAN,
    CAMERA_ORBIT,
    CAMERA_ROTATE,
    CAMERA_TILT,
    CAMERA_ZOOM
};
Camera_Mode currentCam = CAMERA_PAN;

bool SelectableButton(const char* label, bool isSelected) {
    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.6f, 1.0f, 1.0f)); // highlight
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
    }

    bool clicked = ImGui::Button(label);

    if (isSelected)
        ImGui::PopStyleColor(3);

    return clicked;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    // Skip camera movement if ImGui is capturing mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
        
    // Camera mouse control code
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    
    lastX = xpos;
    lastY = ypos;
    
    if (mouseButtonPressed) {
        // Check if CTRL key is held for panning
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
            glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS || 
            currentCam ==  CAMERA_PAN) {
            // Pan the camera
            camera.ProcessMousePan(xoffset, yoffset);
        } else if (currentCam ==  CAMERA_ORBIT) {
            // Orbit the camera
            camera.ProcessMouseMovementOrbit(xoffset, yoffset);
        } else if (currentCam ==  CAMERA_ROTATE) {
            camera.ProcessMouseMovementRotate(xoffset, yoffset);
        } else if (currentCam ==  CAMERA_TILT) {
            camera.ProcessMouseMovementTilt(xoffset, yoffset);
        } else if (currentCam ==  CAMERA_ZOOM) {
            camera.ProcessMouseScroll(yoffset * 0.2);
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
        
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            mouseButtonPressed = true;
        else if (action == GLFW_RELEASE)
            mouseButtonPressed = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
        
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    // Shader switching
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentShader = 0; // Standard
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentShader = 1; // Cel
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        currentShader = 2; // Watercolor
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        currentShader = 3; // Sketch
}

// Function to setup ImGui
void setupImGui(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// Function to load a 3D model
bool loadModelFile(Model& model, const std::string& path) {
    try {
        model = Model(path);
        std::cout << "Model loaded successfully: " << path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        return false;
    }
}

// Function to render ImGui interface
void renderImGui(Model& ourModel, GLFWwindow* window) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()!)
    if (showDemoWindow)
        ImGui::ShowDemoWindow(&showDemoWindow);

    // 2. Show control panel
    if (showControlPanel) {
        // logic for minimizing control panel
        static bool is_minimized = true;
        ImVec2 display_size = ImGui::GetIO().DisplaySize;
        
        float panel_width = 350;

        
        ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        // ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        // ImGui::SetNextWindowSize(ImVec2(panel_width, 600), ImGuiCond_FirstUseEver);
        ImGui::SetWindowPos(ImVec2(display_size.x - panel_width - 20, 20));
        
        if (ImGui::Button(is_minimized ? "Collapse" : "Expand")) {
            is_minimized = !is_minimized;
        }
        
        if (!is_minimized) {
            ImGui::SetWindowSize(ImVec2(panel_width, 50)); 
        } else {
            ImGui::SetWindowSize(ImVec2(panel_width, 600));
        }
        
        
        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Model loading
            char inputBuffer[256] = "";
            strncpy(inputBuffer, modelPath.c_str(), sizeof(inputBuffer) - 1);
            
            // List all model files in the models directory
            // ImGui::Separator();
            ImGui::Text("Available Models:");
            
            std::string modelsDir = "../models";
            
            // Scan the directory
            try {
                if (fs::exists(modelsDir) && fs::is_directory(modelsDir)) {
                    for (const auto& entry : fs::directory_iterator(modelsDir)) {
                        std::string extension = entry.path().extension().string();
                        if (extension == ".obj" || extension == ".fbx" || 
                            extension == ".stl" || extension == ".3ds" ||
                            extension == ".dae" || extension == ".blend") {
                            
                            std::string filename = entry.path().filename().string();
                            if (ImGui::Button(filename.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                                modelPath = entry.path().string();
                                // modelLoaded = loadModelFile(ourModel, modelPath);
                            }
                        }
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                                    "Models directory not found: %s", modelsDir.c_str());
                }
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                                "Error scanning models directory: %s", e.what());
            }

            ImGui::Text("Enter model path:");
            if (ImGui::InputText("##modelpath", inputBuffer, sizeof(inputBuffer))) {
                modelPath = inputBuffer;
            }
            
            if (ImGui::Button("Load Model")) {
                if (!modelPath.empty()) {
                    modelLoaded = loadModelFile(ourModel, modelPath);
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Load Default Cube")) {
                ourModel = Model(); // Creates default cube
                modelLoaded = true;
            }
        }
        
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Light properties
            ImGui::SetNextItemWidth(SLIDER_WIDTH);
            ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f);
            ImGui::SetNextItemWidth(SLIDER_WIDTH);
            ImGui::SliderFloat("Specular Strength", &specularStrength, 0.0f, 1.0f);
            ImGui::SetNextItemWidth(SLIDER_WIDTH);
            ImGui::SliderFloat("Shininess", &shininess, 1.0f, 128.0f);
            
            ImGui::Checkbox("Fixed Lighting (Light follows camera)", &fixed_lighting);
            
            if (!fixed_lighting) {
                ImGui::SetNextItemWidth(SLIDER_WIDTH);
                ImGui::SliderFloat("Light Position X", &lightPosX, -10.0f, 10.0f);
                ImGui::SetNextItemWidth(SLIDER_WIDTH);
                ImGui::SliderFloat("Light Position Y", &lightPosY, -10.0f, 10.0f);
                ImGui::SetNextItemWidth(SLIDER_WIDTH);
                ImGui::SliderFloat("Light Position Z", &lightPosZ, -10.0f, 10.0f);
            }
        }
        
        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Color picker
            ImGui::ColorEdit3("Object Color", (float*)&objectColor);
            
            // Shader selection
            const char* shaderNames[] = { "Standard", "Cel", "Watercolor", "Sketch" };
            ImGui::Combo("Shader", &currentShader, shaderNames, IM_ARRAYSIZE(shaderNames));
        }
        
        ImGui::Separator();
        //ImGui::Checkbox("Show Demo Window", &showDemoWindow);
        
        ImGui::End();
    }

    if (showToolPanel) {
        ImGui::Begin(" ");

        ImGui::SetWindowPos(ImVec2(20, 10.0f)); // adjust position
        ImGui::SetWindowSize(ImVec2(80, 200.0f)); // adjust position

        ImGui::Text("Camera");
        if (SelectableButton("Pan", currentCam == CAMERA_PAN))       currentCam = CAMERA_PAN;
        if (SelectableButton("Orbit", currentCam == CAMERA_ORBIT))   currentCam = CAMERA_ORBIT;
        if (SelectableButton("Rotate", currentCam == CAMERA_ROTATE)) currentCam = CAMERA_ROTATE;
        if (SelectableButton("Tilt", currentCam == CAMERA_TILT))     currentCam = CAMERA_TILT;
        if (SelectableButton("Zoom", currentCam == CAMERA_ZOOM))     currentCam = CAMERA_ZOOM;
        

        // ImGui::EndChild();
        ImGui::End();
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main(int argc, char **argv) {
    std::string modelPath = "../models/Baby_Groot_Funko_Pop.stl";
    if (argc > 1) {
        modelPath = argv[1];
    }

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NPR Renderer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Print OpenGL version info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Setup ImGui
    setupImGui(window);

    // Load shaders
    std::cout << "Attempting to load shader from: shaders/standard.frag" << std::endl;
    Shader standardShader("../shaders/standard.vert", "../shaders/standard.frag");
    Shader celShader("../shaders/Cel.vert", "../shaders/Cel.frag");
    Shader watercolorShader("../shaders/standard.vert", "../shaders/Watercolor.frag");
    Shader sketchShader("../shaders/standard.vert", "../shaders/Sketch.frag");
    
    shaders.push_back(standardShader);
    shaders.push_back(celShader);
    shaders.push_back(watercolorShader);
    shaders.push_back(sketchShader);

    // Load models
    Model ourModel;

    camera.SetOrbitTarget(glm::vec3(0.0f, 0.0f, 0.0f)); 

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        if (currentShader == 2 || currentShader == 3) {
            glClearColor(1.0, 1.0, 1.0, 0.0);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        shaders[currentShader].use();

        // Set shader uniforms
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
                                              static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 
                                              0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shaders[currentShader].setMat4("projection", projection);
        shaders[currentShader].setMat4("view", view);

        // Light properties
        glm::vec3 lightPos(lightPosX, lightPosY, lightPosZ);
        if (fixed_lighting){
            lightPos = camera.Position + camera.Front * 2.0f;
        }
        
        shaders[currentShader].setVec3("lightPos", lightPos);
        shaders[currentShader].setVec3("viewPos", camera.Position);
        shaders[currentShader].setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        
        // Set custom lighting parameters from ImGui
        shaders[currentShader].setFloat("ambientStrength", ambientStrength);
        shaders[currentShader].setFloat("specularStrength", specularStrength);
        shaders[currentShader].setFloat("shininess", shininess);
        
        // Optional: Pass time to shader for animation effects
        shaders[currentShader].setFloat("time", currentFrame);

        // Render the model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        shaders[currentShader].setMat4("model", model);
        
        // Set object color from ImGui
        shaders[currentShader].setVec3("objectColor", 
            glm::vec3(objectColor.x, objectColor.y, objectColor.z));
        
        ourModel.Draw(shaders[currentShader]);
        
        // Render ImGui interface
        renderImGui(ourModel, window);
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Terminate GLFW
    glfwTerminate();
    return 0;
}