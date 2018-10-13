#pragma once
#include <unordered_map>

typedef std::unordered_map<std::string, float> configEntry_t;
typedef std::unordered_map<std::string, configEntry_t> config_t;
extern int configReloadCount;
extern config_t config;

void loadConfig();