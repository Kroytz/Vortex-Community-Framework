enum struct Couples
{
    int iPartnerIndex;
    int iPartnerPlayerId;
    int iWeddingDate;
    int iCPExp;
    int iCPLvl;
    int iCPEarnExp;
    int iTogether;
    int iTogetherPlay;
    int iLuv;
    char szPartnerName[32];
}

static int Couples_Data_Client_ProposeTargetUserId[MAXPLAYERS+1];
static int Couples_Data_Client_ProposeSelectUserId[MAXPLAYERS+1];
static int Couples_Data_Client_ProposeSelectedTime[MAXPLAYERS+1];

Panel Couples_Panel_About;

void CoupleSysOnPluginStart()
{
    RegConsoleCmd("sm_cp",      CoupleSysOnCommandCatchedCouples);
    RegConsoleCmd("sm_couples", CoupleSysOnCommandCatchedCouples);
    RegConsoleCmd("sm_propose", CoupleSysOnCommandCatchedPropose);

    CoupleSysCreateAboutMenu();

    InitUserMessage();
    SetChatPrefix(" [\x0ECouples\x01] ");

}

void CoupleSysOnClientReady(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    CoupleSysFetchUser(client);
}

public Action CoupleSysOnCommandCatchedCouples(int client, int args)
{
    if(!IsPlayerExist(client, false) || !gClientData[client].PID)
        return Plugin_Handled;

    CoupleSysDisplayMainMenu(client);

    return Plugin_Handled;
}

public Action CoupleSysOnCommandCatchedPropose(int client, int args)
{
    if(!IsPlayerExist(client, false) || !gClientData[client].PID)
        return Plugin_Handled;

    CouplesSysDisplayProposeMenu(client);

    return Plugin_Handled;
}

void CoupleSysOnRoundEnd()
{
    float flRoundDuration = GetGameTime() - gServerData.RoundStart;
    if (flRoundDuration <= 0.0)
        return;
    int iRoundDuration = RoundToCeil(flRoundDuration);

    for (int i = 1; i <= MaxClients; ++i)
    {
        if (gClientData[i].iPartnerPlayerId < 1)
            continue;

        gClientData[i].iTogetherPlay += iRoundDuration;
    }
}

