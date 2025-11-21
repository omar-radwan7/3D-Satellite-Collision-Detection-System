#include "Camera.h"
#include <algorithm>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    if (IsOrbiting) {
        return glm::lookAt(Position, Target, Up);
    } else {
        return glm::lookAt(Position, Position + Front, Up);
    }
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (IsOrbiting) {
        // Orbit mode: W/S controls distance
        if (direction == FORWARD) Distance -= velocity;
        if (direction == BACKWARD) Distance += velocity;
        // Zoom limits
        if (Distance < 1.5f) Distance = 1.5f;
        if (Distance > 15.0f) Distance = 15.0f;
        updateCameraVectors();
    } else {
        // Free mode
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

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    if (IsOrbiting) {
        Distance -= yoffset * 0.5f;
        // Zoom limits: Min 1.5 (close), Max 15.0 (far)
        if (Distance < 1.5f) Distance = 1.5f;
        if (Distance > 15.0f) Distance = 15.0f;
        updateCameraVectors();
    } else {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }
}

void Camera::updateCameraVectors()
{
    if (IsOrbiting) {
        // Calculate Position based on Yaw/Pitch and Distance from Target
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        
        // In Orbit mode, "Front" points FROM camera TO target.
        // Actually, standard Euler gives vector pointing AWAY from origin?
        // Standard: Yaw -90, Pitch 0 -> Front (0,0,-1).
        // If we want camera to be at Distance, looking at Target.
        // Position = Target - Front * Distance?
        // Or Position = Target + Direction * Distance.
        // Let's assume "Front" is the direction the camera looks.
        
        Front = glm::normalize(front);
        Position = Target - Front * Distance;
        
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    } else {
        // Free mode
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
}
