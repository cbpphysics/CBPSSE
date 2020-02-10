#include "PapyrusOCBP.h"

//#include "f4se/PapyrusVM.h"
#include "f4se/PapyrusNativeFunctions.h"

#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"

#include <functional>
#include <algorithm>

#include "f4se/NiNodes.h"
#include "f4se/NiExtraData.h"
#include "f4se/BSGeometry.h"

#include "f4se/GameObjects.h"

#include "SimObj.h"

std::unordered_map<UInt32, std::unordered_map<std::string, bool>> boneIgnores;

namespace papyrusOCBP
{
	void ToggleBone(StaticFunctionTag*, Actor* actor, bool toggle, BSFixedString boneName)
	{
		boneIgnores[actor->formID][std::string(boneName.c_str())] = toggle;
	}

	void ClearBoneToggles(StaticFunctionTag*)
	{
		boneIgnores.clear();
	}

}

void papyrusOCBP::RegisterFuncs(VirtualMachine* vm)
{
	vm->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void, Actor*, bool, BSFixedString>("ToggleBone", "OCBP_API", papyrusOCBP::ToggleBone, vm));

	vm->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, void>("ClearBoneToggles", "OCBP_API", papyrusOCBP::ClearBoneToggles, vm));
}