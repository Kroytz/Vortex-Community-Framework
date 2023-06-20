#define PLAYERREWARDMAX 30
#define VIPEXTRAREWARDMAX 20 // 20 Per level
#define CONTINOUSSIGNREWARD 2
#define CONTINOUSSIGNREWARDMAX 100

#define VIPEXPREWARDMIN 4500
#define VIPEXPREWARDOFFSET 1600
#define VIPEXPREWARDMAX 12000

void SignOnPluginStart()
{
    RegConsoleCmd("sm_qd", SignOnCommandCatched);
    RegConsoleCmd("sm_sign", SignOnCommandCatched);
}

public Action SignOnCommandCatched(int client, int args)
{
    if (IsClientInCooldown(client))
        return Plugin_Handled;

    if (!gServerData.core_Store)
    {
        LPrintToChatSingleLine(client, "store core not available");
        return Plugin_Handled;
    }

    SignDoSign(client);
    return Plugin_Handled;
}

void SignDoSign(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT * FROM vcf_sign WHERE pid = '%d';", gClientData[client].PID);

    // query
    gServerData.DBI.Query(SignDoSignCB, szQuery, client);
}

DBCallbackGeneral(SignDoSignCB)
{
    if(dbRs == null)
    {
        LogError("[SignDoSignCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    int iLastSign = 0, continoussign = 0;
    if (dbRs.HasResults && dbRs.FetchRow())
    {
        // 0 = pid
        iLastSign = dbRs.FetchInt(1);
        continoussign = dbRs.FetchInt(2);

        int iLastSignDays = GetTotalDays(iLastSign);
        int iSignDays = GetTotalDays(GetTime());

        if (iLastSignDays == iSignDays)
        {
            // 重复签到
            LPrintToChatSingleLine(client, "sign already signed");
            gClientData[client].Cooldown = GetTime() + 30;

            return;
        }

        if (iLastSignDays != iSignDays-1)
            continoussign = 0;
    }
    else
    {
        SignCreateData(client);

        // 稍后再试
        LPrintToChatSingleLine(client, "try again later error");
        gClientData[client].Cooldown = GetTime() + 5;

        return;
    }

    SignSignSuccess(client, continoussign+1);
}

void SignCreateData(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "INSERT INTO vcf_sign (pid) VALUES ('%d');", gClientData[client].PID);

    // query
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery, client);
}

void SignSignSuccess(int client, int continoussign)
{
    SignSetBonusRatio(client);

    int totalReward = 0;
    int basicReward = PLAYERREWARDMAX;
    int vipReward = 0;
    int continousReward = 0;
    int basicExtra = GetRandomInt(1, PLAYERREWARDMAX);
    int vipExtra = 0;

    if (gClientData[client].VIPLevel > 0)
    {
        vipReward = VIPEXTRAREWARDMAX * gClientData[client].VIPLevel;
        vipExtra = GetRandomInt(1, VIPEXTRAREWARDMAX * gClientData[client].VIPLevel);
    }

    if (continoussign > 1)
    {
        int continousRewardMax = CONTINOUSSIGNREWARD * continoussign;
        if (continousRewardMax > CONTINOUSSIGNREWARDMAX)
            continousRewardMax = CONTINOUSSIGNREWARDMAX;
        
        if (continoussign > CONTINOUSSIGNREWARDMAX)
            continoussign = CONTINOUSSIGNREWARDMAX;
        
        if (continoussign != continousRewardMax)
            continousReward = GetRandomInt(continoussign, continousRewardMax);
        else
            continousReward = CONTINOUSSIGNREWARDMAX;
    }

    totalReward = basicReward + vipReward + continousReward + basicExtra + vipExtra;

    float flSignRatio = gClientData[client].SignBasicRatio + gClientData[client].SignBonusRatio;
    totalReward = RoundToCeil(flSignRatio * float(totalReward));
    
    // 签到成功! 获得_RNDINT_纪元币!
    LPrintToChatSingleLine(client, "sign reward", totalReward, continoussign);
    Store_SetClientCredits(client, Store_GetClientCredits(client) + totalReward);

    // PrintToChat(client, "[基础积分] %d | [VIP 积分] %d", basicReward, vipReward);
    // PrintToChat(client, "[基础随机] %d | [VIP 随机] %d", basicExtra, vipExtra);
    // PrintToChat(client, "[连签额外] %d | [获取倍率] %.2f", continousReward, flSignRatio);
    // PrintToChat(client, "--- [最终获得] %d", totalReward);

    if (gServerData.mod_ZombiePlague)
    {
        if (gClientData[client].VIPLevel > 0)
        {
            int iExpReward = GetRandomInt((VIPEXPREWARDMIN + (gClientData[client].VIPLevel * VIPEXPREWARDOFFSET)), VIPEXPREWARDMAX);
            ZP_SetClientExp(client, ZP_GetClientExp(client) + iExpReward);
            PrintToChat(client, "[VIP] 僵尸感染 经验值增加 + %d", iExpReward);
        }
    }

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "UPDATE vcf_sign SET lastsign = '%d', continoussign = '%d' WHERE pid = '%d';", GetTime(), continoussign, gClientData[client].PID);

    // query
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery, client);
}

int GetTotalDays(int UNIXTimeStamp)
{
    return (UNIXTimeStamp / 86400);
}

void SignSetBonusRatio(int client)
{
    switch (gClientData[client].VIPLevel)
    {
        case 1:  gClientData[client].SignBonusRatio = 0.35;
        case 2:  gClientData[client].SignBonusRatio = 0.7;
        case 3:  gClientData[client].SignBonusRatio = 1.25;
        default: gClientData[client].SignBonusRatio = 0.0;
    }
}

void SignOnNativeInit()
{
    CreateNative("VCF_GetClientSignBasicRatio",       API_GetClientSignBasicRatio);
    CreateNative("VCF_SetClientSignBasicRatio",       API_SetClientSignBasicRatio);
    CreateNative("VCF_GetClientSignBonusRatio",       API_GetClientSignBonusRatio);
    CreateNative("VCF_SetClientSignBonusRatio",       API_SetClientSignBonusRatio);

    MarkNativeAsOptional("ZP_SetClientExp");
    MarkNativeAsOptional("ZP_GetClientExp");
    MarkNativeAsOptional("ZP_GetClientLevel");
    MarkNativeAsOptional("ZP_GetLevelMax");
}

public int API_GetClientSignBasicRatio(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return view_as<int>(gClientData[client].SignBasicRatio);
}

public int API_SetClientSignBasicRatio(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);
    gClientData[client].SignBasicRatio = GetNativeCell(2);
    return 0;
}

public int API_GetClientSignBonusRatio(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);

    return view_as<int>(gClientData[client].SignBonusRatio);
}

public int API_SetClientSignBonusRatio(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);
    gClientData[client].SignBonusRatio = GetNativeCell(2);
    return 0;
}