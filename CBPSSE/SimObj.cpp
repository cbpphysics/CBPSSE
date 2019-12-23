#include "SimObj.h"
#include "f4se/NiNodes.h"
#include "f4se/GameForms.h"
#include "f4se/GameRTTI.h"
#include "log.h"


// Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory

const char *leftBreastName = "Breast_CBP_R_02";
const char *rightBreastName = "Breast_CBP_L_02";
const char *leftButtName = "Butt_CBP_R_01";
const char *rightButtName = "Butt_CBP_L_01";
const char *bellyName = "HDT Belly";

const char *scrotumName = "NPC GenitalsScrotum [GenScrot]";
const char *leftScrotumName = "Penis_Balls_CBP_01";
const char *rightScrotumName = "Penis_Balls_CBP_02";

std::unordered_map<const char *, std::string> configMap = {
	{leftBreastName, "Breast"}, {rightBreastName, "Breast"},
	{leftButtName, "Butt"}, {rightButtName, "Butt"},
	/*{bellyName, "Belly"}*/ };


std::vector<const char *> femaleBones = { leftBreastName, rightBreastName, leftButtName, rightButtName, /*bellyName*/ };

SimObj::SimObj(Actor *actor, config_t &config)
	: things(5){
	id = actor->formID;
}

SimObj::~SimObj() {
}


bool SimObj::bind(Actor *actor, std::vector<const char *>& boneNames, config_t &config)
{
	logger.error("bind\n");


	auto loadedData = actor->unkF0;
	if (loadedData && loadedData->rootNode) {
		bound = true;

		things.clear();
		for (const char * &b : boneNames) {
			BSFixedString cs(b);
			auto bone = loadedData->rootNode->GetObjectByName(&cs);
			if (!bone) {
				logger.info("Failed to find Bone %s for actor %08x\n", b, actor->formID);
			} else {
				logger.info("Doing Bone %s for actor %08x\n", b, actor->formID);
				things.emplace(b, Thing(bone, cs));
			}
		}
		updateConfig(config);
		return  true;
	}
	return false;
}

bool SimObj::actorValid(Actor *actor) {
	if (actor->flags & TESForm::kFlag_IsDeleted)
		return false;
	if (actor && actor->unkF0 && actor->unkF0->rootNode)
		return true;
	return false;
}


void SimObj::update(Actor *actor) {
	if (!bound)
		return;
	logger.error("update\n");
	for (auto &t : things) {
		t.second.update(actor);
	}
	logger.error("end SimObj update\n");
}

bool SimObj::updateConfig(config_t & config) {
	for (auto &t : things) {
		std::string &section = configMap[t.first];
		auto &centry = config[section];
		t.second.updateConfig(centry);
	}
	return true;
}

