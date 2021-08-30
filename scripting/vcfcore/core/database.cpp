#define DATABASECONFIGKV "vcf_core"

#define DBCallbackGeneral(%0) public void %0(Database db, DBResultSet dbRs, const char[] szError, any client)
#define DBCallbackDataPack(%0) public void %0(Database db, DBResultSet dbRs, const char[] szError, DataPack pack)

void DatabaseOnPluginStart()
{
    DatabaseMakeConnection();
}

void DatabaseMakeConnection()
{
    if (gServerData.DBI != null)
        delete gServerData.DBI;
    
    char szError[512];
    gServerData.DBI = SQL_Connect(DATABASECONFIGKV, true, szError, sizeof(szError));
    if (gServerData.DBI == null)
    {
        SetFailState("Failed to connect DB -> %s", szError);
    }

    gServerData.DBI.SetCharset("utf8");

    // Load ModID
    DatabaseGetServerInfo();
    
    // Late load?
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsPlayerExist(i, false))
            OnClientPostAdminCheck(i);
    }
}

void DatabaseGetServerInfo()
{
    int iHostIP = GetConVarInt(FindConVar("hostip"));
    static char sHostIP[64];
    Format(sHostIP, sizeof(sHostIP), "%d.%d.%d.%d:%d", (iHostIP >> 24) & 0x000000FF, (iHostIP >> 16) & 0x000000FF, (iHostIP >> 8) & 0x000000FF, iHostIP & 0x000000FF, GetConVarInt(FindConVar("hostport")));

    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "SELECT id,modid,public FROM `vcf_servers` WHERE `address` = '%s'", sHostIP);
    gServerData.DBI.Query(DatabaseGetModIDCB, szQuery);
}

DBCallbackGeneral(DatabaseGetModIDCB)
{
    if(dbRs == null)
    {
        LogError("[DatabaseGetModIDCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    if (dbRs.FetchRow())
    {
        gServerData.ServerID    = dbRs.FetchInt(0);
        gServerData.ModID       = dbRs.FetchInt(1);

        static char sPublicIP[NORMAL_LINE_LENGTH];
        dbRs.FetchString(2, string(sPublicIP));
        strcopy(string(gServerData.PublicIP), sPublicIP);
    }
    else SetFailState("Failed to get server info from database.");

    return;
}

DBCallbackGeneral(DatabaseQueryAndIgnore)
{
    if(dbRs == null)
    {
        LogError("[DatabaseQueryAndIgnore] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }
}

void DatabaseTryReconnect()
{
    delete gServerData.DBI;
    gServerData.DBI = null;

    char error[255];
    gServerData.DBI = SQL_Connect(DATABASECONFIGKV, true, error, sizeof(error));
    if (gServerData.DBI == null)
    {
        SetFailState("Failed to connect DB -> %s", error);
        return;
    }

    gServerData.DBI.SetCharset("utf8");
}

void DatabaseInsertAdminLog(int admin, const char[] sOperate, int target = -1, const char[] sNote = "")
{
    static char szTarget64[33], szTargetName[33], szAdmin64[33], szAdminNick[33];

    // Validate admin
    if (admin == 0)
    {
        FormatEx(szAdmin64, sizeof(szAdmin64), "Console");
        FormatEx(szAdminNick, sizeof(szAdminNick), "服务器");
    }
    else
    {
        FormatEx(szAdmin64, sizeof(szAdmin64), "%s", gClientData[admin].Steam64);
        FormatEx(szAdminNick, sizeof(szAdminNick), "%s", gClientData[admin].Nick);
    }

    // Validate target
    if (target == -1)
    {
        FormatEx(szTarget64, sizeof(szTarget64), "Server");
        FormatEx(szTargetName, sizeof(szTargetName), "服务器");
    }
    else
    {
        FormatEx(szTarget64, sizeof(szTarget64), "%s", gClientData[target].Steam64);
        FormatEx(szTargetName, sizeof(szTargetName), "%N", target);
    }

    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "INSERT INTO vcf_logs (admin,adminnick,serverid,operate,target,targetname,operationtime,note) VALUES ('%s','%s','%d','%s','%s','%s','%d','%s')", szAdmin64, szAdminNick, gServerData.ServerID, sOperate, szTarget64, szTargetName, GetTime(), sNote);
    gServerData.DBI.Query(DatabaseQueryAndIgnore, szQuery);
}