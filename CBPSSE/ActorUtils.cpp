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

// May want to change this for any armor
bool actorUtils::IsActorTorsoArmorEquipped(Actor* actor) {
    bool isEquipped = false;
    bool isArmorIgnored = false;

    if (!IsActorValid(actor)) {
        logger.Info("Actor is not valid\n");
        return false;
    }
    if (!actor->equipData && !actor->equipData->slots) {
        logger.Info("Actor has no equipData\n");
        return false;
    }

    // 11 IS ARMOR TORSO SLOT (41 minus 30??)
    isEquipped = actor->equipData->slots[11].item;

    // Check if armor is ignored
    if (isEquipped) {
        if (!actor->equipData->slots[11].item) {
            logger.Info("slot 11 item check failed.\n");
            // redundant check but just in case
            return false;
        }
        UInt32 bodyFormID;
        if (!actor->equipData->slots[3].item) {
            logger.Info("slot 3 item check failed.\n");
            bodyFormID = 0;
        }
        else {
            bodyFormID = actor->equipData->slots[3].item->formID;;
        }
        auto torsoFormID = actor->equipData->slots[11].item->formID;

        //logger.Info("torsoFormID: 0x%08x\n", torsoFormID);
        //logger.Info("bodyFormID: 0x%08x\n", bodyFormID);

        auto torsoIdIter = armorIgnore.find(torsoFormID);

        if (torsoIdIter != armorIgnore.end())
            isArmorIgnored = true;
        else if ((torsoFormID == bodyFormID - 1)) {
            auto bodyIdIter = armorIgnore.find(bodyFormID);
            if (bodyIdIter != armorIgnore.end())
                isArmorIgnored = true;
        }
    }

    return isEquipped && !isArmorIgnored;
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
