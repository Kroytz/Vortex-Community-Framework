void RconOnPluginStart()
{
    RegConsoleCmd("sm_rcon", RconOnCommandCatched, "sm_rcon <args>");
}

public Action RconOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_RCON))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    if (args > 0)
    {
        static char szTemp[256];
        GetCmdArg(1, szTemp, sizeof(szTemp));

        BaseCommandsDoRcon(client, szTemp);
    }

    return Plugin_Handled;
}

void BaseCommandsDoRcon(int admin, const char[] szRcon)
{
    char szAdminName[33];
    if (admin == 0)
        FormatEx(szAdminName, sizeof(szAdminName), "Console");
    else
        FormatEx(szAdminName, sizeof(szAdminName), "%N", admin);

    DatabaseInsertAdminLog(admin, "Rcon", _, szRcon);
    LPrintToChatAllSingleLine("admin rcon", szAdminName, szRcon);

    if (admin == 0) // They will already see the response in the console.
    {
        ServerCommand("%s", szRcon);
    }
    else
    {
        char szResponse[4096];
        ServerCommandEx(szResponse, sizeof(szResponse), "%s", szRcon);
        LPrintToChatSingleLine(admin, "admin rcon result", szResponse);
    }
}