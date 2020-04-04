#include "GLCommon.h"

#include "globalStuff.h"
#include "util/tools.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <stdlib.h>
#include <iostream>
#include <map>		
#include <sstream>
#include <iomanip>
#include <float.h>
#include <string>
#include "cModelLoader.h"			
#include "cVAOManager.h"
#include "cGameObject.h"
#include "cShaderManager.h"
#include "TextureManager/cBasicTextureManager.h"
#include "JSONLoader.h"// JSON Stuff
#include "JsonState.h"
#include "cPhysics.h"
#include "cLowPassFilter.h"
#include "cAABB/PhysicsAABBStuff.h"
#include "DebugRenderer/cDebugRenderer.h"	
#include "cLight.h"// Used to visualize the attenuation of the lights...
#include "cFlyCamera/cFlyCamera.h"
#include "skybox/skybox.h"
#include "GFLW_callbacks.h"// Keyboard, error, mouse, etc. are now here
#include "cFBO/cFBO.h"
#include "cAnimatedPlayer/cAnimatedPlayer.h"
#include "cMeshMap.h"
#include "zBMPLoader/BMPLoader.h" // ############ PATH FINDING ##############
#include "cGraph.h"
#include "sPathFinder.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "ImGUI_utils.h"
#include "PhysicsUtils.h"
#include "PhysicsDebug.h"

cFBO* pTheFBO = NULL;

cFlyCamera* g_pFlyCamera = NULL;
cGameObject* pSkyBox = new cGameObject();
glm::vec3 cameraEye = glm::vec3(0.0f, 50.0f, 100.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 50.0f, 0.0f);
glm::vec3 visionVector = glm::normalize(cameraTarget - cameraEye);
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

// This is a "normalized" direction (i.e. the length is 1.0f)
glm::vec3 sexyLightSpotDirection = glm::vec3(0.0f, -1.0f, 0.0f);

bool everythingWireFrame = false;
bool debugger = false;
std::string console;
GLuint shaderProgID;
cVAOManager* pTheVAOManager = new cVAOManager();
cModelLoader* pTheModelLoader = new cModelLoader();
cDebugRenderer* pDebugRenderer = new cDebugRenderer();
cPhysics* pPhysic = new cPhysics();
cBasicTextureManager* pTextureManager = NULL;
extern std::map<unsigned long long /*ID*/, cAABB*> g_mapAABBs_World;
playerController* pPlayerControl;
//extern std::map<unsigned long long /*ID*/, cAABB*> g_vecAABBs_World;

cGraph* theGraph;

// pirateStuff
double timer = 0.0;
double averageDeltaTime;
bool isDroneOn = false;
bool cameraFollowPlayer = false;
int width, height;

// cool effects
bool isBloom = false;
bool isNightVision = false;

// Load up my "scene" objects (now global)
// std::map<std::string, cMesh*> g_map_Mesh;
cMeshMap* theMeshMap = cMeshMap::getTheMeshMap();
std::map<std::string, cGameObject*> g_map_GameObjects;
std::map<float, cGameObject*> closestTransparentObjects;
std::map<std::string, cGameObject*>::iterator selectedGameObject = g_map_GameObjects.begin();
std::map<std::string, cLight*> g_map_pLights;
std::map<std::string, cLight*>::iterator selectedLight = g_map_pLights.begin();
cSceneManager* theSceneManager = cSceneManager::getTheSceneManager();
cAnimatedPlayer* theAnimatedPlayer = cAnimatedPlayer::getAnimatedPlayer();
selectedType cursorType = selectedType::GAMEOBJECT;

