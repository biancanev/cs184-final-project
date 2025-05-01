#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Transform_Operation {
    SCALE_X,
    SCALE_Y,
    SCALE_Z,
    SCALE_UNIFORM,
    ROTATE_X,
    ROTATE_Y,
    ROTATE_Z,
    TRANSLATE_X,
    TRANSLATE_Y,
    TRANSLATE_Z
};

class Transform {
public:
    // Transform properties
    glm::vec3 Scale;
    glm::vec3 Rotation; // In degrees
    glm::vec3 Position;
    
    // Mouse sensitivity for transformations
    float ScaleSensitivity;
    float RotationSensitivity;
    float TranslationSensitivity;
    
    // Current operation mode
    Transform_Operation CurrentOperation;

    // Constructor with default values
    Transform(
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f)
    );
    
    // Get the transformation matrix
    glm::mat4 GetModelMatrix();
    
    // Process mouse movement based on current operation
    void ProcessMouseMovement(float xoffset, float yoffset);
    
    // Set specific transformation mode
    void SetOperation(Transform_Operation operation);
    
    // Reset to default values
    void Reset();
    
    // Apply a uniform scale to make the model a specific size
    void FitToSize(float maxDimension);

private:
    // Apply scale operation with mouse movement
    void ProcessScale(float xoffset, float yoffset);
    
    // Apply rotation operation with mouse movement
    void ProcessRotation(float xoffset, float yoffset);
    
    // Apply translation operation with mouse movement
    void ProcessTranslation(float xoffset, float yoffset);
};

#endif