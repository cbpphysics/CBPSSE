#pragma once
#define DEBUG

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

#include "ActorEntry.h"
#include "ActorUtils.h"
#include "log.h"
#include "Thing.h"
#include "config.h"
#include "SimObj.h"
#include "f4se/GameRTTI.h"
#include "f4se/GameForms.h"
#include "f4se/GameReferences.h"
#include "f4se/NiTypes.h"
#include "f4se/NiNodes.h"
#include "f4se/BSGeometry.h"
#include "f4se/GameThreads.h"
#include "f4se/PluginAPI.h"
#include "f4se/GameStreams.h"
#include "f4se/GameExtraData.h"

#pragma warning(disable : 4996)

using actorUtils::IsActorMale;
using actorUtils::IsActorTorsoArmorEquipped;
using actorUtils::IsActorInPowerArmor;

extern F4SETaskInterface *g_task;


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
    //logger.info("%savObj Name = %s, RTTI = %s\n", ss, avObj->m_name, avObj->GetRTTI()->name);

    //NiNode *node = avObj->GetAsNiNode();
    //if (node) {
    //	logger.info("%snode %s, RTTI %s\n", ss, node->m_name, node->GetRTTI()->name);
    //}
    //return false;
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
    if (!player || !player->unkF0) goto FAILED;

    auto cell = player->parentCell;
    if (!cell) goto FAILED;

    if (cell != curCell) {
        logger.error("cell change %d\n", cell);
        curCell = cell;
        actors.clear();
    } else {
        // Attempt to get cell's objects
        for (int i = 0; i < cell->objectList.count; i++) {
            auto ref = cell->objectList[i];
            if (ref) {
                // Attempt to get actors
                auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);
                if (actor && actor->unkF0) {
                    // Find if actors is already being tracked
                    auto soIt = actors.find(actor->formID);
                    if (soIt == actors.end() &&
                        IsActorInPowerArmor(actor) &&
                        (!playerOnly || (actor->formID == 0x14 && playerOnly)) &&
                        (!maleOnly || (IsActorMale(actor) && maleOnly)) &&
                        (!femaleOnly || (!IsActorMale(actor) && femaleOnly))) {
                        logger.info("Tracking Actor with form ID %08x in cell %ld\n", actor->formID, actor->parentCell);
                        //logger.info("%s\n", actor->unk08-> & 1);
                        //logger.info("%s\n", actor->race->textureModel[1].GetModelName());
                        auto obj = SimObj(actor, config);
                        if (obj.actorValid(actor)) {
                            actors.emplace(actor->formID, obj);
                            actorEntries.emplace_back(ActorEntry{ actor->formID, actor });
                        }
                    }
                    else if (soIt->second.actorValid(actor)) {
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

    // Reload config
    static int count = 0;
    if (configReloadCount && count++ > configReloadCount) {
        count = 0;
        auto reloadActors = loadConfig();
        for (auto &a : actors) {
            a.second.updateConfig(config);
        }

        // Clear actors
        if (reloadActors) {
            actors.clear();
        }
    }

    //logger.error("Updating %d entities\n", actorEntries.size());
    for (auto &a : actorEntries) {
        auto objIterator = actors.find(a.id);
        if (objIterator == actors.end()) {
            //logger.error("Sim Object not found in tracked actors\n");
        }
        else {
            auto &obj = objIterator->second;
            if (obj.isBound()) {
                obj.update(a.actor);
            }
            else {
                if (IsActorTorsoArmorEquipped(a.actor)) {
                    logger.info("torso armor detected on actor %x\n", a.actor->formID);
                    obj.bind(a.actor, boneNames, configArmor);
                }
                else {
                    obj.bind(a.actor, boneNames, config);
                }
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


class ScanDelegate : public ITaskDelegate {
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