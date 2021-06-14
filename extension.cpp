#include "extension.h"
#include "CDetour/detours.h"

AntiDLL antiDLL;
SMEXT_LINK(&antiDLL);

CDetour* pDetour                = nullptr;
IGameConfig* pGameConfig        = nullptr;
IForward* forwardCheatDetected  = nullptr;
IGameEventManager2* gameevents  = nullptr;
//std::vector<std::string> events;

// bool CBaseClient::ProcessListenEvents( CLC_ListenEvents *msg )
DETOUR_DECL_MEMBER1(ProcessListenEvents, bool, CLC_ListenEvents*, msg)
{
    IClient *pClient = (IClient *)((intptr_t)(this + 4));
    smutils->LogError(myself, "pclient initd");
    //
    if (pClient == NULL)
    {
        return DETOUR_MEMBER_CALL(ProcessListenEvents)(msg);
    }
    //
    int client = pClient->GetPlayerSlot() + 1;

    if (pClient->IsFakeClient()) return DETOUR_MEMBER_CALL(ProcessListenEvents)(msg);

    smutils->LogError(myself, "client %i", client);

    int counter = 0;

    for (int i = 0; i < MAX_EVENT_NUMBER; i++)
    {
        if (msg->m_EventArray.Get(i))
        {
            counter++;
        }
    }
    smutils->LogMessage(myself, "%i events for cl %i", counter, client);

    //if (detected)
    //{
    //    forwardCheatDetected->PushCell(client);
    //    forwardCheatDetected->Execute();
    //}

    return DETOUR_MEMBER_CALL(ProcessListenEvents)(msg);
}

bool AntiDLL::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
    if (!gameconfs->LoadGameConfigFile("listen", &pGameConfig, error, maxlen))
    {
        smutils->Format(error, maxlen - 1, "Failed to load gamedata");
        return false;
    }

    CDetourManager::Init(smutils->GetScriptingEngine(), pGameConfig);
    pDetour = DETOUR_CREATE_MEMBER(ProcessListenEvents, "CBaseClient::ProcessListenEvents");

    if (pDetour == nullptr)
    {
        smutils->Format(error, maxlen - 1, "Failed to create interceptor");
        return false;
    }

    pDetour->EnableDetour();

    return true;
}

void AntiDLL::SDK_OnUnload()
{
    gameconfs->CloseGameConfigFile(pGameConfig);
    forwards->ReleaseForward(forwardCheatDetected);
    pDetour->DisableDetour();
}

bool AntiDLL::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
    GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
    //GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);

    return true;
}