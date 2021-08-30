void FlipOnPluginStart()
{
    RegConsoleCmd("sm_flip", Command_Flip, "Flip a coin. Usage: sm_flip");
}

public Action Command_Flip(int client, int params)
{
    if (!IsPlayerExist(client))
        return Plugin_Handled;
    
    static int iValue;
    iValue = GetRandomInt(1, 2);

    static char sCoin[][] = {"head", "tail"};

    static char sName[NORMAL_LINE_LENGTH];
    GetClientName(client, sName, sizeof(sName));

    LPrintToChatAllSingleLine("flip out message", sName, (iValue == 1) ? sCoin[0] : sCoin[1]);

    // PrintToChatAll("%s fliped a coin: %s", sName, (iValue == 1) ? sCoin[0] : sCoin[1]);

    return Plugin_Handled;
}