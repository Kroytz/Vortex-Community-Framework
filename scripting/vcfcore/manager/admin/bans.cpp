#define BANS_WEBSITE "RolandHvH.com"

enum struct ban_info_t
{
    int   m_Target;
    int   m_Length;
    int   m_BanType;
    char  m_Identifier;
    char  m_Reason[256];
}
static   ban_info_t g_AdminSelect[MAXPLAYERS+1];

char szBanType[][] = 
{
    "Server 单服",
    "Mod 模式",
    "Community 社区"
};

enum /*BanType*/
{
    BANTYPE_SERVER,
    BANTYPE_MOD,
    BANTYPE_COMMUNITY
}

void BansOnPluginStart()
{
    RegConsoleCmd("sm_ban", BansOnCommandCatched, "sm_ban <#userid|name> <minutes|0> [reason]");
}

void BansOnSayHookEnd(int client, const char[] szString)
{
    strcopy(g_AdminSelect[client].m_Reason, sizeof(g_AdminSelect[].m_Reason), szString);
    BansGuideMenuFinale(client);
}

public Action BansOnCommandCatched(int client, int args)
{
    if (!AdminIsClientAdmin(client))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    // 参数足够, 直接进行封禁
    if (args >= 2)
    {
        // 查下有没有理由
        bool bReason = false;
        if (args == 3)
            bReason = true;

        // 获取目标字符串
        static char szArgString[64];
        GetCmdArg(1, g_AdminSelect[client].m_Identifier, sizeof(g_AdminSelect[].m_Identifier));
        int target = FindTarget(client, g_AdminSelect[client].m_Identifier, true, false);

        if (target != -1)
            g_AdminSelect[client].m_Target = target;

        // 获取时间
        GetCmdArg(2, szArgString, sizeof(szArgString));
        g_AdminSelect[client].m_Target = StringToInt(szArgString);

        if (bReason)
            GetCmdArg(3, g_AdminSelect[client].m_Reason, sizeof(g_AdminSelect[].m_Reason));
        else
            FormatEx(g_AdminSelect[client].m_Reason, sizeof(g_AdminSelect[].m_Reason), "No Reason Given. 未给予理由.");

        BansGuideMenuFinale(client);
    }
    else // 参数不够, 需要打开引导菜单
    {
        BansDisplayGuideMenuStepOne(client);
    }

    return Plugin_Handled;
}

void BansDisplayGuideMenuStepOne(int client)
{
    Menu menu = new Menu(BansGuideMenuStepOneHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);

    SetGlobalTransTarget(client);

    menu.SetTitle("%t\n ", "ban menu");

    AdminMenuAddTargetsToMenu(menu, client);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);

    SetGlobalTransTarget(LANG_SERVER);
}

public int BansGuideMenuStepOneHandler(Menu menu, MenuAction action, int admin, int slot)
{
    if (action == MenuAction_End)
        delete menu;
    else if (action == MenuAction_Select)
    {
        char info[32];
        menu.GetItem(slot, info, 32);

        int target = GetClientOfUserId(StringToInt(info));

        if (target == 0)
        {
            LPrintToChatSingleLine(admin, "invalid target");
        }
        else
        {
            g_AdminSelect[admin].m_Target = target;
            BansDisplayGuideMenuStepTwo(admin);
        }
    }
}

