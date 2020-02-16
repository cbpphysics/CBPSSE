#include "config.h"
#include "INIReader.h"
#include "log.h"
#include "SimObj.h"
#include "Thing.h"

#include <iostream>
#include <iterator>
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
bool npcOnly = false;
bool detectArmor = false;
bool useWhitelist = false;

config_t config;
config_t configArmor;
configOverrides_t configOverrides;
configOverrides_t configArmorOverrides;

// TODO data structure these
whitelist_t whitelist;
std::vector<std::string> raceWhitelist;

bool LoadConfig() {
    logger.Info("loadConfig\n");

    std::set<std::string> bonesSet;

    bool reloadActors = false;
    auto playerOnlyOld = playerOnly;
    auto femaleOnlyOld = femaleOnly;
    auto maleOnlyOld = maleOnly;
    auto npcOnlyOld = npcOnly;
    auto useWhitelistOld = useWhitelist;

    boneNames.clear();
    config.clear();
    configArmor.clear();
    configOverrides.clear();
    configArmorOverrides.clear();

    // Note: Using INIReader results in a slight double read
    INIReader configReader("Data\\F4SE\\Plugins\\ocbp.ini");
    if (configReader.ParseError() < 0) {
        logger.Error("Can't load 'ocbp.ini'\n");
    }
    logger.Error("Reading CBP Config\n");

    // Read general settings
    playerOnly = configReader.GetBoolean("General", "playerOnly", false);
    npcOnly    = configReader.GetBoolean("General", "npcOnly", false);
    useWhitelist = configReader.GetBoolean("General", "useWhitelist", false);

    if (useWhitelist) {
        maleOnly = false;
        femaleOnly = false;
    }
    else {
        femaleOnly = configReader.GetBoolean("General", "femaleOnly", false);
        maleOnly = configReader.GetBoolean("General", "maleOnly", false);

    }

    reloadActors = (playerOnly ^ playerOnlyOld) ||
                    (femaleOnly ^ femaleOnlyOld) ||
                    (maleOnly ^ maleOnlyOld) ||
                    (npcOnly ^ npcOnlyOld) ||
                    (useWhitelist ^ useWhitelistOld);

    detectArmor = configReader.GetBoolean("General", "detectArmor", false);
    configReloadCount = configReader.GetInteger("Tuning", "rate", 0);

    // Read sections
    auto sections = configReader.Sections();
    for (auto sectionsIter = sections.begin(); sectionsIter != sections.end(); ++sectionsIter) {

        // Split for override section check
        auto overrideStr = std::string("Override:");
        auto splitStr = std::mismatch(overrideStr.begin(), overrideStr.end(), sectionsIter->begin());

        auto overrideAStr = std::string("Override.A:");
        auto splitAStr = std::mismatch(overrideAStr.begin(), overrideAStr.end(), sectionsIter->begin());

        if (*sectionsIter == std::string("Attach")) {
            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter);
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
        else if (*sectionsIter == std::string("Attach.A") && detectArmor) {
            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter);
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
        else if (*sectionsIter == std::string("Whitelist") && useWhitelist) {
            whitelist.clear();
            raceWhitelist.clear();

            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter);
            for (auto& valuesIter : sectionMap) {
                auto& boneName      = valuesIter.first;
                auto& whitelistName = valuesIter.second;

                size_t commaPos;
                do {
                    commaPos = whitelistName.find_first_of(",");
                    auto token = whitelistName.substr(0, commaPos);
                    size_t colonPos = token.find_last_of(":");
                    auto raceName = token.substr(0, colonPos);
                    auto genderStr = token.substr(colonPos + 1);

                    if (colonPos == -1) {
                        whitelist[boneName][token].male = true;
                        whitelist[boneName][token].female = true;
                        raceWhitelist.push_back(token);
                    }
                    else if (genderStr == "male") {
                        whitelist[boneName][raceName].male = true;
                        raceWhitelist.push_back(raceName);
                    }
                    else if (genderStr == "female") {
                        whitelist[boneName][raceName].female = true;
                        raceWhitelist.push_back(raceName);
                    }
                    whitelistName = whitelistName.substr(commaPos + 1);

                    //logger.Info("<token:> %s, <rest:> %s, <commaPos:> %d, <colonPos:> %d\n", token.c_str(), whitelistName.c_str(), commaPos >= 0, colonPos < 0);
                } while (commaPos != -1);
            }
        }
        else if (splitStr.first == overrideStr.end()) {
            // If section name is prefixed with "Override:", grab other half of name for bone
            auto boneName = std::string(splitStr.second, sectionsIter->end());

            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter); 
            for (auto &valuesIt : sectionMap) {
                configOverrides[boneName][valuesIt.first] = configReader.GetFloat(*sectionsIter, valuesIt.first, 0.0);
            }
        }
        else if (splitAStr.first == overrideAStr.end()) {
            // If section name is prefixed with "Override:", grab other half of name for bone
            auto boneName = std::string(splitAStr.second, sectionsIter->end());

            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter);
            for (auto& valuesIt : sectionMap) {
                configArmorOverrides[boneName][valuesIt.first] = configReader.GetFloat(*sectionsIter, valuesIt.first, 0.0);
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

    // replace armor configs with override settings (if any)
    for (auto& boneIter : configArmorOverrides) {
        if (configArmor.count(boneIter.first) > 0) {
            for (auto settingIter : boneIter.second) {
                configArmor[boneIter.first][settingIter.first] = settingIter.second;
            }
        }
    }

    // Remove duplicate entries
    bonesSet = std::set<std::string>(boneNames.begin(), boneNames.end());
    boneNames.assign(bonesSet.begin(), bonesSet.end());

    logger.Error("Finished CBP Config\n");
    return reloadActors;
}

void DumpConfigToLog()
{
    // Log contents of config
    logger.Info("***** Config Dump *****\n");
    for (auto section : config) {
        logger.Info("[%s]\n", section.first.c_str());
        for (auto setting : section.second) {
            logger.Info("%s=%f\n", setting.first.c_str(), setting.second);
        }
    }

    logger.Info("***** ConfigArmor Dump *****\n");
    for (auto section : configArmor) {
        logger.Info("[%s]\n", section.first.c_str());
        for (auto setting : section.second) {
            logger.Info("%s=%f\n", setting.first.c_str(), setting.second);
        }
    }
}

void DumpWhitelistToLog() {
    logger.Info("***** Whitelist Dump *****\n");
    for (auto section : whitelist) {
        logger.Info("[%s]\n", section.first.c_str());
        for (auto setting : section.second) {
            logger.Info("%s= female: %d, male: %d\n", setting.first.c_str(), setting.second.female, setting.second.male);
        }
    }
}