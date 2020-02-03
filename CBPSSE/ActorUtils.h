#pragma once
#include "f4se/GameReferences.h"

namespace actorUtils {
    // The statics needs to not be here
    bool IsActorInPowerArmor(Actor* actor);
    bool IsActorTorsoArmorEquipped(Actor* actor);
    bool IsActorMale(Actor* actor);
}