enum /* ClientItemData */
{
    ITEM_DATA_CODE,
    ITEM_DATA_NAME,
    ITEM_DATA_DESC,
    ITEM_DATA_AMOUNT
}

void InventoryOnPluginStart()
{
    RegConsoleCmd("sm_bag", InventoryOnOnCommandCatched);
    RegConsoleCmd("sm_backpack", InventoryOnOnCommandCatched);
    RegConsoleCmd("sm_bb", InventoryOnOnCommandCatched);

    // arraylist initialize
    for (int i=1; i<MAXPLAYERS+1; ++i)
    {
        gClientData[i].Items = new ArrayList(256);
    }
}

void InventoryOnClientReady(int client)
{
    if (!IsPlayerExist(client) || IsFakeClient(client))
        return;

    InventoryFetchUser(client);
}

void InventoryFetchUser(int client)
{
    if (!IsPlayerExist(client) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT vcf_itemown.`code`,`name`,`desc`,`amount` FROM vcf_itemown, vcf_iteminfo WHERE pid = '%d' AND vcf_itemown.`code` = vcf_iteminfo.`code` AND amount > 0 ORDER BY CONVERT(`name` USING GBK) ASC;", gClientData[client].PID);

    // query
    gServerData.DBI.Query(InventoryFetchUserCB, szQuery, client);
}

// Show bag here.
DBCallbackGeneral(InventoryFetchUserCB)
{
    if(dbRs == null)
    {
        LogError("[InventoryFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    if (!dbRs.HasResults)
    {
        return;
    }

    static char szItemCode[32], szItemName[32], szItemDesc[64];
    static int iAmount;

    if (gClientData[client].Items == null)
    {
        gClientData[client].Items = new ArrayList(128);
    }

    gClientData[client].Items.Clear();

    // Loop all items
    while(dbRs.FetchRow())
    {
        // Fetch amount first
        iAmount = dbRs.FetchInt(3);

        // No item in inventory?
        if (iAmount <= 0)
        {
            // Call delete func
            char szQuery[256];
            Format(szQuery, sizeof(szQuery), "DELETE FROM vcf_itemown WHERE code = '%s' AND pid = '%d';");

            // query
            gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery, client);

            // Next row
            continue;
        }

        dbRs.FetchString(0, szItemCode, 31);
        dbRs.FetchString(1, szItemName, 31);
        dbRs.FetchString(2, szItemDesc, 63);

        ArrayList arrayItemOwn = new ArrayList(5);
        arrayItemOwn.PushString(szItemCode);
        arrayItemOwn.PushString(szItemName);
        arrayItemOwn.PushString(szItemDesc);
        arrayItemOwn.Push(iAmount);
        gClientData[client].Items.Push(arrayItemOwn);
    }
}

public Action InventoryOnOnCommandCatched(int client, int args)
{
    if (IsClientInCooldown(client))
        return Plugin_Handled;

    InventoryListInventory(client);
    return Plugin_Handled;
}

void InventoryListInventory(int client)
{
    // format query
    static char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT vcf_itemown.`code`,`name`,`desc`,`amount` FROM vcf_itemown, vcf_iteminfo WHERE pid = '%d' AND vcf_itemown.`code` = vcf_iteminfo.`code` AND amount > 0 ORDER BY CONVERT(`name` USING GBK) ASC;", gClientData[client].PID);

    // query
    gServerData.DBI.Query(InventoryListInventoryCB, szQuery, client);
}

DBCallbackGeneral(InventoryListInventoryCB)
{
    if(dbRs == null)
    {
        LogError("[InventoryListInventoryCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    // Menu Initalize
    SetGlobalTransTarget(client);

    static char sOption[128];
    Menu menu = new Menu(InventoryMenuHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);
    menu.SetTitle("%t\n ", "bag menu title");

    static char szItemCode[32], szItemName[32], szItemDesc[64];
    static int iAmount;

    if (gClientData[client].Items == null)
    {
        gClientData[client].Items = new ArrayList(128);
    }

    gClientData[client].Items.Clear();

    // Loop all items
    while(dbRs.FetchRow())
    {
        // Fetch amount first
        iAmount = dbRs.FetchInt(3);

        // No item in inventory?
        if (iAmount <= 0)
        {
            // Call delete func (FEATURE)

            // Next row
            continue;
        }

        dbRs.FetchString(0, szItemCode, 31);
        dbRs.FetchString(1, szItemName, 31);
        dbRs.FetchString(2, szItemDesc, 63);

        ArrayList arrayItemOwn = new ArrayList(5);
        arrayItemOwn.PushString(szItemCode);
        arrayItemOwn.PushString(szItemName);
        arrayItemOwn.PushString(szItemDesc);
        arrayItemOwn.Push(iAmount);
        gClientData[client].Items.Push(arrayItemOwn);

        // Call forward
        Action hResult = Plugin_Handled;
        gForwardData._OnBagItemShowPre(client, szItemCode, hResult);

        Format(sOption, 127, "[%dx] %t", iAmount, szItemName);
        menu.AddItem(szItemCode, sOption, (hResult == Plugin_Handled)?ITEMDRAW_DISABLED:ITEMDRAW_DEFAULT);
    }

    if (dbRs.RowCount <= 0)
    {
        Format(sOption, 127, "%t", "bag item empty");
        menu.AddItem("", sOption, ITEMDRAW_DISABLED);
    }

    gClientData[client].Cooldown = GetTime() + 5;

    SetGlobalTransTarget(LANG_SERVER);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int InventoryMenuHandler(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        if (!IsPlayerExist(param1))
        {
            return 0;
        }

        char info[24];
        menu.GetItem(param2, info, sizeof(info));
        int itemid = InventoryGetIDByCodeClient(param1, info);

        InventoryConfirmMenu(param1, itemid);
    }
    else if (action == MenuAction_End) delete menu;

    return 0;
}

void InventoryConfirmMenu(int client, int itemid)
{
    // Menu Initalize
    SetGlobalTransTarget(client);

    static char sOption[256], sTemp[64]; sTemp = "";
    static char szItemCode[32], szItemName[32], szItemDesc[64];
    static int iAmount;
    InventoryGetCodeClient(client, itemid, szItemCode, sizeof(szItemCode));
    InventoryGetNameClient(client, itemid, szItemName, sizeof(szItemName));
    InventoryGetDescClient(client, itemid, szItemDesc, sizeof(szItemDesc));
    iAmount = InventoryGetAmountClient(client, itemid);

    Menu menu = new Menu(InventoryConfirmMenuHandler, MENU_ACTIONS_DEFAULT | MenuAction_DisplayItem);
    if (hasLength(szItemDesc))
    {
        Format(sTemp, 63, "   %t", szItemDesc);
    }

    Format(sOption, 255, " \n%t\n%s\n   %t\n \n ", szItemName, sTemp, "bag item owning", iAmount);
    menu.SetTitle(sOption);

    Format(sOption, 255, "%t", "bag action use");
    menu.AddItem(szItemCode, sOption);

    SetGlobalTransTarget(LANG_SERVER);

    menu.ExitButton = true;
    menu.Display(client, MENU_TIME_FOREVER);
}

public int InventoryConfirmMenuHandler(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        if (!IsPlayerExist(param1))
        {
            return 0;
        }

        char info[24];
        menu.GetItem(param2, info, sizeof(info));

        Action hResult = Plugin_Continue;
        gForwardData._OnBagItemUsePost(param1, info, hResult);
        if (hResult != Plugin_Handled)
            InventoryUpdateItem(param1, info, "-", 1);
    }
    else if (action == MenuAction_End) delete menu;

    return 0;
}

void InventoryUpdateItem(int client, const char[] sItemCode, const char[] sSymbol, int iAmount)
{
    // Prepare Query
    static char sQuery[256];
    Format(sQuery, 255, "SELECT * FROM vcf_itemown WHERE code = '%s' AND pid = '%d';", sItemCode, gClientData[client].PID);

    // FUCK, I NEED DATAPACK TO PASS THE PARAMS
    DataPack pack = new DataPack();
    pack.WriteCell(client);
    pack.WriteString(sItemCode);
    pack.WriteString(sSymbol);
    pack.WriteCell(iAmount);

    // Pack ready, time to do query :)
    gServerData.DBI.Query(InventoryUpdateData, sQuery, pack);
}

DBCallbackDataPack(InventoryUpdateData)
{
    if(dbRs == null)
    {
        LogError("[InventoryUpdateData] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        delete pack;
        return;
    }

    // Read data
    pack.Reset();
    int client = pack.ReadCell();
    char sItemCode[32]; pack.ReadString(sItemCode, 31);
    char sSymbol[4]; pack.ReadString(sSymbol, 4);
    int iAmount = pack.ReadCell();
    delete pack;

    static char sQuery[256];

    if(dbRs.RowCount <= 0)
    {
        // Why there's no data?
        // OK, maybe it's the first time player get this item.
        // Check symbol first.
        if (!strcmp("-", sSymbol, false))
        {
            // What you want to deduct an item player dont have???
            return;
        }

        // Go with '+' +_+
        // Just insert it to the database.
        Format(sQuery, 255, "INSERT INTO vcf_itemown (`pid`,`code`,`amount`) VALUES ('%d','%s','%d');", gClientData[client].PID, sItemCode, iAmount);
    }
    else
    {
        // Do sth with symbols
        Format(sQuery, 255, "UPDATE vcf_itemown SET amount = amount %s %d WHERE code = '%s' AND pid = '%d';", sSymbol, iAmount, sItemCode, gClientData[client].PID);
    }

    // There must be a query here... So no need to check here with hasLength() function.
    gServerData.DBI.Query(DatabaseQueryAndIgnore, sQuery, client);
}

void InventoryOnNativeInit(/*void*/)
{
    CreateNative("VCF_GetBagItem",          API_GetItem);
    CreateNative("VCF_SetBagItem",          API_SetItem);
    CreateNative("VCF_ForceClientUseItem",  API_ForceClientUseItem);
}

// native int BP_GetItem(int client, const char[] sItemCode)
public int API_GetItem(Handle plugin, int numParams)
{
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client) || IsFakeClient(client))
    {
        ThrowNativeError(SP_ERROR_NATIVE, "Client %d is invalid.", client);
        return -1;
    }

    char sItemName[32]; GetNativeString(2, sItemName, 31);

    return InventoryGetAmountByCodeClient(client, sItemName);
}

// native int BP_SetItem(int client, const char[] sItemCode, const char[] sSymbol, int iAmount)
public int API_SetItem(Handle plugin, int numParams)
{
    // Get client
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client) || IsFakeClient(client))
    {
        ThrowNativeError(SP_ERROR_NATIVE, "Client %d is invalid.", client);
        return -1;
    }

    char sItemCode[32]; GetNativeString(2, sItemCode, 31);
    char sSymbol[4]; GetNativeString(3, sSymbol, 3);
    int iAmount = GetNativeCell(4);

    InventoryUpdateItem(client, sItemCode, sSymbol, iAmount);

    return 0;
}

