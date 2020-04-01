#include "JSONLoader.h"
#include "cMeshMap.h"
#include "JsonState.h"
#include "PhysicsUtils.h"

using json = nlohmann::json;

std::string JSONLoader::scene_config = "./configFiles/globalConfig.json";
std::string JSONLoader::light_json = "./configFiles/lights.json";
std::string JSONLoader::gameobjects_json = "./configFiles/gameObjects.json";
std::string JSONLoader::bkp_light_json = "./configFiles/bkplights.json";
std::string JSONLoader::bkp_gameobjects_json = "./configFiles/bkpgameObjects.json";

bool jsonContains(const json& jObj, const std::string& key)
{
	return jObj.find(key) != jObj.end();
}

btRigidBody* JSONLoader::LoadRigidBody(const json& physicsDef)
{
	if (!jsonContains(physicsDef, "shape"))
	{
		std::cout << "Physics defs without a shape!!" << std::endl;
		return nullptr;
	}
	if (!jsonContains(physicsDef, "positionXYZ"))
	{
		std::cout << "Physics defs without a starting position!!" << std::endl;
		return nullptr;
	}

	auto shapeString = physicsDef["shape"];
	btCollisionShape* collisionShape = nullptr;
	auto scale = physicsDef["scale"].get<float>();
	if (shapeString == "sphere" || shapeString == "Sphere" || shapeString == "SPHERE")
	{
		if (!jsonContains(physicsDef, "radius"))
		{
			std::cout << "Sphere defs without a radius!!" << std::endl;
			return nullptr;
		}

		collisionShape = new btSphereShape(physicsDef["radius"].get<float>() * scale);
	}
	else if (shapeString == "box" || shapeString == "Box" || shapeString == "BOX")
	{
		if (!jsonContains(physicsDef, "height"))
		{
			std::cout << "Box defs without a height!!" << std::endl;
			return nullptr;
		}
		if (!jsonContains(physicsDef, "width"))
		{
			std::cout << "Box defs without a width!!" << std::endl;
			return nullptr;
		}
		if (!jsonContains(physicsDef, "depth"))
		{
			std::cout << "Box defs without a depth!!" << std::endl;
			return nullptr;
		}

		collisionShape = new btBoxShape(btVector3(
			physicsDef["width"].get<float>() * scale / 2,
			physicsDef["height"].get<float>() * scale / 2,
			physicsDef["depth"].get<float>() * scale / 2
		));
	}
	else if (shapeString == "cylinder" || shapeString == "Cylinder" || shapeString == "CYLINDER")
	{
		if (!jsonContains(physicsDef, "depth"))
		{
			std::cout << "Cylinder defs without a depth!!" << std::endl;
			return nullptr;
		}
		if (!jsonContains(physicsDef, "radius"))
		{
			std::cout << "Cylinder defs without a radius!!" << std::endl;
			return nullptr;
		}

		collisionShape = new btCylinderShapeZ(btVector3(
			physicsDef["radius"].get<float>() * scale,
			physicsDef["radius"].get<float>() * scale,
			physicsDef["depth"].get<float>() * scale/ 2
		));
	}
	else if (shapeString == "mesh" || shapeString == "Mesh" || shapeString == "MESH")
	{
		auto meshCollider = new btConvexHullShape();

		cMeshMap* theMeshMap = cMeshMap::getTheMeshMap();
		if (!theMeshMap->findMesh(physicsDef["meshName"]))
		{
			std::cout << "Mesh " << physicsDef["meshName"] << " not loaded for convex hull shape" << std::endl;
			return nullptr;
		}

		auto mesh = theMeshMap->getMesh(physicsDef["meshName"]);

		for (auto vertex : mesh->vecVertices)
		{
			meshCollider->addPoint(physicsDef["scale"] * btVector3(vertex.x, vertex.y, vertex.z));
		}
		
		collisionShape = meshCollider;
	}
	else
	{
		std::cout << "Unrecognized collision shape: " << shapeString << std::endl;
		return nullptr;
	}

	float mass = 0.0f;
	if (jsonContains(physicsDef, "mass"))
	{
		mass = physicsDef["mass"].get<float>();
	}
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(
		physicsDef["positionXYZ"][0].get<float>(),
		physicsDef["positionXYZ"][1].get<float>(),
		physicsDef["positionXYZ"][2].get<float>()
	));
	startTransform.setRotation(btQuaternion(
		glm::radians(float(physicsDef["rotationXYZ"][1])),
		glm::radians(float(physicsDef["rotationXYZ"][0])),
		glm::radians(float(physicsDef["rotationXYZ"][2]))
	));
	
	
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		collisionShape->calculateLocalInertia(mass, localInertia);

	btRigidBody::btRigidBodyConstructionInfo bodyDef(
		mass,
		motionState,
		collisionShape,
		localInertia
	);
	
	if (jsonContains(physicsDef, "restitution"))
	{
		bodyDef.m_restitution = physicsDef["restitution"].get<float>();
	}
	else
	{
		bodyDef.m_restitution = 1.0f; // how much energy it losses when it collides
	}

	return new btRigidBody(bodyDef);
}

