#include "Thing.h"
#include "log.h"
#include "SimObj.h"
#include <unordered_map>
#include <string>
#include "config.h"

#pragma warning(disable : 4996)

int configReloadCount = 60;


config_t config;



void loadConfig() {
	char buffer[1024];
	logger.info("loadConfig\n");
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

	configReloadCount = config["Tuning"]["rate"];
}
