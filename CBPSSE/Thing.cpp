#include "Thing.h"
#include "log.h"
#include "f4se\NiNodes.h"
#include <time.h>

#define PI 3.14159265
#define DEBUG 0

Thing::Thing(NiAVObject* obj, BSFixedString& name)
	: boneName(name)
	, velocity(NiPoint3(0, 0, 0))
{
	oldWorldPos = obj->m_worldTransform.pos;

	time = clock();
	firstRun = true;
}

Thing::~Thing() {
}

void showPos(NiPoint3 &p) {
	logger.info("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}

void showRot(NiMatrix43 &r) {
	logger.info("%8.2f %8.2f %8.2f %8.2f\n", r.data[0][0], r.data[0][1], r.data[0][2], r.data[0][3]);
	logger.info("%8.2f %8.2f %8.2f %8.2f\n", r.data[1][0], r.data[1][1], r.data[1][2], r.data[1][3]);
	logger.info("%8.2f %8.2f %8.2f %8.2f\n", r.data[2][0], r.data[2][1], r.data[2][2], r.data[2][3]);
}


float solveQuad(float a, float b, float c) {
	float k1 = (-b + sqrtf(b*b - 4*a*c)) / (2 * a);
	//float k2 = (-b - sqrtf(b*b - 4*a*c)) / (2 * a);
	//logger.error("k2 = %f\n", k2);
	return k1;
}




void Thing::updateConfig(configEntry_t & centry) {
	stiffness = centry["stiffness"];
	stiffness2 = centry["stiffness2"];
	damping = centry["damping"];
	maxOffset = centry["maxoffset"];
	timeTick = centry["timetick"];
	linearX = centry["linearX"];
	linearY = centry["linearY"];
	linearZ = centry["linearZ"];
	rotational = centry["rotational"];
	// Optional entries for backwards compatability 
	if (centry.find("timeStep") != centry.end())
		timeStep = centry["timeStep"];
	else 
		timeStep = 1.0f;
	gravityBias = centry["gravityBias"];
	gravityCorrection = centry["gravityCorrection"];
	cogOffset = centry["cogOffset"];
	if (timeTick <= 1)
		timeTick = 1;

	//zOffset = solveQuad(stiffness2, stiffness, -gravityBias);

	//logger.error("z offset = %f\n", solveQuad(stiffness2, stiffness, -gravityBias));
}

void Thing::dump() {
	//showPos(obj->m_worldTransform.pos);
	//showPos(obj->m_localTransform.pos);
}


static float clamp(float val, float min, float max) {
	if (val < min) return min;
	else if (val > max) return max;
	return val;
}

void Thing::reset() {
	// TODO
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void Thing::update(Actor *actor) {
	auto newTime = clock();
	auto deltaT = newTime - time;

	time = newTime;
	if (deltaT > 64) deltaT = 64;
	if (deltaT < 8) deltaT = 8;

	auto loadedState = actor->unkF0;
	if (!loadedState || !loadedState->rootNode) {
		logger.error("No loaded state for actor %08x\n", actor->formID);
		return;
	}
	auto obj = loadedState->rootNode->GetObjectByName(&boneName);

	if (!obj) {
		logger.error("Couldn't get name for loaded state for actor %08x\n", actor->formID);
		return;
	}

	if (!obj->m_parent) {
		logger.error("Couldn't get bone %s parent for actor %08x\n", boneName.c_str() , actor->formID);
		return;
	}

#if DEBUG
	auto scene_obj = obj;
	while (scene_obj->m_parent && scene_obj->m_name != "skeleton.nif")
	{
		logger.info(scene_obj->m_name);
		logger.info("\n---\n");
		showPos(scene_obj->m_localTransform.pos);
		showPos(scene_obj->m_worldTransform.pos);
		logger.info("---\n");
		showRot(scene_obj->m_localTransform.rot);
		showRot(scene_obj->m_worldTransform.rot);
		logger.info("---\n");
		if (scene_obj->m_parent) {
			showPos((scene_obj->m_worldTransform.rot * scene_obj->m_localTransform.pos) + scene_obj->m_parent->m_worldTransform.pos); // m_worldTransform.pos
			showRot(scene_obj->m_localTransform.rot * scene_obj->m_parent->m_worldTransform.rot); // m_worldTransform.rot
		}
		scene_obj = scene_obj->m_parent;	
	}
#endif

	if (firstRun) {
		orig_local_pos = obj->m_localTransform.pos;
		orig_local_rot = obj->m_localTransform.rot;
	}

#if DEBUG
	logger.error("bone %s for actor %08x\n", boneName.c_str(), actor->formID);
	//showPos(obj->m_parent->m_worldTransform.pos + obj->m_localTransform.pos);
	showPos((obj->m_localTransform.rot.Transpose() * obj->m_localTransform.pos));
#endif

	// Offset to move Center of Mass make rotaional motion more significant
	// This target is 
	NiPoint3 target;

	// TODO: left and right with same parents transforms should be different... example: the butt
	// Relative Left
	//if (obj->m_localTransform.pos.x < 0.0) {
		target = (obj->m_worldTransform.rot * NiPoint3(0, cogOffset, 0)) + obj->m_worldTransform.pos;
	//}
	//// Relative Right
	//else {
	//	target = (obj->m_worldTransform.rot *
	//		(obj->m_localTransform.pos + NiPoint3(0, 0, 0))) + obj->m_parent->m_worldTransform.pos;
	//}

#if DEBUG
	logger.error("World Position: ");
	showPos(obj->m_worldTransform.pos);
	logger.error("Target: ");
	showPos(target);
#endif

	// diff is Difference in position between old and new world position
	NiPoint3 diff = target - oldWorldPos;

	// move up in rotated angle for gravity correction
	diff += obj->m_worldTransform.rot * NiPoint3(0, 0, gravityCorrection);

#if DEBUG
	logger.error("Diff after gravity correction: ");
	showPos(diff);
#endif

	if (fabs(diff.x) > 100 || fabs(diff.y) > 100 || fabs(diff.z) > 100) {
		logger.error("transform reset\n");
		obj->m_localTransform.pos = NiPoint3(0, 0, 0);
		oldWorldPos = target;
		velocity = NiPoint3(0, 0, 0);
		time = clock();
	} else {

		diff *= timeTick / (float)deltaT;
		NiPoint3 posDelta = NiPoint3(0, 0, 0);

		// Compute the "Spring" Force
		NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
		NiPoint3 force = (diff * stiffness) + (diff2 * stiffness2) - NiPoint3(0, 0, gravityBias);

#if DEBUG
		logger.error("Diff2: ");
		showPos(diff2);
		logger.error("Force: ");
		showPos(force);
#endif

		do {
			// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
			//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));
			velocity = (velocity + (force * timeStep)) - (velocity * (damping * timeStep));

			// New position accounting for time
			posDelta += (velocity * timeStep);
			deltaT -= timeTick;
		} while (deltaT >= timeTick);


		NiPoint3 newPos = oldWorldPos + posDelta;
#if DEBUG
		logger.error("posDelta: ");
		showPos(posDelta);
		logger.error("newPos: ");
		showPos(newPos);
#endif
		// clamp the difference to stop the breast severely lagging at low framerates
		diff = newPos- target;

		diff.x = clamp(diff.x, -maxOffset, maxOffset);
		diff.y = clamp(diff.y, -maxOffset, maxOffset);
		diff.z = clamp(diff.z-gravityCorrection, -maxOffset, maxOffset) + gravityCorrection;

		//oldWorldPos = diff + target;

#if DEBUG
		logger.error("diff from newPos: ");
		showPos(diff);
#endif

		//logger.error("set positions\n");
		// move the bones based on the supplied weightings
		// Convert the world translations into local coordinates
		auto invRot = obj->m_localTransform.rot * obj->m_worldTransform.rot.Transpose();
		auto local_diff = invRot * diff;

		//showPos(diff);
		//showPos(local_diff);
		// remove component along bone - might want something closer to world
		//ldiff.y = 0;

		oldWorldPos = diff + target;
		//showRot(obj->m_parent->m_worldTransform.rot * invRot);

#if DEBUG
		logger.error("localTransform.pos: ");
		showPos(obj->m_localTransform.pos);
		logger.error("local_diff: ");
		showPos(local_diff);
#endif
		// scale positions from config
		obj->m_localTransform.pos.x = orig_local_pos.x + (local_diff.x * linearX);
		obj->m_localTransform.pos.y = orig_local_pos.y + (local_diff.y * linearY);
	    obj->m_localTransform.pos.z = orig_local_pos.z + (local_diff.z * linearZ);

		// do some rotation
		auto rdiff = local_diff * rotational;
#if DEBUG
		logger.error("rdiff: ");
		showPos(rdiff);
#endif
		float heading, attitude, bank;
		orig_local_rot.GetEulerAngles(&heading, &attitude, &bank);
		obj->m_localTransform.rot.SetEulerAngles(heading + rdiff.x, attitude + rdiff.y, bank + rdiff.z);

	}
	firstRun = false;
#if DEBUG
	logger.error("end update()\n");
#endif



}