void CoupleSysFetchUser(int client)
{
    if (!IsPlayerExist(client, false) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT date, exp, together, luv, source_id, target_id FROM vcf_couples WHERE source_id = '%d' OR target_id = '%d' LIMIT 1;", gClientData[client].PID, gClientData[client].PID);

    // query
    gServerData.DBI.Query(CoupleSysFetchUserCB, szQuery, client);
}

DBCallbackGeneral(CoupleSysFetchUserCB)
{
    if(dbRs == null)
    {
        LogError("[CoupleSysFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    if (dbRs.HasResults && dbRs.FetchRow())
    {
        gClientData[client].iWeddingDate     = dbRs.FetchInt(0);
        gClientData[client].iCPExp           = dbRs.FetchInt(1);
        gClientData[client].iTogether        = dbRs.FetchInt(2);
        gClientData[client].iLuv             = dbRs.FetchInt(3);

        // check source or target
        int iCPID = dbRs.FetchInt(4);
        if (iCPID == gClientData[client].PID)
            iCPID = dbRs.FetchInt(5);

        // set the correct value
        gClientData[client].iPartnerPlayerId = iCPID;

        int m_iPartner = FindClientByPID(iCPID);
        if (m_iPartner != -1)
        {
            gClientData[client].iPartnerIndex = m_iPartner;

            if (IsPlayerExist(m_iPartner, false))
            {
                Chat(client, "你的CP当前在游戏中哦!");
                Chat(m_iPartner, "你的CP进入了服务器, 快去和他/她贴贴吧!");
            }
        }
    }
    else
    {
        gClientData[client].iPartnerIndex    = -2;
        gClientData[client].iPartnerPlayerId = 0;
        strcopy(gClientData[client].szPartnerName, 32, "单身狗");
        return;
    }
}

void CoupleSysOnClientDisconnect(int client)
{
    if (gClientData[client].iPartnerPlayerId > -1 && (gClientData[client].iTogetherPlay > 0 || gClientData[client].iCPEarnExp > 0))
    {
        char m_szQuery[256];
        FormatEx(m_szQuery, 256, "UPDATE vcf_couples SET exp=exp+%d, together=together+%d WHERE source_id = %d OR target_id = %d", gClientData[client].iCPEarnExp, gClientData[client].iTogetherPlay, gClientData[client].PID, gClientData[client].PID);
        gServerData.DBI.Query(DatabaseQueryAndIgnore, m_szQuery);
    }
    
    Couples_Data_Client_ProposeTargetUserId[client] = 0;
    Couples_Data_Client_ProposeSelectedTime[client] = 0;

    int target = gClientData[client].iPartnerIndex;

    if(target < 1)
        return;

    gClientData[target].iPartnerIndex = -1;
}

void CoupleSysDisplayMainMenu(int client)
{
    Menu menu = new Menu(MenuHandler_CouplesMainMenu);

    char date[64];
    FormatTime(date, 64, "%Y.%m.%d", gClientData[client].iWeddingDate);

    if (gClientData[client].iPartnerPlayerId)
        menu.SetTitle("[Couple] 主菜单 \n对象: %s\n日期: %s\n持久: %d天\n等级: Lv.%d(%dXP)\n共枕: %dh%dm\n好感: %d\n ", gClientData[client].szPartnerName, date, (GetTime()-gClientData[client].iWeddingDate)/86400, gClientData[client].iCPLvl, gClientData[client].iCPExp+gClientData[client].iCPEarnExp, (gClientData[client].iTogether+gClientData[client].iTogetherPlay)/3600, ((gClientData[client].iTogether+gClientData[client].iTogetherPlay)%3600)/60, gClientData[client].iLuv);
    else
        menu.SetTitle("[Couple] 主菜单 \n对象: %s\n ", gClientData[client].szPartnerName);

    AddMenuItemEx(menu, gClientData[client].iPartnerPlayerId == 0 ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED, "receive", "求婚列表");
    AddMenuItemEx(menu, gClientData[client].iPartnerPlayerId == 0 ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED, "propose", "发起求婚");
    AddMenuItemEx(menu, gClientData[client].iPartnerPlayerId != 0 ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED, "divorce", "发起离婚");
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "ranking", "登记列表");
    AddMenuItemEx(menu, ITEMDRAW_DEFAULT, "aboutcp", "功能介绍");

    menu.Display(client, MENU_TIME_FOREVER);
}

public int MenuHandler_CouplesMainMenu(Menu menu, MenuAction action, int client, int itemNum) 
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char info[32];
            menu.GetItem(itemNum, info, 32);
            if(StrEqual(info, "receive"))       CouplesSysDisplayProposeMenu(client);
            else if(StrEqual(info, "propose"))  CouplesSysDisplaySelectMenu(client);
            else if(StrEqual(info, "divorce"))  CoupleSysDisplayDivorceMenu(client);
            // else if(StrEqual(info, "ranking"))  DisplayMenu(Couples_Menu_Ranking, client, 0);
            else if(StrEqual(info, "aboutcp"))  Couples_Panel_About.Send(client, MenuHandler_CouplesAboutCPPanel, 15);
        }
        case MenuAction_End:    delete menu;
        case MenuAction_Cancel: if(itemNum == MenuCancel_ExitBack) FakeClientCommand(client, "sm_i");
    }
}

