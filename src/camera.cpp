#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float roll) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    Roll = roll;
    Target = glm::vec3(0.0f, 0.0f, 0.0f);
    OrbitDistance = glm::length(Position - Target); 
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, float roll) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    Roll = roll;
    updateCameraVectors();
}

void Camera::ResetOrientation() {
    // Reset rotation angles
    Yaw = -90.0f;
    Pitch = 0.0f;
    Roll = 0.0f;
    
    // Reset all vectors
    Front = glm::vec3(0.0f, 0.0f, -1.0f);
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Right = glm::vec3(1.0f, 0.0f, 0.0f);
    Up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Reset orbit parameters
    Target = glm::vec3(0.0f, 0.0f, 0.0f);
    OrbitDistance = 3.0f;
    Position = Target - Front * OrbitDistance;
    
    // Recalculate all vectors with clean state
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Target, Up);
}

void Camera::SetOrbitTarget(glm::vec3 target) {
    Target = target;
    OrbitDistance = glm::length(Position - Target);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
    
    // Calculate new position based on orbit distance
    Position = Target - Front * OrbitDistance;
}
void Camera::ProcessMouseScroll(float yoffset) {
    // Zoom -= yoffset;
    // if (Zoom < 0.5f)
    //     Zoom = 0.5f;
    // if (Zoom > 300.0f)
    //     Zoom = 300.0f;
    float moveAmount = yoffset * 0.5f; // Adjust this multiplier to control sensitivity
    Position += Front * moveAmount;
    
    // If using orbit camera, update orbit distance
    OrbitDistance = glm::length(Position - Target);
}

// check this code later
void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    
    // Calculate the Right vector without roll
    glm::vec3 rightNoRoll = glm::normalize(glm::cross(Front, WorldUp));
    glm::vec3 upNoRoll = glm::normalize(glm::cross(rightNoRoll, Front));
    
    // Apply roll rotation to the Up and Right vectors
    float rollRadians = glm::radians(Roll);
    float cosR = cos(rollRadians);
    float sinR = sin(rollRadians);
    
    // Rotate the Up and Right vectors around the Front vector
    Right.x = rightNoRoll.x * cosR + upNoRoll.x * sinR;
    Right.y = rightNoRoll.y * cosR + upNoRoll.y * sinR;
    Right.z = rightNoRoll.z * cosR + upNoRoll.z * sinR;
    Right = glm::normalize(Right);
    
    Up = glm::normalize(glm::cross(Right, Front));
    
    // Update Position as before
    Position = Target - Front * OrbitDistance;
}

void Camera::ProcessMouseMovementOrbit(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity * 1.5;
    yoffset *= MouseSensitivity * 1.5;

    Yaw   += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
    
    // Calculate new position based on orbit distance
    Position = Target - Front * OrbitDistance;
}

void Camera::ProcessMouseMovementRotate(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity * 1.5;
    yoffset *= MouseSensitivity * 0;

    Yaw   += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
    
    // Calculate new position based on orbit distance
    Position = Target - Front * OrbitDistance;
}

void Camera::ProcessMouseMovementTilt(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity * 0;
    yoffset *= MouseSensitivity * 1.5;

    Yaw   += xoffset;
    Pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
    
    // Calculate new position based on orbit distance
    Position = Target - Front * OrbitDistance;
}

void Camera::ProcessMouseMovementRoll(float xoffset, float yoffset, bool constrainPitch) {
    float rollOffset = xoffset * MouseSensitivity * 1.5f;
    
    Roll += rollOffset;
    
    // Optionally constrain roll within a certain range
    // For example, if you want to limit roll to ±180 degrees:
    if (Roll > 180.0f)
        Roll -= 360.0f;
    if (Roll < -180.0f)
        Roll += 360.0f;
        
    // Update vectors
    updateCameraVectors();
    
    // Calculate new position based on orbit distance
    // Position = Target - Front * OrbitDistance;
}

void Camera::ProcessMousePan(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity * 0.01f; // Adjust sensitivity for panning
    yoffset *= MouseSensitivity * 0.01f;
    
    // Move the target point (and consequently the camera) in the plane perpendicular to view direction
    glm::vec3 panRight = Right * xoffset * OrbitDistance;
    glm::vec3 panUp = Up * yoffset * OrbitDistance;
    
    Target -= panRight;
    Target -= panUp;
    
    // Update position based on new target
    Position = Target - Front * OrbitDistance;
}