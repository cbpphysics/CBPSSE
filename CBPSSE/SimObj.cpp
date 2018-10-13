#include "SimObj.h"
#include "skse64/NiNodes.h"
#include "skse64/GameForms.h"
#include "skse64/GameRTTI.h"
#include "log.h"


// Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory

const char *leftBreastName = "NPC L Breast";
const char *rightBreastName = "NPC R Breast";
const char *leftButtName = "NPC L Butt";
const char *rightButtName = "NPC R Butt";
const char *bellyName = "HDT Belly";

const char *scrotumName = "NPC GenitalsScrotum [GenScrot]";
const char *leftScrotumName = "NPC L GenitalsScrotum [LGenScrot]";
const char *rightScrotumName = "NPC R GenitalsScrotum [RGenScrot]";

std::unordered_map<const char *, std::string> configMap = {
	{leftBreastName, "Breast"}, {rightBreastName, "Breast"},
	{leftButtName, "Butt"}, {rightButtName, "Butt"},
	{bellyName, "Belly"} };


std::vector<const char *> femaleBones = { leftBreastName, rightBreastName, leftButtName, rightButtName, bellyName };

SimObj::SimObj(Actor *actor, config_t &config)
	: things(5){
	id = actor->formID;
}

SimObj::~SimObj() {
}


bool SimObj::bind(Actor *actor, std::vector<const char *>& boneNames, config_t &config)
{
	//logger.error("bind\n");


	auto loadedState = actor->loadedState;
	if (loadedState && loadedState->node) {
		bound = true;

		things.clear();
		for (const char * &b : boneNames) {
			BSFixedString cs(b);
			auto bone = loadedState->node->GetObjectByName(&cs.data);
			if (!bone) {
				logger.info("Failed to find Bone %s for actor %d\n", b, actor->formID);
			} else {
				things.emplace(b, Thing(bone, cs));
			}
		}
		updateConfig(config);
		return  true;
	}
	return false;
}

bool SimObj::actorValid(Actor *actor) {
	if (actor->flags & TESForm::kFlagIsDeleted)
		return false;
	if (actor && actor->loadedState && actor->loadedState->node)
		return true;
	return false;
}


void SimObj::update(Actor *actor) {
	if (!bound)
		return;
	//logger.error("update\n");
	for (auto &t : things) {
		t.second.update(actor);
	}
	//logger.error("end SimObj update\n");
}

bool SimObj::updateConfig(config_t & config) {
	for (auto &t : things) {
		std::string &section = configMap[t.first];
		auto &centry = config[section];
		t.second.updateConfig(centry);
	}
	return true;
}

