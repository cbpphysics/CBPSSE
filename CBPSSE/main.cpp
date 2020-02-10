#include "common/ITypes.h"
#include <string>
#include "f4se/PluginAPI.h"
#include "f4se_common/f4se_version.h"
#include "f4se_common/SafeWrite.h"
#include "f4se/GameAPI.h"
#include "f4se/GameEvents.h"
#include "log.h"
#include "config.h"
#include "PapyrusOCBP.h"


bool RegisterFuncs(VirtualMachine* vm);

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
//F4SEMessagingInterface	* g_messagingInterface = NULL;

//F4SEScaleformInterface		* g_scaleform = NULL;
//F4SESerializationInterface	* g_serialization = NULL;
F4SETaskInterface				* g_task = nullptr;
F4SEPapyrusInterface            * g_papyrus = nullptr;
//IDebugLog	gLog("Data\\F4SE\\Plugins\\hook.log");


void DoHook();



void MessageHandler(F4SEMessagingInterface::Message * msg)
{
    switch (msg->type)
    {
        case F4SEMessagingInterface::kMessage_GameDataReady:
        {
            logger.info("kMessage_GameDataReady\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_GameLoaded:
        {
            logger.info("kMessage_GameLoaded\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_NewGame:
        {
            logger.info("kMessage_NewGame\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_PreLoadGame:
        {
            logger.info("kMessage_PreLoadGame\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_PostLoad:
        {
            logger.info("kMessage_PostLoad\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_PostPostLoad:
        {
            logger.info("kMessage_PostPostLoad\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_PostLoadGame:
        {
            logger.info("kMessage_PostLoadGame\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_PreSaveGame:
        {
            logger.info("kMessage_PreSaveGame\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_PostSaveGame:
        {
            logger.info("kMessage_PostSaveGame\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_DeleteGame:
        {
            logger.info("kMessage_DeleteGame\n");
        }
        break;
        case F4SEMessagingInterface::kMessage_InputLoaded:
        {
            logger.info("kMessage_InputLoaded\n");
        }
        break;

    }
}


extern "C"
{

    bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
    {
        logger.info("CBP Physics F4SE Plugin\n");
        logger.error("Query called\n");


        // populate info structure
        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = "CBP plugin";
        info->version = 24;

        // store plugin handle so we can identify ourselves later
        g_pluginHandle = f4se->GetPluginHandle();

        if (f4se->isEditor)
        {
            logger.error("loaded in editor, marking as incompatible\n");
            return false;
        }
        else if (f4se->runtimeVersion != RUNTIME_VERSION)
        {
            logger.error("unsupported runtime version %08X", f4se->runtimeVersion);
            return false;
        }
        // supported runtime version

        g_papyrus = (F4SEPapyrusInterface*)f4se->QueryInterface(kInterface_Papyrus);
        if (!g_papyrus)
        {
            _WARNING("couldn't get papyrus interface");
        }

        logger.error("Query complete\n");
        return true;
    }

    bool F4SEPlugin_Load(const F4SEInterface * f4se)
    {
        logger.error("CBP Loading\n");

        g_task = (F4SETaskInterface *)f4se->QueryInterface(kInterface_Task);
        if (!g_task)
        {
            logger.error("Couldn't get Task interface\n");
            return false;
        }

        if (g_papyrus)
            g_papyrus->Register(RegisterFuncs);

        // Load initial config before the hook.
        logger.error("Loading Config\n");
        loadConfig();
        //g_messagingInterface->RegisterListener(0, "F4SE", MessageHandler); 
        logger.error("Hooking Game\n");
        DoHook();
        logger.error("CBP Load Complete\n");
        return true;
    }
};

bool RegisterFuncs(VirtualMachine* vm)
{
    papyrusOCBP::RegisterFuncs(vm);
    return true;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
) {
    return true;
}