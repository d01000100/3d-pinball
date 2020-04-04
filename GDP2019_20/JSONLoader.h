#pragma once

#include "cModelLoader.h"	
#include "cGameObject.h"
#include "cLight.h"
#include "cVAOManager.h"
#include "util/tools.h"
#include "SceneManager/cSceneManager.h"
#include <nlohmann/json.hpp>
#include <iostream>	
#include <fstream>	
#include <vector>
#include <thread>
#include <map>
#include "BulletDynamics/Dynamics/btRigidBody.h"

class JSONLoader
{
public:
	static std::string scene_config;
	static std::string light_json;
	static std::string gameobjects_json;
	static std::string bkp_light_json;
	static std::string bkp_gameobjects_json;

	/* Checks if each obj in the json has physics properties,
	 * if it does, creates the rigidbody and add it to the world */
	static btRigidBody* LoadRigidBody(const nlohmann::json& physicsDef);
	/* Legacy loading method */
	static bool JSONLoadMeshes(
		std::map<std::string, cMesh*>* g_map_Mesh,
		cModelLoader* pTheModelLoader);
	/* loads default sphere mesh to apply before loading the actual mesh,
	 * for threaded model loading */
	static void loadDefaultMesh(
		std::string filename);
	/* Legacy loading method */
	static bool JSONLoadMeshes(
		std::map<std::string, cMesh*>* g_map_Mesh);
	/* Reads the json in memory and stores the meshes in an array */
	static bool JSONLoadMeshesSimple();
	/* Thread call to read meshes into array in memory */
	static void LoadMeshes_Thread();
	/* Loads light defs from json file */
	static bool JSONLoadLights(
		std::map<std::string, cLight*>* g_map_pLights,
		GLuint shadProgID);
	/* Loads game object defs from json file */
	static bool JSONLoadGameObjects(
		std::map<std::string, cGameObject*>* g_map_GameObjects);	
	/* Loads the Meshes from memory to the GPU */
	static bool loadMeshToGPU(
		cVAOManager* pTheVAOManager,
		std::map<std::string, cGameObject*>* g_map_GameObjects,
		GLuint shaderProgID);
	/* Saves light json in memory to file */
	static void JSONSaveLights();
	/* Saves game object json in memory to file */
	static void JSONSaveGos();
	/* Loads png textures to memory */
	static bool JSONLoadTextures(
		std::map<std::string, cGameObject*>* pGameObjects,
		cBasicTextureManager* pTextureManager);
	/* Loads config for deferred scenes, cameras, game objects */
	static bool JSONLoadSceneConf();
	/* Loads game objects in scene */
	static bool JSONLoadEntitiesToScene(
		std::map<std::string, cGameObject*>* pGameObjects, 
		std::map<std::string, cLight*>* pLights, cScene* theScene);
	/* loads rigged models in fbx format */
	static bool loadSkinnedMesh(nlohmann::json jsonArray, int index);
};

