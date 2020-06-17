#pragma once

#include "f4se/GameTypes.h"
#include "f4se/PapyrusVM.h"
#include <unordered_map>
#include <string>

class VirtualMachine;
struct StaticFunctionTag;

extern std::unordered_map<UInt32, std::unordered_map<std::string, bool>> boneIgnores; // probably should be moved somewhere else

namespace papyrusOCBP
{
    void RegisterFuncs(VirtualMachine* vm);
};