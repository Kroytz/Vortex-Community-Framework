void KickOnPluginStart()
{
    RegConsoleCmd("sm_kick", KickOnCommandCatched, "sm_kick <#userid|name> [reason]");
}

public Action KickOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_KICK))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[256];
        GetCmdArg(1, szTemp, sizeof(szTemp));
        int target = FindTarget(client, szTemp, true, false);

        if (args > 1)
            GetCmdArg(2, szTemp, sizeof(szTemp));
        else
            FormatEx(szTemp, sizeof(szTemp), "[UNKNOWN] 未给予理由");

        BaseCommandsDoKick(client, target, szTemp);
    }

    BaseCommandsShowKickMenu(client);
    return Plugin_Handled;
}

void BaseCommandsShowKickMenu(int client)
{
    Menu menu = new Menu(BaseCommandsShowKickMenuHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);

    SetGlobalTransTarget(client);

    menu.SetTitle("%t\n ", "kick menu");

    AdminMenuAddTargetsToMenu(menu, client);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);

    SetGlobalTransTarget(LANG_SERVER);
}

public int BaseCommandsShowKickMenuHandler(Menu menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select)
    {
        static char szInfo[256];
        GetMenuItem(menu, itemNum, szInfo, 256);
        int target = GetClientOfUserId(StringToInt(szInfo));
        BaseCommandsDoKick(client, target);
    }

    return 0;
}

void BaseCommandsDoKick(int admin, int target, const char[] szReason = "[UNKNOWN] 未给予理由")
{
    if (target <= 0 || !IsPlayerExist(target, false))
    {
        if (admin == 0)
            PrintToServer("[VCF] Target invalid.");
        else
            LPrintToChatSingleLine(admin, "invalid target");

        return;
    }

    char szAdminName[33];
    if (admin == 0)
    {
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
        PrintToServer("[VCF] Kicked %N .", target);
    }
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);

    DatabaseInsertAdminLog(admin, "Kick", target, szReason);
    LPrintToChatAllSingleLine("admin kick", szAdminName, target, szReason);

    char szMessage[512];
    Format(szMessage, sizeof(szMessage), "You have been kicked.\n您已被踢出.\n\nReason 理由: %s", szReason);
    KickClient(target, szMessage);
}