void CouplesSysDisplaySelectMenu(int client)
{
    int num = gClientData[client].iWeddingDate - GetTime();
    if (num > 0)
    {
        Chat(client, "你还处于离婚后抑郁期, 需等待 \x04%d秒\x01 才能再次组成CP", num);
        return;
    }

    num = Couples_Data_Client_ProposeSelectedTime[client] > GetTime() + 60;
    if (num > 0)
    {
        Chat(client, "请勿频繁发起请求, 请等待 \x04%d秒\x01 后再试", num);
        return;
    }

    Menu menu = new Menu(MenuHandler_CouplesSelectMenu);
    menu.SetTitle("[Couple] 选择求婚对象\n ");

    char m_szId[8];
    for (int target = 1; target <= MaxClients; ++target)
    {
        // Current in game                  self?
        if(!IsPlayerExist(target, false) || target == client)
            continue;

        // Not loaded
        if (gClientData[target].PID == -1)
            continue;

        // has couple
        if (gClientData[target].iPartnerPlayerId != 0)
            continue;

        FormatEx(m_szId, 8, "%d", GetClientUserId(target));
        AddMenuItemEx(menu, ITEMDRAW_DEFAULT, m_szId, "%N", target);
    }

    if (GetMenuItemCount(menu) < 1)
    {
        Chat(client, "大家都有老公/老婆啦, 晚点再看看有没有和你一样的单身狗吧!");
        delete menu;
        CoupleSysDisplayMainMenu(client);
        return;
    }

    SetMenuExitBackButton(menu, true);
    SetMenuExitButton(menu, true);
    DisplayMenu(menu, client, 0);
}

public int MenuHandler_CouplesSelectMenu(Menu menu, MenuAction action, int source, int itemNum) 
{
    switch(action)
    {
        case MenuAction_Select:
        {
            char info[32];
            menu.GetItem(itemNum, info, 32);
            
            int userid = StringToInt(info);
            int target = GetClientOfUserId(userid);

            if(!IsPlayerExist(target, false) || IsFakeClient(target) || gClientData[target].iPartnerPlayerId != 0)
            {
                Chat(source, "你选择的目标已经有 老公/老婆 啦!");
                CoupleSysDisplayMainMenu(source);
                return;
            }

            Couples_Data_Client_ProposeTargetUserId[source] = userid;
            Couples_Data_Client_ProposeSelectedTime[source] = GetTime();
            CouplesSysDisplayProposeMenu(target);

            Chat(source, "已将你的请求发送给 \x0E%N", target);
        }
        case MenuAction_End:    CloseHandle(menu);
        case MenuAction_Cancel: if(itemNum == MenuCancel_ExitBack) CoupleSysDisplayMainMenu(source);
    }
}

void CouplesSysDisplayProposeMenu(int target)
{
    Menu menu = CreateMenu(MenuHandler_CouplesProposeMenu);
    menu.SetTitle("[Couple] 求婚列表\n ");

    int userid = GetClientUserId(target);
    
    char m_szId[8];
    for (int source = 1; source <= MaxClients; ++source)
    {
        // Current in game                  self?
        if(!IsPlayerExist(source, false) || source == target)
            continue;

        // Not loaded
        if (gClientData[source].PID == -1)
            continue;

        // has couple
        if (gClientData[source].iPartnerPlayerId != 0)
            continue;

        // not client
        if (Couples_Data_Client_ProposeTargetUserId[source] != userid)
            continue;

        FormatEx(m_szId, 8, "%d", GetClientUserId(source));
        AddMenuItemEx(menu, ITEMDRAW_DEFAULT, m_szId, "%N", source);
    }

    if (GetMenuItemCount(menu) < 1)
    {
        Chat(target, "你想多啦单身狗! 没有人向你求婚!");
        delete menu;
        CoupleSysDisplayMainMenu(target);
        return;
    }

    SetMenuExitBackButton(menu, true);
    SetMenuExitButton(menu, true);
    DisplayMenu(menu, target, 0);
}

public int MenuHandler_CouplesProposeMenu(Menu menu, MenuAction action, int target, int itemNum) 
{
    switch(action)
    {
        case MenuAction_Select:
        {
            char info[32];
            menu.GetItem(itemNum, info, 32);
            
            int userid = StringToInt(info);
            int source = GetClientOfUserId(userid);

            if(!IsPlayerExist(source, false) || gClientData[source].iPartnerPlayerId != 0)
            {
                Chat(target, "你选择的对象暂时不可用...");
                CoupleSysDisplayMainMenu(target);
                return;
            }

            Couples_Data_Client_ProposeSelectUserId[target] = userid;
            CoupleSysDisplayConfrimMenu(target, source);
        }
        case MenuAction_End:    delete menu;
        case MenuAction_Cancel: if(itemNum == MenuCancel_ExitBack) CouplesSysDisplayProposeMenu(target);
    }
}

