#include "PhysicsDebug.h"
#include <iostream>
#include "globalStuff.h"
#include "nConvert.h"

void PhysicsDebug::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	glm::vec4 glm_color(nConvert::vecFromBullet(color), 1.0f);
	::pDebugRenderer->addLine(
		nConvert::vecFromBullet(from),
		nConvert::vecFromBullet(to),
		glm_color);
}

void PhysicsDebug::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
	int lifeTime, const btVector3& color)
{
}

void PhysicsDebug::reportErrorWarning(const char* warningString)
{
	std::cout << "physics error: " << warningString << std::endl;
}

void PhysicsDebug::draw3dText(const btVector3& location, const char* textString)
{
}

void PhysicsDebug::setDebugMode(int debugMode)
{
}

int PhysicsDebug::getDebugMode() const
{
	return DBG_DrawWireframe;
}
