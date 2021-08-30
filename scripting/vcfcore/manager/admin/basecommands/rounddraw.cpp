void RoundDrawOnPluginStart()
{
    RegConsoleCmd("sm_rounddraw", RoundDrawOnCommandCatched, "sm_rounddraw");
}

public Action RoundDrawOnCommandCatched(int client, int args)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_SLAY))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    BaseCommandsDoRoundDraw(client);
    return Plugin_Handled;
}

void BaseCommandsDoRoundDraw(int admin)
{
    CS_TerminateRound(12.0, CSRoundEnd_Draw, false);

    DatabaseInsertAdminLog(admin, "RoundDraw");
    LPrintToChatAllSingleLine("admin rounddraw", admin);
}