#include "config.h"
#include "INIReader.h"
#include "log.h"
#include "SimObj.h"
#include "Thing.h"

#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"
#include "f4se_common/Utilities.h"
#include "f4se/GameData.h"

#define DEBUG 0
#pragma warning(disable : 4996)

int configReloadCount = 60;
bool playerOnly = false;
bool femaleOnly = false;
bool maleOnly = false;
bool npcOnly = false;
bool useWhitelist = false;

config_t config;
std::map<UInt32, armorOverrideData> configArmorOverrideMap;
std::unordered_set<UInt32> usedSlots;
std::map<std::multiset<UInt64>, config_t> cachedConfigs;

// TODO data structure these
whitelist_t whitelist;
std::vector<std::string> raceWhitelist;

UInt32 GetFormIDFromString(std::string const& configString)
{
    size_t colonPos = configString.find_first_of(":");
    auto pluginFormID = configString.substr(0, colonPos);
    std::string pluginName = "";
    if (colonPos != std::string::npos && colonPos < configString.length() - 1) {
        pluginName = configString.substr(colonPos + 1);
    }

    for (auto digit : pluginFormID) {
        if (!std::isxdigit(digit)) {
            logger.Error("Invalid FormID %s, invalid hex character %c\n", pluginFormID.c_str(), digit);
            return -1;
        }
    }

    UInt32 formID = std::stoul(pluginFormID, nullptr, 16);

    if (!pluginName.empty()) {
        auto dataHandler = *g_dataHandler;
        auto modInfo = dataHandler->LookupLoadedModByName(pluginName.c_str());

        if (!modInfo) {
            logger.Error("Plugin with name %s does not exist\n", pluginName.c_str());
            return -1;
        }

        bool isLightPlugin = modInfo->recordFlags & modInfo->kRecordFlags_ESL;
        size_t maxPartialIdLength = isLightPlugin ? 3 : 6;

        if (pluginFormID.length() > maxPartialIdLength) {
            logger.Error("Invalid FormID %s, too many characters when%s plugin name specified\n", pluginFormID.c_str(), isLightPlugin ? " light" : "");
            return -1;
        }

        UInt32 loadIndex = isLightPlugin ? dataHandler->GetLoadedLightModIndex(pluginName.c_str()) : dataHandler->GetLoadedModIndex(pluginName.c_str());
        formID |= loadIndex << 24;
    }
    else {
        if (pluginFormID.length() > 8) {
            logger.Error("Invalid FormID %s, too many characters\n", pluginFormID.c_str());
            return -1;
        }
    }

    return formID;
}

