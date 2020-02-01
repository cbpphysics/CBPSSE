#include "ActorEntry.h"
#include "f4se/GameExtraData.h"

bool ActorEntry::isInPowerArmor() {
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