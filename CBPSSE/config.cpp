#include "Thing.h"
#include "log.h"
#include "SimObj.h"
#include <unordered_map>
#include <string>
#include "config.h"

#pragma warning(disable : 4996)

int configReloadCount = 60;


config_t config;


const char* leftBreastName_FG = "Breast_CBP_L_02";
const char* rightBreastName_FG = "Breast_CBP_R_02";
const char* leftButtName_FG = "Butt_CBP_L_01";
const char* rightButtName_FG = "Butt_CBP_R_01";

const char* leftBreastName_CBBE = "LBreast_skin";
const char* rightBreastName_CBBE = "RBreast_skin";
const char* leftButtName_CBBE = "LButtFat_skin";
const char* rightButtName_CBBE = "RButtFat_skin";

void loadConfig() {
	char buffer[1024];
	//logger.info("loadConfig\n");
	FILE *fh = fopen("Data\\F4SE\\Plugins\\CBPConfig.txt", "r");
	if (!fh) {
		logger.error("Failed to open config file CBPConfig.txt\n");
		//Console_Print("Failed to open config file CBPConfig.txt");
		configReloadCount = 0;
		return;
	}

	//Console_Print("Reading CBP Config");
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
			}
		}
	} while (!feof(fh));
	fclose(fh);

	// temporary config setup until ini format
	if (config["Breast"]["FG"] == 1.0) {
		femaleBones.push_back(leftBreastName_FG);
		femaleBones.push_back(rightBreastName_FG);
		configMap.insert({ leftBreastName_FG, "Breast" });
		configMap.insert({ rightBreastName_FG, "Breast" });
	}
	else {
		// Default is CBBE
		femaleBones.push_back(leftBreastName_CBBE);
		femaleBones.push_back(rightBreastName_CBBE);
		configMap.insert({ leftBreastName_CBBE, "Breast" });
		configMap.insert({ rightBreastName_CBBE, "Breast" });
	}
	if (config["Butt"]["FG"] == 1.0) {
		femaleBones.push_back(leftButtName_FG);
		femaleBones.push_back(rightButtName_FG);
		configMap.insert({ leftButtName_FG, "Butt" });
		configMap.insert({ rightButtName_FG, "Butt" });
	}
	else {
		// Default is CBBE
		femaleBones.push_back(leftButtName_CBBE);
		femaleBones.push_back(rightButtName_CBBE);
		configMap.insert({ leftButtName_CBBE, "Butt" });
		configMap.insert({ rightButtName_CBBE, "Butt" });
	}
	configReloadCount = config["Tuning"]["rate"];
}
