enum
{
    ADMINMENU_TITLE,
    ADMINMENU_COMMAND
}

void AdminMenuOnPluginStart()
{
    RegConsoleCmd("sm_vadmin", AdminMenuOnCommandCatched);

    AdminMenuLoadMenu();
}

public Action AdminMenuOnCommandCatched(int client, int args)
{
    if (!AdminIsClientAdmin(client))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    AdminMenuShowAdminMenu(client);
    return Plugin_Handled;
}

void AdminMenuShowAdminMenu(int client)
{
    char sOption[256], sCommand[256];
    Menu menu = new Menu(AdminMenuShowAdminMenuHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);

    SetGlobalTransTarget(client);

    menu.SetTitle("%t\n ", "admin menu");

    int iSize = gServerData.AdminMenu.Length;
    if (iSize == 0)
    {
        FormatEx(sOption, sizeof(sOption), "%t", "empty");
        menu.AddItem("", sOption, ITEMDRAW_DISABLED);
    }
    else
    {
        for (int i = 0; i < iSize; ++i)
        {
            AdminMenuGetTitle(i, sOption, sizeof(sOption));
            AdminMenuGetCommand(i, sCommand, sizeof(sCommand));
            FormatEx(sOption, sizeof(sOption), "%t", sOption);
            menu.AddItem(sCommand, sOption);
        }
    }

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);

    SetGlobalTransTarget(LANG_SERVER);
}

public int AdminMenuShowAdminMenuHandler(Menu menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select)
    {
        static char szInfo[256];
        GetMenuItem(menu, itemNum, szInfo, 256);
        FakeClientCommand(client, szInfo);
    }

    return 0;
}

void AdminMenuLoadMenu()
{
    if (gServerData.AdminMenu != null)
        delete gServerData.AdminMenu;

    gServerData.AdminMenu = new ArrayList(128);

    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "SELECT * FROM `vcf_adminmenu` ORDER BY CONVERT(`title` USING GBK) ASC;");
    gServerData.DBI.Query(AdminMenuLoadMenuCB, szQuery);
}

DBCallbackGeneral(AdminMenuLoadMenuCB)
{
    if(dbRs == null)
    {
        LogError("[AdminMenuLoadMenuCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    while (dbRs.FetchRow())
    {
        int iMod = dbRs.FetchInt(2);
        if (iMod != 0 && iMod != gServerData.ModID)
            continue;

        char sString[64];
        ArrayList arraySlot = new ArrayList(128);

        // Title
        dbRs.FetchString(0, sString, sizeof(sString));
        arraySlot.PushString(sString);

        // Command
        dbRs.FetchString(1, sString, sizeof(sString));
        arraySlot.PushString(sString);

        gServerData.AdminMenu.Push(arraySlot);
    }
}

void AdminMenuGetTitle(int iD, char[] sString, int iMaxLen)
{
    ArrayList arraySlot = gServerData.AdminMenu.Get(iD);
    arraySlot.GetString(ADMINMENU_TITLE, sString, iMaxLen);
}

void AdminMenuGetCommand(int iD, char[] sString, int iMaxLen)
{
    ArrayList arraySlot = gServerData.AdminMenu.Get(iD);
    arraySlot.GetString(ADMINMENU_COMMAND, sString, iMaxLen);
}

int AdminMenuAddTargetsToMenu(Menu menu, int source_client, bool alive_only = false, bool allowbot = false)
{
    char user_id[12];
    char name[MAX_NAME_LENGTH];
    char display[MAX_NAME_LENGTH+12];
    
    int num_clients;
    
    for (int i = 1; i <= MaxClients; i++)
    {
        if (!IsPlayerExist(i, false))
            continue;
        
        if (!allowbot && IsFakeClient(i))
            continue;
        
        // 关于这边为什么没用Exist: 上面用过了不用再判断合法了
        if (alive_only && !IsPlayerAlive(i))
            continue;

        // 下属不能啵上司嘴！！！
        if (gClientData[source_client].Access < gClientData[i].Access)
            continue;
        
        IntToString(GetClientUserId(i), user_id, sizeof(user_id));
        GetClientName(i, name, sizeof(name));
        Format(display, sizeof(display), "%s (%s)", name, user_id);
        menu.AddItem(user_id, display);
        num_clients++;
    }
    
    return num_clients;
}