#include "Camera.h"
#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    Distance = glm::length(position - Target);
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    
    if (IsOrbiting) {
        // In orbit mode, WASD moves the camera in/out (Zoom) or rotates? 
        // Usually scroll is zoom. WASD could pan? 
        // Let's make W/S zoom in orbit mode too for accessibility
        if (direction == FORWARD)
            Distance -= velocity;
        if (direction == BACKWARD)
            Distance += velocity;
        if (Distance < 0.1f) Distance = 0.1f;
        updateCameraVectors();
    } else {
        // Free flight
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    if (IsOrbiting) {
        Distance -= yoffset * 0.5f;
        if (Distance < 0.1f) Distance = 0.1f;
        updateCameraVectors();
    } else {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }
}

void Camera::updateCameraVectors() {
    if (IsOrbiting) {
        // Calculate Position based on Angle + Distance around Target
        // Note: Standard math uses 0 as East, OpenGL 0 Yaw is -Z (North). 
        // We adjust to match standard orbital feel.
        
        float yawRad = glm::radians(Yaw);
        float pitchRad = glm::radians(Pitch);

        glm::vec3 offset;
        offset.x = Distance * cos(pitchRad) * cos(yawRad);
        offset.y = Distance * sin(pitchRad);
        offset.z = Distance * cos(pitchRad) * sin(yawRad);
        
        Position = Target + offset;
        Front = glm::normalize(Target - Position);
    } 
    else {
        // FPS Camera: Calculate Front based on Angle
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
    }

    // Recalculate Right and Up
    Right = glm::normalize(glm::cross(Front, WorldUp)); 
    Up    = glm::normalize(glm::cross(Right, Front));
}
