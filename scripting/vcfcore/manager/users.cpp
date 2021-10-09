#include "vcfcore/manager/users/basehelp.cpp"
#include "vcfcore/manager/users/funcommands.cpp"
#include "vcfcore/manager/users/inventory.cpp"
#include "vcfcore/manager/users/sign.cpp"
#include "vcfcore/manager/users/vip.cpp"
// #include "vcfcore/manager/users/pwsupport.cpp"
// #include "vcfcore/manager/users/warningsys.cpp"

void UsersOnPluginStart()
{
    RegConsoleCmd("sm_vuser", UsersOnCommandCatched, "Show user center");

    FunCommandsOnPluginStart();
    InventoryOnPluginStart();
    SignOnPluginStart();

    // Iterate slowly...
    // WarningSysOnInit();
}

void UsersOnClientPostAdminCheck(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    UsersOnClientConnectedMessage(client);
    UsersFetchUser(client);
    // PWSupportOnClientInit(client);
}

void UsersOnClientConnectedMessage(int client)
{
    if (IsFakeClient(client))
        return;
    
    char szTemp[64]; char szCity[128];
    GetClientIP(client, szTemp, sizeof(szTemp));
    if (!gServerData.core_93xGeoIP)
    {
        if (!GeoipCountry(szTemp, szCity, sizeof(szCity)))
            FormatEx(szCity, sizeof(szCity), "Earth");
    }
    else
    {
        char szNetwork[64];
        checkip_93x(szTemp, szCity, 128, szNetwork, 64, 1);
    }

    LPrintToChatAllSingleLine("connect announce", client, szCity);
}

void UsersOnClientDisconnect(int client)
{
    #pragma unused client
    // PWSupportOnClientDisconnect(client);
}

void UsersFetchUser(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "SELECT pid, rulehonor, otherhonor, steamhonor, warningcredits, cleartimes, totalpurchase FROM vcf_users WHERE auth = '%s';", gClientData[client].Steam64);
    gServerData.DBI.Query(UsersFetchUserCB, szQuery, client);
}

DBCallbackGeneral(UsersFetchUserCB)
{
    if (!IsPlayerExist(client, false))
        return;

    if (dbRs == null)
    {
        LogError("[UsersFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }
    
    if (dbRs.FetchRow())
    {
        gClientData[client].PID             = dbRs.FetchInt(0);
        gClientData[client].HonorPointRule  = dbRs.FetchInt(1);
        gClientData[client].HonorPointOther = dbRs.FetchInt(2);
        gClientData[client].HonorPointSteam = dbRs.FetchInt(3);
        gClientData[client].WarningPoint    = dbRs.FetchInt(4);
        gClientData[client].WarningPointClearTimes = dbRs.FetchInt(5);
        gClientData[client].TotalPurchase = dbRs.FetchFloat(6);

        // UsersCheckVIPLevel(client);
        // WarningPointCheck(client);
        UsersOnClientReady(client);

        gForwardData._OnClientLoaded(client);
    }
    else
    {
        UsersInsertUser(client);
        UsersFetchUser(client);
    }
}

void UsersInsertUser(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "INSERT INTO vcf_users (auth) VALUES ('%s')", gClientData[client].Steam64);
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery, client);
}

void UsersOnClientReady(int client)
{
    AdminOnClientReady(client);
    InviteOnClientReady(client);
    InventoryOnClientReady(client);
    VIPOnClientReady(client);
}

public Action UsersOnCommandCatched(int client, int args)
{
    UsersShowMainMenu(client);
    return Plugin_Handled;
}

void UsersShowMainMenu(int client)
{
    SetGlobalTransTarget(client);

    Menu menu = new Menu(UsersMainMenuHandler);
    menu.SetTitle("%t\n ", "vusers main menu title");
    
    char szInfo[128];

    Format(szInfo, sizeof(szInfo), "%t", "vusers item info");
    menu.AddItem("", szInfo);

    Format(szInfo, sizeof(szInfo), "%t", "vusers item sign");
    menu.AddItem("", szInfo);

    Format(szInfo, sizeof(szInfo), "%t", "vusers item bag");
    menu.AddItem("", szInfo);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int UsersMainMenuHandler(Menu menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select)
    {
        switch (itemNum)
        {
            case 0: UsersInfoMenu(client);
            case 1: FakeClientCommand(client, "sm_qd");
            case 2: InventoryListInventory(client);
        }
    }
    else if (action == MenuAction_End) delete menu;
}

