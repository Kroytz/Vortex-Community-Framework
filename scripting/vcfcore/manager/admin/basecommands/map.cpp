void MapOnPluginStart()
{
    RegConsoleCmd("sm_map", MapOnCommandCatched, "sm_map <map>");
}

public Action MapOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_MAP))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[512];
        GetCmdArg(1, szTemp, sizeof(szTemp));

        BaseCommandsDoMap(client, szTemp);
    }

    return Plugin_Handled;
}

void BaseCommandsDoMap(int admin, const char[] szMapName)
{
    char szTemp[512];
    if (FindMap(szMapName, szTemp, sizeof(szTemp)) == FindMap_NotFound)
    {
        if (admin == 0)
            PrintToServer("[VCF] Can't find map: %s", szMapName);
        else
            LPrintToChatSingleLine(admin, "map not found", szMapName);
        return;
    }

    char szAdminName[33];
    if (admin == 0)
    {
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
        PrintToServer("[VCF] Changing map to: %s ...", szTemp);
    }
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);

    DatabaseInsertAdminLog(admin, "Map", _, szTemp);
    LPrintToChatAllSingleLine("admin map", szAdminName, szTemp);

    DataPack dp = new DataPack();
    dp.WriteString(szMapName);
    CreateTimer(3.0, MapTimerChangeMap, dp);
}

public Action MapTimerChangeMap(Handle timer, DataPack dp)
{
    char szMap[512];

    dp.Reset();
    dp.ReadString(szMap, sizeof(szMap));
    delete dp;

    ForceChangeLevel(szMap, "sm_map Command");

    return Plugin_Stop;
}