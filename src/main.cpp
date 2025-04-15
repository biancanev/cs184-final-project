#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Shader index
int currentShader = 0;
std::vector<Shader> shaders;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
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

int main() {
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
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW - this is different from GLAD
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

    // Load shaders
    Shader standardShader("shaders/standard.vert", "shaders/standard.frag");
    Shader celShader("shaders/standard.vert", "shaders/cel.frag");
    Shader watercolorShader("shaders/standard.vert", "shaders/watercolor.frag");
    Shader sketchShader("shaders/standard.vert", "shaders/sketch.frag");
    
    shaders.push_back(standardShader);
    shaders.push_back(celShader);
    shaders.push_back(watercolorShader);
    shaders.push_back(sketchShader);

    // Load models
    Model ourModel;

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
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        shaders[currentShader].setVec3("lightPos", lightPos);
        shaders[currentShader].setVec3("viewPos", camera.Position);
        shaders[currentShader].setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        
        // Optional: Pass time to shader for animation effects
        shaders[currentShader].setFloat("time", currentFrame);

        // Render the model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        shaders[currentShader].setMat4("model", model);
        ourModel.Draw(shaders[currentShader]);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate GLFW
    glfwTerminate();
    return 0;
}