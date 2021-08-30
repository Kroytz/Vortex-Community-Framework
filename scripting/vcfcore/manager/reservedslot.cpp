void ReservedSlotOnPluginStart()
{
    // Convar initialize
    gCvarList.RESERVEDSLOT_ENABLE      = CreateConVar("vcf_slots_enable","1", "Enable the reserved slots feature? 0 = Disable, 1 = Enable", 0, true, 0.0, true, 1.0);
    gCvarList.RESERVEDSLOT_VISIBLESLOT = FindConVar("sv_visiblemaxplayers");

    gCvarList.RESERVEDSLOT_VIPSLOTS    = CreateConVar("vcf_vip_slots","0","Number of Vip player slots, if VIP slots feature disabled ,whatever you input will be 0", 0, true, 0.0);
    gCvarList.RESERVEDSLOT_ADMINSLOTS  = CreateConVar("vcf_admin_slots", "0", "Number of reserved player slots for admins.", 0, true, 0.0);

    // Convar change hook
    gCvarList.RESERVEDSLOT_ENABLE.AddChangeHook(ReservedSlotOnCvarChange);
    gCvarList.RESERVEDSLOT_VIPSLOTS.AddChangeHook(ReservedSlotOnCvarChange);
    gCvarList.RESERVEDSLOT_ADMINSLOTS.AddChangeHook(ReservedSlotOnCvarChange);
}

public void ReservedSlotOnCvarChange(ConVar convar, const char[] oldValue, const char[] newValue)
{
    /* Reserved slots or hidden slots have been disabled - reset sv_visiblemaxplayers */
    int slotcount = gCvarList.RESERVEDSLOT_VIPSLOTS.IntValue + gCvarList.RESERVEDSLOT_ADMINSLOTS.IntValue;
    if (gCvarList.RESERVEDSLOT_ENABLE.IntValue == 0)
        slotcount = 0;

    if (slotcount == 0)
    {
        ReservedSlotResetVisibleMax();
    }
    else
    {
        ReservedSlotRecheckVisibleMaxSlots();
    }
}

void ReservedSlotResetVisibleMax()
{
    gCvarList.RESERVEDSLOT_VISIBLESLOT.IntValue = -1;
}

void ReservedSlotRecheckVisibleMaxSlots()
{
    int clients = GetClientCount(false);
    int vipreserved = gCvarList.RESERVEDSLOT_VIPSLOTS.IntValue;
    int adminreserved = gCvarList.RESERVEDSLOT_ADMINSLOTS.IntValue;
    int limited = GetMaxHumanPlayers() - adminreserved - vipreserved;
    int num = limited;
    
    if (clients == GetMaxHumanPlayers()) // 满人
    {
        num = GetMaxHumanPlayers();
    }
    else if (limited <= (GetMaxHumanPlayers() - vipreserved - adminreserved)) // 没满, 也没超过VIP预留数量
    {
        num = limited + vipreserved;
    }
    else if (limited <= (GetMaxHumanPlayers() - adminreserved)) // 没满, 但是超过VIP预留数量
    {
        num = GetMaxHumanPlayers();
    }
    else
    {
        num = limited;
    }
    
    gCvarList.RESERVEDSLOT_VISIBLESLOT.IntValue = num;
}

void ReservedSlotOnClientPostAdminCheck(int client)
{
    int onlineusers = GetClientCount(false);
    int vipreserved = gCvarList.RESERVEDSLOT_VIPSLOTS.IntValue;
    int adminreserved = gCvarList.RESERVEDSLOT_ADMINSLOTS.IntValue;
    int limit = GetMaxHumanPlayers() - vipreserved - adminreserved;
    int viplimit = GetMaxHumanPlayers() - adminreserved;

    // 人数已经够多了, 那么开始检查预留通道
    if (onlineusers > limit)
    {
        // 有预留通道权限, 开始检查极端情况.
        if (ReservedSlotIsClientHasReservation(client))
        {
            // 已经满人到狗管理通道了?
            if (onlineusers > viplimit)
            {
                int target = ReservedSlotSelectKickClient();
                if (target == -1)
                {
                    CreateTimer(0.1, ReservedSlotOnTimedKick, client);
                }
                else
                {
                    CreateTimer(0.1, ReservedSlotOnTimedKick, target);
                }
            }
        }
        else // 没有通道权限
        {
            // 人数已经到了通道?
            if (onlineusers > limit)
            {
                CreateTimer(0.1, ReservedSlotOnTimedKick, client);
            }
        }
    }
}

public Action ReservedSlotOnTimedKick(Handle timer, any client)
{    
    if (!client || !IsClientInGame(client))
    {
        return Plugin_Handled;
    }

    if (gCvarList.RESERVEDSLOT_ENABLE.BoolValue)
    {
        if (gClientData[client].VIPLevel == 0)
        {
            KickClient(client, "服务器已满, 预留通道仅限VIP及管理员使用.\nServer is full, the reserved slots are for VIPs and admins only");
        }
        else
        {
            KickClient(client, "服务器已满, 最终预留通道仅限管理员使用, 请稍后再试.\nServer is full, the reserved slots are admins only");
        }
    }
    else
        KickClient(client, "服务器已满.\nServer is full");

    ReservedSlotRecheckVisibleMaxSlots();

    return Plugin_Handled;
}

int ReservedSlotSelectKickClient()
{
    float highestSpecValue;
    int highestSpecValueId;
    
    bool specFound;
    
    float value;
    
    for (int i=1; i<=MaxClients; i++)
    {    
        if (!IsPlayerExist(i, false))
        {
            continue;
        }
        
        value = 0.0;
            
        if (IsClientInGame(i))
        {
            if (IsClientObserver(i))
            {
                specFound = true;
                
                if (value > highestSpecValue)
                {
                    highestSpecValue = value;
                    highestSpecValueId = i;
                }
            }
        }
    }
    
    if (specFound)
    {
        return highestSpecValueId;
    }
    
    return -1;
}

bool ReservedSlotIsClientHasReservation(int client)
{
    if (gClientData[client].Access > 0 || gClientData[client].VIPLevel > 0)
        return true;

    return false;
}