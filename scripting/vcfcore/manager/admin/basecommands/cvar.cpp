void CvarOnPluginStart()
{
    RegConsoleCmd("sm_cvar", CvarOnCommandCatched, "sm_cvar <cvar> [value]");
    RegConsoleCmd("sm_resetcvar", ResetCvarOnCommandCatched, "sm_resetcvar <cvar>");

    gServerData.ProtectedCvar = new StringMap();
    CvarLoadProtectedCvar();
}

void CvarLoadProtectedCvar()
{
    // Add sth you want protect here
    gServerData.ProtectedCvar.SetValue("rcon_password", 1);
}

public Action CvarOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_CONVARS))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[256]; char szNewValue[256];
        GetCmdArg(1, szTemp, sizeof(szTemp));

        if (args > 1)
            GetCmdArg(2, szNewValue, sizeof(szNewValue));
        else
            szNewValue = "";

        BaseCommandsDoCvar(client, szTemp, szNewValue);
    }

    return Plugin_Handled;
}

void BaseCommandsDoCvar(int admin, const char[] szCvar, char[] szNewValue)
{
    ConVar cvar = FindConVar(szCvar);
    if (cvar == null)
    {
        return;
    }

    char szValue[255];
    if (!hasLength(szNewValue))
    {
        cvar.GetString(szValue, sizeof(szValue));

        if (admin == 0)
            PrintToServer("[VCF] Cvar '%s': %s", szCvar, szValue);
        else
            LPrintToChatSingleLine(admin, "admin cvar value", szCvar, szValue);

        return;
    }

    if (StrEqual(szCvar, "servercfgfile", false) || StrEqual(szCvar, "lservercfgfile", false))
    {
        int pos = StrContains(szNewValue, ";", true);
        if (pos != -1)
        {
            szNewValue[pos] = '\0';
        }
    }

    cvar.SetString(szNewValue, true);
    strcopy(szValue, sizeof(szValue), szNewValue);

    int iProtect = 0; gServerData.ProtectedCvar.GetValue(szCvar, iProtect);
    if (iProtect > 0)
        FormatEx(szValue, sizeof(szValue), "*** PROTECTED ***");

    char szAdminName[33];
    if (admin == 0)
    {
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
        PrintToServer("[VCF] Set cvar %s value to %s .", szCvar, szValue);
    }
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);

    LPrintToChatAllSingleLine("admin cvar", szAdminName, szCvar, szValue);

    FormatEx(szValue, sizeof(szValue), "%s -> %s", szCvar, szNewValue);
    DatabaseInsertAdminLog(admin, "Cvar", _, szValue);
}

public Action ResetCvarOnCommandCatched(int client, int args)
{
    if (!AdminIsClientAdmin(client))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[256];
        GetCmdArg(1, szTemp, sizeof(szTemp));

        BaseCommandsDoResetCvar(client, szTemp);
    }

    return Plugin_Handled;
}

void BaseCommandsDoResetCvar(int admin, const char[] szCvar)
{
    ConVar cvar = FindConVar(szCvar);
    if (cvar == null)
    {
        return;
    }

    cvar.RestoreDefault();

    DatabaseInsertAdminLog(admin, "Cvar Reset");
    LPrintToChatAllSingleLine("admin cvar reset", admin, szCvar);
}