#include "extension.h"
#include "CDetour/detours.h"

// ?
adlltf2 adlltf2;
SMEXT_LINK(&adlltf2);

// detour pointers
CDetour* ProcessListenEventsDetour      = nullptr;

// our gamedata pointer
IGameConfig* pGameConfig                = nullptr;

// forwards
IForward* adlltf2_CheaterDetected       = nullptr;
IForward* adlltf2_ProcessedListenEvents = nullptr;

// for our gameevents pointer
IGameEventManager2* gameeventmgr        = nullptr;

bool adlltf2::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
    GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_CURRENT(GetEngineFactory, gameeventmgr, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
    //GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
    return true;
}

// bool CBaseClient::ProcessListenEvents( CLC_ListenEvents* msg )
DETOUR_DECL_MEMBER1(ProcessListenEvents, bool, CLC_ListenEvents*, msg)
{
    // get a pclient pointer from our this pointer
    // i got this from trial and error because mUlTiPlE iNhErItAnCe
    IClient *pClient = (IClient *)((intptr_t)(this + 4));
    // something has gone wrong or we're looking at a bot, return orig func
    if (pClient == NULL || pClient->IsFakeClient())
    {
        return DETOUR_MEMBER_CALL(ProcessListenEvents)(msg);
    }
    // get actual client index here
    int client = pClient->GetPlayerSlot() + 1;

    // num of events client is listening to
    int events;

    // iterate thru the MAX num of events
    for (int i = 0; i < MAX_EVENT_NUMBER; i++)
    {
        // are they listening to this event? ++ the counter
        if (msg->m_EventArray.Get(i))
        {
            const char* name = (reinterpret_cast<INetMessage*>(msg))->ToString();
            //smutils->LogMessage(myself, "%s event", name);
            events++;
        }
    }
    // debug
    smutils->LogMessage(myself, "%i events for cl %i", events, client);

    // client has too many events! smack that mf'in forward
    if (events > 220)
    {
        adlltf2_CheaterDetected->PushCell(client);
        adlltf2_CheaterDetected->PushCell(events);
        adlltf2_CheaterDetected->Execute();
    }
    else if (events >= 0)
    {
        adlltf2_ProcessedListenEvents->PushCell(client);
        adlltf2_ProcessedListenEvents->PushCell(events);
        adlltf2_ProcessedListenEvents->Execute();
    }

    // return the original func, this may be unneccecary in the future
    return DETOUR_MEMBER_CALL(ProcessListenEvents)(msg);
}

bool adlltf2::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
    // gamedata nonexistant
    if (!gameconfs->LoadGameConfigFile("adlltf2", &pGameConfig, error, maxlen))
    {
        smutils->Format(error, maxlen - 1, "Failed to load gamedata");
        return false;
    }

    // start our detour mgr
    CDetourManager::Init(smutils->GetScriptingEngine(), pGameConfig);



    // bool CBaseClient::ProcessListenEvents(CLC_ListenEvents *msg)
    ProcessListenEventsDetour = DETOUR_CREATE_MEMBER(ProcessListenEvents, "CBaseClient::ProcessListenEvents");
    if (ProcessListenEventsDetour == nullptr)
    {
        smutils->Format(error, maxlen - 1, "failed to detour CBaseClient::ProcessListenEvents");
        return false;
    }
    ProcessListenEventsDetour->EnableDetour();

    /*  IForward *CreateForward(const char *name,
        ExecType et,
        unsigned int num_params,
        const ParamType *types,
        ...);
    */
    adlltf2_CheaterDetected       = forwards->CreateForward("adlltf2_CheaterDetected",       ET_Event, 2, NULL, Param_Cell, Param_Cell);
    adlltf2_ProcessedListenEvents = forwards->CreateForward("adlltf2_ProcessedListenEvents", ET_Event, 2, NULL, Param_Cell, Param_Cell);

    return true;
}

void adlltf2::SDK_OnUnload()
{
    gameconfs->CloseGameConfigFile(pGameConfig);
    forwards->ReleaseForward(adlltf2_CheaterDetected);
    forwards->ReleaseForward(adlltf2_ProcessedListenEvents);
    ProcessListenEventsDetour ->DisableDetour();
}
