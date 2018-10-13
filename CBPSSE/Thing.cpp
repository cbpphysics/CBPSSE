#include "Thing.h"
#include "log.h"
#include "skse64\NiNodes.h"
#include <time.h>

Thing::Thing(NiAVObject *obj, BSFixedString &name)
	: boneName(name)
	, velocity(NiPoint3(0, 0, 0))
{
	oldWorldPos = obj->m_worldTransform.pos;
	time = clock();
}

Thing::~Thing() {
}

void showPos(NiPoint3 &p) {
	logger.info("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}

void showRot(NiMatrix33 &r) {
	logger.info("%8.2f %8.2f %8.2f\n", r.data[0][0], r.data[0][1], r.data[0][2]);
	logger.info("%8.2f %8.2f %8.2f\n", r.data[1][0], r.data[1][1], r.data[1][2]);
	logger.info("%8.2f %8.2f %8.2f\n", r.data[2][0], r.data[2][1], r.data[2][2]);
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

	auto loadedState = actor->loadedState;
	if (!loadedState || !loadedState->node) {
		logger.error("No loaded state for actor %08x\n", actor->formID);
		return;
	}
	auto obj = loadedState->node->GetObjectByName(&boneName.data);
	if (!obj)
		return;

	//Offset to move Center of Mass make rotaional motion more significant  
	NiPoint3 target = obj->m_parent->m_worldTransform * NiPoint3(0, cogOffset, 0);
	//logger.error("Target: ");
	//showPos(target);
	NiPoint3 diff = target - oldWorldPos;
	diff += obj->m_parent->m_worldTransform.rot * NiPoint3(0, 0, gravityCorrection);

	if (fabs(diff.x) > 100 || fabs(diff.y) > 100 || fabs(diff.z) > 100) {
		//logger.error("transform reset\n");
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
		//showPos(diff);
		//showPos(force);

		do {
			// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
			//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));
			velocity = (velocity + (force * timeStep)) - (velocity * (damping * timeStep));

			// New position accounting for time
			posDelta += (velocity * timeStep);
			deltaT -= timeTick;
		} while (deltaT >= timeTick);


		NiPoint3 newPos = oldWorldPos + posDelta;
		// clamp the difference to stop the breast severely lagging at low framerates
		auto diff = newPos - target;
		diff.x = clamp(diff.x, -maxOffset, maxOffset);
		diff.y = clamp(diff.y, -maxOffset, maxOffset);
		diff.z = clamp(diff.z-gravityCorrection, -maxOffset, maxOffset) + gravityCorrection;

		oldWorldPos = diff + target;

		//logger.error("set positions\n");
		// move the bones based on the supplied weightings
		// Convert the world translations into local coordinates
		auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();
		auto ldiff = invRot * diff;

		// remove component along bone - might want something closer to worldY
		//ldiff.y = 0;

		oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;

		obj->m_localTransform.pos.x = ldiff.x * linearX;
		obj->m_localTransform.pos.y = ldiff.y * linearY;
		obj->m_localTransform.pos.z = ldiff.z * linearZ;
		auto rdiff = ldiff * rotational;
		obj->m_localTransform.rot.SetEulerAngles(0, 0, rdiff.z);


	}
	//logger.error("end update()\n");




}