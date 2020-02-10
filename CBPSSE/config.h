#pragma once
#include <unordered_map>
#include "f4se/GameReferences.h"

class Configuration {
};

typedef std::unordered_map<std::string, float> configEntry_t;
typedef std::unordered_map<std::string, configEntry_t> config_t;
typedef std::unordered_map<std::string, configEntry_t> configOverrides_t;

extern bool playerOnly;
extern bool femaleOnly;
extern bool maleOnly;
extern bool detectArmor;

extern int configReloadCount;
extern config_t config;
extern config_t configArmor;

bool loadConfig();