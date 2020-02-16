#pragma once
#include <unordered_map>
#include <vector>

#include "f4se/GameReferences.h"

class Configuration {
};

struct whitelistSex {
    bool male;
    bool female;
};

typedef std::unordered_map<std::string, float> configEntry_t;
typedef std::unordered_map<std::string, configEntry_t> config_t;
typedef std::unordered_map<std::string, configEntry_t> configOverrides_t;
typedef std::unordered_map<std::string, std::unordered_map<std::string, whitelistSex>> whitelist_t;

extern bool playerOnly;
extern bool femaleOnly;
extern bool maleOnly;
extern bool npcOnly;
extern bool detectArmor;
extern bool useWhitelist;

extern int configReloadCount;
extern config_t config;
extern config_t configArmor;
extern whitelist_t whitelist;
extern std::vector<std::string> raceWhitelist;
bool LoadConfig();
void DumpWhitelistToLog();