bool JSONLoader::JSONLoadMeshes(std::map<std::string, cMesh*>* g_map_Mesh, cModelLoader* pTheModelLoader)
{
	std::cout << "loading meshes...";
	std::ifstream inFile(gameobjects_json.c_str());
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	for (index = 0; index < jsonArray.size(); index++)
	{
		std::string name = jsonArray[index]["meshName"];
		std::string location = jsonArray[index]["meshURL"];

		// LOAD RIGGED THING
		if (jsonArray[index].find("isSkinnedMesh") != jsonArray[index].end())
		{
			if(jsonArray[index]["isSkinnedMesh"])
			{
				bool didLoad;
				didLoad = loadSkinnedMesh(jsonArray,index);
				continue;
			}
		}

		// ASSIMP NORMAL STATIC LOAD
		std::string errs;
		cMesh* tempMesh = new cMesh();
		pTheModelLoader->LoadModel_Assimp(location.c_str(), *tempMesh, errs);
		g_map_Mesh->insert({ name.c_str(),tempMesh });
	}
	//std::cout << j;
	std::cout << "[OK]\n" << index << " models loaded" << std::endl;
	return true;
}

void JSONLoader::loadDefaultMesh(std::string filename)
{
	std::string errs;
	cMesh* tempMesh = new cMesh();
	pTheModelLoader->LoadModel_Assimp(filename, *tempMesh, errs);
	//::g_map_Mesh.insert({ "sphereMesh",tempMesh });
	cMeshMap* theMeshMap = cMeshMap::getTheMeshMap();
	theMeshMap->addMesh("sphereMesh",tempMesh);
}

bool JSONLoader::JSONLoadMeshes(std::map<std::string, cMesh*>* g_map_Mesh)
{
	std::cout << "loading meshes...";
	cModelLoader* pTheModelLoader = new cModelLoader();
	std::ifstream inFile(gameobjects_json.c_str());
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	for (index = 0; index < jsonArray.size(); index++)
	{
		std::string name = jsonArray[index]["meshName"];
		std::string location = jsonArray[index]["meshURL"];

		// LOAD RIGGED THING
		if (jsonArray[index].find("isSkinnedMesh") != jsonArray[index].end())
		{
			if(jsonArray[index]["isSkinnedMesh"])
			{
				loadSkinnedMesh(jsonArray,index);
				continue;
			}
		}

		// ASSIMP NORMAL STATIC LOAD
		std::string errs;
		cMesh* tempMesh = new cMesh();
		pTheModelLoader->LoadModel_Assimp(location.c_str(), *tempMesh, errs);
		//g_map_Mesh->insert({ name.c_str(),tempMesh });
		cMeshMap* theMeshMap = cMeshMap::getTheMeshMap();
		theMeshMap->addMesh(name,tempMesh);
	}
	//std::cout << j;
	std::cout << "[OK]\n" << index << " models loaded" << std::endl;
	return true;
}

