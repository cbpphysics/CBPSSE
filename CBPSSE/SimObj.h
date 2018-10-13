#pragma once

#include <unordered_map>
#include <vector>
#include "skse64/GameReferences.h"
#include "Thing.h"
#include "config.h"

class SimObj {
	UInt32 id = 0;
	bool bound = false;
public:
	std::unordered_map<const char *, Thing> things;
	SimObj(Actor *actor, config_t &config);
	SimObj() {}
	~SimObj();
	bool bind(Actor *actor, std::vector<const char *> &boneNames, config_t &config);
	bool actorValid(Actor *actor);
	void update(Actor *actor);
	bool updateConfig(config_t &config);
	bool isBound() { return bound; }

};


extern std::vector<const char *> femaleBones;
extern const char *leftBreastName;
extern const char *rightBreastName;
extern const char *leftButtName;
extern const char *rightButtName;
extern const char *bellyName;