// weirdly specific functions
void rotateTieFighter();
void fluctuateReflectionSphere();

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(1280, 720, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	// glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	// Mouse callbacks
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	void ProcessAsyncMouse(GLFWwindow * window);
	void ProcessAsyncKeys(GLFWwindow * window);

	//	OpenGL and GLFW are good to go
	cShaderManager* pTheShaderManager = new cShaderManager();
	cShaderManager::cShader vertexShad;
	vertexShad.fileName = "assets/shaders/vertexShader01.glsl";
	cShaderManager::cShader fragShader;
	fragShader.fileName = "assets/shaders/fragmentShader01.glsl";
	if (!pTheShaderManager->createProgramFromFile("SimpleShader", vertexShad, fragShader))
	{
		std::cout << "Error: didn't compile the shader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return -1;
	}
	shaderProgID = pTheShaderManager->getIDFromFriendlyName("SimpleShader");

	::pTextureManager = new cBasicTextureManager();
	
	// SkyBoxTexture
	setSkyBoxTexture();
	
	//JSON Loader for objects
	PhysicsUtils::newPhysicsWorld();
	::pTextureManager->SetBasePath("assets/textures");
	JSONLoader::loadDefaultMesh("assets/models/Sphere_Radius_1_XYZ_n_uv.ply");
	JSONLoader::JSONLoadMeshesSimple();
	JSONLoader::JSONLoadGameObjects(&::g_map_GameObjects);
	//JSONLoader::LoadMeshes_Thread();
	JSONLoader::JSONLoadTextures(&::g_map_GameObjects, ::pTextureManager);
	JSONLoader::loadMeshToGPU(pTheVAOManager, &::g_map_GameObjects, shaderProgID);
	selectedGameObject = ::g_map_GameObjects.begin();

	//	//	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	glEnable(GL_DEPTH);			// Write to the depth buffer
	glEnable(GL_DEPTH_TEST);	// Test with buffer when drawing

	//cPhysics* pPhysic = new cPhysics();
	cLowPassFilter avgDeltaTimeThingy;

	// Let there be lights... I guess
	JSONLoader::JSONLoadLights(&::g_map_pLights,shaderProgID);
	selectedLight = ::g_map_pLights.begin();

	::g_pFlyCamera = new cFlyCamera();
	::g_pFlyCamera->eye = ::g_map_GameObjects["cameraPosition0"]->positionXYZ;
	::g_pFlyCamera->cameraLookAt(::g_map_GameObjects["cameraTarget0"]->positionXYZ);
	::g_pFlyCamera->movementSpeed = 100.0f;

	::theSceneManager->init();

	// Get the initial time
	double lastTime = glfwGetTime();
	std::cout << "start loop!" << std::endl;

	createSkyBoxObject();

	pDebugRenderer->initialize();

	ImGUI_utils::init(window);

	PhysicsUtils::init();
	PhysicsUtils::theWorld->setDebugDrawer(new PhysicsDebug());
	
	while (!glfwWindowShouldClose(window))
	{
		// Get the initial time
		double currentTime = glfwGetTime();
		// Frame time... (how many seconds since last frame)
		double deltaTime = currentTime - lastTime;
		timer += currentTime - lastTime;
		lastTime = currentTime;

		const double SOME_HUGE_TIME = 0.1;	// 100 ms;
		if (deltaTime > SOME_HUGE_TIME) { deltaTime = SOME_HUGE_TIME; }
		avgDeltaTimeThingy.addValue(deltaTime);

		ProcessAsyncKeys(window);
		ProcessAsyncMouse(window);
		glUseProgram(shaderProgID);
		
		std::stringstream ssTitle;
		tools::setWindowTitle(&ssTitle);
		glfwSetWindowTitle(window, ssTitle.str().c_str());
		
		theSceneManager->update();
		// theSceneManager->updateStencil(window);
		PhysicsUtils::theWorld->stepSimulation(deltaTime, 10);
		PhysicsUtils::paddleInputListen(window);
		PhysicsUtils::collisionListen();
		PhysicsUtils::launcherUpdate(window, deltaTime);
		
		switch (cursorType)
		{
		case selectedType::GAMEOBJECT:tools::drawGameObjectXYZ(pDebugRenderer); break;
		case selectedType::LIGHT:tools::drawLightXYZ(pDebugRenderer);break;
		case selectedType::SOUND:break;
		}

		//	Update the objects through physics
		averageDeltaTime = avgDeltaTimeThingy.getAverage();
		IntegrationStep_AAB(::g_map_GameObjects,float(averageDeltaTime));

		if (::debugger)
		{
			PhysicsUtils::theWorld->debugDrawWorld();

			glm::mat4 p, v; float ratio;
			glfwGetFramebufferSize(window, &width, &height);
			ratio = width / float(height);
			p = glm::perspective(0.6f, ratio, 0.1f, 15000.0f);
			v = glm::mat4(1.0f);
			v = glm::lookAt(::g_pFlyCamera->eye, ::g_pFlyCamera->getAtInWorldSpace(), ::g_pFlyCamera->getUpVector());
			pDebugRenderer->RenderDebugObjects(v, p, 0.01f);
		}

		::theSceneManager->lastPass(window);		

		glfwPollEvents();
		
		ImGUI_utils::render();
		
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();

	// Delete everything
	delete pTheModelLoader;
	delete pTheVAOManager;
	PhysicsUtils::cleanUp();
	for (const auto& str_obj_pair : ::g_map_GameObjects)
	{
		delete str_obj_pair.second;
	}
	::g_map_GameObjects.clear();

	exit(EXIT_SUCCESS);
}