void CoupleSysDisplayConfrimMenu(int target, int source)
{
    Menu menu = CreateMenu(MenuHandler_CouplesConfirmMenu);
    menu.SetTitle("[Couple] 结婚登记\n ");

    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "你收到了一个来自 %N 的CP邀请", source);
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "组成CP后14天内不能申请解除");
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "组成CP后可以享受多种福利");
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "你确定要接受这个邀请吗\n ");

    AddMenuItemEx(menu, ITEMDRAW_DEFAULT, "accept", "接受");
    AddMenuItemEx(menu, ITEMDRAW_DEFAULT, "refuse", "拒绝");

    SetMenuExitButton(menu, false);
    DisplayMenu(menu, target, 0);
}

public int MenuHandler_CouplesConfirmMenu(Menu menu, MenuAction action, int target, int itemNum) 
{
    if(action == MenuAction_Select) 
    {
        char info[32];
        menu.GetItem(itemNum, info, 32);
        
        int source = GetClientOfUserId(Couples_Data_Client_ProposeSelectUserId[target]);
        
        Couples_Data_Client_ProposeSelectUserId[source] = 0;
        Couples_Data_Client_ProposeSelectUserId[target] = 0;
        Couples_Data_Client_ProposeTargetUserId[source] = 0;
        Couples_Data_Client_ProposeTargetUserId[target] = 0;

        //accept?
        if (StrEqual(info, "accept"))
        {
            if (!IsPlayerExist(source, false) || gClientData[source].iPartnerPlayerId > 0)
            {
                Chat(target, "操作无效. 你已经结婚了, 渣男! [%d -> %d]", source, gClientData[source].iPartnerPlayerId);
                CouplesSysDisplaySelectMenu(target);
                return;
            }

            Couples_GetMarried(source, target);
        }
        //refuse?
        else
        {
            if(!IsPlayerExist(source, false))
                return;

            Chat(target, "你拒绝了 \x0E%N\x01 的CP邀请", source);
            Chat(source, "\x0E%N\x01 拒绝了你的CP邀请", target);
        }
    }
    else if(action == MenuAction_End)
        delete menu;
}


void Couples_GetMarried(int source, int target)
{
    DataPack m_hPack = new DataPack();
    m_hPack.WriteCell(GetClientUserId(source));
    m_hPack.WriteCell(GetClientUserId(target));
    m_hPack.WriteCell(gClientData[source].PID);
    m_hPack.WriteCell(gClientData[target].PID);
    m_hPack.Reset();

    //SQL CALL
    char m_szQuery[128];
    Format(m_szQuery, 128, "INSERT INTO vcf_couples VALUES (default, %d, %d, %d, 0, 0, 0);", gClientData[source].PID, gClientData[target].PID, GetTime());
    gServerData.DBI.Query(Couples_SQLCallback_UpdateCP, m_szQuery, m_hPack);
}

