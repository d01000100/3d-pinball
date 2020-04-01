#pragma once
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "nConvert.h"
#include "GLCommon.h"
#include "cGameObject.h"

struct PhysicsUtils
{
	static btDiscreteDynamicsWorld *theWorld;
	static cGameObject* leftPaddleObj, *rightPaddleObj,
		*launcherObj, *ballObj;
	static void newPhysicsWorld();
	static void init();
	static void inputListen(GLFWwindow* window);
};

