/**
* This is the main code of the project.
* @file space.cpp
* @author Leonardo Andrade, Cristian Bastidas, Ricardo Erazo, Jhosel Guill�n
* @version 1.0 15/1/20
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <math.h>
#include <iostream>
#include <fstream>


// #define STB_IMAGE_IMPLEMENTATION 
// #include <learnopengl/stb_image.h>

const float PI = 3.14159265359f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

unsigned int loadTexture(const char* path);
unsigned int loadCubemap(const char* faces[]);
void loadNumbers(char const* path, float data[]);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
int cameraSpeed = 1;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 shipMovement(float a, float b, float j, float k, glm::vec3 init, float t, float fase) {
	return init + glm::vec3(cos(a * t) - pow(cos(b * t), j), sin(a * t) - pow(sin(b * t), k), sin(t) - fase);
}

int main() {
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Space", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// Build and compile shaders
	Shader shader("shaders/cubemaps.vs", "shaders/cubemaps.fs");
	Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
	Shader ourShader("shaders/shader_exercise16_mloading.vs", "shaders/shader_exercise16_mloading.fs");

	// Load 3D Models
	Model mars("model/mars/scene.gltf");
	Model ring("model/haloring/scene.gltf");
	Model charon("model/charon/scene.gltf");
	Model pelican("model/pelican/scene.gltf");
	Model phantom("model/phantom/scene.gltf");
	Model precursors("model/precursors/scene.gltf");

	// Preloaded instace positions for ships
	float xmove[50], ymove[50], zmove[50], abjk[200];
	loadNumbers("data/xmove.txt", xmove);
	loadNumbers("data/ymove.txt", ymove);
	loadNumbers("data/zmove.txt", zmove);
	loadNumbers("data/abjk.txt", abjk);

	// SkyBox Vertices
	float skyboxVertices[108];
	loadNumbers("data/skybox.txt", skyboxVertices);

	// SkyBox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	// Load textures
	// unsigned int cubeTexture = loadTexture("textures/container.jpg");
	// std::cout << "Textures loaded" << std::endl;

	// SkyBox cube faces
	const char* faces[6]{
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/front.jpg",
		"textures/skybox/back.jpg"
	};

	// Load cubemap textures
	unsigned int cubemapTexture = loadCubemap(faces);

	// Shader configuration
	shader.use();
	shader.setInt("texture1", 0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// Render Loop
	while (!glfwWindowShouldClose(window)) {
		// Frame
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		processInput(window);

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Camera Scene
		ourShader.use();
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		// Mars
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.0f, 9.5f, -9.0f));
		model = glm::scale(model, glm::vec3(11.0f, 11.0f, 11.0f));
		model = glm::rotate(model, currentFrame / 100, glm::vec3(0.0f, 1.0f, 0.0f));
		ourShader.setMat4("model", model);
		mars.Draw(ourShader);

		// Ring
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.2f, 9.5f, -7.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, -0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 0.08f, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, currentFrame / 100, glm::vec3(1.0f, 0.0f, 0.0f));
		ourShader.setMat4("model", model);
		ring.Draw(ourShader);

		// Charon
		model = glm::mat4(1.0f);
		glm::vec3 init = glm::vec3(15.0f, 9.5f, -7.5f);
		float t = currentFrame / 10.0f;
		model = glm::translate(model, shipMovement(1, 2, 2, 1, init, currentFrame / 50.0f, 0.0f));
		model = glm::scale(model, glm::vec3(-0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, -PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
		ourShader.setMat4("model", model);
		charon.Draw(ourShader);

		for (int i = 0; i < 50; i++) {
			// Pelican
			model = glm::mat4(1.0f);
			init = glm::vec3(15.0f - xmove[i], 9.5f - ymove[i], -6.5f - zmove[i]);
			model = glm::translate(
				model,
				shipMovement(
					abjk[i],
					abjk[50 + i],
					abjk[100 + i],
					abjk[150 + i],
					init,
					currentFrame / 100.0f,
					-PI / 2
				)
			);
			model = glm::scale(model, glm::vec3(0.0001f, 0.0001f, 0.0001f));
			model = glm::rotate(model, -PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
			ourShader.setMat4("model", model);
			pelican.Draw(ourShader);

			// Phantom
			model = glm::mat4(1.0f);
			init = glm::vec3(2.0f + xmove[i], 9.5f - ymove[i], -5.5f - zmove[i]);
			model = glm::translate(
				model,
				shipMovement(
					abjk[i],
					abjk[50 + i],
					abjk[100 + i],
					abjk[150 + i],
					init,
					currentFrame / 100.0f,
					0.0f
				)
			);
			model = glm::scale(model, glm::vec3(0.0005f, 0.0005f, 0.0005f));
			model = glm::rotate(model, PI / 2, glm::vec3(0.0f, 1.0f, 0.0f));
			ourShader.setMat4("model", model);
			phantom.Draw(ourShader);
		}

		// Precursors
		model = glm::mat4(1.0f);
		init = glm::vec3(2.0f, 9.5f, -10.0f);
		t = currentFrame / 10.0f;
		model = glm::translate(model, shipMovement(2, 1, 2, 1, init, currentFrame / 50.0f, -PI / 2));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		model = glm::rotate(model, -PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
		ourShader.setMat4("model", model);
		precursors.Draw(ourShader);

		// Draw SkyBox
		// Change depth function so depth test passes when values are equal to depth buffer's content
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();

		// Remove translation from the view matrix
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);

		// SkyBox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// Set depth function back to default
		glDepthFunc(GL_LESS);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// De-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwTerminate();
	return 0;
}

/**
* Process all input
*
* Query GLFW whether relevant keys are pressed/released this frame and react accordingly.
*/
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, cameraSpeed * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, cameraSpeed * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, cameraSpeed * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, cameraSpeed * deltaTime);

	// Use LEFT CTRL to move fast
	cameraSpeed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 4 : 1;
}


/**
* Window callback
*
* glfw: whenever the window size changed (by OS or user resize) this callback function executes
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// Make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


/**
* Mouse move callback
*
* Whenever the mouse move, this callback is called.
*/
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;

	// Reversed since y-coordinates go from bottom to top
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


/**
* Mouse scroll callback
*
* Whenever the mouse scroll wheel scrolls, this callback is called.
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


/**
* Loads float numbers from a file
*
* Loads float numbers from a TXT file and
* stores it on a new array.
*
* @param path Numbers file path (string)
* @param data[] The array for storing the numbers.
*/
void loadNumbers(char const* path, float data[]) {
	std::fstream skyboxData(path, std::ios_base::in);
	float number;
	int i = 0;
	while (skyboxData >> number) {
		data[i] = number;
		i++;
	}
	skyboxData.close();
	cout << i << " floats have been loaded." << std::endl;
}


/**
* Loads a texture
*
* Loads a texture from its path.
*
* @param path Texture path (string)
* @return The texture ID
*/
unsigned int loadTexture(char const* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	std::cout << "Texture loaded." << std::endl;
	return textureID;
}


/**
* Loads a cubemap texture
*
* You must use 6 individual texture faces and you can generate it from a HDR image.
* Then you have to convert the HDR image into a CubeMap jpg. Finally split each face
* into individual images.
*
*	order:
*	    +X (right)
*	    -X (left)
*	    +Y (top)
*	    -Y (bottom)
*	    +Z (front)
*	    -Z (back)
*
* @param faces Faces texture paths (array of size 6)
* @return The texture ID
*/
unsigned int loadCubemap(const char* faces[]) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++) {
		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	std::cout << "Cubemap textures loaded." << std::endl;
	return textureID;
}