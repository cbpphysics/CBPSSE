#include <algorithm>

#include "ActorUtils.h"
#include "log.h"

#include "f4se/GameExtraData.h"
#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"

std::string actorUtils::GetActorRaceEID(Actor* actor) {
    return std::string(actor->race->editorId.c_str());
}

bool actorUtils::IsActorMale(Actor *actor)
{
    TESNPC* actorNPC = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

    auto npcSex = actorNPC ? CALL_MEMBER_FN(actorNPC, GetSex)() : 1;

    if (npcSex == 0) //Actor is male
        return true;
    else
        return false;
}

bool actorUtils::IsActorInPowerArmor(Actor* actor) {
    if (!actor)
        return false;
    return !actor->extraDataList->HasType(kExtraData_PowerArmor);
}

// May want to change this for any armor
bool actorUtils::IsActorTorsoArmorEquipped(Actor* actor) {
    bool isEquipped = false;
    if (!actor)
        return false;
    // 11 IS ARMOR TORSO SLOT (41 minus 30??)
    isEquipped = actor->equipData->slots[11].item;
    return isEquipped;
}

bool actorUtils::IsActorTrackable(Actor* actor) {
    bool inRaceWhitelist = find(raceWhitelist.begin(), raceWhitelist.end(), actorUtils::GetActorRaceEID(actor)) != raceWhitelist.end();
    return IsActorInPowerArmor(actor) &&
            (!playerOnly || (actor->formID == 0x14 && playerOnly)) &&
            (!maleOnly || (IsActorMale(actor) && maleOnly)) &&
            (!femaleOnly || (!IsActorMale(actor) && femaleOnly)) &&
            (!npcOnly || (actor->formID != 0x14 && npcOnly)) &&
            (!useWhitelist || (inRaceWhitelist && useWhitelist));
}

bool actorUtils::IsBoneInWhitelist(Actor* actor, std::string boneName) {
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