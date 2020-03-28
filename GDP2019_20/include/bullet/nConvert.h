#pragma once
#include "btBulletDynamicsCommon.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace nConvert
{
    inline btVector3 vecToBullet(const glm::vec3& v) 
    {
        return { v.x, v.y, v.z };
    }
    inline glm::vec3 vecFromBullet(const btVector3& v) 
    {
        return glm::vec3(v.x(), v.y(), v.z());
    }

    inline void transformMat4(
        const btTransform& transformIn,
        glm::mat4& transformOut
    ) 
    {
        transformIn.getOpenGLMatrix(&transformOut[0][0]);
    }
}
