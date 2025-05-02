#include "transform.h"

Transform::Transform(glm::vec3 scale, glm::vec3 rotation, glm::vec3 position)
    : Scale(scale),
      Rotation(rotation),
      Position(position),
      ScaleSensitivity(0.01f),
      RotationSensitivity(0.5f),
      TranslationSensitivity(0.01f),
      CurrentOperation(SCALE_UNIFORM) {
}

glm::mat4 Transform::GetModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    
    // Apply transformations 
    model = glm::translate(model, Position);
    
    // Apply rotations 
    model = glm::rotate(model, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Apply scale
    model = glm::scale(model, Scale);
    
    return model;
}

void Transform::ProcessMouseMovement(float xoffset, float yoffset) {
    switch (CurrentOperation) {
        case SCALE_X:
        case SCALE_Y:
        case SCALE_Z:
        case SCALE_UNIFORM:
            ProcessScale(xoffset, yoffset);
            break;
            
        case ROTATE_X:
        case ROTATE_Y:
        case ROTATE_Z:
            ProcessRotation(xoffset, yoffset);
            break;
            
        case TRANSLATE_X:
        case TRANSLATE_Y:
        case TRANSLATE_Z:
            ProcessTranslation(xoffset, yoffset);
            break;
    }
}

void Transform::SetOperation(Transform_Operation operation) {
    CurrentOperation = operation;
}

void Transform::Reset() {
    Scale = glm::vec3(1.0f, 1.0f, 1.0f);
    Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    Position = glm::vec3(0.0f, 0.0f, 0.0f);
}


void Transform::ProcessScale(float xoffset, float yoffset) {
    float scaleChange = yoffset * ScaleSensitivity;
    float uniformScale;
    
    switch (CurrentOperation) {
        case SCALE_X:
            Scale.x = fmax(0.01f, Scale.x + scaleChange);
            break;
            
        case SCALE_Y:
            Scale.y = fmax(0.01f, Scale.y + scaleChange);
            break;
            
        case SCALE_Z:
            Scale.z = fmax(0.01f, Scale.z + scaleChange);
            break;
            
        case SCALE_UNIFORM:
            uniformScale = fmax(0.01f, 1.0f + scaleChange);
            Scale *= uniformScale;
            break;
            
        case ROTATE_X:
        case ROTATE_Y:
        case ROTATE_Z:
        case TRANSLATE_X:
        case TRANSLATE_Y:
        case TRANSLATE_Z:
            break;
    }
}

void Transform::ProcessRotation(float xoffset, float yoffset) {
    float rotationChange = xoffset * RotationSensitivity;
    
    switch (CurrentOperation) {
        case ROTATE_X:
            Rotation.x += rotationChange;
            break;
            
        case ROTATE_Y:
            Rotation.y += rotationChange;
            break;
            
        case ROTATE_Z:
            Rotation.z += rotationChange;
            break;
            
        case SCALE_X:
        case SCALE_Y:
        case SCALE_Z:
        case SCALE_UNIFORM:
        case TRANSLATE_X:
        case TRANSLATE_Y:
        case TRANSLATE_Z:
            break;
    }
    
    // Normalize angles to [0, 360)
    Rotation.x = fmod(Rotation.x, 360.0f);
    Rotation.y = fmod(Rotation.y, 360.0f);
    Rotation.z = fmod(Rotation.z, 360.0f);
}

void Transform::ProcessTranslation(float xoffset, float yoffset) {
    float translationChange = xoffset * TranslationSensitivity;
    float verticalChange = yoffset * TranslationSensitivity;
    
    switch (CurrentOperation) {
        case TRANSLATE_X:
            Position.x += translationChange;
            break;
            
        case TRANSLATE_Y:
            Position.y += verticalChange; 
            break;
            
        case TRANSLATE_Z:
            Position.z -= translationChange;
            break;
            
        case SCALE_X:
        case SCALE_Y:
        case SCALE_Z:
        case SCALE_UNIFORM:
        case ROTATE_X:
        case ROTATE_Y:
        case ROTATE_Z:
            break;
    }
}