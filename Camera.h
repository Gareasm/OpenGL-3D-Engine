#pragma once 
#include <glm/glm.hpp>
#include<GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>


class Camera {
public:
	// Position of the camera
	glm::vec3 pos;
	// Direction the camera is looking at
	glm::vec3 front;
	// The 'up' direction of the camera
	glm::vec3 up;
	// The camera's right vector (used for strafing)
	glm::vec3 right;

	// Euler angles
	float yaw;
	float pitch;
	float fov;
	// Camera options
	float movementSpeed;
	float mouseSensitivity;

	Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f, float fov = 45.0f) {
		this->pos = pos;
		this->yaw = yaw;
		this->pitch = pitch;
		this->fov = fov;
		this->worldUp = worldUp;
		this->movementSpeed = 10.5f;
		this->mouseSensitivity = 0.1f;
		updateCameraVectors();
	}

	// Function to calculate the view matrix
	glm::mat4 getViewMatrix() {
		// Use glm::lookAt for a proper 3D view matrix
		return glm::lookAt(pos, pos + front, up);
	}

	// Processes input received from a keyboard
	void processKeyboard(float deltaTime, int direction) {
		float velocity = movementSpeed * deltaTime;
		if (direction == GLFW_KEY_W)
			pos += front * velocity;
		if (direction == GLFW_KEY_S)
			pos -= front * velocity;
		if (direction == GLFW_KEY_A)
			pos -= right * velocity;
		if (direction == GLFW_KEY_D)
			pos += right * velocity;
		if (direction == GLFW_KEY_E)
			pos += up * velocity;
		if (direction == GLFW_KEY_Q)
			pos -= up * velocity;
	}

	// Processes input received from a mouse
	void processMouseMovement(float xoffset, float yoffset) {
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		updateCameraVectors();
	}
	void processGUIMovement(float xoffset, float yoffset) {


		yaw += xoffset;
		pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		updateCameraVectors();
	}

private:
	glm::vec3 worldUp;

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors() {
		// Calculate the new front vector
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newFront.y = sin(glm::radians(pitch));
		newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(newFront);
		// Also re-calculate the right and up vector
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}
};