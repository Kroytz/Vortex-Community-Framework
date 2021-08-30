void SlayOnPluginStart()
{
    RegConsoleCmd("sm_slay", BaseCommandsOnSlayCommandCatched, "sm_slay <#userid>");
}

public Action BaseCommandsOnSlayCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_SLAY))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[256];
        GetCmdArg(1, szTemp, sizeof(szTemp));
        int target = AdminFindTarget(szTemp);

        BaseCommandsDoSlay(client, target);
    }

    BaseCommandsShowSlayMenu(client);
    return Plugin_Handled;
}

void BaseCommandsShowSlayMenu(int client)
{
    Menu menu = new Menu(BaseCommandsShowSlayMenuHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);

    SetGlobalTransTarget(client);

    menu.SetTitle("%t\n ", "slay menu");

    AdminMenuAddTargetsToMenu(menu, client, true);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);

    SetGlobalTransTarget(LANG_SERVER);
}

public int BaseCommandsShowSlayMenuHandler(Menu menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select)
    {
        static char szInfo[256];
        GetMenuItem(menu, itemNum, szInfo, 256);
        int target = GetClientOfUserId(StringToInt(szInfo));
        BaseCommandsDoSlay(client, target);
    }

    return 0;
}

void BaseCommandsDoSlay(int admin, int target)
{
    if (target <= 0 || !IsPlayerExist(target))
    {
        LPrintToChatSingleLine(admin, "invalid target");
        return;
    }

    DatabaseInsertAdminLog(admin, "Slay");

    static char szAdminName[33];
    if (admin == 0)
    {
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
        PrintToServer("[VCF] Slayed %N .", target);
    }
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);
    
    LPrintToChatAllSingleLine("admin slay", szAdminName, target);

    ExtinguishEntity(target);
    ForcePlayerSuicide(target);
}