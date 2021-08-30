void VIPOnClientReady(int client)
{
    if (!IsPlayerExist(client) || IsFakeClient(client))
        return;

    VIPFetchUser(client);
}

void VIPFetchUser(int client)
{
    if (!IsPlayerExist(client) || IsFakeClient(client))
        return;

    // format query
    char szQuery[256];
    FormatEx(szQuery, sizeof(szQuery), "SELECT `viplevel` FROM vcf_vip WHERE pid = '%d' AND modid IN ('%d', '0');", gClientData[client].PID, gServerData.ModID);

    // query
    gServerData.DBI.Query(VIPFetchUserCB, szQuery, client);
}

DBCallbackGeneral(VIPFetchUserCB)
{
    if(dbRs == null)
    {
        LogError("[VIPFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }

    if (dbRs.HasResults && dbRs.FetchRow())
    {
        gClientData[client].VIPLevel = dbRs.FetchInt(0);
    }
}