// native int VCF_ForceClientUseItem(int client, const char[] sItemCode);
public int API_ForceClientUseItem(Handle plugin, int numParams)
{
    // Get client
    int client = GetNativeCell(1);

    if (!IsPlayerExist(client, false) || IsFakeClient(client))
    {
        ThrowNativeError(SP_ERROR_NATIVE, "Client %d is invalid.", client);
        return -1;
    }

    char sItemCode[32]; GetNativeString(2, sItemCode, 31);

    Action hResult = Plugin_Continue;
    gForwardData._OnBagItemUsePost(client, sItemCode, hResult);
    if (hResult != Plugin_Handled)
        InventoryUpdateItem(client, sItemCode, "-", 1);

    return ((hResult == Plugin_Handled) ? false : true);
}

// =====================
// ==   General API   ==
// =====================
void InventoryGetCodeClient(int client, int iD, char[] sString, int iMaxLen)
{
    ArrayList arrayItemOwn = gClientData[client].Items.Get(iD);
    arrayItemOwn.GetString(ITEM_DATA_CODE, sString, iMaxLen);
}

void InventoryGetNameClient(int client, int iD, char[] sString, int iMaxLen)
{
    ArrayList arrayItemOwn = gClientData[client].Items.Get(iD);
    arrayItemOwn.GetString(ITEM_DATA_NAME, sString, iMaxLen);
}

