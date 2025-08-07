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
#include "model.h"

#include "Object.h"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//OpenGL Course - Create 3D and 2D Graphics With C++


bool GUI = true;
float const height = 1200;
float const width = 1200;





// Camera class

Camera camera(glm::vec3(0.0f, 0.0f, -3.0f));
glm::vec3 objectColor(1.0f, 0.0f, 0.5f);
glm::vec3 lightPos(2.2f, 0.0f, -6.0f);
glm::vec3 lightCol(1.0f, 1.0f, 1.0f);


float lastX = height / 2.0f;
float lastY = width / 2.0f;
bool firstMouse = true;
bool captureMouse = true;

glm::vec3 mod2trans(0.0f, 0.0f, 0.0f);


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



	Shader shaderProgram("default.vert", "default.frag");
	Shader lightShader("light.vert", "light.frag");
	Shader modelShader("model.vert", "model.frag");
	modelShader.Activate();
	


	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;



	

	// load models
	// -----------

	//Model ourModel("models/backpack/backpack.obj");
	Model ourModel2("models/subaru_impreza.glb");
	Model ourModel("models/modern_luxury_wedding_arch_house_building_design.glb");


	//Model ourModel("Aristotle.obj");
	std::cout << "Model loaded with " << ourModel.meshes.size() << " meshes" << std::endl;
	if (ourModel.meshes.empty()) {
		std::cout << "ERROR: Failed to load model or model has no meshes!" << std::endl;
		return -1;
	}

	std::cout << "=== MODEL DEBUG INFO ===" << std::endl;
	std::cout << "Number of meshes: " << ourModel.meshes.size() << std::endl;
	std::cout << "Number of textures loaded: " << ourModel.textures_loaded.size() << std::endl;

	// Debug each mesh
	for (size_t i = 0; i < ourModel.meshes.size(); ++i) {
		auto& mesh = ourModel.meshes[i];
		std::cout << "\nMesh " << i << ":" << std::endl;
		std::cout << "  Vertices: " << mesh.vertices.size() << std::endl;
		std::cout << "  Indices: " << mesh.indices.size() << std::endl;
		std::cout << "  Textures: " << mesh.textures.size() << std::endl;

		// Debug textures for this mesh
		for (size_t j = 0; j < mesh.textures.size(); ++j) {
			auto& tex = mesh.textures[j];
			std::cout << "    Texture " << j << ": " << tex.type
				<< " (ID: " << tex.id << ", Path: '" << tex.path << "')" << std::endl;
		}
	}

	// Test if any textures were loaded at all
	if (ourModel.textures_loaded.empty()) {
		std::cout << "\nWARNING: No textures were loaded from GLB file!" << std::endl;
		std::cout << "This could be normal for GLB files with embedded textures." << std::endl;
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	//ERROS HAPPENING HERE LIGHT IS UNDEFINED AND CUBES SAY INCOMPLETE TYPE NOT ALLOED
	
	LightSrc lightSrc;



	std::vector<Object> objs;


	objs.emplace_back(Cube());  
	objs.emplace_back(Cube());
	objs.emplace_back(Sphere());

	
	


	while (!glfwWindowShouldClose(window)) {
		// Calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		lightSrc.pos = lightPos;



		if (!captureMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		

		// Process keyboard input
		processInput(window, camera, deltaTime);

		// Clear the screen
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		 
		lightSrc.draw(lightShader, camera, lightPos, lightCol, (float) height / width);

		for (Object& obj : objs) {  // Use reference
			obj.draw(shaderProgram, camera, lightPos, lightCol, (float)height / width);
		}
		
		


		//tcube.draw(shaderProgram, camera, lightPos, lightCol);

		// Activate shader and set uniforms
		modelShader.Activate();

		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);

		// Add lighting uniforms
		modelShader.setVec3("lightPos", lightPos);
		modelShader.setVec3("lightColor", lightCol); // White light
		modelShader.setVec3("viewPos", camera.pos);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); // Scale down by 100x
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 6.0f));
		/*
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		*/
		modelShader.setMat4("model", model);
		ourModel.Draw(modelShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, mod2trans);
		model = glm::scale(model, glm::vec3(0.007f, 0.007f, 0.007f)); // Scale down by 100x
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelShader.setMat4("model", model);

		ourModel2.Draw(modelShader);




		if (GUI) {
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

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
			
			
			ImGui::DragFloat3("Model Position", &mod2trans.x, 0.01f, -1000.0f, 1000.0f);

			

			
			
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();



			ImGui::Begin("Light Settings", &GUI);
			ImGui::DragFloat3("Light Position", &lightPos.x, 0.1f, -100.0f, 100.0f);
			ImGui::ColorPicker3("Light Color", &lightCol.r);
			ImGui::End();


			for (int i = 0; i < objs.size(); ++i) {
				
				Object& obj = objs[i];
				std::string frame = "Object " + std::to_string(i + 1) + "##" + std::to_string(i);
		
				ImGui::Begin(frame.c_str(), &GUI);
				ImGui::DragFloat3("Position", &obj.pos.x, 0.1f, -1000.0f, 1000.0f);
				ImGui::DragFloat("Scale", &obj.size, 0.1f, -0.01f, 1000.0f);
				ImGui::DragFloat("Rotate", &obj.angle, 0.1f, 0.0f, 360.0f);
				ImGui::ColorPicker3("Color", &obj.col.r);
				ImGui::End();
			}

			// Render ImGui
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	
	//glDeleteTextures(1, &texture);
	modelShader.Delete();
	//
	glfwDestroyWindow(window);
	glfwTerminate();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}