public void Couples_SQLCallback_UpdateCP(Handle owner, Handle hndl, const char[] error, DataPack pack)
{
    int source = GetClientOfUserId(pack.ReadCell());
    int target = GetClientOfUserId(pack.ReadCell());
    int srcpid = pack.ReadCell();
    int tgrpid = pack.ReadCell();
    CloseHandle(pack);

    bool SourceValid = IsPlayerExist(source, false);
    bool TargetValid = IsPlayerExist(target, false);

    if(hndl == INVALID_HANDLE)
    {
        if(TargetValid)
            Chat(target, "数据库错误, 请截图联系管理员! >> CoupleSys.GetMarried.DBNull >> sPId: %d | tPId: %d", srcpid, tgrpid);

        if(SourceValid)
            Chat(source, "数据库错误, 请截图联系管理员! >> CoupleSys.GetMarried.DBNull >> sPId: %d | tPId: %d", srcpid, tgrpid);

        return;
    }

    if(!SQL_GetAffectedRows(hndl))
    {
        if(TargetValid)
            Chat(target, "内部错误, 请截图联系管理员! >> CoupleSys.GetMarried.NoAffRow >> sPId: %d | tPId: %d", srcpid, tgrpid);

        if(SourceValid)
            Chat(source, "内部错误, 请截图联系管理员! >> CoupleSys.GetMarried.NoAffRow >> sPId: %d | tPId: %d", srcpid, tgrpid);

        return;
    }

    if (SourceValid && TargetValid)
    {
        gClientData[source].iPartnerIndex = target;
        gClientData[source].iPartnerPlayerId = gClientData[target].PID;
        gClientData[source].iWeddingDate = GetTime();
        FormatEx(gClientData[source].szPartnerName, 32, "%N", target);

        gClientData[target].iPartnerIndex = source;
        gClientData[target].iPartnerPlayerId = gClientData[source].PID;
        gClientData[target].iWeddingDate = GetTime();
        FormatEx(gClientData[target].szPartnerName, 32, "%N", source);

        ChatAll("\x10恭喜 \x0E%N\x10 和 \x0E%N\x10 结成CP, 可喜可贺!", source, target);
        
        CoupleSysDisplayMainMenu(source);
        CoupleSysDisplayMainMenu(target);
    }
    else if (!SourceValid || !TargetValid)
    {
        if (SourceValid)
        {
            gClientData[source].iPartnerIndex = -1;
            gClientData[source].iPartnerPlayerId = tgrpid;
            gClientData[source].iWeddingDate = GetTime();
            strcopy(gClientData[source].szPartnerName, 32, "未知");

            Chat(source, "系统已保存你们的数据, 但你的CP已离线, 无法享受新婚祝福.");
            CoupleSysDisplayMainMenu(source);
        }
        else if (TargetValid)
        {
            gClientData[target].iPartnerIndex = -1;
            gClientData[target].iPartnerPlayerId = srcpid;
            gClientData[target].iWeddingDate = GetTime();
            strcopy(gClientData[target].szPartnerName, 32, "未知");

            Chat(target, "系统已保存你们的数据, 但你的CP已离线, 无法享受新婚祝福.");
            CoupleSysDisplayMainMenu(target);
        }
    }
}

void CoupleSysDisplayDivorceMenu(int client)
{
    if((GetTime() - gClientData[client].iWeddingDate) < 1209600)
    {
        Chat(client, "新组成CP后 14天 内不能申请解除...");
        CoupleSysDisplayMainMenu(client);
        return;
    }

    Menu menu = CreateMenu(MenuHandler_CouplesDivorceMenu);
    menu.SetTitle("[Couple]  离婚登记台");

    char date[64];
    FormatTime(date, 64, "%Y.%m.%d %H:%M:%S", gClientData[client].iWeddingDate);

    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "你当前配偶 %s", gClientData[client].szPartnerName);
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "你们结合于 %s", date);
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "你们已结合 %d 天", (GetTime()-gClientData[client].iWeddingDate)/86400);
    AddMenuItemEx(menu, ITEMDRAW_DISABLED, "", "离婚手续费 %d 信用点\n公式: (180-天数)*500\n ", (180-((GetTime()-gClientData[client].iWeddingDate)/86400))*500);

    AddMenuItemEx(menu, ITEMDRAW_DEFAULT, "accept", "我已经不爱TA了");
    AddMenuItemEx(menu, ITEMDRAW_DEFAULT, "refuse", "我还是很爱TA的");

    SetMenuExitBackButton(menu, false);
    SetMenuExitButton(menu, false);
    DisplayMenu(menu, client, 0);
}

