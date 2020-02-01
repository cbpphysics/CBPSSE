#include "config.h"
#include "INIReader.h"
#include "log.h"
#include "SimObj.h"
#include "Thing.h"

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>

#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"
#include "f4se_common/Utilities.h"

#define DEBUG 0
#pragma warning(disable : 4996)

int configReloadCount = 60;
bool playerOnly = false;
bool femaleOnly = false;
bool maleOnly = false;
bool detectArmor = false;

config_t config;
config_t configArmor;
configOverrides_t configOverrides;

void loadConfig() {
    logger.info("loadConfig\n");
    boneNames.clear();
    std::set<std::string> bonesSet;

    config.clear();

    // Note: Using INIReader results in a slight double read
    INIReader configReader("Data\\F4SE\\Plugins\\ocbp.ini");
    if (configReader.ParseError() < 0) {
        logger.error("Can't load 'ocbp.ini'\n");
    }
    logger.error("Reading CBP Config\n");

    // Read general settings
    playerOnly = configReader.GetBoolean("General", "playerOnly", false);
    femaleOnly = configReader.GetBoolean("General", "femaleOnly", false);
    maleOnly = configReader.GetBoolean("General", "maleOnly", false);
    detectArmor = configReader.GetBoolean("General", "detectArmor", false);
    configReloadCount = configReader.GetInteger("Tuning", "rate", 0);

    // Read sections
    auto sections = configReader.Sections();
    for (auto sectionIt = sections.begin(); sectionIt != sections.end(); ++sectionIt) {

        // Split for override section check
        auto overrideStr = std::string("Override:");
        auto splitStr = std::mismatch(overrideStr.begin(), overrideStr.end(), sectionIt->begin());

        if (*sectionIt == std::string("Attach")) {
            // Get section contents
            auto sectionMap = configReader.Section(*sectionIt);
            for (auto& valuesIter : sectionMap) {
                auto& boneName = valuesIter.first;
                auto& attachName = valuesIter.second;
                boneNames.push_back(boneName);
                // Find specified bone section and insert map values into config
                if (sections.find(attachName) != sections.end()) {
                    auto attachMapSection = configReader.Section(attachName);
                    for (auto& attachIter : attachMapSection) {
                        auto& keyName = attachIter.first;
                        config[boneName][keyName] = configReader.GetFloat(attachName, keyName, 0.0);
                    }
                }
            }
        }
        else if (*sectionIt == std::string("Attach.A") && detectArmor) {
            // Get section contents
            auto sectionMap = configReader.Section(*sectionIt);
            for (auto &valuesIter : sectionMap) {
                auto &boneName = valuesIter.first;
                auto &attachName = valuesIter.second;
                boneNames.push_back(boneName);
                // Find specified bone section and insert map values into configArmor
                if (sections.find(attachName) != sections.end()) {
                    auto attachMapSection = configReader.Section(attachName);
                    for (auto &attachIter : attachMapSection) {
                        auto& keyName = attachIter.first;
                        configArmor[boneName][keyName] = configReader.GetFloat(attachName, keyName, 0.0);
                    }
                }
            }
        }
        else if (splitStr.first == overrideStr.end()) {
            // If section name is prefixed with "Override:", grab other half of name for bone
            auto boneName = std::string(splitStr.second, sectionIt->end());

            // Get section contents
            auto sectionMap = configReader.Section(*sectionIt); 
            for (auto &valuesIt : sectionMap) {
                configOverrides[boneName][valuesIt.first] = configReader.GetFloat(*sectionIt, valuesIt.first, 0.0);
            }
        }
    }

    // replace configs with override settings (if any)
    for (auto &boneIter : configOverrides) {
        if (config.count(boneIter.first) > 0) {
            for (auto settingIter : boneIter.second) {
                config[boneIter.first][settingIter.first] = settingIter.second;
            }
        }
    }

    // Remove duplicate entries
    bonesSet = std::set<std::string>(boneNames.begin(), boneNames.end());
    boneNames.assign(bonesSet.begin(), bonesSet.end());

    logger.error("Finished CBP Config\n");
}

void dumpConfigtoLog()
{
    // Log contents of config
    for (auto section : config) {
        logger.info("[%s]\n", section.first.c_str());
        for (auto setting : section.second) {
            logger.info("%s=%f\n", setting.first.c_str(), setting.second);
        }
    }
}