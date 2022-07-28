#include <algorithm>

#include "ActorUtils.h"
#include "log.h"

#include "f4se/GameExtraData.h"
#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"

std::string actorUtils::GetActorRaceEID(Actor* actor) {
    if (!IsActorValid(actor)) {
        logger.Info("GetActorRaceEID: no actor!\n");
        return "";
    }

    return std::string(actor->race->editorId.c_str());
}

bool actorUtils::IsActorMale(Actor *actor)
{
    if (!IsActorValid(actor)) {
        logger.Info("IsActorMale: no actor!\n");
        return false;
    }

    TESNPC* actorNPC = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

    auto npcSex = actorNPC ? CALL_MEMBER_FN(actorNPC, GetSex)() : 1;

    if (npcSex == 0) //Actor is male
        return true;
    else
        return false;
}

bool actorUtils::IsActorInPowerArmor(Actor* actor) {
    bool isInPowerArmor = false;
    if (!IsActorValid(actor)) {
        logger.Info("IsActorInPowerArmor: no actor!\n");
        return true; // will force game to not call Update
    }
    if (!actor->extraDataList) {
        logger.Info("IsActorInPowerArmor: no extraDataList!\n");
        return true; // will force game to not call Update
    }

    isInPowerArmor = actor->extraDataList->HasType(kExtraData_PowerArmor);
    //logger.Info("is in power armor: %d\n", isInPowerArmor);
    return isInPowerArmor;
}

bool actorUtils::IsActorTrackable(Actor* actor) {
    if (!IsActorValid(actor)) {
        logger.Info("IsActorTrackable: actor is not valid.\n");
        return false;
    }

    bool inRaceWhitelist = find(raceWhitelist.begin(), raceWhitelist.end(), actorUtils::GetActorRaceEID(actor)) != raceWhitelist.end();
    return (!playerOnly || (actor->formID == 0x14 && playerOnly)) &&
           (!maleOnly || (IsActorMale(actor) && maleOnly)) &&
           (!femaleOnly || (!IsActorMale(actor) && femaleOnly)) &&
           (!npcOnly || (actor->formID != 0x14 && npcOnly)) &&
           (!useWhitelist || (inRaceWhitelist && useWhitelist));
}

bool actorUtils::IsActorValid(Actor* actor) {
    if (!actor) {
        logger.Info("IsActorValid: actor is null\n");
        return false;
    }
    if (actor->flags & TESForm::kFlag_IsDeleted) {
        logger.Info("IsActorValid: actor has deleted flag\n");
        return false;
    }
    if (actor && actor->unkF0 && actor->unkF0->rootNode) {
        return true;
    }
    logger.Info("IsActorValid: actor is not valid state\n");
    return false;
}

bool actorUtils::IsBoneInWhitelist(Actor* actor, std::string boneName) {
    if (!IsActorValid(actor)) {
        logger.Info("IsBoneInWhitelist: actor is not valid.\n");
        return false;
    }

    auto raceEID = actorUtils::GetActorRaceEID(actor);
    if (whitelist.find(boneName) != whitelist.end()) {
        auto racesMap = whitelist.at(boneName);
        if (racesMap.find(raceEID) != racesMap.end()) {
            if (IsActorMale(actor))
                return racesMap.at(raceEID).male;
            else
                return racesMap.at(raceEID).female;
        }
    }
    return false;
}

const actorUtils::EquippedArmor actorUtils::GetActorEquippedArmor(Actor* actor, UInt32 slot) {
    bool isEquipped = false;
    bool isArmorIgnored = false;

    if (!actorUtils::IsActorValid(actor)) {
        logger.Error("Actor is not valid");
        return actorUtils::EquippedArmor{ nullptr, nullptr };
    }
    if (!actor->equipData || !actor->equipData->slots) {
        logger.Error("Actor has no equipData");
        return actorUtils::EquippedArmor{ nullptr, nullptr };
    }

    isEquipped = actor->equipData->slots[slot].item;

    // Check if armor is ignored
    if (isEquipped) {
        if (!actor->equipData->slots[slot].item) {
            logger.Error("slot %d item check failed.", slot);
            // redundant check but just in case
            return actorUtils::EquippedArmor{ nullptr, nullptr };
        }
        return actorUtils::EquippedArmor{ actor->equipData->slots[slot].item, actor->equipData->slots[slot].model };
    }

    return actorUtils::EquippedArmor{ nullptr, nullptr };
}

config_t actorUtils::BuildConfigForActor(Actor* actor) {
    config_t baseConfig = config;

    for (auto overrideConfigIter = configArmorOverrideMap.rbegin(); overrideConfigIter != configArmorOverrideMap.rend(); ++overrideConfigIter) {
        auto data = overrideConfigIter->second;

        std::vector<actorUtils::EquippedArmor> equippedList;
        for (auto slot : data.slots) {
            auto equipped = actorUtils::GetActorEquippedArmor(actor, slot);
            if (equipped.armor && equipped.model) {
                equippedList.push_back(equipped);
            }
        }

        auto armorFormID = data.armors.begin();
        for (; armorFormID != data.armors.end(); ++armorFormID) {
            bool breakOutside = false;
            for (auto equipped : equippedList) {
                if (*armorFormID == equipped.armor->formID || *armorFormID == equipped.model->formID) {
                    if (data.isWhitelist) {
                        for (auto val : data.config) {
                            if (data.config[val.first].empty()) {
                                baseConfig.erase(val.first);
                            }
                            else {
                                baseConfig[val.first] = val.second;
                            }
                        }
                    }

                    breakOutside = true;
                    break;
                }
            }

            if (breakOutside) {
                break;
            }
        }

        if (!data.isWhitelist && armorFormID == data.armors.end() && !equippedList.empty()) {
            for (auto val : data.config) {
                if (data.config[val.first].empty()) {
                    baseConfig.erase(val.first);
                }
                else {
                    baseConfig[val.first] = val.second;
                }
            }
        }
    }

    return baseConfig;
}