bool JSONLoader::JSONLoadMeshesSimple()
{
	std::cout << "loading meshes...";
	cModelLoader* pTheModelLoader = new cModelLoader();
	std::ifstream inFile(gameobjects_json.c_str());
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	for (index = 0; index < jsonArray.size(); index++)
	{
		std::string name = jsonArray[index]["meshName"];
		std::string location = jsonArray[index]["meshURL"];

		// LOAD RIGGED THING
		if (jsonArray[index].find("isSkinnedMesh") != jsonArray[index].end())
		{
			if(jsonArray[index]["isSkinnedMesh"])
			{
				loadSkinnedMesh(jsonArray,index);
				continue;
			}
		}

		// ASSIMP NORMAL STATIC LOAD
		std::string errs;
		cMesh* tempMesh = new cMesh();
		pTheModelLoader->LoadModel_Assimp(location.c_str(), *tempMesh, errs);
		//g_map_Mesh->insert({ name.c_str(),tempMesh });
		cMeshMap* theMeshMap = cMeshMap::getTheMeshMap();
		theMeshMap->addMesh(name,tempMesh);
	}
	//std::cout << j;
	std::cout << "[OK]\n" << index << " models loaded" << std::endl;
	return true;
}

void JSONLoader::LoadMeshes_Thread()
{
	std::thread thread1(JSONLoadMeshesSimple);
	thread1.detach();
}

bool JSONLoader::JSONLoadLights(std::map<std::string, cLight*>* g_map_pLights, GLuint shadProgID)
{
	std::cout << "loading lights...";
	std::ifstream inFile(light_json.c_str());
	std::ofstream outFile(bkp_light_json.c_str());
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	JsonState* theJsonState = JsonState::getTheJsonState();
	theJsonState->JSONLights = jsonArray;

	for (index = 0; index < jsonArray.size(); index++)
	{
		std::string name = jsonArray[index]["name"];
		glm::vec3 positionXYZ = glm::vec3(
			jsonArray[index]["positionXYZ"][0],
			jsonArray[index]["positionXYZ"][1],
			jsonArray[index]["positionXYZ"][2]);		
		float LinearAtten = jsonArray[index]["LinearAtten"];
        float QuadraticAtten = jsonArray[index]["QuadraticAtten"];
		glm::vec3 diffuse = glm::vec3(
			jsonArray[index]["diffuse"][0],
			jsonArray[index]["diffuse"][1],
			jsonArray[index]["diffuse"][2]);
		glm::vec3 specular = glm::vec3(
			jsonArray[index]["specular"][0],
			jsonArray[index]["specular"][1],
			jsonArray[index]["specular"][2]);	
		glm::vec3 direction = glm::vec3(
			jsonArray[index]["direction"][0],
			jsonArray[index]["direction"][1],
			jsonArray[index]["direction"][2]);
		float type = jsonArray[index]["type"];
		float innerAngle = jsonArray[index]["innerAngle"];
		float outerAngle = jsonArray[index]["outerAngle"];
		float lightW = jsonArray[index]["lightW"];
		float lightSwitch = jsonArray[index]["lightSwitch"];
		cLight* tempLight = new cLight("theLights", name, shadProgID, index);
		// load values to tempLight
		tempLight->positionXYZ = positionXYZ;
		tempLight->LinearAtten = LinearAtten;
		tempLight->QuadraticAtten = QuadraticAtten;
		tempLight->diffuse = diffuse;
		tempLight->specular = specular;
		tempLight->setAT(direction);
		tempLight->type = type;
		tempLight->innerAngle = innerAngle;
		tempLight->outerAngle = outerAngle;
		tempLight->lightW = lightW;
		tempLight->lightSwitch = lightSwitch;
		//load to global map ::g_map_pLights["light0"] = &light0;
		g_map_pLights->insert({ name.c_str(),tempLight });
	}
	//std::cout << j;
	std::cout << "[OK]\n" << index << " lights loaded" << std::endl;
	outFile << jsonArray;
	return true;
}

