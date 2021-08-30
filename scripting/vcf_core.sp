#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <cstrike>
#include <dhooks>
#include <geoip>
#include <vcf>
#include <zombieplague>
// #include <json>
// #include <system2>

// I dont know whether is ok...perhaps it requires more experiments.
#undef AUTOLOAD_EXTENSIONS
#include <laper32>

#pragma semicolon 1
#pragma newdecls required

#define LoopAllPlayers(%1) for(int %1=1;%1<=MaxClients;++%1)\
if(IsPlayerExist(%1, false))

// Store natives
native int Store_GetClientCredits(int client);
native void Store_SetClientCredits(int client, int credits);
native void Store_GiveItem(int client, int itemid, int purchase=0, int expiration=0, int price=0);
native int Store_GetItemidByUniqueid(const char[] itemname);

#include "vcfcore/global.cpp"
#include "vcfcore/version.cpp"

#include "vcfcore/core/api.cpp"
#include "vcfcore/core/database.cpp"

#include "vcfcore/manager/hook.cpp"
#include "vcfcore/manager/vtools.cpp"
#include "vcfcore/manager/users.cpp"
#include "vcfcore/manager/admin.cpp"
#include "vcfcore/manager/reservedslot.cpp"
#include "vcfcore/manager/invite.cpp"

public Plugin myinfo =
{
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_LINK
}

/**
 * @brief API load function
 */
public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
    return APIOnInit();
}

public void OnLibraryAdded(const char[] sLibrary)
{
    // Validate library
    if (!strcmp(sLibrary, "zombieplague", false))
    {
        gServerData.mod_ZombiePlague = true;
    }
    else if (StrContains(sLibrary, "store", false) != -1)
    {
        gServerData.core_Store = (GetFeatureStatus(FeatureType_Native, "Store_GetClientCredits") == FeatureStatus_Available);
    }
}

public void OnLibraryRemoved(const char[] sLibrary)
{
    // Validate library
    if (!strcmp(sLibrary, "zombieplague", false))
    {
        gServerData.mod_ZombiePlague = false;
    }
    else if (StrContains(sLibrary, "store", false) != -1)
    {
        gServerData.core_Store = (GetFeatureStatus(FeatureType_Native, "Store_GetClientCredits") == FeatureStatus_Available);
    }
}

/**
 * @brief This framework is initializing.
 */
public void OnPluginStart()
{
    // Load translation
    LoadTranslations("vcf.phrases");

    LoadTranslations("common.phrases");
    LoadTranslations("basebans.phrases");
    LoadTranslations("core.phrases");

    HookOnPluginStart();
    VToolsOnPluginStart();
    DatabaseOnPluginStart();
    UsersOnPluginStart();
    AdminOnPluginStart();
    ReservedSlotOnPluginStart();
    InviteOnPluginStart();
    VersionOnPluginStart();

    AutoExecConfig(true, "vcf");
    
    RegConsoleCmd("sm_vcfdebug", vcfdebugcmd);
    
}

public Action vcfdebugcmd(int client, int args)
{
    PrintToChatAll("%s", gServerData.PublicIP);
    PrintToChatAll("Steam64: %s | PID: %d", gClientData[client].Steam64, gClientData[client].PID);
    PrintToChatAll("Access: %d | Flags: %d | Nick: %s", gClientData[client].Access, gClientData[client].Flags, gClientData[client].Nick);
    PrintToChatAll("Purchase: %.2f | VIPLevel: %d", gClientData[client].TotalPurchase, gClientData[client].VIPLevel);
    PrintToChatAll("WarningPoint: %d | ClearTimes: %d", gClientData[client].WarningPoint, gClientData[client].WarningPointClearTimes);
    PrintToChatAll("Honor Point:: <Steam, Rule, Other> = <%d, %d, %d>", gClientData[client].HonorPointSteam, gClientData[client].HonorPointRule, gClientData[client].HonorPointOther);
}
/**
 * @brief The map is initializing.
 */
public void OnMapStart()
{
    VToolsOnMapStart();
    VersionOnMapStart();
}

/**
 * @brief Map time has ended.
 */
public void OnMapEnd()
{

}

/**
 * @brief Called when client is disconnecting.
 *
 * @param client			The client index.
 */
public void OnClientDisconnect(int client)
{
	UsersOnClientDisconnect(client);
}

/**
 * @brief Called when client has been disconnected.
 * 
 * @param client            The client index.
 */
public void OnClientDisconnect_Post(int client)
{
    VToolsOnClientDisconnectPost(client);

    gClientData[client].ResetVars();
}

/**
 * @brief Called when client has already joined the game.
 * 
 * @param client            The client index.
 */
public void OnClientPostAdminCheck(int client)
{
    gClientData[client].ResetVars();

    // Use steamid64 as auth
    if (!IsFakeClient(client) && !GetClientAuthId(client, AuthId_SteamID64, gClientData[client].Steam64, sizeof(gClientData[].Steam64)))
    {
        KickClient(client, "无法获取到您的 SteamID, 请尝试重新连接服务器.\nUnable to get your SteamID, please try reconnect");
        return;
    }

    UsersOnClientPostAdminCheck(client);
    AdminOnClientPostAdminCheck(client);
    ReservedSlotOnClientPostAdminCheck(client);
}