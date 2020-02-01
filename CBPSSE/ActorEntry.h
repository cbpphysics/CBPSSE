#pragma once
#include "f4se/GameReferences.h"

class ActorEntry {
    public:
    UInt32 id;
    Actor* actor;

    bool IsInPowerArmor();
    bool IsTorsoArmorEquipped();
    bool IsMale();
};