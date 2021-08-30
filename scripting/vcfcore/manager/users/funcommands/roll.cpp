void RollOnPluginStart()
{
    RegConsoleCmd("sm_roll", Command_Roll, "Roll a dice. Usage: sm_roll <lb> <ub>");
}

public Action Command_Roll(int client, int params)
{
    if (!IsPlayerExist(client, false)) 
        return Plugin_Handled;

    static int iOutputValue, a, b;

    switch (params)
    {
        case 0: { a = 1; b = 100; }

        case 1:
        {
            static char sArg[SMALL_LINE_LENGTH];
            GetCmdArg(1, sArg, sizeof(sArg));
            a = 1;
            b = abs(StringToInt(sArg));
        }

        case 2:
        {
            static char sLb[SMALL_LINE_LENGTH], sUb[SMALL_LINE_LENGTH];
            GetCmdArg(1, sLb, sizeof(sLb));
            GetCmdArg(2, sUb, sizeof(sUb));
            a = abs(StringToInt(sLb));
            b = abs(StringToInt(sUb));
        }
        
        default: { a = 1; b = 100; }
    }

    static int i1, i2;
    if (a > b) { i1 = b; i2 = a; }
    else { i1 = a; i2 = b; }

    iOutputValue = GetRandomInt(i1, i2);

    static char sName[NORMAL_LINE_LENGTH];
    GetClientName(client, sName, sizeof(sName));
    LPrintToChatAllSingleLine("roll out message", sName, i1, i2, iOutputValue);
    // PrintToChatAll("%s rolls (%d - %d): %d", sName, i1, i2, iOutputValue);

    return Plugin_Handled;
}