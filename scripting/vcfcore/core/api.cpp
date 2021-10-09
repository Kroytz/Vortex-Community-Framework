/**
 * @section Struct of forwards used by the plugin.
 **/
enum struct ForwardData
{
    Handle OnClientLoaded;

    Handle OnBagItemShowPre;
    Handle OnBagItemUsePost;

    /**
     * @brief Initializes all forwards.
     * @note  APIOnInit()
     */
    void OnInit()
    {
        this.OnClientLoaded = CreateGlobalForward("VCF_OnClientLoaded", ET_Ignore, Param_Cell);

        // forward Action VCF_OnBagItemShowPre(int client, const char[] sItemCode);
        this.OnBagItemShowPre = CreateGlobalForward("VCF_OnBagItemShowPre", ET_Hook, Param_Cell, Param_String);

        // forward void   VCF_OnBagItemUsePost(int client, const char[] sItemCode);
        this.OnBagItemUsePost = CreateGlobalForward("VCF_OnBagItemUsePost", ET_Hook, Param_Cell, Param_String);
    }

    void _OnClientLoaded(int client)
    {
        Call_StartForward(this.OnClientLoaded);
        Call_PushCell(client);
        Call_Finish();
    }

    void _OnBagItemShowPre(int client, const char[] sItemCode, Action &hResult)
    {
        Call_StartForward(this.OnBagItemShowPre);
        Call_PushCell(client);
        Call_PushString(sItemCode);
        Call_Finish(hResult);
    }

    void _OnBagItemUsePost(int client, const char[] sItemCode, Action &hResult)
    {
        Call_StartForward(this.OnBagItemUsePost);
        Call_PushCell(client);
        Call_PushString(sItemCode);
        Call_Finish(hResult);
    }
}

/**
 * @section Convert forward data indexes into array.
 */
ForwardData gForwardData;

/**
 * @brief API module init function.
 * @note  AskPluginLoad2()
 **/
APLRes APIOnInit()
{
    gForwardData.OnInit();

    MarkNativeAsOptional("ZP_OnClientUpdated");

    MarkNativeAsOptional("Store_GetClientCredits");
    MarkNativeAsOptional("Store_SetClientCredits");
    MarkNativeAsOptional("Store_GiveItem");
    MarkNativeAsOptional("Store_GetItemidByUniqueid");

    MarkNativeAsOptional("checkip_93x");

    // Fire to sub modules
    APIOnNativeInit();
    VToolsOnNativeInit();
    UsersOnNativeInit();
    AdminOnNativeInit();

    // Register library
    RegPluginLibrary("vcf_core");

    // Return on success
    return APLRes_Success;
}

/**
 * @brief Initializes all natives.
 **/
void APIOnNativeInit(/*void*/)
{
    // Create main natives
    CreateNative("VCF_GetServerPublicIP", API_GetServerPublicIP);
    CreateNative("VCF_GetClientAccess",   API_GetClientAccess);
    CreateNative("VCF_GetClientPID",      API_GetClientPID);
    CreateNative("VCF_GetClientVIPLevel", API_GetClientVIPLevel);
}

public int API_GetServerPublicIP(Handle hPlugin, int iNumParams)
{
    int maxlen = GetNativeCell(2);

    return SetNativeString(1, gServerData.PublicIP, maxlen);
}

public int API_GetClientAccess(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);
    
    // Return the value
    return gClientData[client].Access;
}

public int API_GetClientPID(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);
    
    // Return the value
    return gClientData[client].PID;
}

public int API_GetClientVIPLevel(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);
    
    // Return the value
    return gClientData[client].VIPLevel;
}