void InventoryGetDescClient(int client, int iD, char[] sString, int iMaxLen)
{
    ArrayList arrayItemOwn = gClientData[client].Items.Get(iD);
    arrayItemOwn.GetString(ITEM_DATA_DESC, sString, iMaxLen);
}

int InventoryGetAmountClient(int client, int iD)
{
    ArrayList arrayItemOwn = gClientData[client].Items.Get(iD);
    return arrayItemOwn.Get(ITEM_DATA_AMOUNT);
}

int InventoryGetIDByCodeClient(int client, const char[] sCode)
{
    static char sItemCode[32];

    int iSize = gClientData[client].Items.Length;
    for (int i = 0; i < iSize; ++i)
    {
        InventoryGetCodeClient(client, i, sItemCode, sizeof(sItemCode));

        // If code match
        if (!strcmp(sCode, sItemCode, false))
        {
            return i;
        }
    }

    return -1;
}

int InventoryGetAmountByCodeClient(int client, const char[] sCode)
{
    static char sItemCode[32];

    int iSize = gClientData[client].Items.Length;
    for (int i = 0; i < iSize; ++i)
    {
        InventoryGetCodeClient(client, i, sItemCode, sizeof(sItemCode));

        // If code match
        if (!strcmp(sCode, sItemCode, false))
        {
            return InventoryGetAmountClient(client, i);
        }
    }

    return -1;
}