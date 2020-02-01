#pragma once
#include <unordered_map>
#include "f4se/GameReferences.h"

class Configuration {
};

typedef std::unordered_map<std::string, float> configEntry_t;
typedef std::unordered_map<std::string, configEntry_t> config_t;
typedef std::unordered_map<std::string, configEntry_t> configOverrides_t;

bool IsActorMale(Actor* actor);

extern bool playerOnly;
extern bool femaleOnly;
extern bool maleOnly;
extern int configReloadCount;
extern config_t config;

void loadConfig();