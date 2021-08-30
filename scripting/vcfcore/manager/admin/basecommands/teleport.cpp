void TeleportOnPluginStart()
{
    RegConsoleCmd("sm_teleport", TeleportOnCommandCatched, "sm_teleport");
}

public Action TeleportOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_SLAY))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    Util_ShowTeleportMenu(client);

    return Plugin_Handled;
}

static void Util_ShowTeleportMenu(int client)
{
    char szOption[128];
    Menu menu = new Menu(MenuHandler_TeleportType);
    
    SetGlobalTransTarget(client);

    menu.SetTitle("%t\n ", "teleport menu");
    
    Format(szOption, sizeof(szOption), "%t", "teleport menu to");
    menu.AddItem("1", szOption);

    Format(szOption, sizeof(szOption), "%t", "teleport menu back");
    menu.AddItem("2", szOption);

    Format(szOption, sizeof(szOption), "%t", "teleport menu ab");
    menu.AddItem("3", szOption);

    menu.ExitBackButton = true;
    menu.Display(client, 30);
}

public int MenuHandler_TeleportType(Menu menu, MenuAction action, int admin, int slot)
{
    if(action == MenuAction_End)
        delete menu;
    else if(action == MenuAction_Cancel && slot == MenuCancel_ExitBack && g_hTopMenu != null)
        g_hTopMenu.Display(admin, TopMenuPosition_LastCategory);
    else if(action == MenuAction_Select)
    {
        char info[4];
        menu.GetItem(slot, info, 4);
        Util_ShowTeleporTargetMenu(admin, StringToInt(info));
    }
}

static void Util_ShowTeleporTargetMenu(int client, int type)
{
    Menu menu = new Menu(MenuHandler_TeleportTarget);
    
    _iTPType[client] = type;
    
    switch(type)
    {
        case 1: menu.SetTitle("选择要传送的玩家");
        case 2: menu.SetTitle("选择要传送的玩家");
        case 3: menu.SetTitle("选择要传送的玩家A");
    }

    char userid[8], name[32], buffer[40];
    for(int target = MinClients; target <= MaxClients; ++target)
        if (IsPlayerExist(target))
        {
            IntToString(GetClientUserId(target), userid, 8);
            GetClientName(target, name, 32);
            FormatEx(buffer, 40, "%32s (%8s)", name, userid);
            menu.AddItem(userid, buffer);
        }

    menu.ExitBackButton = true;
    menu.Display(client, 30);
}

public int MenuHandler_TeleportTarget(Menu menu, MenuAction action, int client, int slot)
{
    if(action == MenuAction_End)
        delete menu;
    else if(action == MenuAction_Cancel && slot == MenuCancel_ExitBack && g_hTopMenu != null)
        Util_ShowTeleportMenu(client);
    else if(action == MenuAction_Select)
    {
        char info[8];
        menu.GetItem(slot, info, 8);
        _iTarget[client] = StringToInt(info);
        int target = GetClientOfUserId(_iTarget[client]);

        switch(_iTPType[client])
        {
            case 1: Util_TeleportDest(client, client, target);
            case 2: Util_TeleportDest(client, target, client);
            case 3: DisplayAgainMenu(client);
        }
    }
}

void DisplayAgainMenu(int client)
{
    int source = GetClientOfUserId(_iTarget[client]);

    Menu menu = new Menu(MenuHandler_TeleportTarget2);

    menu.SetTitle("选择要传送的玩家B");
    
    char uid[8], name[64];
    for(int i = 1; i <= MaxClients; i++)
    {
        if(i == source || !IsPlayerExist(i))
            continue;

        int userid = GetClientUserId(i);
        IntToString(userid, uid, 8);
        FormatEx(name, 64, "%N (%d)", i, userid);
        menu.AddItem(uid, name);
    }

    menu.ExitBackButton = true;
    menu.Display(client, 30);
}

public int MenuHandler_TeleportTarget2(Menu menu, MenuAction action, int client, int slot)
{
    if(action == MenuAction_End)
        delete menu;
    else if(action == MenuAction_Cancel && slot == MenuCancel_ExitBack && g_hTopMenu != null)
        Util_ShowTeleportMenu(client);
    else if(action == MenuAction_Select) 
    {
        char info[8];
        menu.GetItem(slot, info, 8);
        Util_TeleportDest(client, GetClientOfUserId(_iTarget[client]), GetClientOfUserId(StringToInt(info)));
    }
}

void Util_TeleportDest(int admin, int source, int target)
{
    if(!IsPlayerExist(source) || !IsPlayerExist(target))
    {
        LPrintToChatSingleLine(admin, "invalid target");
        return;
    }

    float fPos[3], fAgl[3];
    GetClientAbsOrigin(target, fPos);
    GetClientEyeAngles(target, fAgl);
    TeleportEntity(source, fPos, fAgl, NULL_VECTOR);

    char szTemp[256];
    FormatEx(szTemp, sizeof(szTemp), "Source: %N -> %N : Target");

    LPrintToChatAllSingleLine("admin teleport", admin, source, target);
    DatabaseInsertAdminLog(admin, "Teleport", source, szTemp);
}