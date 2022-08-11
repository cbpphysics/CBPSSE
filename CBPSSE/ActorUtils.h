#pragma once
#include "f4se/GameReferences.h"
#include "config.h"

namespace actorUtils {
    struct EquippedArmor {
        const TESForm* armor;
        const TESForm* model;
    };
    
    std::string GetActorRaceEID(Actor* actor);
    NiAVObject* GetBaseSkeleton(Actor* actor);
    bool IsActorInPowerArmor(Actor* actor);
    bool IsActorMale(Actor* actor);
    bool IsActorTrackable(Actor* actor);
    bool IsActorValid(Actor* actor);
    bool IsBoneValid(Actor* actor, BSFixedString* boneName);
    bool IsBoneInWhitelist(Actor* actor, std::string boneName);

    const EquippedArmor GetActorEquippedArmor(Actor* actor, UInt32 slot);
    config_t BuildConfigForActor(Actor* actor);
}