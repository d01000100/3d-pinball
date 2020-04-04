#pragma once
#include "LinearMath/btIDebugDraw.h"

/*
 * Class that implements the bullet debug drawer.
 *
 * It tells bullet how to draw things on the screen, mainly collision bodies
 * For now, it draws all bodies with lines
 */
class PhysicsDebug : public btIDebugDraw
{
public:
	/*
	 * How to draw a line from one point to another in a specific color.
	 * 
	 * We use a shader to draw lines that do not consider lighting to be able to
	 * see them easily
	 */
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
	/* Empty implementation */
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
		const btVector3& color) override;
	/* Print an error to the standard output */
	void reportErrorWarning(const char* warningString) override;
	/* Empty implementation */
	void draw3dText(const btVector3& location, const char* textString) override;
	/* Empty implementation */
	void setDebugMode(int debugMode) override;
	/* returns DBG_DrawWireframe */
	int getDebugMode() const override;
};