#include "globalStuff.h"
#include "util/tools.h"
#include "PhysicsUtils.h"
#include "Pinball.h"
#include "JsonState.h"

cGameObject* PhysicsUtils::leftPaddleObj = nullptr,
           *PhysicsUtils::rightPaddleObj = nullptr,
           * PhysicsUtils::launcherObj = nullptr,
           *PhysicsUtils::lastHitObj = nullptr,
           *PhysicsUtils::ballObj = nullptr;
btDiscreteDynamicsWorld *PhysicsUtils::theWorld = nullptr;

btPairCachingGhostObject* PhysicsUtils::bottomGhost = nullptr;

float PhysicsUtils::springHoldTime = 0.f,
	PhysicsUtils::springHoldMax = 5.f;

std::string PhysicsUtils::launcherState = "waiting";

void PhysicsUtils::newPhysicsWorld()
{
	auto mCollisions = new btDefaultCollisionConfiguration();
	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	auto mCollisionDispatcher = new btCollisionDispatcher(mCollisions);
	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	auto mOverlappingPairs = new btDbvtBroadphase();
	mOverlappingPairs->getOverlappingPairCache()->setInternalGhostPairCallback(
		new btGhostPairCallback() 
	);
	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	auto mConstraints = new btSequentialImpulseConstraintSolver;
	theWorld = new btDiscreteDynamicsWorld(
		mCollisionDispatcher,
		mOverlappingPairs,
		mConstraints,
		mCollisions
	);

	theWorld->setGravity({ 0,-100,0 });
}

