#pragma once
#include <set>
#include "cGameObject.h"

struct Pinball
{
	static std::set<cGameObject*> pointGivers;
	static int points, lives;
};

