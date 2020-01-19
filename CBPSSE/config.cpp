#include "Thing.h"
#include "log.h"
#include "SimObj.h"
#include <unordered_map>
#include <string>
#include "config.h"
#include <set>

#include "f4se/GameObjects.h"
#include "f4se/GameRTTI.h"
#include "f4se_common/Utilities.h"

#pragma warning(disable : 4996)

int configReloadCount = 60;
bool playerOnly = false;
bool femaleOnly = false;
bool maleOnly = false;

config_t config;


//const char* leftBreastName_FG = "Breast_CBP_L_02";
//const char* rightBreastName_FG = "Breast_CBP_R_02";
//const char* leftButtName_FG = "Butt_CBP_L_01";
//const char* rightButtName_FG = "Butt_CBP_R_01";
//
//const char* leftBreastName_CBBE = "LBreast_skin";
//const char* rightBreastName_CBBE = "RBreast_skin";
//const char* leftButtName_CBBE = "LButtFat_skin";
//const char* rightButtName_CBBE = "RButtFat_skin";

void loadConfig() {
    char buffer[1024];
    std::set<std::string> bonesSet;
    //logger.info("loadConfig\n");
    FILE *fh = fopen("Data\\F4SE\\Plugins\\CBPConfig.txt", "r");
    if (!fh) {
        logger.error("Failed to open config file CBPConfig.txt\n");
        //Console_Print("Failed to open config file CBPConfig.txt");
        configReloadCount = 0;
        return;
    }

    //Console_Print("Reading CBP Config");
    // Rewrite this eventually
    config.clear();
    do {
        auto str = fgets(buffer, 1023, fh);
        //logger.error("str %s\n", str);
        if (str && strlen(str) > 1) {
            if (str[0] != '#') {
                char *tok0 = strtok(str, ".");
                char *tok1 = strtok(NULL, " ");
                char *tok2 = strtok(NULL, " ");

                if (tok0 && tok1 && tok2) {
                    config[std::string(tok0)][std::string(tok1)] = atof(tok2);
                }
                if (std::string(tok0) != "Tuning" && std::string(tok0) != "General") {
                    boneNames.push_back(std::string(tok0));
                }
            }
        }
    } while (!feof(fh));
    fclose(fh);
    bonesSet = std::set<std::string>(boneNames.begin(), boneNames.end());
    boneNames.assign(bonesSet.begin(), bonesSet.end());

    playerOnly = config["General"]["playerOnly"] == 1;
    femaleOnly = config["General"]["femaleOnly"] == 1;
    maleOnly = config["General"]["maleOnly"] == 1;
    configReloadCount = config["Tuning"]["rate"];
    
}

bool IsActorMale(Actor* actor)
{
    TESNPC* actorNPC = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

    auto npcSex = actorNPC ? CALL_MEMBER_FN(actorNPC, GetSex)() : 1;

    if (npcSex == 0) //Actor is male
        return true;
    else
        return false;
}