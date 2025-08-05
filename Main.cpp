#include<iostream>
#include<vector>
#include<math.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

//OpenGL Course - Create 3D and 2D Graphics With C++
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
bool GUI = true;
float const height = 1200;
float const width = 1200;
// Camera class
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
		this->movementSpeed = 2.5f;
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
Camera camera(glm::vec3(0.0f, 0.0f, -3.0f));
glm::vec3 objectColor(1.0f, 0.0f, 0.5f);
glm::vec3 lightPos(-2.2f, 0.0f, -6.0f);


float lastX = height / 2.0f;
float lastY = width / 2.0f;
bool firstMouse = true;
bool captureMouse = true;
bool animateLight = false;


// Key input polling loop, to be called in the main loop
void processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(deltaTime, GLFW_KEY_W);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(deltaTime, GLFW_KEY_S);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(deltaTime, GLFW_KEY_A);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(deltaTime, GLFW_KEY_D);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.processKeyboard(deltaTime, GLFW_KEY_E);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.processKeyboard(deltaTime, GLFW_KEY_Q);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		captureMouse = false;
		firstMouse = true;
	}
}
// Mouse callback function
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	
	if (!captureMouse) {
		return;
	}
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	camera.processMouseMovement(xoffset, yoffset);
}
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	// Zoom in when scrolling up (positive yoffset)
	if (yoffset > 0) {
		camera.fov -= 2.0f;
	}
	// Zoom out when scrolling down (negative yoffset)
	else {
		camera.fov += 2.0f;
	}
	// Clamp the FoV to a reasonable range
	if (camera.fov < 1.0f) {
		camera.fov = 1.0f;
	}
	if (camera.fov > 90.0f) {
		camera.fov = 90.0f;
	}
}
void generateSphereData(float radius, int sectors, int stacks, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * M_PI / sectors;
	float stackStep = M_PI / stacks;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stacks; ++i)
	{
		stackAngle = M_PI / 2 - i * stackStep;      // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectors+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectors; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			vertices.push_back(nx);
			vertices.push_back(ny);
			vertices.push_back(nz);

			// vertex texture coordinate (s, t)
			
		}
	}

	// generate CCW index list of sphere triangles
	// k1--k1+1
	// |  / |
	// | /  |
	// k2--k2+1
	unsigned int k1, k2;
	for (int i = 0; i < stacks; ++i)
	{
		k1 = i * (sectors + 1);     // beginning of current stack
		k2 = k1 + sectors + 1;      // beginning of next stack

		for (int j = 0; j < sectors; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stacks - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}
}