void BansDisplayGuideMenuStepTwo(int admin)
{
    Menu menu = new Menu(BansGuideMenuStepTwoHandler);
    
    menu.SetTitle("%t\n ", "ban menu");

    menu.AddItem("", "永久 Permanant", AdminHasClientFlag(admin, ADMFLAG_ROOT) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
    menu.AddItem("", "10分钟 10 Mins");
    menu.AddItem("", "30分钟 30 Mins");
    menu.AddItem("", "60分钟 60 Mins");
    menu.AddItem("", "1天 1 Days");
    menu.AddItem("", "3天 3 Days");

    menu.ExitButton = false;
    menu.Display(admin, 0);
}

public int BansGuideMenuStepTwoHandler(Menu menu, MenuAction action, int admin, int slot)
{
    if (action == MenuAction_End)
        delete menu;
    else if (action == MenuAction_Select)
    {
        switch(slot)
        {
            case 0: g_AdminSelect[admin].m_Length =    0;
            case 1: g_AdminSelect[admin].m_Length =   10;
            case 2: g_AdminSelect[admin].m_Length =   30;
            case 3: g_AdminSelect[admin].m_Length =   60;
            case 4: g_AdminSelect[admin].m_Length = 1440;
            case 5: g_AdminSelect[admin].m_Length = 4320;
        }

        BansDisplayGuideMenuStepThree(admin);
    }
}

void BansDisplayGuideMenuStepThree(int admin)
{
    Menu menu = new Menu(BansGuideMenuStepThreeHandler);
    
    menu.SetTitle("%t\n ", "ban menu");

    menu.AddItem("使用外挂", "使用外挂");
    menu.AddItem("恶意捣乱", "恶意捣乱");
    menu.AddItem("宣传广告", "宣传广告");
    menu.AddItem("骚扰他人", "骚扰他人");
    menu.AddItem("队友伤害", "队友伤害");
    menu.AddItem("破坏规则", "破坏规则");
    menu.AddItem("自定义理由", "自定义理由"); // Slot 6

    PrintToChat(admin, "=========== Bans ===========");
    PrintToChat(admin, "若要使用自定义利用请使用指令");
    PrintToChat(admin, "sm_ban <#userid或者名字> <时间(分钟)|0为永久> <原因>");
    PrintToChat(admin, "============================");

    menu.ExitButton = false;
    menu.Display(admin, 0);
}

public int BansGuideMenuStepThreeHandler(Menu menu, MenuAction action, int admin, int slot)
{
    if (action == MenuAction_End)
        delete menu;
    else if (action == MenuAction_Select)
    {
        if (slot < 6)
        {
            char reason[32];
            menu.GetItem(slot, reason, 32);
            strcopy(g_AdminSelect[admin].m_Reason, 256, reason);

            BansGuideMenuFinale(admin);
        }
        else
        {
            PrintToChat(admin, "请在聊天框内输入你的理由并按下回车");
            gClientData[admin].SayHookType = SAYHOOK_ADMINBAN;
        }
    }
}

void BansGuideMenuFinale(int admin)
{
    Menu menu = new Menu(BansGuideMenuFinaleHandler);
    
    if (g_AdminSelect[admin].m_Target != -1)
        menu.SetTitle("对象: %N\n时长: %d分钟\n原因: %s\n \n请选择封禁模式: \n ", g_AdminSelect[admin].m_Target, g_AdminSelect[admin].m_Length, g_AdminSelect[admin].m_Reason);
    else
        menu.SetTitle("对象: %s\n时长: %d分钟\n原因: %s\n \n请选择封禁模式: \n ", g_AdminSelect[admin].m_Identifier, g_AdminSelect[admin].m_Length, g_AdminSelect[admin].m_Reason);

    menu.AddItem("null", "单服封禁", ITEMDRAW_DISABLED);
    menu.AddItem("null", "模式封禁");
    menu.AddItem("null", "社区封禁", AdminHasClientFlag(admin, ADMFLAG_ROOT) ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);

    menu.ExitButton = false;
    menu.Display(admin, 0);
}

public int BansGuideMenuFinaleHandler(Menu menu, MenuAction action, int admin, int slot)
{
    if(action == MenuAction_End)
        delete menu;
    else if(action == MenuAction_Select)
    {
        g_AdminSelect[admin].m_BanType = slot;
        BansDoBan(admin, g_AdminSelect[admin].m_Target, gClientData[g_AdminSelect[admin].m_Target].Steam64, g_AdminSelect[admin].m_BanType, g_AdminSelect[admin].m_Length, g_AdminSelect[admin].m_Reason);
    }
}

void BansDoBan(int admin, int target, const char[] szIdentifier, int banType, int time, const char[] reason)
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

    if (target > 0) // 游戏内封禁?
    {
        FormatEx(szName, sizeof(szName), "%s", DatabaseGetPlayerName(target));

        int displaytime = time / 60; Format(szTemp, sizeof(szTemp), "Min");
        if (displaytime >= 1440)
        {
            displaytime /= 1440; Format(szTemp, sizeof(szTemp), "Day");
        }
        else if (displaytime >= 60)
        {
            displaytime /= 60; Format(szTemp, sizeof(szTemp), "Hour");
        }

        if (time == 0)
        {
            Format(szTemp, sizeof(szTemp), "Forever 永久封禁");
        }

        KickClient(target, 
        "======== [Ban Info] 封禁信息 ========\n[Steam64] 特征码 : %s\n[BanType] 封禁类型 : %s\n[Length] 时长 : %d %s\n[Admin] 管理员: %s\n[Reason] 理由 : %s\n官网 Website: %s", 
        gClientData[target].Steam64, 
        szBanType[banType], 
        displaytime, szTemp, 
        szAdminName, 
        reason, 
        BANS_WEBSITE
        );

        LPrintToChatAllSingleLine("admin ban", admin, target, szBanType[banType], displaytime, szTemp);
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
    FormatEx(szQuery, sizeof(szQuery), "INSERT INTO vcf_bans (auth,name,modid,serverid,adminauth,adminname,bantime,expiretime,bantype,reason) VALUES ('%s','%s','%d','%d','%s','%s','%d','%d','%d','%s');", 
    szIdentifier, szName, gServerData.ModID, gServerData.ServerID, szAdmin64, szAdminName, GetTime(), GetTime() + time, banType, reason);
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
    FormatEx(szQuery, sizeof(szQuery), "SELECT * FROM vcf_bans WHERE ((serverid = '%d' AND bantype = '0') OR (modid = '%d' AND bantype = '1') OR (bantype = '2')) AND auth = '%s' AND (expiretime >= '%d' OR expiretime = bantime);", gServerData.ServerID, gServerData.ModID, gClientData[client].Steam64, GetTime());
    gServerData.DBI.Query(BansLoadBansCB, szQuery, client);
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
        int banType = dbRs.FetchInt(9);
        char szAdmin[33]; dbRs.FetchString(6, szAdmin, sizeof(szAdmin));
        char szReason[256]; dbRs.FetchString(10, szReason, sizeof(szReason));
        int expiretime = dbRs.FetchInt(8); expiretime -= GetTime();
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
        "======== [Ban Info] 封禁信息 ========\n[Steam64] 特征码 : %s\n[BanType] 封禁类型 : %s\n[Expire] 解封 : %d %s\n[Admin] 管理员: %s\n[Reason] 理由 : %s\n官网 Website: %s", 
        gClientData[client].Steam64, 
        szBanType[banType], 
        expiretime, szQuantity, 
        szAdmin, 
        szReason, 
        BANS_WEBSITE
        );

        return;
    }
}

// int BansConvertStringToBanType(const char[] str)
// {
//     if (StrContains(str, "server", false) != -1)
//         return 0;
//     else if (StrContains(str, "mod", false) != -1)
//         return 1;
//     else if (StrContains(str, "community", false) != -1)
//         return 2;
// }