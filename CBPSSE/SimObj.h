#pragma once

#include <unordered_map>
#include <vector>
#include "f4se/GameReferences.h"
#include "Thing.h"
#include "config.h"

class SimObj {

public:
    enum class Gender
    {
        Male,
        Female,
        Unassigned
    };

    std::unordered_map<std::string, Thing> things;
    SimObj(Actor *actor, config_t &config);
    SimObj() {}
    ~SimObj();
    bool AddBonesToThings(Actor* actor, std::vector<std::string>& boneNames);
    bool Bind(Actor *actor, std::vector<std::string> &boneNames, config_t &config);
    Gender GetGender();
    std::string GetRaceEID();
    void Reset();
    void Update(Actor *actor);
    bool UpdateConfig(config_t& config);
    bool IsBound() { return bound; }
private:
    UInt32 id = 0;
    bool bound = false;
    Gender gender;
    std::string raceEid;
};

extern std::vector<std::string> boneNames;