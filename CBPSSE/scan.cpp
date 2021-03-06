#pragma once


#include <cstdlib>
#include <cstdio>

#include <typeinfo>

#include <memory>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <atomic>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <atomic>
#include <functional>

#include <unordered_set>
#include <unordered_map>

#include "log.h"
#include "Thing.h"
#include "config.h"
#include "SimObj.h"
#include "skse64/GameRTTI.h"
#include "skse64/GameForms.h"
#include "skse64/GameReferences.h"
#include "skse64/NiTypes.h"
#include "skse64/NiNodes.h"
#include "skse64/NiGeometry.h"
#include "skse64/GameThreads.h"
#include "skse64/PluginAPI.h"
#include "skse64/GameStreams.h"


#pragma warning(disable : 4996)


extern SKSETaskInterface *g_task;


//void UpdateWorldDataToChild(NiAVObject)
void dumpTransform(NiTransform t) {
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[0][0], t.rot.data[0][1], t.rot.data[0][2]);
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[1][0], t.rot.data[1][1], t.rot.data[1][2]);
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[2][0], t.rot.data[2][1], t.rot.data[2][2]);

	Console_Print("%8.2f %8.2f %8.2f", t.pos.x, t.pos.y, t.pos.z);
	Console_Print("%8.2f", t.scale);
}


bool visitObjects(NiAVObject  *parent, std::function<bool(NiAVObject*, int)> functor, int depth = 0) {
	if (!parent) return false;
	NiNode * node = parent->GetAsNiNode();
	if (node) {
		if (functor(parent, depth))
			return true;

		for (UInt32 i = 0; i < node->m_children.m_emptyRunStart; i++) {
			NiAVObject * object = node->m_children.m_data[i];
			if (object) {
				if (visitObjects(object, functor, depth+1))
					return true;
			}
		}
	}
	else if (functor(parent, depth))
		return true;

	return false;
}

std::string spaces(int n) {
	auto s = std::string(n , ' ');
	return s;
}

bool printStuff(NiAVObject *avObj, int depth) {
	std::string sss = spaces(depth);
	const char *ss = sss.c_str();
	logger.info("%savObj Name = %s, RTTI = %s\n", ss, avObj->m_name, avObj->GetRTTI()->name);

	NiNode *node = avObj->GetAsNiNode();
	if (node) {
		logger.info("%snode %s, RTTI %s\n", ss, node->m_name, node->GetRTTI()->name);
	}
	return false;
}


void dumpVec(NiPoint3 p) {
	logger.info("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}




template<class T>
inline void safe_delete(T*& in) {
	if (in) {
		delete in;
		in = NULL;
	}
}



std::unordered_map<UInt32, SimObj> actors;

TESObjectCELL *curCell = nullptr;


struct ActorEntry {
	UInt32 id;
	Actor *actor;
};

void updateActors() {
	//LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	//LARGE_INTEGER frequency;

	//QueryPerformanceFrequency(&frequency);
	//QueryPerformanceCounter(&startingTime);

	// We scan the cell and build the list every time - only look up things by ID once
	// we retain all state by actor ID, in a map - it's cleared on cell change
	std::vector<ActorEntry> actorEntries;

	//logger.error("scan Cell\n");
	auto player = DYNAMIC_CAST(LookupFormByID(0x14), TESForm, Actor);
	if (!player || !player->loadedState) goto FAILED;

	auto cell = player->parentCell;
	if (!cell) goto FAILED;

	if (cell != curCell) {
		logger.error("cell change %d\n", cell);
		curCell = cell;
		actors.clear();
	} else {
		for (int i = 0; i < cell->refData.maxSize; i++) {
			auto ref = cell->refData.refArray[i];
			if (ref.unk08 != NULL && ref.ref) {
				auto actor = DYNAMIC_CAST(ref.ref, TESObjectREFR, Actor);
				if (actor && actor->loadedState) {
					auto soIt = actors.find(actor->formID);
					if (soIt == actors.end()) {
						//logger.info("Tracking Actor with form ID %08x in cell %ld\n", actor->formID, actor->parentCell);
						auto obj = SimObj(actor, config);
						if (obj.actorValid(actor)) {
							actors.emplace(actor->formID, obj);
							actorEntries.emplace_back(ActorEntry{ actor->formID, actor });
						}
					} else if (soIt->second.actorValid(actor)) {
						actorEntries.emplace_back(ActorEntry{ actor->formID, actor });
					}
				}
			}
		}
	}

	//static bool done = false;
	//if (!done && player->loadedState->node) {
	//	visitObjects(player->loadedState->node, printStuff);
	//	BSFixedString cs("UUNP");
	//	auto bodyAV = player->loadedState->node->GetObjectByName(&cs.data);
	//	BSTriShape *body = bodyAV->GetAsBSTriShape();
	//	logger.info("GetAsBSTriShape returned  %lld\n", body);
	//	auto geometryData = body->geometryData;
	//	//logger.info("Num verts = %d\n", geometryData->m_usVertices);


	//	done = true;
	//}

	static int count = 0;
	if (configReloadCount && count++ > configReloadCount) {
		count = 0;
		loadConfig();
		for (auto &a : actors) {
			a.second.updateConfig(config);
		}
	}
	//logger.error("Updating %d entites\n", actorEntries.size());
	for (auto &a : actorEntries) {
		auto objIt = actors.find(a.id);
		if (objIt == actors.end()) {
			logger.error("missing Sim Object\n");
		} else {
			auto &obj = objIt->second;
			if (obj.isBound()) {
				obj.update(a.actor);
			} else {
				obj.bind(a.actor, femaleBones, config);
			}
		}
	}

FAILED:
	return;
	//QueryPerformanceCounter(&endingTime);
	//elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	//elapsedMicroseconds.QuadPart *= 1000000000LL;
	//elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	//logger.info("Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);
}


class ScanDelegate : public TaskDelegate {
public:
	virtual void Run() {
		updateActors();
	}
	virtual void Dispose() {
		delete this;
	}

};


void scaleTest() {
	g_task->AddTask(new ScanDelegate());
	return;
}