bool JSONLoader::JSONLoadGameObjects(std::map<std::string, cGameObject*>* g_map_GameObjects)
{
	std::cout << "loading objects...";
	std::ifstream inFile(gameobjects_json);
	std::ofstream outFile("./configFiles/log.txt");
	std::ofstream outFile2(bkp_gameobjects_json);
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	// integrate JsonState
	JsonState* theJsonState = JsonState::getTheJsonState();
	theJsonState->JSONObjects = jsonArray;

	for (index = 0; index < jsonArray.size(); index++)
	{
		std::string friendlyName = jsonArray[index]["friendlyName"];
		outFile << "friendlyName: " << friendlyName << std::endl;
		std::string meshName = jsonArray[index]["meshName"];
		outFile << "meshName: " << meshName << std::endl;
		std::string meshURL = jsonArray[index]["meshURL"];
		outFile << "meshURL: " << meshURL << std::endl;
		glm::vec3 positionXYZ = glm::vec3(
			jsonArray[index]["positionXYZ"][0],
			jsonArray[index]["positionXYZ"][1],
			jsonArray[index]["positionXYZ"][2]);
		outFile << "positionXYZ: " << glm::to_string(positionXYZ) << std::endl;
		glm::vec3 rotationXYZ = glm::vec3(
			jsonArray[index]["rotationXYZ"][0],
			jsonArray[index]["rotationXYZ"][1],
			jsonArray[index]["rotationXYZ"][2]);
		outFile << "rotationXYZ: " << glm::to_string(rotationXYZ) << std::endl;
		float scale = jsonArray[index]["scale"];
		glm::vec4 objectColourRGBA = glm::vec4(
			jsonArray[index]["objectColourRGBA"][0],
			jsonArray[index]["objectColourRGBA"][1],
			jsonArray[index]["objectColourRGBA"][2],
			jsonArray[index]["objectColourRGBA"][3]);
		outFile << "objectColourRGBA: " << glm::to_string(objectColourRGBA) << std::endl;
		glm::vec4 diffuseColour = glm::vec4(
			jsonArray[index]["diffuseColour"][0],
			jsonArray[index]["diffuseColour"][1],
			jsonArray[index]["diffuseColour"][2],
			jsonArray[index]["diffuseColour"][3]);
		outFile << "diffuseColour: " << glm::to_string(diffuseColour) << std::endl;
		glm::vec4 specularColour = glm::vec4(
			jsonArray[index]["specularColour"][0],
			jsonArray[index]["specularColour"][1],
			jsonArray[index]["specularColour"][2],
			jsonArray[index]["specularColour"][3]);
		outFile << "specularColour: " << glm::to_string(specularColour) << std::endl;
		glm::vec3 velocity = glm::vec3(
			jsonArray[index]["velocity"][0],
			jsonArray[index]["velocity"][1],
			jsonArray[index]["velocity"][2]);
		outFile << "velocity: " << glm::to_string(velocity) << std::endl;
		glm::vec3 accel = glm::vec3(
			jsonArray[index]["accel"][0],
			jsonArray[index]["accel"][1],
			jsonArray[index]["accel"][2]);
		outFile << "accel: " << glm::to_string(accel) << std::endl;
		float inverseMass = jsonArray[index]["inverseMass"];

		int physicsShapeType = jsonArray[index]["physicsShapeType"];
		bool isWireframe = jsonArray[index]["isWireframe"];
		glm::vec4 debugColour = glm::vec4(
			jsonArray[index]["debugColour"][0],
			jsonArray[index]["debugColour"][1],
			jsonArray[index]["debugColour"][2],
			jsonArray[index]["debugColour"][3]);
		outFile << "debugColour: " << glm::to_string(debugColour) << std::endl;
		bool isVisible = jsonArray[index]["isVisible"];

		cGameObject* tempGameObject = new cGameObject();
		if (jsonArray[index].find("texture") != jsonArray[index].end())
		{
			for (int i = 0; i < jsonArray[index]["texture"].size(); i++)
			{
				tempGameObject->textures.push_back(jsonArray[index]["texture"][i]);
			}
		}
		if (jsonArray[index].find("textureRatio") != jsonArray[index].end())
		{
			for (int i = 0; i < jsonArray[index]["textureRatio"].size(); i++)
			{
				tempGameObject->textureRatio.push_back(jsonArray[index]["textureRatio"][i]);
			}
		}
		if (jsonArray[index].find("alphaTransparency") != jsonArray[index].end())
		{
			tempGameObject->alphaTransparency = jsonArray[index]["alphaTransparency"];
		}
		tempGameObject->friendlyName = friendlyName;
		tempGameObject->meshName = meshName;
		tempGameObject->meshURL = meshURL;
		tempGameObject->positionXYZ = positionXYZ;
		tempGameObject->setOrientation(rotationXYZ);
		tempGameObject->scale = scale;
		tempGameObject->objectColourRGBA = objectColourRGBA;
		tempGameObject->diffuseColour = diffuseColour;
		tempGameObject->specularColour = specularColour;
		tempGameObject->velocity = velocity;
		tempGameObject->accel = accel;
		tempGameObject->inverseMass = inverseMass;
		tempGameObject->physicsShapeType = (eShapeTypes)physicsShapeType;
		tempGameObject->isWireframe = isWireframe;
		tempGameObject->debugColour = debugColour;
		tempGameObject->isVisible = isVisible;

		// Physics
		if (jsonContains(jsonArray[index], "Physics"))
		{
			auto physicsDef = jsonArray[index]["Physics"];
			physicsDef["positionXYZ"] = jsonArray[index]["positionXYZ"];
			physicsDef["rotationXYZ"] = jsonArray[index]["rotationXYZ"];
			physicsDef["scale"] = jsonArray[index]["scale"];
			physicsDef["meshName"] = jsonArray[index]["meshName"];
			tempGameObject->rigidBody = LoadRigidBody(physicsDef);
			if (tempGameObject->rigidBody)
			{
				PhysicsUtils::theWorld->addRigidBody(tempGameObject->rigidBody);
			}
		}
		
		g_map_GameObjects->insert({ friendlyName.c_str(),tempGameObject });
	}
	//std::cout << j;
	std::cout << "[OK]\n" << index << " objects loaded" << std::endl;
	outFile2 << jsonArray;
	return true;
}

