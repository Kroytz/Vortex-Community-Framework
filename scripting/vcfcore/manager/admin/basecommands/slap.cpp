void SlapOnPluginStart()
{
    RegConsoleCmd("sm_slap", SlapOnCommandCatched, "sm_slap <#userid> [damage]");
}

public Action SlapOnCommandCatched(int client, int args)
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

        static int damage = 0;
        if (args > 1)
        {
            GetCmdArg(2, szTemp, sizeof(szTemp));
            damage = StringToInt(szTemp);
        }

        BaseCommandsDoSlap(client, target, damage);
    }

    BaseCommandsShowSlapMenu(client);
    return Plugin_Handled;
}

void BaseCommandsShowSlapMenu(int client)
{
    Menu menu = new Menu(BaseCommandsShowSlapMenuHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);

    SetGlobalTransTarget(client);

    menu.SetTitle("%t\n ", "slap menu");

    AdminMenuAddTargetsToMenu(menu, client, true);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);

    SetGlobalTransTarget(LANG_SERVER);
}

public int BaseCommandsShowSlapMenuHandler(Menu menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select)
    {
        static char szInfo[256];
        GetMenuItem(menu, itemNum, szInfo, 256);
        int target = GetClientOfUserId(StringToInt(szInfo));
        BaseCommandsDoSlap(client, target);
    }

    return 0;
}

void BaseCommandsDoSlap(int admin, int target, int damage = 0)
{
    if (target <= 0 || !IsPlayerExist(target))
    {
        LPrintToChatSingleLine(admin, "invalid target");
        return;
    }

    char szAdminName[33];
    if (admin == 0)
    {
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
        PrintToServer("[VCF] Slapped %N for %d damage.", target, damage);
    }
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);

    char sNote[256];
    Format(sNote, sizeof(sNote), "slapped %N for %d damage", target, damage);
    DatabaseInsertAdminLog(admin, "Slap", target, sNote);

    LPrintToChatAllSingleLine("admin slap", szAdminName, target, damage);

    SlapPlayer(target, damage, true);
}