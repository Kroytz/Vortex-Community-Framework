void BansOnPluginStart()
{
    RegConsoleCmd("sm_ban", BansOnCommandCatched, "sm_ban <#userid|name> <minutes|0> [reason]");
}

public Action BansOnCommandCatched(int client, int args)
{
    if (!AdminIsClientAdmin(client))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    // 查下有没有理由
    bool bReason = false;
    if (args == 3)
        bReason = true;

    // 获取目标字符串
    static char szArgString[64], szTargetIdentifier[64];
    GetCmdArg(1, szTargetIdentifier, sizeof(szTargetIdentifier));
    int target = FindTarget(client, szTargetIdentifier, true, false);

    GetCmdArg(2, szArgString, sizeof(szArgString));
    int time = StringToInt(szArgString);

    if (bReason)
        GetCmdArg(3, szArgString, sizeof(szArgString));
    else
        FormatEx(szArgString, sizeof(szArgString), "[UNKNOWN] 未给予理由");

    BansDoBan(client, target, szTargetIdentifier, time, szArgString);

    return Plugin_Handled;
}

void BansDoBan(int admin, int target, const char[] szIdentifier, int time, const char[] reason)
{
    char szName[33]; static char szTemp[32];
    char szAdminName[33], szAdmin64[33];
    if (admin == 0)
    {
        FormatEx(szAdminName, sizeof(szAdminName), "服务器");
        FormatEx(szAdmin64, sizeof(szAdmin64), "Console");
    }
    else
    {
        FormatEx(szAdminName, sizeof(szAdminName), "%s", gClientData[admin].Nick);
        FormatEx(szAdmin64, sizeof(szAdmin64), "%s", gClientData[admin].Steam64);
    }

    time *= 60;

    if (target >= 0) // 游戏内封禁?
    {
        FormatEx(szName, sizeof(szName), "%N", target);

        int displaytime = time / 60; Format(szTemp, sizeof(szTemp), "[Min] 分");
        if (displaytime >= 1440)
        {
            displaytime /= 1440; Format(szTemp, sizeof(szTemp), "[Day] 天");
        }
        else if (displaytime >= 60)
        {
            displaytime /= 60; Format(szTemp, sizeof(szTemp), "[Hour] 时");
        }

        if (time == 0)
        {
            Format(szTemp, sizeof(szTemp), "[Never] 永久封禁");
        }

        KickClient(target, 
        "======== [Ban Info] 封禁信息 ========\n[Steam64] 特征码 : %s\n[ModID] 模式 : %d\n[Length] 时长 : %d %s\n[Admin] 管理员: %s\n[Reason] 理由 : %s\n官网 Website: VEraCS.NeT", 
        gClientData[target].Steam64, 
        gServerData.ModID, 
        displaytime, szTemp, 
        szAdminName, 
        reason
        );

        LPrintToChatAllSingleLine("admin ban", admin, target, displaytime, szTemp);
    }
    else // 游戏外封禁?
    {
        // 游戏外封禁强制使用64封禁
        if (szIdentifier[0] == '#' || szIdentifier[0] == 'S')
        {
            LPrintToChatSingleLine(admin, "invalid identifier");
            return;
        }

        FormatEx(szName, sizeof(szName), "[Offline] 离线封禁");
    }

    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "INSERT INTO vcf_bans (auth,name,modid,adminauth,adminname,bantime,expiretime,reason) VALUES ('%s','%d','%s','%d','%d','%s');", 
    szIdentifier, szName, gServerData.ModID, szAdmin64, szAdminName, GetTime(), GetTime() + time, reason);
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery);
}

void BansOnClientPostAdminCheck(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    BansLoadBans(client);
}

void BansLoadBans(int client)
{
    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "SELECT * FROM vcf_bans WHERE modid = '%d' AND auth = '%s' AND (expiretime >= '%d' OR expiretime = bantime);", gServerData.ModID, gClientData[client].Steam64, GetTime());
    gServerData.DBI.Query(BansLoadBansCB, szQuery);
}

DBCallbackGeneral(BansLoadBansCB)
{
    if(dbRs == null)
    {
        LogError("[BansLoadBansCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    if(dbRs.FetchRow())
    {
        char szAdmin[33]; dbRs.FetchString(5, szAdmin, sizeof(szAdmin));
        char szReason[256]; dbRs.FetchString(6, szReason, sizeof(szReason));
        int expiretime = dbRs.FetchInt(7); expiretime -= GetTime();
        char szQuantity[32]; Format(szQuantity, sizeof(szQuantity), "S");

        if (expiretime <= 0)
        {
            expiretime = 0; Format(szQuantity, sizeof(szQuantity), "Never");
        }
        else if (expiretime >= 86400)
        {
            expiretime /= 86400; Format(szQuantity, sizeof(szQuantity), "D");
        }
        else if (expiretime >= 3600)
        {
            expiretime /= 3600; Format(szQuantity, sizeof(szQuantity), "H");
        }
        else if (expiretime >= 60)
        {
            expiretime /= 60; Format(szQuantity, sizeof(szQuantity), "M");
        }

        KickClient(client, 
        "======== [Ban Info] 封禁信息 ========\n[Steam64] 特征码 : %s\n[ModID] 模式 : %d\n[Expire] 解封 : %d %s\n[Admin] 管理员: %s\n[Reason] 理由 : %s\n官网 Website: VEraCS.NeT", 
        gClientData[client].Steam64, 
        gServerData.ModID, 
        expiretime, szQuantity, 
        szAdmin, 
        szReason
        );

        return;
    }
}