bool JSONLoader::loadMeshToGPU(cVAOManager* pTheVAOManager,
	std::map<std::string, cGameObject*>* g_map_GameObjects,
	GLuint shaderProgID	)
{
	size_t count = 0;
	cMeshMap* theMeshMap = cMeshMap::getTheMeshMap();
	
	for (auto itGO = g_map_GameObjects->begin(); itGO != g_map_GameObjects->end(); itGO++)
	{
		if(itGO->second->pAS) continue;
		//if(g_map_Mesh->find(itGO->second->meshName) == g_map_Mesh->end()) continue;
		if(!(theMeshMap->findMesh(itGO->second->meshName))) continue;
		auto* pMesh = theMeshMap->getMesh(itGO->second->meshName);
		if(pMesh->status == "loaded")
		{
			sModelDrawInfo drawInfo;
			
			pMesh = theMeshMap->getMesh(itGO->second->meshName);
			pTheVAOManager->LoadModelIntoVAO(
				itGO->second->meshName,
				*(pMesh),
				drawInfo, shaderProgID);
			
			pMesh = theMeshMap->getMesh(itGO->second->meshName);
			pMesh->status = "on_gpu";
			count++;
		}
	}//for (int index...
	if(count > 0)
	{
		std::cout << "loaded meshes this frame: " << count << std::endl;
	}
	return true;
}

