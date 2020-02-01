#include "ActorEntry.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"

bool ActorEntry::IsMale()
{
    TESNPC* actorNPC = DYNAMIC_CAST(this->actor->baseForm, TESForm, TESNPC);

    auto npcSex = actorNPC ? CALL_MEMBER_FN(actorNPC, GetSex)() : 1;

    if (npcSex == 0) //Actor is male
        return true;
    else
        return false;
}

bool ActorEntry::IsInPowerArmor() {
    if (!this->actor)
        return false;
    return !this->actor->extraDataList->HasType(kExtraData_PowerArmor);
}

bool ActorEntry::IsTorsoArmorEquipped() {
    bool isEquipped = false;
    if (!this->actor)
        return false;
    // 11 IS ARMOR TORSO SLOT (41 minus 30??)
    isEquipped = this->actor->equipData->slots[11].item;
    return isEquipped;
}