bool LoadConfig() {
    logger.Info("loadConfig\n");

    config_t configOverrides;
    std::map<UInt32, config_t> configArmorBoneOverrides;
    std::set<std::string> bonesSet;
    std::unordered_map<std::string, UInt32> priorityNameMappings;

    bool reloadActors = false;
    auto playerOnlyOld = playerOnly;
    auto femaleOnlyOld = femaleOnly;
    auto maleOnlyOld = maleOnly;
    auto npcOnlyOld = npcOnly;
    auto useWhitelistOld = useWhitelist;

    boneNames.clear();
    config.clear();
    configArmorOverrideMap.clear();
    usedSlots.clear();
    cachedConfigs.clear();

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

    configReloadCount = configReader.GetInteger("Tuning", "rate", 0);

    // Read sections
    auto sections = configReader.Sections();
    auto prioritySection = sections.find("Priority");
    bool detectArmorCompat = configReader.GetBoolean("General", "detectArmor", false) && (prioritySection == sections.end() || configReader.Section(*prioritySection).empty());

    // Backwards compatibility for detectArmor style method
    if (detectArmorCompat) {
        priorityNameMappings["A"] = 0;
        configArmorOverrideMap[0].slots.emplace(11);
        usedSlots.emplace(11);
        configArmorOverrideMap[0].isFilterInverted = false;
    }

    for (auto sectionsIter = sections.begin(); sectionsIter != sections.end(); ++sectionsIter) {

        // Split for override section check
        auto overrideStr = std::string("Override:");
        auto splitOverrideStr = std::mismatch(overrideStr.begin(), overrideStr.end(), sectionsIter->begin());

        auto subOverrideStr = std::string("Override.");
        auto splitSubOverrideStr = std::mismatch(subOverrideStr.begin(), subOverrideStr.end(), sectionsIter->begin());

        auto subAttachStr = std::string("Attach.");
        auto splitSubAttachStr = std::mismatch(subAttachStr.begin(), subAttachStr.end(), sectionsIter->begin());

        auto armorStr = std::string("Armor.");
        auto splitArmorStr = std::mismatch(armorStr.begin(), armorStr.end(), sectionsIter->begin());

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
        else if (splitSubAttachStr.first == subAttachStr.end()) {
            auto attachSubname = std::string(splitSubAttachStr.second, sectionsIter->end());

            UInt32 attachPriority;
            auto mapEntry = priorityNameMappings.find(attachSubname);
            if (mapEntry != priorityNameMappings.end()) {
                attachPriority = mapEntry->second;
            }
            else {
                std::string priorityMapping = configReader.Get("Priority", attachSubname, "");
                try {
                    attachPriority = std::stoul(priorityMapping);
                }
                catch (const std::exception&) {
                    continue;
                }

                priorityNameMappings[attachSubname] = attachPriority;
            }

            auto sectionMap = configReader.Section(*sectionsIter);
            for (auto& valuesIter : sectionMap) {
                auto& boneName = valuesIter.first;
                auto& attachName = valuesIter.second;
                boneNames.push_back(boneName);
                // Find specified bone section and insert map values into config_t in configArmorOverrideMap at attachPriority
                if (attachName.empty()) {
                    // "Touch" the map to add empty entry for bone in config_t to signal deletion later when building config from overrides
                    configArmorOverrideMap[attachPriority].config[boneName];
                } 
                else if (sections.find(attachName) != sections.end()) {
                    auto attachMapSection = configReader.Section(attachName);
                    for (auto& attachIter : attachMapSection) {
                        auto& keyName = attachIter.first;
                        configArmorOverrideMap[attachPriority].config[boneName][keyName] = configReader.GetFloat(attachName, keyName, 0.0);
                    }
                }
            }
        }
        else if (splitArmorStr.first == armorStr.end()) {
            auto armorSubname = std::string(splitArmorStr.second, sectionsIter->end());

            UInt32 armorPriority;
            auto mapEntry = priorityNameMappings.find(armorSubname);
            if (mapEntry != priorityNameMappings.end()) {
                armorPriority = mapEntry->second;
            }
            else {
                std::string priorityMapping = configReader.Get("Priority", armorSubname, "");
                try {
                    armorPriority = std::stoul(priorityMapping);
                }
                catch (const std::exception&) {
                    continue;
                }

                priorityNameMappings[armorSubname] = armorPriority;
            }

            std::string slots = configReader.Get(*sectionsIter, "slots", "");
            if (slots.empty()) {
                continue;
            }

            size_t commaPos;
            do {
                commaPos = slots.find_first_of(",");
                auto token = slots.substr(0, commaPos);
                slots = slots.substr(commaPos + 1);

                UInt32 slot;
                try {
                    slot = std::stoul(token);
                }
                catch (const std::exception&) {
                    continue;
                }

                configArmorOverrideMap[armorPriority].slots.emplace(slot - 30);
                usedSlots.emplace(slot - 30);
            } while (commaPos != std::string::npos);

            configArmorOverrideMap[armorPriority].isFilterInverted = configReader.GetBoolean(*sectionsIter, "invertFilter", false);

            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter);
            for (auto& valuesIter : sectionMap) {
                auto& key = valuesIter.first;
                if (key == "invertFilter" || key == "slots") {
                    continue;
                }

                auto formID = GetFormIDFromString(valuesIter.second);
                if (formID == -1) {
                    continue;
                }

                configArmorOverrideMap[armorPriority].armors.emplace(formID);
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
        else if (splitOverrideStr.first == overrideStr.end()) {
            // If section name is prefixed with "Override:", grab other half of name for bone
            auto boneName = std::string(splitOverrideStr.second, sectionsIter->end());

            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter); 
            for (auto &valuesIt : sectionMap) {
                configOverrides[boneName][valuesIt.first] = configReader.GetFloat(*sectionsIter, valuesIt.first, 0.0);
            }
        }
        else if (splitSubOverrideStr.first == subOverrideStr.end()) {
            // If section name is prefixed with "Override.", grab other half for priority and name of bone
            auto overrideSection = std::string(splitSubOverrideStr.second, sectionsIter->end());

            size_t colonPos = overrideSection.find_first_of(":");
            auto overrideSubname = overrideSection.substr(0, colonPos);
            auto boneName = overrideSection.substr(colonPos + 1);

            UInt32 overridePriority;
            auto mapEntry = priorityNameMappings.find(overrideSubname);
            if (mapEntry != priorityNameMappings.end()) {
                overridePriority = mapEntry->second;
            }
            else {
                std::string priorityMapping = configReader.Get("Priority", overrideSubname, "");
                try {
                    overridePriority = std::stoul(priorityMapping);
                }
                catch (const std::exception&) {
                    continue;
                }

                priorityNameMappings[overrideSubname] = overridePriority;
            }

            // Get section contents
            auto sectionMap = configReader.Section(*sectionsIter);
            for (auto& valuesIt : sectionMap) {
                configArmorBoneOverrides[overridePriority][boneName][valuesIt.first] = configReader.GetFloat(*sectionsIter, valuesIt.first, 0.0);
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
    for (auto conf : configArmorBoneOverrides) {
        for (auto& boneIter : conf.second) {
            if (configArmorOverrideMap[conf.first].config.count(boneIter.first) > 0) {
                for (auto settingIter : boneIter.second) {
                    configArmorOverrideMap[conf.first].config[boneIter.first][settingIter.first] = settingIter.second;
                }
            }
        }
    }

    // Remove duplicate entries
    bonesSet = std::set<std::string>(boneNames.begin(), boneNames.end());
    boneNames.assign(bonesSet.begin(), bonesSet.end());

    // "Delete" bones specified in [Attach] but not [Attach.A] from the latter for compatibility with presets that remove breast bone jiggle when chest armor equipped
    if (detectArmorCompat) {
        for (auto boneName : boneNames) {
            if (configArmorOverrideMap[0].config.find(boneName) == configArmorOverrideMap[0].config.end()) {
                configArmorOverrideMap[0].config[boneName];
            }
        }
    }

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

    logger.Info("***** ConfigArmorOverride Dump *****\n");
        for (auto conf : configArmorOverrideMap)
        {
            logger.Info("** Slot-Armor Map priority %ul **\n", conf.first);
            logger.Info("[Slots]\n");
            for (auto slot : conf.second.slots) {
                logger.Info("%ul\n", slot);
            }
            logger.Info("[Armors]\n");
            for (auto formID : conf.second.armors) {
                logger.Info("%ul\n", formID);
            }
            logger.Info("** Config priority %ul **\n", conf.first);
            for (auto section : conf.second.config) {
                logger.Info("[%s]\n", section.first.c_str());
                for (auto setting : section.second) {
                    logger.Info("%s=%f\n", setting.first.c_str(), setting.second);
                }
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