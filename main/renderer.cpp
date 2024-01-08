#include "renderer.hpp"

namespace
{
	void glfw_callback_error_(int aErrNum, char const* aErrDesc)
	{
		std::fprintf(stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow* aWindow, int aKey, int, int aAction, int)
	{
		auto app = reinterpret_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(aWindow));

		if (GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction)
		{
			glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
			return;
		}

		if (GLFW_KEY_EQUAL == aKey && GLFW_PRESS == aAction)
		{
			app->movingLightRotation += 1.0f;
		}

		if (GLFW_KEY_MINUS == aKey && GLFW_PRESS == aAction)
		{
			app->movingLightRotation -= 1.0f;
		}

		if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
		{
			//enableToonShading = !enableToonShading;
			app->pauseAnimation = !app->pauseAnimation;
		}
	}

	void frameBufferResizeCallback(GLFWwindow* inWindow, int width, int height)
	{
		auto app = reinterpret_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(inWindow));

		if (width > 0 && height > 0)
		{
			app->windowWidth = width;
			app->windowHeight = height;

			app->projection = glm::perspective(glm::radians(45.0f), static_cast<float>(app->windowWidth) / app->windowHeight, 0.1f, 100.0f);
		}
	}

	void mouseMoveCallback(GLFWwindow* inWindow, double xpos, double ypos)
	{
		auto app = reinterpret_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(inWindow));

		auto xOffset = static_cast<float>(xpos - app->lastMousePosition.x);
		auto yOffset = static_cast<float>(app->lastMousePosition.y - ypos);

		if (app->firstEnter)
		{
			app->lastMousePosition.x = static_cast<float>(xpos);
			app->lastMousePosition.y = static_cast<float>(ypos);
			app->firstEnter = false;
		}

		app->lastMousePosition.x = static_cast<float>(xpos);
		app->lastMousePosition.y = static_cast<float>(ypos);

		if (app->rightMouseButtonDown)
		{
			app->camera.ProcessMouseMovement(xOffset, yOffset);
		}
	}

	void mouseButtonCallback(GLFWwindow* inWindow, int button, int action, int mods)
	{
		auto app = reinterpret_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(inWindow));

		if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
		{
			app->rightMouseButtonDown = true;
		}

		if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
		{
			app->rightMouseButtonDown = false;
		}
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if (window)
			glfwDestroyWindow(window);
	}
}

void OpenGLRenderer::startUp()
{
	// Initialize GLFW
	if (GLFW_TRUE != glfwInit())
	{
		char const* msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwInit() failed with '%s' (%d)", msg, ecode);
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	//GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback(&glfw_callback_error_);

	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_DEPTH_BITS, 24);

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#	endif // ~ !NDEBUG

	window = glfwCreateWindow(
		windowWidth,
		windowHeight,
		kWindowTitle,
		nullptr, nullptr
	);

	if (!window)
	{
		char const* msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwCreateWindow() failed with '%s' (%d)", msg, ecode);
	}

	//GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	glfwSetKeyCallback(window, &glfw_callback_key_);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	// Set up drawing stuff
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if (!gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress))
		throw Error("gladLoaDGLLoader() failed - cannot load GL API!");

	std::printf("RENDERER %s\n", glGetString(GL_RENDERER));
	std::printf("VENDOR %s\n", glGetString(GL_VENDOR));
	std::printf("VERSION %s\n", glGetString(GL_VERSION));
	std::printf("SHADING_LANGUAGE_VERSION %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here

	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize(window, &iwidth, &iheight);

	glViewport(0, 0, iwidth, iheight);

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
}

void OpenGLRenderer::loadShaders()
{
	defaultShader = ShaderProgram{ {{GL_VERTEX_SHADER, "./assets/shaders/default.vert"},
							{GL_FRAGMENT_SHADER, "./assets/shaders/default.frag"}} };

	glUseProgram(defaultShader.programId());

	defaultShader.setInt("albedoMap", 0);
	defaultShader.setInt("secondTexture", 1);

	quadShader = ShaderProgram{ {{GL_VERTEX_SHADER, "./assets/shaders/quad.vert"},
								 {GL_FRAGMENT_SHADER, "./assets/shaders/quad.frag"}} };

	skyboxShader = ShaderProgram{ {{GL_VERTEX_SHADER, "./assets/shaders/skybox.vert"},
								   {GL_FRAGMENT_SHADER, "./assets/shaders/skybox.frag"}} };

	shader = Shader("./assets/shaders/default.vert", "./assets/shaders/default.frag");
}

