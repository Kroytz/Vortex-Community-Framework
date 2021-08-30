void WhoOnPluginStart()
{
    RegConsoleCmd("sm_who", WhoOnCommandCatched, "sm_who");
}

public Action WhoOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_BAN))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    BaseCommandsDoWho(client);

    return Plugin_Handled;
}

void BaseCommandsDoWho(int admin)
{
    static char szName[64], szIP[64]; static int iUid;
    PrintToConsole(admin, "============================== 出道信息 ==============================");
    PrintToConsole(admin, "#UserID    |    Name    |    Steam64    |    IP    | Access | Nick");

    LoopAllPlayers(target)
    {
        if (IsFakeClient(target))
            continue;

        iUid = GetClientUserId(target);
        GetClientName(target, szName, 64);
        GetClientIP(target, szIP, 64);

        if (gClientData[target].Access > gClientData[admin].Access)
        {
            FormatEx(szIP, sizeof(szIP), "想屁吃呢");
        }

        PrintToConsole(admin, "#%d   %s   %s   %s   %d  %s", iUid, szName, gClientData[target].Steam64, szIP, gClientData[target].Access, gClientData[target].Nick);
    }

    PrintToConsole(admin, "=====================================================================");
}