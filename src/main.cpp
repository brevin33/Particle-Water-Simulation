#include <GLFW/glfw3.h>
#include <Mesmerize/Renderer.h>
#include <Mesmerize/Defaults.h>
#include <filesystem>
#include "camera.h"
#define STB_IMAGE_STATIC
#include "stb_image.h"

constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
constexpr bool fullScreen = false;


// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);


void main() {
	std::cout << std::filesystem::current_path() << std::endl;
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window;
	if (fullScreen) {
		window = glfwCreateWindow(glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
			glfwGetVideoMode(glfwGetPrimaryMonitor())->height, "example",
			glfwGetPrimaryMonitor(), nullptr);
	}
	else {
		window = glfwCreateWindow(WIDTH, HEIGHT, "example", nullptr, nullptr);
	}

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	MZ::setup(window, 1, "../../../include/dep/MesmerizeRenderer/");

	MZ::setDefferedShader("../../../include/dep/MesmerizeRenderer/shaders/defferedFrag.spv", nullptr, 0, nullptr, 0);


	// load cubemap data
	std::vector<stbi_uc> cubemapData;
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("../../../include/dep/MesmerizeRenderer/textures/skybox/right.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../include/dep/MesmerizeRenderer/textures/skybox/left.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../include/dep/MesmerizeRenderer/textures/skybox/top.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../include/dep/MesmerizeRenderer/textures/skybox/bottom.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../include/dep/MesmerizeRenderer/textures/skybox/front.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../include/dep/MesmerizeRenderer/textures/skybox/back.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);

	MZ::TextureID cubemap = MZ::createConstTexture(cubemapData.data(), texWidth, texHeight, MZ::IFSRGBA8, true);

	MZ::setSkybox(cubemap);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), MZ::getRenderWidth() / (float)MZ::getRenderHeight(), 0.1f, 1000.0f);
		proj[1][1] *= -1;
		glm::mat4 cambuf[] = { view, proj };

		MZ::updateCPUMutUniformBuffer(MZ::mainCameraBuffer, cambuf, sizeof(glm::mat4) * 2, 0);

		MZ::drawFrame();
	}

	MZ::cleanup();
	glfwDestroyWindow(window);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}