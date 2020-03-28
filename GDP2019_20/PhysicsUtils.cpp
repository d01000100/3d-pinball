#include "PhysicsUtils.h"
#include "PhysicsDebug.h"

auto mCollisions = new btDefaultCollisionConfiguration();
///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
auto mCollisionDispatcher = new btCollisionDispatcher(mCollisions);
///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
auto mOverlappingPairs = new btDbvtBroadphase();
///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
auto mConstraints = new btSequentialImpulseConstraintSolver;
btDiscreteDynamicsWorld* PhysicsUtils::theWorld = new btDiscreteDynamicsWorld(
	mCollisionDispatcher,
	mOverlappingPairs,
	mConstraints,
	mCollisions
);