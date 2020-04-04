#pragma once
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "nConvert.h"
#include "GLCommon.h"
#include "cGameObject.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

struct PhysicsUtils
{
	/* Pointer to the single physics world instance used by the game*/
	static btDiscreteDynamicsWorld *theWorld;
	/* left / right: paddle pointers (to control paddles)
	 * launcher object: pointer (to apply forces to it)
	 * ballObj: the bouncy ball
	 * lastHitObj: to not add points if ball doesn't collide with other obj first
	 */
	static cGameObject* leftPaddleObj, *rightPaddleObj,
		*launcherObj, *ballObj, *lastHitObj;
	// "waiting" | "charging" | "launching"
	static std::string launcherState;
	/* Time since loading spring, max loading time */
	static float springHoldTime, springHoldMax;
	/* Ghost object to detect if ball has fallen below the paddles,
	 * depletes a life if so.*/
	static btPairCachingGhostObject *bottomGhost;
	/* Instantiates world instance */
	static void newPhysicsWorld();
	/* Sets pointers to important gameobjects,
	 * Adds constraints to the objects previously declared */
	static void init();
	/* Key bindings for the paddles */
	static void paddleInputListen(GLFWwindow* window);
	/* Function to differentiate between the objects
	 * with which the ball collides */
	static void collisionListen();
	/* for "charging" the launcher, more charge = more velocity */
	static void launcherUpdate(GLFWwindow* window, float deltaTime);
	/*
	 * Deleting all the pointers used for the physics world.
	 */
	static void cleanUp();
private:
	/*
	 * Pointers to objects needed to create the physics world.
	 */
	static btDefaultCollisionConfiguration* mCollisions;
	static btCollisionDispatcher* mCollisionDispatcher;
	static btDbvtBroadphase* mOverlappingPairs;
	static btGhostPairCallback* ghostCallback;
	static btSequentialImpulseConstraintSolver* mConstraints;
};