void PhysicsUtils::init()
{
	//theWorld->setGravity({ 0,0,0 });
	// Add hinge constraint to left paddle
	// Assuming that "leftPaddle" is a box with little height
	if (tools::pFindObjectByFriendlyNameMap("leftPaddle"))
	{
		leftPaddleObj = ::g_map_GameObjects["leftPaddle"];
		if (leftPaddleObj->rigidBody)
		{
			auto body = leftPaddleObj->rigidBody;
			auto shape = body->getCollisionShape();
			btVector3 aabbMin, aabbMax;
			shape->getAabb(body->getWorldTransform(), aabbMin, aabbMax);
			auto pos = body->getWorldTransform().getOrigin();
			btHingeConstraint* hinge = new btHingeConstraint(
				*body, btVector3(aabbMin.getX(), aabbMin.getY(), 0) - pos, btVector3(0, 0, 1), true
			);
			hinge->setLimit(glm::radians(-30.f), glm::radians(30.f));
			theWorld->addConstraint(hinge);
		}
	}

	// Add hinge constraint to right paddle
	if (tools::pFindObjectByFriendlyNameMap("rightPaddle"))
	{
		rightPaddleObj = ::g_map_GameObjects["rightPaddle"];
		if (rightPaddleObj->rigidBody)
		{
			auto body = rightPaddleObj->rigidBody;
			auto shape = body->getCollisionShape();
			btVector3 aabbMin, aabbMax;
			shape->getAabb(body->getWorldTransform(), aabbMin, aabbMax);
			auto pos = body->getWorldTransform().getOrigin();
			btHingeConstraint* hinge = new btHingeConstraint(
				*body, btVector3(aabbMax.getX(), aabbMin.getY(), 0) - pos, btVector3(0, 0, 1), true
			);
			hinge->setLimit(glm::radians(-30.f), glm::radians(30.f));
			theWorld->addConstraint(hinge);
		}
	}

	//Add slider constraint to the launcher
	if (tools::pFindObjectByFriendlyNameMap("boxy"))
	{
		launcherObj = ::g_map_GameObjects["boxy"];
		if (launcherObj->rigidBody)
		{
			btTransform frame;
	        frame.setIdentity();
	        frame.setRotation( btQuaternion( btVector3(0,0,1),-SIMD_PI/2) );
			btSliderConstraint* slider = new btSliderConstraint(
				*(launcherObj->rigidBody),frame,false);
			slider->setLowerLinLimit(-30.f);
			slider->setUpperLinLimit(0.f);
			slider->setLowerAngLimit(0);
			slider->setUpperAngLimit(0);
			theWorld->addConstraint(slider);
			slider->setDbgDrawSize(btScalar(35.f));
		}
	}

	//Add 6dof constraint to the ball
	if (tools::pFindObjectByFriendlyNameMap("sphere"))
	{
		ballObj = ::g_map_GameObjects["sphere"];
		if (ballObj->rigidBody)
		{
			auto ballBody = ballObj->rigidBody;
			auto ballPos = ballBody->getWorldTransform().getOrigin();
			
			btVector3 lowerSliderLimit = btVector3(-52.5, -25, 0);
			btVector3 hiSliderLimit = btVector3(97, 160, 0);
			btTransform frame;
	        frame.setIdentity();
	        //frame.setRotation( btQuaternion( btVector3(1,0,0),0) );
			btGeneric6DofConstraint* ball6dof = new btGeneric6DofConstraint(
				*(ballObj->rigidBody),frame,true);
			ball6dof->setLinearLowerLimit(lowerSliderLimit - ballPos);
			ball6dof->setLinearUpperLimit(hiSliderLimit - ballPos);
			ball6dof->setDbgDrawSize(1.0f);
			theWorld->addConstraint(ball6dof);
		}
	}

	// Add Ghost Object to determine if the player has lost
	bottomGhost = new btPairCachingGhostObject();
	btTransform ghostTransform;
	ghostTransform.setOrigin({ 0, -30, 0 });
	bottomGhost->setWorldTransform(ghostTransform);
	bottomGhost->setCollisionShape(new btBoxShape(btVector3(60, 5, 5)));
	bottomGhost->setCollisionFlags(bottomGhost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	theWorld->addCollisionObject(bottomGhost, 2, 1);
}

void PhysicsUtils::inputListen(GLFWwindow* window)
{
	float paddleVel = 12.0f;
	if (leftPaddleObj)
	{
		if (leftPaddleObj->rigidBody)
		{
			auto left_key_status = glfwGetKey(window, GLFW_KEY_LEFT);
			auto body = leftPaddleObj->rigidBody;
			body->activate(true);
			float x, y, z;
			body->getWorldTransform().getRotation().getEulerZYX(z,y,x);
			z = glm::degrees(z);
			// Because of our setup, negative degrees are counter clockwise
			// and positive degrees are clockwise
			if (left_key_status == GLFW_PRESS)
			{
				if (z < 28)
				{
					body->setAngularVelocity(btVector3(0, 0, paddleVel));
				}
				else
				{
					body->setAngularVelocity(btVector3(0, 0, 0));
				}
			}
			if (left_key_status == GLFW_RELEASE)
			{
				if (z > -28.f)
				{
					body->setAngularVelocity(btVector3(0, 0, -paddleVel));
				}
				else
				{
					body->setAngularVelocity(btVector3(0, 0, 0));
				}
			}
		}
	}

	if (rightPaddleObj)
	{
		if (rightPaddleObj->rigidBody)
		{
			auto right_key_status = glfwGetKey(window, GLFW_KEY_RIGHT);
			auto body = rightPaddleObj->rigidBody;
			body->activate(true);
			float x, y, z;
			body->getWorldTransform().getRotation().getEulerZYX(z, y, x);
			z = glm::degrees(z);
			// Because of our setup, negative degrees are counter clockwise
			// and positive degrees are clockwise
			if (right_key_status == GLFW_PRESS)
			{
				if (z > -28)
				{
					body->setAngularVelocity(btVector3(0, 0, -paddleVel));
				} else
				{
					body->setAngularVelocity(btVector3(0, 0, 0));
				}
			}
			if (right_key_status == GLFW_RELEASE)
			{
				if (z < 28.f)
				{
					body->setAngularVelocity(btVector3(0, 0, paddleVel));
				} else
				{
					body->setAngularVelocity(btVector3(0, 0, 0));
				}
			}
		}
	}

	
}

void PhysicsUtils::collisionListen()
{
	if (Pinball::lives > 0)
	{
		auto dispatcher = theWorld->getCollisionWorld()->getDispatcher();
		int nCollisions = dispatcher->getNumManifolds();
		for (int i = 0; i < nCollisions; i++)
		{
			auto manifold = dispatcher->getManifoldByIndexInternal(i);
			if (manifold->getNumContacts())
			{
				auto bodyA = manifold->getBody0();
				auto bodyB = manifold->getBody1();
				auto objA = (cGameObject*)bodyA->getUserPointer();
				auto objB = (cGameObject*)bodyB->getUserPointer();

				if (objA && objB && objA != lastHitObj && objB != lastHitObj)
				{
					lastHitObj = objB;
					if ((Pinball::pointGivers.count(objA) ||
						Pinball::pointGivers.count(objB)))
					{
						Pinball::points += 5;
					}
					// If we hit the diamond, we get a super duper life
					if (objB->friendlyName == "diamond")
					{
						Pinball::lives++;
					}
				}
			}
		}

		int nOverlapping = bottomGhost->getNumOverlappingObjects();

		for (int i = 0; i < nOverlapping; i++)
		{
			auto trigger = bottomGhost->getOverlappingObject(i);
			auto triggerObj = (cGameObject*)trigger->getUserPointer();
			if (triggerObj->friendlyName == "sphere")
			{
				Pinball::lives--;
				if (Pinball::lives > 0)
				{
					auto theJson = JsonState::getTheJsonState();
					int sphereIndex = theJson->findObject(triggerObj);
					auto sphereOGPos = theJson->JSONObjects[sphereIndex]["positionXYZ"];
					triggerObj->rigidBody->getWorldTransform().setOrigin(
						{ sphereOGPos[0], sphereOGPos[1], sphereOGPos[2] }
					);
				}
			}
		}
	}
}

void PhysicsUtils::launcherUpdate(GLFWwindow* window, float deltaTime)
{
	if (!launcherObj) return;
	if (!launcherObj->rigidBody) return;

	auto theJson = JsonState::getTheJsonState();
	int launcherIndex = theJson->findObject(launcherObj);
	auto jLauncher = theJson->JSONObjects[launcherIndex];

	auto body = launcherObj->rigidBody;
	if (launcherState == "waiting")
	{
		body->setMassProps(0, { 0,0,0 });

		if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		{
			launcherState = "charging";
		}
	}

	if (launcherState == "charging")
	{
		body->setLinearVelocity(btVector3(0, -5, 0));
		float mass = 1.0f;
		if (jLauncher["Physics"].find("mass") != jLauncher["Physics"].end())
			mass = jLauncher["Physics"]["mass"];
		btVector3 localInertia;
		body->getCollisionShape()->calculateLocalInertia(mass, localInertia);
		body->setMassProps(mass, localInertia);
		
		if (springHoldTime < springHoldMax)
		{
			springHoldTime += deltaTime;
		}

		if (glfwGetKey(window, GLFW_KEY_M) != GLFW_PRESS)
		{
			launcherState = "launching";
		}
	}

	if (launcherState == "launching")
	{
		auto launcherOGPos = jLauncher["positionXYZ"];
		body->setLinearVelocity(btVector3(0, 1, 0) * springHoldTime * 80.f);

		if (launcherOGPos[1].get<float>() - body->getWorldTransform().getOrigin().getY() < 1.f)
		{
			launcherState = "waiting";
			springHoldTime = 0;
		}
	}
}
