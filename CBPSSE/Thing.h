#pragma once
#include <f4se\NiTypes.h>
#include <f4se\NiObjects.h>
#include <f4se\GameReferences.h>
#include <time.h>
#include "config.h"

class Thing {
    BSFixedString boneName;
    NiPoint3 oldWorldPos;
    NiPoint3 velocity;
    clock_t time;

public:
    float stiffness = 0.5f;
    float stiffness2 = 0.0f;
    float damping = 0.2f;
    float maxOffsetX = 5.0f;
    float maxOffsetY = 5.0f;
    float maxOffsetZ = 5.0f;
    float cogOffsetX = 0.0f;
    float cogOffsetY = 0.0f;
    float cogOffsetZ = 0.0f;

    float gravityBias = 0.0f;
    float gravityCorrection = 0.0f;
    float timeTick = 4.0f;
    float linearX = 0.0f;
    float linearY = 0.0f;
    float linearZ = 0.0f;
    float rotationalX = 0.0f;
    float rotationalY = 0.0f;
    float rotationalZ = 0.0f;

    float rotateLinearX = 0.0f;
    float rotateLinearY = 0.0f;
    float rotateLinearZ = 0.0f;

    float rotateRotationX = 0.0f;
    float rotateRotationY = 0.0f;
    float rotateRotationZ = 0.0f;

    float timeStep = 0.016f;

    Thing(NiAVObject *obj, BSFixedString &name);
    ~Thing();

    void updateConfig(configEntry_t &centry);
    void update(Actor *actor);	
    void reset(Actor * actor);

    void showPos(NiPoint3& p);
    void showRot(NiMatrix43& r);
};