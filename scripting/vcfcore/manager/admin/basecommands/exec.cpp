void ExecOnPluginStart()
{
    RegConsoleCmd("sm_exec", ExecOnCommandCatched, "sm_exec <filename>");
}

public Action ExecOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_CONFIG))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[256];
        GetCmdArg(1, szTemp, sizeof(szTemp));
        FormatEx(szTemp, sizeof(szTemp), "cfg/%s");

        BaseCommandsDoExec(client, szTemp);
    }

    return Plugin_Handled;
}

void BaseCommandsDoExec(int admin, const char[] szConfig)
{
    if (!FileExists(szConfig))
    {
        LPrintToChatSingleLine(admin, "file not found");
        return;
    }

    static char szTemp[256];
    strcopy(szTemp, sizeof(szTemp), szConfig);
    ReplaceString(szTemp, sizeof(szTemp), "cfg/", "", false);
    ServerCommand("exec \"%s\"", szTemp);

    char szAdminName[33];
    if (admin == 0)
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);

    DatabaseInsertAdminLog(admin, "Exec", _, szTemp);
    LPrintToChatAllSingleLine("admin exec", szAdminName, szTemp);
}