void OpenGLRenderer::loadModels()
{
	if (house.load("./assets/models/House.obj"))
	{
		house.createBuffers();
	}

	if (ground.load("./assets/models/Plane.obj"))
	{
		ground.createBuffers();
	}

	if (tree.load("./assets/models/tree.obj"))
	{
		tree.createBuffers();
	}

	if (trunk.load("./assets/models/trunk.obj"))
	{
		trunk.createBuffers();
	}

	if (table.load("./assets/models/Table.obj"))
	{
		table.createBuffers();
	}

	sphere = createSphere(1.0f, 32, 32);

	sphere.createBuffers();

	if (dragon.load("./assets/models/dragon.obj"))
	{
		dragon.createBuffers();
	}

	if (crate.load("./assets/models/cube.obj"))
	{
		crate.createBuffers();
	}

	if (dog.load("./assets/models/dog/12228_Dog_v1_L2.obj"))
	{
		dog.createBuffers();
	}

	wooden = Model("./assets/models/wooden/wooden.obj");

	plants = Model("./assets/models/plants/plants.obj");

	signature = Model("./assets/models/signature.obj");
}

void OpenGLRenderer::loadTextures()
{
	houseTexture.load("./assets/textures/aiStandardSurface1_baseColor.png");
	groundTexture.load("./assets/textures/CartoonGrass.jpg");

	treeTexture.load("./assets/textures/tree.png");
	trunkTexture.load("./assets/textures/trunk.png");
	defaultTexture.load("./assets/textures/default.png");

	crateDiffuseTexture.load("./assets/textures/CrateDiffuse.png");
	crateSpecularTexture.load("./assets/textures/CrateSpecular.png");
	signatureTexture.load("./assets/textures/signature.jpg");

	tableTexture.load("./assets/textures/Albedo_4K__slxoejhp.jpg");

	std::vector<std::string> faces =
	{
		"./assets/textures/skybox/CloudyCrown_Midday_Right.png",
		"./assets/textures/skybox/CloudyCrown_Midday_Left.png",
		"./assets/textures/skybox/CloudyCrown_Midday_Up.png",
		"./assets/textures/skybox/CloudyCrown_Midday_Down.png",
		"./assets/textures/skybox/CloudyCrown_Midday_Front.png",
		"./assets/textures/skybox/CloudyCrown_Midday_Back.png"
	};

	cubemapTexture.loadCubemap(faces);
}

void OpenGLRenderer::loadGeometry()
{
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void OpenGLRenderer::loadResources()
{
	loadShaders();
	loadModels();
	loadTextures();

	loadGeometry();
}

ObjModel OpenGLRenderer::createSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
	ObjModel model;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	MeshVertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f);
	MeshVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);

	model.mesh.vertices.push_back(topVertex);

	float phiStep = glm::pi<float>() / stackCount;
	float thetaStep = 2.0f * glm::pi<float>() / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32_t i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32_t j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			MeshVertex vertex;

			// spherical to cartesian
			vertex.position.x = radius * std::sinf(phi) * std::cosf(theta);
			vertex.position.y = radius * std::cosf(phi);
			vertex.position.z = radius * std::sinf(phi) * std::sinf(theta);

			vertex.normal = glm::normalize(vertex.position);

			vertex.texcoord.x = theta / (glm::pi<float>() * 2.0f);
			vertex.texcoord.y = phi / glm::pi<float>();

			model.mesh.vertices.push_back(vertex);
		}
	}

	model.mesh.vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32_t i = 1; i <= sliceCount; ++i)
	{
		model.mesh.indices.push_back(0);
		model.mesh.indices.push_back(i + 1);
		model.mesh.indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			model.mesh.indices.push_back(baseIndex + i * ringVertexCount + j);
			model.mesh.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			model.mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			model.mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			model.mesh.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			model.mesh.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32_t southPoleIndex = (uint32_t)model.mesh.vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		model.mesh.indices.push_back(southPoleIndex);
		model.mesh.indices.push_back(baseIndex + i);
		model.mesh.indices.push_back(baseIndex + i + 1);
	}

	return model;
}