//bool JSONLoader::JSONSaveLights(std::map<std::string, cLight*>* g_map_pLights)
//{
//	std::cout << "saving lights...";
//	std::ofstream outFile("./configFiles/lights.json");
//	auto index = g_map_pLights->begin();
//	int x = 0;
//	json jsonArray;
//
//	for (index; index != g_map_pLights->end(); index++, x++)
//	{
//		json jsonObject;
//		jsonObject["name"] = index->second->getName();
//		jsonObject["positionXYZ"][0] = index->second->positionXYZ.x;
//		jsonObject["positionXYZ"][1] = index->second->positionXYZ.y;
//		jsonObject["positionXYZ"][2] = index->second->positionXYZ.z;
//		jsonObject["LinearAtten"] = index->second->LinearAtten;
//		jsonObject["QuadraticAtten"] = index->second->QuadraticAtten;
//		jsonObject["diffuse"][0] = index->second->diffuse.x;
//		jsonObject["diffuse"][1] = index->second->diffuse.y;
//		jsonObject["diffuse"][2] = index->second->diffuse.z;
//		jsonObject["specular"][0] = index->second->specular.x;
//		jsonObject["specular"][1] = index->second->specular.y;
//		jsonObject["specular"][2] = index->second->specular.z;
//		jsonObject["direction"][0] = index->second->getCurrentAT().x;
//		jsonObject["direction"][1] = index->second->getCurrentAT().y;
//		jsonObject["direction"][2] = index->second->getCurrentAT().z;
//		jsonObject["type"] = index->second->type;
//		jsonObject["innerAngle"] = index->second->innerAngle;
//		jsonObject["outerAngle"] = index->second->outerAngle;
//		jsonObject["lightW"] = index->second->lightW;
//		jsonObject["lightSwitch"] = index->second->lightSwitch;
//		jsonArray[x] = jsonObject;
//	}
//	//std::cout << jsonArray;
//	outFile << jsonArray;
//	std::cout << "[OK]\n" << x << " lights saved!" << std::endl;
//	return true;
//}

void JSONLoader::JSONSaveLights()
{
	std::cout << "saving lights...";
	std::ofstream outFile("./configFiles/lights.json");
	JsonState* theJsonState = JsonState::getTheJsonState();
	outFile << theJsonState->JSONLights;
	std::cout << "[OK]" << std::endl;
}

