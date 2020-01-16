#pragma once

#include <unordered_map>
#include <vector>
#include "f4se/GameReferences.h"
#include "Thing.h"
#include "config.h"

class SimObj {
	UInt32 id = 0;
	bool bound = false;
public:
	std::unordered_map<std::string, Thing> things;
	SimObj(Actor *actor, config_t &config);
	SimObj() {}
	~SimObj();
	bool bind(Actor *actor, std::vector<std::string> &boneNames, config_t &config);
	bool actorValid(Actor *actor);
	void update(Actor *actor);
	bool updateConfig(config_t &config);
	bool isBound() { return bound; }

};


extern std::vector<std::string> femaleBones;
//extern std::unordered_map<const char*, std::string> configMap;