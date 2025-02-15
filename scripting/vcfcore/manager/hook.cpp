void HookOnPluginStart()
{
    HookEvent("round_start", HookOnRoundStart);
    HookEvent("round_end", HookOnRoundEnd);
    HookEvent("player_spawn", HookOnPlayerSpawn);
    HookEvent("player_death", HookOnPlayerDeath);
    HookEvent("player_jump", HookOnPlayerJump);
}

public void HookOnRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    gServerData.RoundStart = GetGameTime();
}

public void HookOnRoundEnd(Event event, const char[] name, bool dontBroadcast)
{
    CoupleSysOnRoundEnd();
}

public void HookOnPlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));

    if (!IsPlayerExist(client, false))
        return;

    VToolsOnPlayerSpawn(client);
}

public void HookOnPlayerDeath(Event event, const char[] name, bool dontBroadcast)
{
    int client = GetClientOfUserId(event.GetInt("userid"));

    if (!IsPlayerExist(client, false))
        return;

    VToolsOnPlayerDeath(client);
}

public void HookOnPlayerJump(Event hEvent, char[] sName, bool dontBroadcast) 
{
    int client = GetClientOfUserId(hEvent.GetInt("userid"));

    if (!IsPlayerExist(client))
        return;

    VToolsOnPlayerJump(client);
}

public void ZP_OnClientUpdated(int client, int attacker)
{
    if (!IsPlayerExist(client))
        return;

    VToolsOnClientUpdated(client);
}

public Action OnPlayerRunCmd(int client, int& buttons, int& impulse, float vel[3], float angles[3], int& weapon, int& subtype, int& cmdnum, int& tickcount, int& seed, int mouse[2])
{
    if (!IsPlayerExist(client))
        return Plugin_Continue;

    static int oldbuttons[MAXPLAYERS+1] = {0, ...};

    VToolsOnPlayerRunCmd(client, oldbuttons[client], buttons, impulse, vel, angles, weapon, subtype, cmdnum, tickcount, seed, mouse);

    oldbuttons[client] = buttons;
    return Plugin_Continue;
}

public Action OnClientSayCommand(int client, const char[] command, const char[] sArgs)
{
    if (gClientData[client].SayHookType != SAYHOOK_NONE)
    {
        switch(gClientData[client].SayHookType)
        {
            case SAYHOOK_INVITE:
            {
                InviteTryActiveCode(client, sArgs);
            }
            case SAYHOOK_ADMINBAN:
            {
                BansOnSayHookEnd(client, sArgs);
            }
        }

        gClientData[client].SayHookType = SAYHOOK_NONE;
        return Plugin_Handled;
    }

    if (!strcmp(sArgs, "/?"))
    {
        BaseHelpDisplay(client);
        return Plugin_Handled;
    }

    return Plugin_Continue;
}