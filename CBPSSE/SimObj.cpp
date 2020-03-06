#include "f4se/NiNodes.h"
#include "f4se/GameForms.h"
#include "f4se/GameRTTI.h"

#include "ActorUtils.h"
#include "config.h"
#include "log.h"
#include "PapyrusOCBP.h"
#include "SimObj.h"

using actorUtils::IsBoneInWhitelist;
using actorUtils::IsActorInPowerArmor;

// Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory
std::vector<std::string> boneNames;

SimObj::SimObj(Actor *actor, config_t &config)
    : things(4) {
    id = actor->formID;
}

SimObj::~SimObj() {
}


bool SimObj::Bind(Actor *actor, std::vector<std::string>& boneNames, config_t &config)
{
//	logger.error("bind\n");

    auto loadedData = actor->unkF0;
    if (loadedData && loadedData->rootNode) {
        bound = true;

        things.clear();
        for (std::string b : boneNames) {
            const char* bone_c_str = b.c_str();
            BSFixedString cs(bone_c_str);
            auto bone = loadedData->rootNode->GetObjectByName(&cs);
            if (!bone) {
                logger.Info("Failed to find Bone %s for actor %08x\n", b.c_str(), actor->formID);
            } else {
                //logger.info("Doing Bone %s for actor %08x\n", b, actor->formID);
                things.emplace(b, Thing(bone, cs));
            }
        }
        UpdateConfig(config);
        return  true;
    }
    return false;
}

void SimObj::Update(Actor *actor) {
    if (!bound)
        return;
    //logger.error("update\n");
    for (auto &t : things) {

        // Might be a better way to do this
        if (boneIgnores.find(actor->formID) != boneIgnores.end()) {
            auto actorBoneMap = boneIgnores.at(actor->formID);
            if (actorBoneMap.find(t.first) != actorBoneMap.end()) {
                if (actorBoneMap.at(t.first)) {
                    continue;
                }
            }
        }

        if (!useWhitelist || (IsBoneInWhitelist(actor, t.first) && useWhitelist) &&
            !IsActorInPowerArmor(actor))
        {
            t.second.Update(actor);
        }
    }
    //logger.error("end SimObj update\n");
}

bool SimObj::UpdateConfig(config_t & config) {
    for (auto &thing : things) {
        thing.second.UpdateConfig(config[std::string(thing.first)]);
    }
    return true;
}