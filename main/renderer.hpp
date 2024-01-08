#pragma once

#include <glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "defaults.hpp"

#include "camera.hpp"
#include "ObjModel.hpp"
#include "texture.hpp"

#include <learnopengl/model.h>

#include "timer.hpp"

struct ScreenQuad
{
	// renderQuad() renders a 1x1 XY quad in NDC
	// -----------------------------------------
	void create()
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	void draw()
	{
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	unsigned int quadVAO = 0;
	unsigned int quadVBO;
};

namespace
{
	constexpr char const* kWindowTitle = "OpenGL Scene";

	void glfw_callback_error_(int, char const*);

	void glfw_callback_key_(GLFWwindow*, int, int, int, int);

	void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

	void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

class OpenGLRenderer
{
public:
	OpenGLRenderer() {}

	~OpenGLRenderer() {}

	void startUp();

	void loadShaders();
	void loadModels();
	void loadTextures();
	void loadGeometry();
	void loadResources();

	void updateInput(float deltaTime);
	void updateTreeRotation();
	void drawDog();
	void drawDragon();
	void drawMovingLight();
	void drawCrate();
	void drawTree(const glm::vec3& translation, const glm::vec3& scale);
	void drawTable(const glm::vec3& translation, const glm::vec3& scale);
	void drawModel(const ObjModel& objModel, const glm::vec3& translation, const glm::vec3& scale);
	void drawModel();
	void drawScene();

	void updateConstantMovement();

	void updateUniforms();

	void update();

	void render();

	void run();

	void shutdown();

	ObjModel createSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

	GLFWwindow* getWindow() const { return window; }

	int getWindowWidth() const { return windowWidth; }
	int getWindowHeight() const { return windowHeight; }

public:
	int windowWidth = 1920;
	int windowHeight = 1080;

	bool rightMouseButtonDown = false;

	float movingLightRotation = 1.0f;

	bool pauseAnimation = false;

	glm::vec2 lastMousePosition;

	glm::mat4 projection = glm::mat4(1.0f);

	Camera camera = Camera({ 0.0f, 30.0, 100.0f });

	bool firstEnter = false;

	float shininess = 128.0f;

private:
	ShaderProgram defaultShader;
	ShaderProgram quadShader;
	ShaderProgram skyboxShader;

	ObjModel house;
	ObjModel ground;

	ObjModel tree;
	ObjModel trunk;
	ObjModel table;

	ObjModel sphere;

	ObjModel dragon;

	ObjModel crate;

	ObjModel dog;

	Model wooden;

	Model plants;

	Model signature;

	Shader shader;

	ModelTexture houseTexture;
	ModelTexture cubemapTexture;
	ModelTexture groundTexture;
	ModelTexture treeTexture;
	ModelTexture trunkTexture;
	ModelTexture defaultTexture;
	ModelTexture tableTexture;
	ModelTexture crateDiffuseTexture;
	ModelTexture crateSpecularTexture;
	ModelTexture signatureTexture;

	glm::vec3 lightPosition{ -20.0f, 20.0f, 20.0f };

	glm::vec3 movingLightPosition{ 3.0f, 5.0f, 15.0f };

	glm::vec3 movingLightColor{ 1.0f, 0.0f, 0.0f };

	glm::vec3 ambientColor{ 0.1f, 0.1f, 0.1f };
	glm::vec3 lightColor{ 1.0f, 1.0f, 1.0f };
	glm::vec3 directionalLightColor{ 1.0f, 1.0f, 1.0f };

	float dogOffset = 20.0f;

	bool bShowDemoWindow = false;
	bool bShowImGUIWindow = true;
	bool bShowAnotherWindow = false;

	bool enableToonShading = false;

	float headHorizontalAngle = 0.0f;
	float headVerticalAngle = 10.0f;
	float tailHorizontalAngle = 0.0f;
	float tailVerticalAngle = -10.0f;
	float tailWiggleAngle = 0.0f;
	bool tailWiggleDirectionLeft = true;
	float legsAngle = 0.0f;
	bool legsMovementDirectionForward = true;
	bool isMoving = false;

	GLFWwindow* window = nullptr;

	float frameTime = 0.0f;

	float treeRotation = 0.0f;

	float treeRotationDirection = 1.0f;

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
};