void OpenGLRenderer::updateInput(float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(UP, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(DOWN, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera.SetMaxSpeed();
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		camera.SetMinSpeed();
	}
	else
	{
		camera.SetNormalSpeed();
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		dogOffset += 0.02f;
		isMoving = true;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		dogOffset -= 0.02f;
		isMoving = true;
	}
}

void OpenGLRenderer::updateTreeRotation()
{
	treeRotation += 0.1f * treeRotationDirection;

	if (treeRotation > 5.0f || treeRotation < -5.0f)
	{
		treeRotationDirection = -treeRotationDirection;
	}
}

void OpenGLRenderer::drawDog()
{
	// Torso
	trunkTexture.use();

	glm::vec3 translation = { 0.0f, 1.5f, dogOffset };
	glm::vec3 scale = { 0.6f, 0.6f, 1.2f };

	auto model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	sphere.draw();

	// Legs
	translation = { -0.3f, 0.75f, -0.6f + dogOffset };
	scale = { 0.5f * 0.3f, 2.0f * 0.3f, 0.5f * 0.3f };

	model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(legsAngle), { 1.0f, 0.0f, 0.0f });
	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	sphere.draw();

	translation = { 0.3f, -2.5f * 0.3f + 1.5f, -0.6f + dogOffset };
	scale = { 0.5f * 0.3f, 0.6f, 0.5f * 0.3f };

	model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(-legsAngle), { 1.0f, 0.0f, 0.0f });
	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	sphere.draw();

	translation = { 0.3f, -2.5f * 0.3f + 1.5f, 2.0 * 0.3f + dogOffset };
	scale = { 0.5f * 0.3f, 0.6f, 0.5f * 0.3f };

	model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(legsAngle), { 1.0f, 0.0f, 0.0f });
	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	sphere.draw();

	translation = { -0.3f, -2.5f * 0.3f + 1.5f, 0.6 + dogOffset };
	scale = { 0.5f * 0.3f, 0.6f, 0.5f * 0.3f };

	model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(-legsAngle), { 1.0f, 0.0f, 0.0f });
	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	sphere.draw();

	// Tail
	model = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f + 1.5f, -3.8f * 0.3f + dogOffset });
	model = glm::rotate(model, glm::radians(-30.0f), { 1.0f, 0.0f, 0.0f });
	model = glm::rotate(model, glm::radians(tailVerticalAngle), { 1.0f, 0.0f, 0.0f });
	model = glm::rotate(model, glm::radians(tailHorizontalAngle), { 0.0f, 1.0f, 0.0f });
	model = glm::rotate(model, glm::radians(tailWiggleAngle), { 0.0f, 1.0f, 0.0f });
	model = glm::scale(model, { 0.5f * 0.3f, 0.5f * 0.3f, 1.8f * 0.3f });

	defaultShader.setMat4("model", model);

	sphere.draw();

	// Head
	model = glm::translate(glm::mat4(1.0f), { 0.0f, 2.5f * 0.3f + 1.5f, 3.0f * 0.3f + dogOffset });
	model = glm::scale(model, { 1.5f * 0.3f, 1.55f * 0.3f, 1.6f * 0.3f });

	defaultShader.setMat4("model", model);

	sphere.draw();

	// Nose
	model = glm::translate(glm::mat4(1.0f), { 0.0f, 2.2f * 0.3f + 1.5f, 4.2f * 0.3f + dogOffset });
	model = glm::scale(model, { 0.8f * 0.3f, 0.5f * 0.3f, 1.5f * 0.3f });

	defaultShader.setMat4("model", model);

	sphere.draw();

	// Ear
	model = glm::translate(glm::mat4(1.0f), { -0.8f * 0.3f, 3.8f * 0.3f + 1.5f, 2.6f * 0.3f + dogOffset });
	model = glm::scale(model, { 0.5f * 0.3f, 0.3f, 0.5f * 0.3f });

	defaultShader.setMat4("model", model);

	sphere.draw();

	model = glm::translate(glm::mat4(1.0f), { 0.8f * 0.3f, 3.8f * 0.3f + 1.5f, 2.6f * 0.3f + dogOffset });
	model = glm::scale(model, { 0.5f * 0.3f, 0.3f, 0.5f * 0.3f });

	defaultShader.setMat4("model", model);

	sphere.draw();

	model = glm::translate(glm::mat4(1.0f), { 0.5f * 0.3f, 3.0f * 0.3f + 1.5f, 4.4f * 0.3f + dogOffset });
	model = glm::scale(model, { 0.25f * 0.3f, 0.25f * 0.3f, 0.25f * 0.3f });

	defaultShader.setMat4("model", model);
	defaultShader.setVec3("color", { 0.0f, 0.0f, 0.0f });

	sphere.draw();

	model = glm::translate(glm::mat4(1.0f), { -0.5f * 0.3f, 3.0f * 0.3f + 1.5f, 4.4f * 0.3f + dogOffset });
	model = glm::scale(model, { 0.25f * 0.3f, 0.25f * 0.3f, 0.25f * 0.3f });

	defaultShader.setMat4("model", model);
	defaultShader.setVec3("color", { 0.0f, 0.0f, 0.0f });

	sphere.draw();

	defaultShader.setVec3("color", { 1.0f, 1.0f, 1.0f });
}