//bool JSONLoader::JSONSaveGameObjects(std::map<std::string, cGameObject*>* g_map_GameObjects)
//{
//	std::cout << "saving gameObjects...";
//	std::ofstream outFile("./configFiles/gameObjects.json");
//	std::map<std::string, cGameObject*>::iterator index = g_map_GameObjects->begin();
//	int x = 0;
//	json jsonArray;
//
//	for (index; index != g_map_GameObjects->end(); index++, x++)
//	{
//		json jsonObject;
//		jsonObject["friendlyName"] = index->second->friendlyName;
//		jsonObject["meshName"] = index->second->meshName;
//		jsonObject["meshURL"] = index->second->meshURL;
//		jsonObject["positionXYZ"][0] = index->second->positionXYZ.x;
//		jsonObject["positionXYZ"][1] = index->second->positionXYZ.y;
//		jsonObject["positionXYZ"][2] = index->second->positionXYZ.z;
//		jsonObject["rotationXYZ"][0] = index->second->getEulerAngle().x;
//		jsonObject["rotationXYZ"][1] = index->second->getEulerAngle().y;
//		jsonObject["rotationXYZ"][2] = index->second->getEulerAngle().z;
//		jsonObject["scale"] = index->second->scale;
//		jsonObject["objectColourRGBA"][0] = index->second->objectColourRGBA.x;
//		jsonObject["objectColourRGBA"][1] = index->second->objectColourRGBA.y;
//		jsonObject["objectColourRGBA"][2] = index->second->objectColourRGBA.z;
//		jsonObject["objectColourRGBA"][3] = index->second->objectColourRGBA.w;
//		jsonObject["diffuseColour"][0] = index->second->diffuseColour.x;
//		jsonObject["diffuseColour"][1] = index->second->diffuseColour.y;
//		jsonObject["diffuseColour"][2] = index->second->diffuseColour.z;
//		jsonObject["diffuseColour"][3] = index->second->diffuseColour.w;
//		jsonObject["specularColour"][0] = index->second->specularColour.x;
//		jsonObject["specularColour"][1] = index->second->specularColour.y;
//		jsonObject["specularColour"][2] = index->second->specularColour.z;
//		jsonObject["specularColour"][3] = index->second->specularColour.w;
//		jsonObject["velocity"][0] = index->second->velocity.x;
//		jsonObject["velocity"][1] = index->second->velocity.y;
//		jsonObject["velocity"][2] = index->second->velocity.z;
//		jsonObject["accel"][0] = index->second->accel.x;
//		jsonObject["accel"][1] = index->second->accel.y;
//		jsonObject["accel"][2] = index->second->accel.z;
//		jsonObject["inverseMass"] = index->second->inverseMass;
//		jsonObject["physicsShapeType"] = index->second->physicsShapeType;
//		jsonObject["isWireframe"] = index->second->isWireframe;
//		jsonObject["debugColour"][0] = index->second->debugColour.x;
//		jsonObject["debugColour"][1] = index->second->debugColour.y;
//		jsonObject["debugColour"][2] = index->second->debugColour.z;
//		jsonObject["debugColour"][3] = index->second->debugColour.w;
//		jsonObject["texture"][0] = index->second->textures[0];
//		jsonObject["texture"][1] = index->second->textures[1];
//		jsonObject["texture"][2] = index->second->textures[2];
//		jsonObject["texture"][3] = index->second->textures[3];
//		jsonObject["textureRatio"][0] = index->second->textureRatio[0];
//		jsonObject["textureRatio"][1] = index->second->textureRatio[1];
//		jsonObject["textureRatio"][2] = index->second->textureRatio[2];
//		jsonObject["textureRatio"][3] = index->second->textureRatio[3];
//		jsonObject["isVisible"] = index->second->isVisible;
//		jsonObject["alphaTransparency"] = index->second->alphaTransparency;
//		jsonArray[x] = jsonObject;
//	}
//	//std::cout << jsonArray;
//	outFile << jsonArray;
//	std::cout << "[OK]\n" << x << " lights saved!" << std::endl;
//	return true;
//}

void JSONLoader::JSONSaveGos()
{
	std::cout << "saving gameObjects...";
	std::ofstream outFile("./configFiles/gameObjects.json");
	JsonState* theJsonState = JsonState::getTheJsonState();
	theJsonState->mergeAllObjects(::g_map_GameObjects);
	outFile << theJsonState->JSONObjects;
	std::cout << "[OK]" << std::endl;
}

bool JSONLoader::JSONLoadTextures(std::map<std::string, cGameObject*>* pGameObjects, cBasicTextureManager* pTextureManager)
{
	for (auto itGO = pGameObjects->begin();
		itGO != pGameObjects->end();
		itGO++)
	{
		for (int i = 0; i < itGO->second->NUMBEROFTEXTURES; i++)
		{
			if (itGO->second->textures[i] != "");
			{
				pTextureManager->Create2DTextureFromBMPFile(itGO->second->textures[i].c_str(), true);
			}
		}
	}
	return true;
}

