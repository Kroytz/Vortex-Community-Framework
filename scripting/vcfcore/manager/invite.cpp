// Rewards
char szAmmoPackReward[][] = { "zm_ap45", "zm_ap90", "zm_ap120", "zm_ap150" };
char szEXPReward[][] = { "zm_exp1k5", "zm_exp4k5", "zm_exp6k5", "zm_exp10k" };
char szWingsReward[][] = { "wing_perfectworld_wings01", "wing_perfectworld_wings07", "wing_perfectworld_wings08" };
char szSkinReward[][] = { "playerskin_huaji", "playerskin_goku", "playerskin_doomslayer" };

#define RANDOMBASESTRING "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define INVITE_CREDIT_REWARD 300

void InviteOnClientReady(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    InviteFetchUser(client);
}

void InviteFetchUser(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    // format query
    static char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT code,entered,invcount FROM vcf_invite WHERE pid = '%d';", gClientData[client].PID);

    // query
    gServerData.DBI.Query(InviteFetchUserCB, szQuery, client);
}

DBCallbackGeneral(InviteFetchUserCB)
{
    if(dbRs == null)
    {
        LogError("[InviteFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    // Check if there's his data
    if(dbRs.HasResults && dbRs.FetchRow())
    {
        dbRs.FetchString(0, gClientData[client].Code, 32); // 0
        gClientData[client].Entered = view_as<bool>(dbRs.FetchInt(1));
        gClientData[client].Count = dbRs.FetchInt(2);

        return;
    }
    else
    {
        gClientData[client].Entered = false;
        InviteCreateData(client);
    }
}

void InviteCreateData(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "INSERT INTO vcf_invite (pid) VALUES ('%d');", gClientData[client].PID);

    // query
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery, client);
}

void InviteUpdateData(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "UPDATE vcf_invite SET code = '%s', entered = '%d', invcount = '%d' WHERE pid = '%d';", gClientData[client].Code, view_as<int>(gClientData[client].Entered), gClientData[client].Count, gClientData[client].PID);

    // query
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery, client);
}

void InviteOnPluginStart()
{
    RegConsoleCmd("sm_invite", InviteOnCommandCatched);
}

public Action InviteOnCommandCatched(int client, int args)
{
    InviteDisplayMenu(client);
    return Plugin_Handled;
}

void InviteDisplayMenu(int client)
{
    if (!IsPlayerExist(client, false))
        return;

    SetGlobalTransTarget(client);

    char szInfo[256];
    Menu menu = new Menu(InviteMenuHandler);

    menu.SetTitle("%t\n ", "invite menu title");

    // 邀请码创建/更改\n邀请码: %s
    Format(szInfo, 255, "%t\n ", "invite item generate", (hasLength(gClientData[client].Code))?gClientData[client].Code:"[N/A]");
    menu.AddItem("generate", szInfo, (!hasLength(gClientData[client].Code))?ITEMDRAW_DEFAULT:ITEMDRAW_DISABLED);

    Format(szInfo, 255, "%t\n ", "invite item reward", gClientData[client].Count);
    menu.AddItem("reward", szInfo);

    Format(szInfo, 255, "%t", "invite item enter");
    menu.AddItem("enter", szInfo, (!gClientData[client].Entered)?ITEMDRAW_DEFAULT:ITEMDRAW_DISABLED);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);

    SetGlobalTransTarget(LANG_SERVER);
}

public int InviteMenuHandler(Handle menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select) 
    {
        char info[32];
        GetMenuItem(menu, itemNum, info, 32);

        if (StrEqual(info, "generate"))
        {
            if (gClientData[client].Cooldown > GetTime())
                return;

            // 尝试设定激活码: %s ...
            LPrintToChatSingleLine(client, "invite try generate code");
            RandomStringEx(RANDOMBASESTRING, gClientData[client].szTemp, 10);
            InviteTryGenerateCode(client);
            gClientData[client].Cooldown = GetTime() + 10;
        }
        else if (StrEqual(info, "reward"))
        {
            if (gClientData[client].Count > 0)
            {
                if (gServerData.core_Store)
                {
                    Store_SetClientCredits(client, Store_GetClientCredits(client) + gClientData[client].Count * INVITE_CREDIT_REWARD);

                    // 成功领取邀请奖励 {1} 积分!
                    LPrintToChatSingleLine(client, "invite reward", gClientData[client].Count * INVITE_CREDIT_REWARD);

                    gClientData[client].Count = 0;
                    InviteUpdateData(client);
                }
                else
                {
                    LPrintToChatSingleLine(client, "store core not available");
                }
            }
            else
            {
                // 您已经没有剩余奖励可以领取了!
                LPrintToChatSingleLine(client, "invite no reward remain");
            }
        }
        else if (StrEqual(info, "enter"))
        {
            gClientData[client].SayHookType = SAYHOOK_INVITE;

            // 请在聊天框内输入您收到的邀请码.
            LPrintToChatSingleLine(client, "invite type code try active");
        }
    }
    else if (action == MenuAction_End) delete menu;
}

