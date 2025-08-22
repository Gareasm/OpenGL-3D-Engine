#pragma once
#include "Object.h"
#include<math.h>
/*


class Engine{
public:
	//Globals 
	bool GUI = true;
	float const height = 1200;
	float const width = 1200;

	// Camera class
	Camera camera(glm::vec3(0.0f, 0.0f, -3.0f));

	//Light information
	glm::vec3 lightPos(7.0f, 7.0f, 0.0f);
	glm::vec3 lightCol(1.0f, 1.0f, 1.0f);

	//Mouse callback globals
	float lastX = height / 2.0f;
	float lastY = width / 2.0f;
	bool firstMouse = true;
	bool captureMouse = true;


	GLFWwindow* window;

	Engine() {

	}

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
	// Mouse callback function-pos
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
	// Mouse callback function-scroll
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

	void init() {
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


		stbi_set_flip_vertically_on_load(true);

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
	}
	



};

*/