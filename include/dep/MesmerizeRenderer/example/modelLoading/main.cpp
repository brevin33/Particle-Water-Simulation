#include <GLFW/glfw3.h>
#include <Mesmerize/Renderer.h>
#include "Model.h"
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


void spin(MZ::VertexBufferID instanceBuffer) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	MZ::updateCPUMutVertexBuffer(instanceBuffer, &model, sizeof(glm::mat4));
}

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

	MZ::setup(window,1, "../../../");

	MZ::setDefferedShader("../../../shaders/defferedFrag.spv", nullptr, 0, nullptr, 0);

	//loading shader
	std::vector<MZ::VertexValueType> vertValues = Vertex::getVertexValueTypes();
	MZ::VertexValueType instanceValue = MZ::VTfloat4x4;
	MZ::ShaderStages textureStages = MZ::SSFrag;
	MZ::ShaderStages bufferStages = MZ::SSVert;
	MZ::ShaderID unlitShader = MZ::createShader("../../../shaders/unlitVert.spv", "../../../shaders/unlitFrag.spv",1, &textureStages, 1, &bufferStages, 1, vertValues.data(), vertValues.size(), &instanceValue, 1, MZ::BackCull);

	// load model from file
	Model backpack("../../../models/backpack/backpack.obj");

	// loading model Textures
	std::unordered_map<std::string, MZ::TextureID> pathToTexture;
	std::vector<std::vector<MZ::TextureID>> backpackTextures(backpack.modelTextures.size());
	for (size_t i = 0; i < backpack.modelTextures.size(); i++)
	{
		for (size_t j = 0; j < backpack.modelTextures[i].size(); j++)
		{
			std::string path = backpack.modelTextures[i][j];
			if (pathToTexture.find(path) != pathToTexture.end()) {
				backpackTextures[i].push_back(pathToTexture[path]);
				continue;
			}
			MZ::TextureID textureID = MZ::createConstTexture(path);
			pathToTexture[path] = textureID;
			backpackTextures[i].push_back(textureID);
		}
	}

	//creating material
	std::unordered_map<uint32_t, MZ::MaterialID> assimpMaterialToMZMaterial;
	std::vector<MZ::MaterialID> backpackMaterials;
	for (size_t i = 0; i < backpack.assimpMaterialID.size(); i++)
	{
		uint32_t assimpMat = backpack.assimpMaterialID[i];
		if (assimpMaterialToMZMaterial.find(assimpMat) != assimpMaterialToMZMaterial.end()) {
			backpackMaterials.push_back(assimpMaterialToMZMaterial[assimpMat]);
			continue;
		}
		MZ::MaterialID matID = MZ::createMaterial(unlitShader, backpackTextures[i].data(), backpackTextures[i].size(), &MZ::mainCameraBuffer, 1);
		backpackMaterials.push_back(matID);
		assimpMaterialToMZMaterial[assimpMat] = matID;
	}

	//loading Vertices and Indices
	std::vector<MZ::VertexBufferID> backpackVertexBuffers;
	std::vector<MZ::IndexBufferID> backpackIndexBuffers;
	for (size_t i = 0; i < backpack.modelVertices.size(); i++)
	{
		backpackVertexBuffers.push_back(MZ::createConstVertexBuffer(backpack.modelVertices[i].data(), backpack.modelVertices[i].size(), sizeof(Vertex) * backpack.modelVertices[i].size()));
		backpackIndexBuffers.push_back(MZ::createConstIndexBuffer(backpack.modelIndices[i].data(), backpack.modelIndices[i].size() * sizeof(uint32_t)));
	}

	//creating a instance Buffer
	glm::mat4 modelMatrix = glm::mat4(1);
	MZ::VertexBufferID instanceBuffer = MZ::createCPUMutVertexBuffer(&modelMatrix, 1, sizeof(glm::mat4), sizeof(glm::mat4));

	//telling the renderer to render backpack
	for (size_t i = 0; i < backpack.modelVertices.size(); i++)
	{
		MZ::BoundingSphere boundingSphere(glm::vec3(0.0f,0.0f,0.0f), 2.0f);
		MZ::addRenderObject(backpackMaterials[i], backpackVertexBuffers[i], backpackIndexBuffers[i], instanceBuffer, boundingSphere);
	}

	backpack.unload();

	// load cubemap data
	std::vector<stbi_uc> cubemapData;
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("../../../textures/skybox/right.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../textures/skybox/left.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../textures/skybox/top.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../textures/skybox/bottom.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../textures/skybox/front.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);
	pixels = stbi_load("../../../textures/skybox/back.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	cubemapData.insert(cubemapData.end(), pixels, pixels + (texHeight * texWidth * 4));
	stbi_image_free(pixels);

	uint64_t adisfhaoi = cubemapData.size();

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
		
		spin(instanceBuffer);

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