void OpenGLRenderer::drawDragon()
{
	defaultTexture.use();

	glm::vec3 translation = { -2.0f, 2.5f, 15.0f };

	auto model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::scale(model, glm::vec3(0.5f));

	defaultShader.setMat4("model", model);

	dragon.draw();

	translation = { 1.0f, 2.5f, 15.0f };

	model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::scale(model, glm::vec3(0.5f));

	defaultShader.setMat4("model", model);

	defaultShader.setBool("enableSpecular", true);

	dragon.draw();

	defaultShader.setBool("enableSpecular", false);
}

void OpenGLRenderer::drawMovingLight()
{
	defaultTexture.use();

	auto rx0 = 0.0f;
	auto rz0 = 20.0f;

	auto x = movingLightPosition.x;
	auto z = movingLightPosition.z;

	auto a = glm::radians(movingLightRotation);

	auto x0 = (x - rx0) * std::cos(a) - (z - rz0) * std::sin(a) + rx0;
	auto z0 = (x - rx0) * std::sin(a) + (z - rz0) * std::cos(a) + rz0;

	if (pauseAnimation)
	{
		movingLightPosition.x = x0;
		movingLightPosition.z = z0;
	}

	auto model = glm::translate(glm::mat4(1.0f), movingLightPosition);
	model = glm::scale(model, { 0.5f, 0.5f, 0.5f });

	defaultShader.setMat4("model", model);
	defaultShader.setVec3("color", movingLightColor);

	sphere.draw();

	defaultShader.setVec3("color", glm::vec3(1.0f));

	model = glm::translate(glm::mat4(1.0f), lightPosition);

	defaultShader.setMat4("model", model);
	defaultShader.setVec3("color", lightColor);

	sphere.draw();
}

void OpenGLRenderer::drawCrate()
{
	glActiveTexture(GL_TEXTURE0);

	crateDiffuseTexture.use();

	glActiveTexture(GL_TEXTURE1);

	crateSpecularTexture.use();

	glm::vec3 translation = { -5.0f, 1.0f, 20.0f };

	auto model = glm::translate(glm::mat4(1.0f), translation);

	defaultShader.setMat4("model", model);

	defaultShader.setBool("enableSpecular", true);
	defaultShader.setBool("useSpecularMap", true);

	crate.draw();

	defaultShader.setBool("enableSpecular", false);
	defaultShader.setBool("useSpecularMap", false);
}

void OpenGLRenderer::drawTree(const glm::vec3& translation, const glm::vec3& scale)
{
	treeTexture.use();

	auto model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(treeRotation), glm::vec3(0.0f, 0.0f, 1.0f));

	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	tree.draw();

	trunkTexture.use();

	trunk.draw();
}

void OpenGLRenderer::drawTable(const glm::vec3& translation, const glm::vec3& scale)
{
	tableTexture.use();

	auto model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	table.draw();
}

void OpenGLRenderer::drawModel(const ObjModel& objModel, const glm::vec3& translation, const glm::vec3& scale)
{
	auto model = glm::translate(glm::mat4(1.0f), translation);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	model = glm::scale(model, scale);

	defaultShader.setMat4("model", model);

	objModel.draw();
}