void UsersInfoMenu(int client)
{
    SetGlobalTransTarget(client);

    Menu menu = new Menu(UserInfoMenuHandler);
    menu.SetTitle("%t\n ", "vusers info menu title");
    
    char szInfo[128];

    FormatEx(szInfo, sizeof(szInfo), "PiD: %d", gClientData[client].PID);
    menu.AddItem("", szInfo, ITEMDRAW_DISABLED);

    FormatEx(szInfo, sizeof(szInfo), "Steam64: %s", gClientData[client].Steam64);
    menu.AddItem("", szInfo);

    FormatEx(szInfo, sizeof(szInfo), "%t: Lv. %d", "vusers info level", gClientData[client].VIPLevel);
    menu.AddItem("", szInfo, ITEMDRAW_DISABLED);

    Format(szInfo, sizeof(szInfo), "%t: %.2f %t", "vusers info totalpurchase", gClientData[client].TotalPurchase, "vusers purchase unit");
    menu.AddItem("", szInfo, ITEMDRAW_DISABLED);
    
    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int UserInfoMenuHandler(Menu menu, MenuAction action, int client, int itemNum)
{
    switch (action)
    {
        case MenuAction_End: delete menu;

        case MenuAction_Select:
        {
            switch (itemNum)
            {
                case 1: PrintToChat(client, "Steam64: %s", gClientData[client].Steam64);
            }
        }
    }
}

void UsersCheckVIPLevel(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    float total = gClientData[client].TotalPurchase;
    if (total >= 1000.0)
        gClientData[client].VIPLevel = 3;
    else if (total >= 500.0)
        gClientData[client].VIPLevel = 2;
    else if (total >= 100.0)
        gClientData[client].VIPLevel = 1;
    else
        gClientData[client].VIPLevel = 0;
}

bool IsClientInCooldown(int client)
{
    // 已经过了
    if (gClientData[client].Cooldown < GetTime())
    {
        return false;
    }
    else
    {
        LPrintToChatSingleLine(client, "try again later cooldown");
        return true;
    }
}

void UsersOnNativeInit(/*void*/)
{
    InventoryOnNativeInit();
    SignOnNativeInit();

    CreateNative("VCF_GetClientTotalPurchase",      API_GetClientTotalPurchase);
    CreateNative("VCF_GetClientWarningPoint",       API_GetClientWarningPoint);
    CreateNative("VCF_SetClientWarningPoint",       API_SetClientWarningPoint);
    CreateNative("VCF_GetClientHonorPointSteam",    API_GetClientHonorPointSteam);
    CreateNative("VCF_SetClientHonorPointSteam",    API_SetClientHOnorPointSteam);
    CreateNative("VCF_GetClientHonorPointRule",     API_GetClientHonorPointRule);
    CreateNative("VCF_SetClientHonorPointRule",     API_SetClientHonorPointRUle);
    CreateNative("VCF_GetClientHonorPointOther",    API_GetClientHonorPointOther);
    CreateNative("VCF_SetClientHonorPointOther",    API_SetClientHonorPointOther);
    CreateNative("VCF_GetWarningPointClearTimes",   API_GetWarningPointClearTimes);
}

public int API_GetClientTotalPurchase(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return view_as<int>(gClientData[client].TotalPurchase);
}

public int API_GetClientWarningPoint(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return gClientData[client].WarningPoint;
}

public int API_SetClientWarningPoint(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client, false)) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%i)", client);

    int iValue = GetNativeCell(2);

    if (iValue < 0) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid value (%i)", iValue);

    gClientData[client].WarningPoint = iValue;
    return 0;
}

public int API_GetClientHonorPointSteam(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return gClientData[client].HonorPointSteam;
}

public int API_SetClientHOnorPointSteam(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client, false)) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%i)", client);

    int iValue = GetNativeCell(2);

    if (iValue < 0) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid value (%i)", iValue);

    gClientData[client].HonorPointSteam = iValue;
    return 0;
}

public int API_GetClientHonorPointRule(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return gClientData[client].HonorPointRule;
}

public int API_SetClientHonorPointRUle(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client, false)) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%i)", client);

    int iValue = GetNativeCell(2);

    if (iValue < 0) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid value (%i)", iValue);

    gClientData[client].HonorPointRule = iValue;
    return 0;
}

public int API_GetClientHonorPointOther(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return gClientData[client].HonorPointOther;
}

public int API_SetClientHonorPointOther(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client, false)) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%i)", client);

    int iValue = GetNativeCell(2);

    if (iValue < 0) return ThrowNativeError(SP_ERROR_NATIVE, "Invalid value (%i)", iValue);

    gClientData[client].HonorPointOther = iValue;
    return 0;
}

public int API_GetWarningPointClearTimes(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return gClientData[client].WarningPointClearTimes;
}
