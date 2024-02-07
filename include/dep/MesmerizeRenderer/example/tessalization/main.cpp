#include <GLFW/glfw3.h>
#include <Mesmerize/Renderer.h>
#include <Mesmerize/Defaults.h>
#include "../modelLoading/camera.h"
constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
using namespace MZ;

struct Vertex {
	glm::vec3 pos;
	glm::vec2 color;

	static std::array<VertexValueType, 2> getVertexValues() {
		std::array<VertexValueType, 2> vertValues;
		vertValues[0] = VTfloat3;
		vertValues[1] = VTfloat2;
		return vertValues;
	}
};

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 0.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

TextureID hightMap;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void addTessalizedPlane();

void main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tessalization Example", nullptr, nullptr);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetCursorPosCallback(window, mouse_callback);
	MZ::setup(window, 1, "../../../");
	MZ::setDefferedShader("../../../shaders/defferedFrag.spv", nullptr, 0, nullptr, 0);

	hightMap = createConstTexture("../../../textures/iceland_heightmap.png");

	addTessalizedPlane();

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
}


void makePlane(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
	int res = 60;
	int width = 600;
	int height = 600;
	std::array<std::array<int, 2>, 4> quad;

	for (int i = -height; i < height; i += (height*2)/res)
	{
		for (int j = -width; j < width; j += (width*2)/res)
		{
			Vertex v;
			v.pos = glm::vec3(i,0,j);
			v.color = glm::vec2((i+height)/(float)(height*2), (j+width)/(float)(width*2));
			vertices.push_back(v);
		}
	}

	for (int j = 0; j < res-1; j++)
	{
		for (int i = 0; i < res-1; i++)
		{
			indices.push_back(i + j * res);
			indices.push_back((i+1) + j * res);
			indices.push_back(i + (j+1) * res);
			indices.push_back((i+1) + (j+1) * res);
		}
	}
}


void addTessalizedPlane() {
	std::vector<Vertex> plane;
	std::vector<uint32_t> indices;
	makePlane(plane, indices);
	VertexBufferID planeVertexBuffer = createConstVertexBuffer(plane.data(), plane.size(), plane.size() * sizeof(Vertex));
	IndexBufferID planeIndexBuffer = createConstIndexBuffer(indices.data(), indices.size() * sizeof(uint32_t));
	ShaderStages stages[2] = {SSTessCon | SSTessEval, SSTessCon | SSTessEval };
	ShaderStages stagesTexture[1] = {SSTessEval};
	ShaderID tessShader = createShader("../../../shaders/tessalizationVert.spv", "../../../shaders/tessalizationFrag.spv", "../../../shaders/tessalizationTesc.spv", 
		"../../../shaders/tessalizationTese.spv", 1, stagesTexture, 1, stages ,2, Vertex::getVertexValues().data(), Vertex::getVertexValues().size(), nullptr, 0, FrontCull);
	glm::mat4 model = glm::mat4(1);
	UniformBufferID modelBuffer = createCPUMutUniformBuffer(&model, sizeof(glm::mat4), sizeof(glm::mat4));
	std::array<UniformBufferID, 2> tessUniformBuffers = { mainCameraBuffer , modelBuffer };
	MaterialID tessMaterial = createMaterial(tessShader, &hightMap, 1, tessUniformBuffers.data(), tessUniformBuffers.size());
	RenderObjectID terrain = addRenderObject(tessMaterial, planeVertexBuffer, planeIndexBuffer);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
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
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}