bool JSONLoader::JSONLoadSceneConf()
{
	std::cout << "loading scenes!...";
	std::ifstream inFile(scene_config);
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	cSceneManager* theSceneManager = cSceneManager::getTheSceneManager();
	theSceneManager->numScenes = jsonArray["numberOfScenes"];
	theSceneManager->numCameras = jsonArray["numberOfCameras"];

	for (index = 0; index < jsonArray["scenes"].size(); index++)
	{
		std::string name = jsonArray["scenes"][index]["name"];
		std::string jsonPath = jsonArray["scenes"][index]["jsonPath"];
		std::string effect;
		if (jsonArray["scenes"][index].find("effect") != jsonArray["scenes"][index].end())
		{
			effect = jsonArray["scenes"][index]["effect"];
		}
		std::vector<int> cameraNumber;
		if (jsonArray["scenes"][index].find("cameraNumber") != jsonArray["scenes"][index].end())
		{
			for (int cIndex = 0; cIndex < jsonArray["scenes"][index]["cameraNumber"].size(); cIndex++)
			{
				cameraNumber.push_back(jsonArray["scenes"][index]["cameraNumber"][cIndex]);
			}
		}
		int width = 1280, height = 720;
		if (jsonArray["scenes"][index].find("width") != jsonArray["scenes"][index].end())
		{
			width = jsonArray["scenes"][index]["width"];
		}
		if (jsonArray["scenes"][index].find("height") != jsonArray["scenes"][index].end())
		{
			height = jsonArray["scenes"][index]["height"];
		}
		
		//std::cout << "name: " << name << std::endl;
		//std::cout << "jsonPath: " << jsonPath << std::endl;
		//std::cout << "effect: " << effect << std::endl;
		//std::cout << "cameraNumber: " << cameraNumber << std::endl;
		//std::cout << "width: " << width << std::endl;
		//std::cout << "height: " << height << std::endl;
		
		auto* theScene = new cScene(name,jsonPath,effect,cameraNumber,width,height);
		theSceneManager->addScene(theScene);
	}
	std::cout << "[OK]\n" << index << " Scenes loaded successfully!" << std::endl;
	return true;
}

bool JSONLoader::JSONLoadEntitiesToScene(
		std::map<std::string, cGameObject*>* pGameObjects,
		std::map<std::string, cLight*>* pLights,
		cScene* theScene)
{
	std::cout << "loading objects to: " << theScene->name << std::endl;
	std::ifstream inFile(theScene->jsonPath);
	json jsonArray;
	int index = 0;
	inFile >> jsonArray;

	cSceneManager* theSceneManager = cSceneManager::getTheSceneManager();

	for (index = 0; index < jsonArray["gameObjects"].size(); index++)
	{
		std::string friendlyName = jsonArray["gameObjects"][index]["friendlyName"];
		if(tools::pFindObjectByFriendlyNameMap(friendlyName))
		{
			theScene->addGameObject((*pGameObjects)[friendlyName]);
		}
	}	
	std::cout << "[OK]\n" << index << " objects loaded successfully to " << theScene->name << std::endl;
	
	for (index = 0; index < jsonArray["lights"].size(); index++)
	{
		std::string name = jsonArray["lights"][index]["name"];
		if(tools::pFindLightByNameMap(name))
		{
			theScene->addLight((*pLights)[name]);
		}
	}	
	std::cout << "[OK]\n" << index << " lights loaded successfully to " << theScene->name << std::endl;
	
	return true;
}

bool JSONLoader::loadSkinnedMesh(nlohmann::json jsonArray, int index)
{
	auto selector = jsonArray[index];
	auto theGameObject = ::g_map_GameObjects.at(selector["friendlyName"]);
	auto pSkinnedMesh = new cSimpleAssimpSkinnedMesh();
	theGameObject->pAS = new cAnimationState(pSkinnedMesh);
	
	// START OF: Loading bind pose mesh into the VAO manager (so we can draw it)
	theGameObject->pAS->pSM->LoadMeshFromFile(theGameObject->meshName, theGameObject->meshURL);
	// Get the draw info, to load into the VAO
	sModelDrawInfo* pDI = theGameObject->pAS->pSM->CreateModelDrawInfoObjectFromCurrentModel();

	if (pDI)
	{
		//std::cout << pDI->numberOfVertices << " vertices" << std::endl;
		//std::cout << pDI->numberOfTriangles << " triangles" << std::endl;
		pTheVAOManager->LoadModelDrawInfoIntoVAO(*pDI, shaderProgID);
	}
	
	for(auto c = 0; c < selector["zAnimations"].size();c++)
	{
		auto anim = selector["zAnimations"][c];
		theGameObject->pAS->pSM->LoadMeshAnimation(anim["animName"],anim["location"]);
		
		theGameObject->pAS->loadAnimationDetails(anim["animName"],anim["isReplaceable"]);
		// make first animation, the default one
		if(c==0) theGameObject->pAS->makeDefaultAnimation(anim["animName"]);
	}
	return true;
}