public int MenuHandler_CouplesDivorceMenu(Menu menu, MenuAction action, int client, int itemNum) 
{
    switch(action)
    {
        case MenuAction_Select:
        {
            char info[128];
            menu.GetItem(itemNum, info, 128);
            
            if(StrEqual(info, "refuse", false))
            {
                CoupleSysDisplayMainMenu(client);
                return;
            }
            
            int credits = (90-((GetTime()-gClientData[client].iWeddingDate)/86400))*500*-1;

            if(credits > 0)
                credits = 0;

            if(Store_GetClientCredits(client) < credits)
            {
                Chat(client, "\x07你的积分不足以办理离婚手续...");
                return;
            }

            char m_szQuery[256];
            Format(m_szQuery, 256, "DELETE FROM vcf_couples WHERE source_id = %d or target_id = %d", gClientData[client].PID, gClientData[client].PID);
            gServerData.DBI.Query(SQLCallback_UpdateDivorce, m_szQuery, GetClientUserId(client));
        }
        case MenuAction_End: CloseHandle(menu);
    }
}

public void SQLCallback_UpdateDivorce(Handle owner, Handle hndl, const char[] error, int userid)
{
    int client = GetClientOfUserId(userid);

    if (!client)
        return;

    if (hndl == INVALID_HANDLE)
    {
        Chat(client, "数据库错误, 请截图联系管理员! >> CoupleSys.UpdateDivorce.DBNull >> PId: %d", gClientData[client].PID);
        return;
    }

    ChatAll("\x10%N\x05 解除了和 \x10%s\x05 的 CP 关系, 他们的关系维持了 \x02%d\x05 天.", client, gClientData[client].szPartnerName, (GetTime()-gClientData[client].iWeddingDate)/86400);

    int target = gClientData[client].iPartnerIndex;
    if (target > 0)
    {
        CoupleSysOnClientReady(target);
        gClientData[target].iWeddingDate = GetTime()+1209600;
    }

    CoupleSysOnClientReady(client);
    gClientData[client].iWeddingDate = GetTime()+1209600;
    CoupleSysDisplayMainMenu(client);
}

void CoupleSysCreateAboutMenu()
{
    Couples_Panel_About = new Panel();

    Couples_Panel_About.DrawText("[Couple] 系统说明");
    Couples_Panel_About.DrawText(" ");
    Couples_Panel_About.DrawText("组成CP需要两厢情愿, 无法单方面组CP");
    Couples_Panel_About.DrawText("发起请求之后需要对方确认才能成功");
    Couples_Panel_About.DrawText("组成CP后 14 天内不能解除");
    Couples_Panel_About.DrawText("解除CP后 14 天内不能再组");
    Couples_Panel_About.DrawText("CP能为你提供一定的加成");
    // Couples_Panel_About.DrawText("按Y输入[%内容]可以发送CP频道");
    // Couples_Panel_About.DrawText("还有更多的功能正在开发中...");
    Couples_Panel_About.DrawText("希望你和你的另一半玩得开心");
    Couples_Panel_About.DrawText(" ");

    Couples_Panel_About.DrawItem("返回");
}

public int MenuHandler_CouplesAboutCPPanel(Menu menu, MenuAction action, int client, int itemNum)
{
    if (action == MenuAction_Select)
        CoupleSysDisplayMainMenu(client);
}

void CoupleSysOnNativeInit(/*void*/)
{
    CreateNative("VCF_GetClientCouplePID", API_GetClientCouplePID);
    CreateNative("VCF_GetClientCoupleInGameIndex", API_GetClientCoupleInGameIndex);

    CreateNative("VCF_ModifyClientCoupleExpEarn", API_ModifyClientCoupleExpEarn);
}

public int API_GetClientCouplePID(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);
    return gClientData[client].iPartnerPlayerId;
}

public int API_GetClientCoupleInGameIndex(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);
    return gClientData[client].iPartnerIndex;
}

public int API_ModifyClientCoupleExpEarn(Handle hPlugin, int iNumParams)
{
    int client = GetNativeCell(1);
    int delta = GetNativeCell(2);
    gClientData[client].iCPEarnExp += delta;

    return 0;
}