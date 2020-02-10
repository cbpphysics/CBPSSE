#pragma once

#include <unordered_map>
#include <vector>
#include "f4se/GameReferences.h"
#include "Thing.h"
#include "config.h"

class SimObj {
    UInt32 id = 0;
    bool bound = false;
public:
    std::unordered_map<std::string, Thing> things;
    SimObj(Actor *actor, config_t &config);
    SimObj() {}
    ~SimObj();
    bool Bind(Actor *actor, std::vector<std::string> &boneNames, config_t &config);
    bool ActorValid(Actor *actor);
    void Update(Actor *actor);
    bool UpdateConfig(config_t &config);
    bool IsBound() { return bound; }

};

extern std::vector<std::string> boneNames;
//extern std::unordered_map<std::string, std::string> configMap;
//extern std::unordered_map<std::string, std::string> configMapArmor;