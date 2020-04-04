#pragma once
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "nConvert.h"
#include "GLCommon.h"
#include "cGameObject.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

struct PhysicsUtils
{
	static btDiscreteDynamicsWorld *theWorld;
	static cGameObject* leftPaddleObj, *rightPaddleObj,
		*launcherObj, *ballObj, *lastHitObj;
	// "waiting" | "charging" | "launching"
	static std::string launcherState;
	static float springHoldTime, springHoldMax;
	static btPairCachingGhostObject *bottomGhost;
	static void newPhysicsWorld();
	static void init();
	static void inputListen(GLFWwindow* window);
	static void collisionListen();
	static void launcherUpdate(GLFWwindow* window, float deltaTime);
};

