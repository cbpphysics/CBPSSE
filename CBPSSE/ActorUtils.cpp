#include "ActorUtils.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"

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

// should change this for any armor
bool actorUtils::IsActorTorsoArmorEquipped(Actor* actor) {
    bool isEquipped = false;
    if (!actor)
        return false;
    // 11 IS ARMOR TORSO SLOT (41 minus 30??)
    isEquipped = actor->equipData->slots[11].item;
    return isEquipped;
}