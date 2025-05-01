#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.h"
#include "model.h"
#include "shader.h"
#include "transform.h"

#include <filesystem>
#include <iostream>
#include <string>
namespace fs = std::filesystem;

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int SLIDER_WIDTH = 200;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -15.0f);
Transform modelTransform;
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
bool showGrid = true;

ImVec4 objectColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
float ambientStrength = 0.1f;
float outlineThickness = 0.03f;
float specularStrength = 0.5f;
float shininess = 32.0f;
float lightPosX = 1.2f;
float lightPosY = 1.0f;
float lightPosZ = 2.0f;
bool useWhiteBackground = false;

std::string modelPath = "";
bool modelLoaded = false;

std::string texturePath = "";
bool textureLoaded = false;

ImVec4 col1 = ImVec4(.9f, .3f, .2f, 1.f);
ImVec4 col2 = ImVec4(.2f, .5f, .8f, 1.f);
ImVec4 col3 = ImVec4(.1f, .7f, .4f, 1.f);
ImVec4 col4 = ImVec4(.8f, .6f, .1f, 1.f);

float edgeIntensityValue = 2.5f;
float edgeNoiseValue = .8f;
float granulationValue = .7f;
float paperVisibilityValue = .3f;
float transparencyValue = .85f;

enum Tool_Mode {
        CAMERA_PAN,
        CAMERA_ORBIT,
        CAMERA_ROTATE,
        CAMERA_TILT,
        CAMERA_ZOOM,
        CAMERA_ROLL,
        MODEL_SCALE,
        MODEL_TRANSLATE_X,
        MODEL_TRANSLATE_Y,
        MODEL_TRANSLATE_Z,
        MODEL_ROTATE_X,
        MODEL_ROTATE_Y,
        MODEL_ROTATE_Z
};
Tool_Mode currentTool = CAMERA_PAN;