void OpenGLRenderer::drawModel()
{
	// draw skybox as last
	//glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	glDepthMask(GL_FALSE);
	skyboxShader.use();

	// remove translation from the view matrix
	auto view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	skyboxShader.setMat4("view", view);

	glBindVertexArray(skyboxVAO);
	cubemapTexture.use();
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//glDepthFunc(GL_LESS); // set depth function back to default
	glDepthMask(GL_TRUE);

	defaultShader.use();

	drawTree({ -20.0f, 0.0f, -10.0f }, { 2.0f, 2.0f, 2.0f });

	drawTree({ 20.0f, 0.0f, -10.0f }, { 2.0f, 2.0f, 2.0f });

	drawTree({ -20.0f, 0.0f, 10.0f }, { 2.0f, 2.0f, 2.0f });

	drawTree({ 20.0f, 0.0f, 10.0f }, { 2.0f, 2.0f, 2.0f });

	drawTable({ 0.0f, 2.5f, 15.0f }, { 1.0f, 1.0f, 1.0f });

	auto model = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f));

	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	defaultShader.setMat4("model", model);
	houseTexture.use();

	house.draw();

	groundTexture.use();

	ground.draw();
}

void OpenGLRenderer::drawScene()
{
	glClearColor(0.4f, 0.6f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	drawModel();

	drawDog();

	drawDragon();

	drawMovingLight();

	drawCrate();

	wooden.Draw(shader);

	glEnable(GL_BLEND);

	auto model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
	model = glm::scale(model, glm::vec3(0.01f));

	shader.setMat4("model", model);

	plants.Draw(shader);

	glDisable(GL_BLEND);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 10.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));

	shader.setMat4("model", model);

	signature.Draw(shader);
}

void OpenGLRenderer::updateConstantMovement()
{
	if (tailWiggleAngle > 8 || tailWiggleAngle < -8)
	{
		tailWiggleDirectionLeft = !tailWiggleDirectionLeft;
	}
	if (tailWiggleDirectionLeft)
	{
		tailWiggleAngle += 1.7f;
	}
	else {
		tailWiggleAngle -= 1.7f;
	}
	if (isMoving) {
		if (legsAngle > 20 || legsAngle < -20)
		{
			legsMovementDirectionForward = !legsMovementDirectionForward;
		}
		if (legsMovementDirectionForward)
		{
			legsAngle += 6.0;
		}
		else {
			legsAngle -= 6.0;
		}
		isMoving = false;
	}
}

void OpenGLRenderer::updateUniforms()
{
	defaultShader.use();

	auto model = glm::mat4(1.0f);
	auto view = camera.GetViewMatrix();
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(windowWidth) / windowHeight, 0.1f, 500.0f);

	defaultShader.setMat4("model", model);
	defaultShader.setMat4("view", view);
	defaultShader.setMat4("projection", projection);

	defaultShader.setVec3("lightPosition", lightPosition);
	defaultShader.setVec3("movingLightPosition", movingLightPosition);
	defaultShader.setVec3("ambientColor", ambientColor);
	defaultShader.setVec3("lightColor", lightColor);
	defaultShader.setVec3("movingLightColor", movingLightColor);
	defaultShader.setVec3("directionalLightColor", directionalLightColor);
	defaultShader.setVec3("viewPosition", camera.Position);
	defaultShader.setFloat("shininess", shininess);
	defaultShader.setBool("enableToonShading", enableToonShading);

	skyboxShader.use();

	skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);

	shader.use();

	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	shader.setVec3("lightPosition", lightPosition);
	shader.setVec3("movingLightPosition", movingLightPosition);
	shader.setVec3("ambientColor", ambientColor);
	shader.setVec3("lightColor", lightColor);
	shader.setVec3("movingLightColor", movingLightColor);
	shader.setVec3("directionalLightColor", directionalLightColor);
	shader.setVec3("viewPosition", camera.Position);
	shader.setFloat("shininess", shininess);
	shader.setBool("enableToonShading", enableToonShading);
}

void OpenGLRenderer::update()
{

}

void OpenGLRenderer::render()
{

}

void OpenGLRenderer::run()
{

}

void OpenGLRenderer::shutdown()
{
	if (window)
		glfwDestroyWindow(window);
}