int main() {
	//initialize glfw
	glfwInit();

	//handshake glfw with proper OpenGL version 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Create window obj with 800x800 dimesnions 
	GLFWwindow* window = glfwCreateWindow(height, width, "OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Capture the cursor for first-person camera controls
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse callback function
	
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	
	

	//load glad to config OpenGL
	gladLoadGL();
	glViewport(0, 0, height, width);
	glEnable(GL_DEPTH_TEST);

	if (GUI) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

	}

	float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	Shader shaderProgram("default.vert", "default.frag");
	Shader lightShader("light.vert", "light.frag");

	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();

	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));
	// Generates Element Buffer Object and links it to indices
	EBO EBO1(indices, sizeof(indices));

	// Links VBO to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3*sizeof(float)));
	


	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	

	std::vector<float> sphereVertices;
	std::vector<unsigned int> sphereIndices;
	generateSphereData(1.0f, 36, 18, sphereVertices, sphereIndices);

	VAO VAO_sphere;
	VAO_sphere.Bind();
	VBO VBO_sphere(sphereVertices.data(), sphereVertices.size() * sizeof(float));
	EBO EBO_sphere(sphereIndices.data(), sphereIndices.size() * sizeof(unsigned int));
	VAO_sphere.LinkAttrib(VBO_sphere, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0); // Position
	VAO_sphere.Unbind();
	VBO_sphere.Unbind();
	EBO_sphere.Unbind();

	//texture
	/*
	stbi_set_flip_vertically_on_load(true);
	int widthImg, heightImg, numColCh;
	unsigned char* bytes = stbi_load("brick.png", &widthImg, &heightImg, &numColCh, 0);

	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytes);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint tex0Uni = glGetUniformLocation(shaderProgram.ID, "tex0");
	shaderProgram.Activate();
	glUniform1i(tex0Uni, 0);
	*/

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
	};


	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		// Calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		if(!captureMouse){
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if (animateLight) {
			float rotationAngle = 0.0001; // Small angle in radians for incremental rotation

			// Store the original y and z values temporarily
			float originalY = lightPos.y;
			float originalZ = lightPos.z;

			// Apply the x-axis rotation formula
			lightPos.y = originalY * cos(rotationAngle) - originalZ * sin(rotationAngle);
			lightPos.z = originalY * sin(rotationAngle) + originalZ * cos(rotationAngle);
		}

		// Process keyboard input
		processInput(window, camera, deltaTime);

		glClearColor(0.09f, 0.25f, 0.5f, 1.0f);
		
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderProgram.Activate();

		
		shaderProgram.setVec3("objectColor", objectColor);

		int lightCol = glGetUniformLocation(shaderProgram.ID, "lightColor");
		glUniform3f(lightCol, 1.0f, 1.0f, 1.0f); //color of our light


		// 1. Start the ImGUI frame

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		projection = glm::perspective(glm::radians(camera.fov), height / width, 0.1f, 100.0f);

		int projLoc = glGetUniformLocation(shaderProgram.ID, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = camera.getViewMatrix();// Get the updated view matrix from the camera
		int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		//glBindTexture(GL_TEXTURE_2D, texture);
		VAO1.Bind();
		shaderProgram.setVec3("lightPos", lightPos);
		shaderProgram.setVec3("viewPos", camera.pos);
		/*
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 10.0f * i + 6;
			//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		*/
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, cubePositions[0]);
		float angle = 10.0f * 0 + 6;
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		


		glm::mat4 sphereModel = glm::mat4(1.0f);
		lightShader.Activate();

		VAO_sphere.Bind();

		
		
		int projLoc1 = glGetUniformLocation(lightShader.ID, "projection");
		glUniformMatrix4fv(projLoc1, 1, GL_FALSE, glm::value_ptr(projection));

		int modelLoc1 = glGetUniformLocation(lightShader.ID, "model");
	

		int viewLoc1 = glGetUniformLocation(lightShader.ID, "view");
		glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(view));
		
		

		sphereModel = glm::translate(sphereModel, lightPos);
		sphereModel = glm::scale(sphereModel, glm::vec3(0.8f));
		glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(sphereModel));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

		/*
		sphereModel = glm::rotate(sphereModel, (float)glfwGetTime() * glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereModel = glm::translate(sphereModel, glm::vec3(0.0f, 4.0f, -5.0f));
		sphereModel = glm::rotate(sphereModel, (float)glfwGetTime() * glm::radians(90.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphereModel));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
		*/

		if (GUI) {
			ImGui::Begin("Camera Debug", &GUI);
			ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera.pos.x, camera.pos.y, camera.pos.z);
			ImGui::Text("Camera Front: (%.2f, %.2f, %.2f)", camera.front.x, camera.front.y, camera.front.z);
			ImGui::SliderFloat("FOV", &camera.fov, 1, 90);
			if (ImGui::SliderFloat("Yaw", &camera.yaw, -180.0f, 180.0f)) {
				camera.processGUIMovement(camera.yaw, 0);
			}
			if (ImGui::SliderFloat("Pitch", &camera.pitch, -89.0f, 89.0f)) {
				camera.processGUIMovement(0, camera.pitch);
			}
			if (ImGui::Button("Capture Mouse"))
			{
				captureMouse = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			ImGui::Checkbox("Animate Light", &animateLight);
			ImGui::DragFloat3("Light Position", &lightPos.x, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat3("Object Color", &objectColor.x, 0.01f, 0.0f, 1.0f);
			ImGui::End();
		}

		// 3. Render the ImGUI data
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	//glDeleteTextures(1, &texture);
	shaderProgram.Delete();
	//
	glfwDestroyWindow(window);
	glfwTerminate();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}
