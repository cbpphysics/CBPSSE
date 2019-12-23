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
	NiPoint3 orig_local_pos;
	NiMatrix43 orig_local_rot;
	clock_t time;

public:
	float stiffness = 0.5f;
	float stiffness2 = 0.0f;
	float damping = 0.2f;
	float maxOffset = 5.0f;
	float cogOffset = 0.0f;
	float gravityBias = 0.0f;
	float gravityCorrection = 0.0f;
	//float zOffset = 0.0f;	// Computed based on GravityBias value
	float timeTick = 4.0f;
	float linearX = 0;
	float linearY = 0;
	float linearZ = 0;
	float rotational = 0.1;
	float timeStep = 1.0f;

	boolean firstRun;

	//std::unordered_map<const char*, NiMatrix43> boneRotationMatrices;

	Thing(NiAVObject *obj, BSFixedString &name);
	~Thing();

	void updateConfig(configEntry_t &centry);
	void dump();

	void update(Actor *actor);	
	void reset();

};