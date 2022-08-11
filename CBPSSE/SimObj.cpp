#include "f4se/NiNodes.h"
#include "f4se/GameForms.h"
#include "f4se/GameRTTI.h"

#include "ActorUtils.h"
#include "config.h"
#include "log.h"
#include "PapyrusOCBP.h"
#include "SimObj.h"

using actorUtils::GetBaseSkeleton;
using actorUtils::IsBoneInWhitelist;
using actorUtils::IsActorInPowerArmor;

// Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory
std::vector<std::string> boneNames;

SimObj::SimObj(Actor *actor, config_t &config)
    : things(4) {
    id = actor->formID;
    gender = Gender::Unassigned;
    raceEid = std::string("");
}

SimObj::~SimObj() {
}

bool SimObj::AddBonesToThings(Actor* actor, std::vector<std::string>& boneNames) {
    logger.Error("%s\n", __func__);
    if (!actor) {
        return false;
    }
    auto loadedData = actor->unkF0;

    if (loadedData && loadedData->rootNode) {
        for (std::string b : boneNames) {
            if (!useWhitelist || (IsBoneInWhitelist(actor, b) && useWhitelist)) {
                logger.Error("%s: adding Bone %s for actor %08x\n", __func__, b.c_str(), actor->formID);
                BSFixedString cs(b.c_str());
                auto bone = loadedData->rootNode->GetObjectByName(&cs);
                auto findBone = things.find(b);
                if (!bone) {
                    logger.Info("%s: Failed to find Bone %s for actor %08x\n", __func__, b.c_str(), actor->formID);
                }
                else if (findBone == things.end()) {
                    //logger.info("Doing Bone %s for actor %08x\n", b, actor->formID);
                    things.emplace(b, Thing(bone, cs));
                }
            }
        }
    }
    return true;
}

bool SimObj::Bind(Actor *actor, std::vector<std::string>& boneNames, config_t &config)
{
    if (!actor) {
        return false;
    }
    auto loadedData = actor->unkF0;
    if (loadedData && loadedData->rootNode) {
        bound = true;

        things.clear();
        
        if (actorUtils::IsActorMale(actor)) {
            gender = Gender::Male;
        }
        else {
            gender = Gender::Female;
        }

        raceEid = actorUtils::GetActorRaceEID(actor);

        AddBonesToThings(actor, boneNames);
        UpdateConfig(config);
        return  true;
    }
    return false;
}

SimObj::Gender SimObj::GetGender() {
    return gender;
}

std::string SimObj::GetRaceEID() {
    return raceEid;
}

void SimObj::Reset() {
    bound = false;
    things.clear();
}

void SimObj::Update(Actor *actor) {
    if (!bound)
        return;

    for (auto &t : things) {
        //logger.Info("SimObj update: doing thing %s\n", t.first.c_str());
        
        // Might be a better way to do this
        if (boneIgnores.find(actor->formID) != boneIgnores.end()) {
            auto actorBoneMap = boneIgnores.at(actor->formID);
            if (actorBoneMap.find(t.first) != actorBoneMap.end()) {
                if (actorBoneMap.at(t.first)) {
                    continue;
                }
            }
        }

        if (!useWhitelist ||
            (IsBoneInWhitelist(actor, t.first) && useWhitelist) &&
            !IsActorInPowerArmor(actor) &&
            NULL != GetBaseSkeleton(actor)) {
            t.second.UpdateThing(actor);
        }
    }
}

bool SimObj::UpdateConfig(config_t& config) {
    logger.Error("%s\n", __func__);
    for (auto &thing : things) {
        //logger.Info("%s: Updating config for Thing %s\n", __func__, thing.first.c_str());
        thing.second.UpdateConfig(config[std::string(thing.first)]);
    }
    return true;
}