bool SelectableButton(const char* label, bool isSelected) {
        if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f)); // highlight
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
        }

        bool clicked = ImGui::Button(label);

        if (isSelected)
                ImGui::PopStyleColor(3);

        return clicked;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

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
                    glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS || currentTool == CAMERA_PAN) {
                        // Pan the camera
                        camera.ProcessMousePan(xoffset, yoffset);
                } else if (currentTool == CAMERA_ORBIT) {
                        // Orbit the camera
                        camera.ProcessMouseMovementOrbit(xoffset, yoffset);
                } else if (currentTool == CAMERA_ROTATE) {
                        camera.ProcessMouseMovementRotate(xoffset, yoffset);
                } else if (currentTool == CAMERA_TILT) {
                        camera.ProcessMouseMovementTilt(xoffset, yoffset);
                } else if (currentTool == CAMERA_ZOOM) {
                        camera.ProcessMouseScroll(yoffset * 0.2);
                } else if (currentTool == CAMERA_ROLL) {
                        camera.ProcessMouseMovementRoll(xoffset, yoffset);
                } else if (currentTool == MODEL_SCALE) {
                        modelTransform.SetOperation(SCALE_UNIFORM);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
                } else if (currentTool == MODEL_ROTATE_X) {
                        modelTransform.SetOperation(ROTATE_X);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
                } else if (currentTool == MODEL_ROTATE_Y) {
                        modelTransform.SetOperation(ROTATE_Y);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
                } else if (currentTool == MODEL_ROTATE_Z) {
                        modelTransform.SetOperation(ROTATE_Z);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
                } else if (currentTool == MODEL_TRANSLATE_X) {
                        modelTransform.SetOperation(TRANSLATE_X);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
                } else if (currentTool == MODEL_TRANSLATE_Y) {
                        modelTransform.SetOperation(TRANSLATE_Y);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
                } else if (currentTool == MODEL_TRANSLATE_Z) {
                        modelTransform.SetOperation(TRANSLATE_Z);
                        modelTransform.ProcessMouseMovement(xoffset, yoffset);
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
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

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
                camera.ResetOrientation();
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

                ImGui::SameLine();

                if (ImGui::Button(showGrid ? "Hide Reference Plane" : "Show Reference Plane")) {
                        showGrid = !showGrid;
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
                                                if (extension == ".obj" || extension == ".fbx" || extension == ".stl" ||
                                                    extension == ".3ds" || extension == ".dae" ||
                                                    extension == ".blend") {

                                                        std::string filename = entry.path().filename().string();
                                                        if (ImGui::Button(
                                                                    filename.c_str(),
                                                                    ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
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
                                        ourModel.meshes[0].textures.clear();
                                        textureLoaded = false;
                                        texturePath = "";
                                        std::cout << "Texture removed from model" << std::endl;
                                        Texture newTexture;
                                        ourModel.replaceTextures({newTexture});
                                        modelLoaded = loadModelFile(ourModel, modelPath);
                                        ourModel.replaceTextures({newTexture});
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
                        char texInputBuffer[256] = "";
                        strncpy(texInputBuffer, texturePath.c_str(), sizeof(texInputBuffer) - 1);
                        // Color picker
                        ImGui::ColorEdit3("Object Color", (float*)&objectColor);
                        // std::cout << "Color: R=" << objectColor.x
                        // << " G=" << objectColor.y
                        // << " B=" << objectColor.z
                        // << std::endl;

                        ImGui::SetNextItemWidth(SLIDER_WIDTH);
                        ImGui::SliderFloat("Outline Thickness", &outlineThickness, 0.0f, 0.1f, "%.3f");

                        ImGui::Checkbox("White Background", &useWhiteBackground);

                        // Texture picker
                        ImGui::Text("Available Textures:");

                        std::string textureDir = "../textures";

                        // Scan the directory
                        try {
                                if (fs::exists(textureDir) && fs::is_directory(textureDir)) {
                                        for (const auto& entry : fs::directory_iterator(textureDir)) {
                                                std::string extension = entry.path().extension().string();
                                                if (extension == ".jpg" || extension == ".jpeg" ||
                                                    extension == ".png") {

                                                        std::string filename = entry.path().filename().string();
                                                        if (ImGui::Button(
                                                                    filename.c_str(),
                                                                    ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                                                                texturePath = entry.path().string();
                                                        }
                                                }

                                                if (ImGui::Button("Load Model")) {
                                                        if (!modelPath.empty()) {
                                                                ourModel.meshes[0].textures.clear();
                                                                textureLoaded = false;
                                                                texturePath = "";
                                                                std::cout << "Texture removed from model" << std::endl;
                                                                Texture newTexture;
                                                                ourModel.replaceTextures({newTexture});
                                                                modelLoaded = loadModelFile(ourModel, modelPath);
                                                                ourModel.replaceTextures({newTexture});
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

                                                ImGui::Checkbox("Fixed Lighting (Light follows camera)",
                                                                &fixed_lighting);

                                                if (!fixed_lighting) {
                                                        ImGui::SetNextItemWidth(SLIDER_WIDTH);
                                                        ImGui::SliderFloat("Light Position X", &lightPosX, -10.0f,
                                                                           10.0f);
                                                        ImGui::SetNextItemWidth(SLIDER_WIDTH);
                                                        ImGui::SliderFloat("Light Position Y", &lightPosY, -10.0f,
                                                                           10.0f);
                                                        ImGui::SetNextItemWidth(SLIDER_WIDTH);
                                                        ImGui::SliderFloat("Light Position Z", &lightPosZ, -10.0f,
                                                                           10.0f);
                                                }
                                        }

                                        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
                                                char texInputBuffer[256] = "";
                                                strncpy(texInputBuffer, texturePath.c_str(),
                                                        sizeof(texInputBuffer) - 1);
                                                // Color picker
                                                ImGui::ColorEdit3("Object Color", (float*)&objectColor);
                                                // std::cout << "Color: R=" << objectColor.x
                                                // << " G=" << objectColor.y
                                                // << " B=" << objectColor.z
                                                // << std::endl;

                                                // Texture picker
                                                ImGui::Text("Available Textures:");

                                                std::string textureDir = "../textures";

                                                // Scan the directory
                                                try {
                                                        if (fs::exists(textureDir) && fs::is_directory(textureDir)) {
                                                                for (const auto& entry :
                                                                     fs::directory_iterator(textureDir)) {
                                                                        std::string extension =
                                                                                entry.path().extension().string();
                                                                        if (extension == ".jpg" ||
                                                                            extension == ".jpeg" ||
                                                                            extension == ".png") {

                                                                                std::string filename =
                                                                                        entry.path()
                                                                                                .filename()
                                                                                                .string();
                                                                                if (ImGui::Button(
                                                                                            filename.c_str(),
                                                                                            ImVec2(ImGui::GetContentRegionAvail()
                                                                                                           .x,
                                                                                                   0))) {
                                                                                        texturePath =
                                                                                                entry.path().string();
                                                                                }
                                                                        }
                                                                }
                                                        } else {
                                                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                                                                   "Textures directory not found: %s",
                                                                                   textureDir.c_str());
                                                        }
                                                } catch (const std::exception& e) {
                                                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                                                           "Error scanning textures directory: %s",
                                                                           e.what());
                                                }

                                                ImGui::Text("Enter texture path:");
                                                if (ImGui::InputText("##texturepath", texInputBuffer,
                                                                     sizeof(texInputBuffer))) {
                                                        texturePath = texInputBuffer;
                                                }

                                                if (ImGui::Button("Load Color/Texture")) {
                                                        if (!texturePath.empty()) {
                                                                Texture newTexture;

                                                                // Load the texture as a diffuse texture
                                                                if (newTexture.loadTextureFromFile(texturePath)) {
                                                                        // Replace textures in the model
                                                                        ourModel.replaceTextures({newTexture});

                                                                        textureLoaded = true;
                                                                        std::cout << "Texture loaded and applied to "
                                                                                     "model: "
                                                                                  << texturePath << std::endl;
                                                                } else {
                                                                        std::cerr << "Failed to load texture: "
                                                                                  << texturePath << std::endl;
                                                                        textureLoaded = false;
                                                                }
                                                        } else {
                                                                Texture newTexture;
                                                                ourModel.replaceTextures({newTexture});
                                                        }
                                                }
                                                ImGui::SameLine();
                                                if (ImGui::Button("Remove Texture")) {
                                                        // Clear textures from the model
                                                        if (!ourModel.meshes.empty()) {
                                                                ourModel.meshes[0].textures.clear();
                                                                textureLoaded = false;
                                                                texturePath = "";
                                                                std::cout << "Texture removed from model" << std::endl;
                                                                Texture newTexture;
                                                                ourModel.replaceTextures({newTexture});
                                                        }
                                                }

                                                // Shader selection
                                                const char* shaderNames[] = {"Standard", "Cel", "Watercolor", "Sketch"};
                                                ImGui::Combo("Shader", &currentShader, shaderNames,
                                                             IM_ARRAYSIZE(shaderNames));
                                        }

                                        ImGui::Separator();
                                        // ImGui::Checkbox("Show Demo Window", &showDemoWindow);

                                        ImGui::End();
                                }

                                if (showToolPanel) {
                                        ImGui::Begin(" ");

                                        ImGui::SetWindowPos(ImVec2(20, 10.0f));    // adjust position
                                        ImGui::SetWindowSize(ImVec2(100, 400.0f)); // adjust position

                                        ImGui::Text("Camera");
                                        if (SelectableButton("Pan", currentTool == CAMERA_PAN))
                                                currentTool = CAMERA_PAN;
                                        if (SelectableButton("Orbit", currentTool == CAMERA_ORBIT))
                                                currentTool = CAMERA_ORBIT;
                                        if (SelectableButton("Rotate", currentTool == CAMERA_ROTATE))
                                                currentTool = CAMERA_ROTATE;
                                        if (SelectableButton("Tilt", currentTool == CAMERA_TILT))
                                                currentTool = CAMERA_TILT;
                                        if (SelectableButton("Roll", currentTool == CAMERA_ROLL))
                                                currentTool = CAMERA_ROLL;
                                        if (SelectableButton("Zoom", currentTool == CAMERA_ZOOM))
                                                currentTool = CAMERA_ZOOM;
                                        ImGui::Text("Model");
                                        if (SelectableButton("Scale", currentTool == MODEL_SCALE))
                                                currentTool = MODEL_SCALE;
                                        if (SelectableButton("Rotate X", currentTool == MODEL_ROTATE_X))
                                                currentTool = MODEL_ROTATE_X;
                                        if (SelectableButton("Rotate Y", currentTool == MODEL_ROTATE_Y))
                                                currentTool = MODEL_ROTATE_Y;
                                        if (SelectableButton("Rotate Z", currentTool == MODEL_ROTATE_Z))
                                                currentTool = MODEL_ROTATE_Z;
                                        if (SelectableButton("Translate X", currentTool == MODEL_TRANSLATE_X))
                                                currentTool = MODEL_TRANSLATE_X;
                                        if (SelectableButton("Translate Y", currentTool == MODEL_TRANSLATE_Y))
                                                currentTool = MODEL_TRANSLATE_Y;
                                        if (SelectableButton("Translate Z", currentTool == MODEL_TRANSLATE_Z))
                                                currentTool = MODEL_TRANSLATE_Z;

                                        // ImGui::EndChild();
                                        ImGui::End();
                                }

                                // Render ImGui
                                ImGui::Render();
                                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                        }

                        int main(int argc, char** argv) {
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
                                GLFWwindow* window =
                                        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NPR Renderer", NULL, NULL);
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
                                Shader gridShader("../shaders/grid.vert", "../shaders/grid.frag");

                                shaders.push_back(standardShader);
                                shaders.push_back(celShader);
                                shaders.push_back(watercolorShader);
                                shaders.push_back(sketchShader);

                                Texture noiseTexture, paperTexture;
                                if (!noiseTexture.loadTextureFromFile("../textures/noise.png"))
                                        std::cerr << "Failed to load noise texture" << std::endl;
                                if (!paperTexture.loadTextureFromFile("../textures/paper.png"))
                                        std::cerr << "Failed to load paper texture" << std::endl;

                                Model gridModel;
                                Model ourModel;

                                gridModel = Model();
                                gridModel.createGrid(5.0f, 100);

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
                                        if (currentShader == 2 || currentShader == 3)
                                                glClearColor(1.0, 1.0, 1.0, 0.0);
                                        else
                                                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

                                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                                        // Activate shader
                                        shaders[currentShader].use();

                                        // Set shader uniforms
                                        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                                                static_cast<float>(SCR_WIDTH) /
                                                                                        static_cast<float>(SCR_HEIGHT),
                                                                                0.1f, 100.0f);
                                        glm::mat4 view = camera.GetViewMatrix();
                                        shaders[currentShader].setMat4("projection", projection);
                                        shaders[currentShader].setMat4("view", view);

                                        // Light properties
                                        glm::vec3 lightPos(lightPosX, lightPosY, lightPosZ);
                                        if (fixed_lighting) {
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
                                        glm::mat4 model = modelTransform.GetModelMatrix();
                                        shaders[currentShader].setMat4("model", model);

                                        if (!ourModel.meshes.empty()) {

                                                if (textureLoaded) {
                                                        // Texture is loaded
                                                        ourModel.meshes[0].textures[0].bind(0);
                                                        shaders[currentShader].setInt("hasTexture", 1);
                                                        shaders[currentShader].setInt("texture_diffuse1", 0);
                                                } else {
                                                        // No texture, use solid color
                                                        // std::cout << "no texture" << std::endl;
                                                        shaders[currentShader].setInt("hasTexture", 0);
                                                }

                                                if (currentShader == 2) {
                                                        noiseTexture.bind(1);
                                                        paperTexture.bind(1);
                                                        shaders[currentShader].setInt("u_noise_texture", 1);
                                                        shaders[currentShader].setInt("u_paper_texture", 2);

                                                        shaders[currentShader].setVec3(
                                                                "u_color1", glm::vec3(col1.x, col1.y, col1.z));
                                                        shaders[currentShader].setVec3(
                                                                "u_color2", glm::vec3(col2.x, col2.y, col2.z));
                                                        shaders[currentShader].setVec3(
                                                                "u_color3", glm::vec3(col3.x, col3.y, col3.z));
                                                        shaders[currentShader].setVec3(
                                                                "u_color4", glm::vec3(col4.x, col4.y, col4.z));

                                                        shaders[currentShader].setFloat("u_edge_intensity",
                                                                                        edgeIntensityValue);
                                                        shaders[currentShader].setFloat("u_edge_noise", edgeNoiseValue);
                                                        shaders[currentShader].setFloat("u_granulation",
                                                                                        granulationValue);
                                                        shaders[currentShader].setFloat("u_paper_visibility",
                                                                                        paperVisibilityValue);
                                                        shaders[currentShader].setFloat("u_transparency",
                                                                                        transparencyValue);

                                                        glEnable(GL_BLEND);
                                                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                                                }
                                        }

                                        // Set object color from ImGui
                                        shaders[currentShader].setVec3(
                                                "objectColor", glm::vec3(objectColor.x, objectColor.y, objectColor.z));
                                        // shaders[currentShader].setVec3("objectColor", glm::vec3(0.7, 0.1, 0.1));

                                        ourModel.Draw(shaders[currentShader]);

                                        if (showGrid) {
                                                // Enable transparency
                                                glEnable(GL_BLEND);
                                                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                                                // Use grid shader
                                                gridShader.use();

                                                // Set standard uniforms
                                                gridShader.setMat4("projection", projection);
                                                gridShader.setMat4("view", view);
                                                gridShader.setMat4("model", glm::mat4(1.0f));

                                                // Draw the grid
                                                gridModel.Draw(gridShader);

                                                // Restore OpenGL state
                                                glDisable(GL_BLEND);
                                        }

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
                        Shader gridShader("../shaders/grid.vert", "../shaders/grid.frag");
                        Shader outlineShader("shaders/outline.vert", "shaders/outline.frag");

                        shaders.push_back(standardShader);
                        shaders.push_back(celShader);
                        shaders.push_back(watercolorShader);
                        shaders.push_back(sketchShader);

                        Model gridModel;
                        Model ourModel;

                        gridModel = Model();
                        gridModel.createGrid(5.0f, 100);

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
                                if (useWhiteBackground || currentShader == 2 || currentShader == 3) {
                                        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
                                } else {
                                        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray / black
                                }

                                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                                // Activate shader
                                shaders[currentShader].use();

                                // Set shader uniforms
                                glm::mat4 projection = glm::perspective(
                                        glm::radians(camera.Zoom),
                                        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
                                glm::mat4 view = camera.GetViewMatrix();
                                shaders[currentShader].setMat4("projection", projection);
                                shaders[currentShader].setMat4("view", view);

                                // Light properties
                                glm::vec3 lightPos(lightPosX, lightPosY, lightPosZ);
                                if (fixed_lighting) {
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
                                glm::mat4 model = modelTransform.GetModelMatrix();
                                shaders[currentShader].setMat4("model", model);

                                if (!ourModel.meshes.empty()) {
                                        if (textureLoaded) {
                                                // Texture is loaded
                                                ourModel.meshes[0].textures[0].bind(0);
                                                shaders[currentShader].setInt("hasTexture", 1);
                                                shaders[currentShader].setInt("texture_diffuse1", 0);
                                        } else {
                                                // No texture, use solid color
                                                // std::cout << "no texture" << std::endl;
                                                shaders[currentShader].setInt("hasTexture", 0);
                                        }
                                }

                                // Set object color from ImGui
                                shaders[currentShader].setVec3("objectColor",
                                                               glm::vec3(objectColor.x, objectColor.y, objectColor.z));
                                // shaders[currentShader].setVec3("objectColor", glm::vec3(0.7, 0.1, 0.1));

                                ourModel.Draw(shaders[currentShader]);

                                if (showGrid) {
                                        // Enable transparency
                                        glEnable(GL_BLEND);
                                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                                        // Use grid shader
                                        gridShader.use();

                                        // Set standard uniforms
                                        gridShader.setMat4("projection", projection);
                                        gridShader.setMat4("view", view);
                                        gridShader.setMat4("model", glm::mat4(1.0f));

                                        // Draw the grid
                                        gridModel.Draw(gridShader);

                                        // Restore OpenGL state
                                        glDisable(GL_BLEND);
                                }

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