void InviteTryGenerateCode(int client)
{
    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT * FROM vcf_invite WHERE code = '%s'", gClientData[client].szTemp);

    // query
    gServerData.DBI.Query(InviteTryGenerateCodeCB, szQuery, client);
}

DBCallbackGeneral(InviteTryGenerateCodeCB)
{
    if(dbRs == null)
    {
        LogError("[InviteTryGenerateCodeCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
            return;
        }

        return;
    }

    // Check if there's data
    if(dbRs.HasResults && dbRs.FetchRow())
    {
        // 激活码重复, 请重新输入!
        LPrintToChatSingleLine(client, "invite code already exist");
        return;
    }
    else
    {
        // 激活码更改成功!
        LPrintToChatSingleLine(client, "invite generate success");
        strcopy(gClientData[client].Code, sizeof(gClientData[].Code), gClientData[client].szTemp);
        InviteUpdateData(client);
        return;
    }
}

void InviteTryActiveCode(int client, const char[] szCode)
{
    if(StrEqual(szCode, gClientData[client].Code))
    {
        // 想用自己的邀请码?? 做梦呢??
        LPrintToChatSingleLine(client, "invite cant use self code");
        return;
    }

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT streamer FROM vcf_invite WHERE code = '%s'", szCode);
    strcopy(gClientData[client].szTemp, sizeof(gClientData[].szTemp), szCode);

    // query
    gServerData.DBI.Query(InviteTryActiveCodeCB, szQuery, client);
}

DBCallbackGeneral(InviteTryActiveCodeCB)
{
    if(dbRs == null)
    {
        LogError("[InviteFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    // Check if there's data
    if(dbRs.HasResults && dbRs.FetchRow())
    {
        gClientData[client].Entered = true;
        InviteUpdateData(client);
        InviteAddCodeCount(gClientData[client].szTemp);

        InviteRandomStreamerReward(client, dbRs.FetchInt(0));

        // 邀请码填写成功!
        LPrintToChatSingleLine(client, "invite activate success");

        LoopAllPlayers(i)
        {
            if (i == client)
                continue;

            InviteFetchUser(i);
        }

        return;
    }
    else
    {
        // 无法在数据库中找到该邀请码, 请重试!
        LPrintToChatSingleLine(client, "invite cant find code");
        return;
    }
}

void InviteAddCodeCount(const char[] code)
{
    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "UPDATE vcf_invite SET invcount = invcount + 1, total = total + 1 WHERE code = '%s'", code);

    // query
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery);
}

void InviteRandomStreamerReward(int client, int streamerlevel)
{
    if (streamerlevel == 0)
        return;

    int iReward = GetRandomInt(1, 100);

    switch (streamerlevel) // 1 = official | 2 = streamer
    {
        case 1:
        {
            if (iReward <= 75) // Ammopack
            {
                InventoryUpdateItem(client, szAmmoPackReward[GetRandomInt(0, 3)], "+", 5);
                LPrintToChatSingleLine(client, "invite free ammopack");
            }
            else if (iReward <= 100) // EXP
            {
                InventoryUpdateItem(client, szEXPReward[GetRandomInt(0, 3)], "+", 1);
                LPrintToChatSingleLine(client, "invite free exp");
            }
        }

        case 2:
        {
            if (iReward <= 50) // Ammopack
            {
                InventoryUpdateItem(client, szAmmoPackReward[GetRandomInt(0, 3)], "+", 5);
                LPrintToChatSingleLine(client, "invite free ammopack");
            }
            else if (iReward <= 85) // EXP
            {
                InventoryUpdateItem(client, szEXPReward[GetRandomInt(0, 3)], "+", 1);
                LPrintToChatSingleLine(client, "invite free exp");
            }
            else if (iReward <= 95) // Wings - 30 days
            {
                Store_GiveItem(client, Store_GetItemidByUniqueid(szWingsReward[GetRandomInt(0, 2)]), GetTime(), GetTime() + 2678400, 0);
                LPrintToChatSingleLine(client, "invite free wing");
            }
            else if (iReward <= 100) // Skins - lifetime
            {
                Store_GiveItem(client, Store_GetItemidByUniqueid(szSkinReward[GetRandomInt(0, 2)]));
                LPrintToChatSingleLine(client, "invite free skin");
            }
        }
    }
}

stock void RandomStringEx(const char[] random, char[] buffer, int maxLen)
{
    maxLen--;
    
    int randlen = strlen(random);

    int n = 0;
    int c = 0;

    while(n < maxLen)
    {
        c = GetRandomInt(0, randlen);
        buffer[n++] = random[c];
    }

    buffer[maxLen] = '\0';
}