// This is not a dynamic array because it would be bad for 24/7 map servers.
#define OLD_CONNECTION_QUEUE 10

// Old connection queue
char g_Names[OLD_CONNECTION_QUEUE][CHAT_LINE_LENGTH];
char g_SteamIDs[OLD_CONNECTION_QUEUE][SMALL_LINE_LENGTH];
char g_IPs[OLD_CONNECTION_QUEUE][SMALL_LINE_LENGTH];
int g_Tracker;
int g_Size;

void LastOnPluginStart()
{
    RegConsoleCmd("sm_last", Command_Last, "Gets the last players what have disconnected.");
}

void LastOnClientDisconnect(int client)
{
    if (!IsFakeClient(client)) { InsertInfo(client); }
}

public Action Command_Last(int client, int params)
{
    if (!AdminHasClientFlag(client, ADMINFLAG_BAN))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    static char sName[CHAT_LINE_LENGTH];
    static char sAuthID[SMALL_LINE_LENGTH];
    static char sIP[SMALL_LINE_LENGTH];

    PrintToConsole(client, "");
    PrintToConsole(client, "%19s %20s %15s", "name", "steam32", "ip");
    PrintToConsole(client, "======================================================");

    for (int i = 0; i < g_Size; i++)
    {
        GetInfo(i, sName, sizeof(sName), sAuthID, sizeof(sAuthID), sIP, sizeof(sIP));

        PrintToConsole(client, "%19s %20s %15s", sName, sAuthID, sIP);
    }
    PrintToConsole(client, "======================================================");
    PrintToConsole(client, "%d old connections saved.", g_Size);
    PrintToConsole(client, "");

    return Plugin_Handled;
}

void InsertInfo(int client)
{
    // Scan to see if this entry is the last entry in the list
    // If it is, then update the name and access
    // If it is not, then insert it again.

    if (g_Size > 0)
    {
        static char sIP[SMALL_LINE_LENGTH], sAuth[SMALL_LINE_LENGTH];

        GetClientAuthId(client, AuthId_Steam2, sAuth, sizeof(sAuth));
        GetClientIP(client, sIP, sizeof(sAuth));

        static int last = 0;

        if (g_Size < sizeof(g_SteamIDs)) { last = g_Size - 1; }
        else
        {
            last = g_Tracker - 1;

            if (last < 0) { last = g_Size - 1; }
        }
        
        // need to check ip too, or all the nosteams will while it doesn't work with their illegitimate server
        if (!strcmp(sAuth, g_SteamIDs[last], false) && !strcmp(sIP, g_IPs[last], false))
        {
            GetClientName(client, g_Names[last], sizeof(g_Names[]));

            return;
        }
    }
    
    // Need to insert the entry
    int target = 0;

    // Queue is not yet full
    if (g_Size < sizeof(g_SteamIDs))
    {
        target = g_Size;

        g_Size++;
    }
    else
    {
        target = g_Tracker;

        g_Tracker++;

        // If we reached the end of the array, then move to the front
        if (g_Tracker == sizeof(g_SteamIDs)) { g_Tracker = 0; }
    }

    GetClientAuthId(client, AuthId_Steam2, g_SteamIDs[target], sizeof(g_SteamIDs[]));
    GetClientName(client, g_Names[target], sizeof(g_Names[]));
    GetClientIP(client, g_IPs[target], sizeof(g_IPs[]));
}

void GetInfo(int i, char[] sName, int sizeName, char[] sAuthID, int sizeAuth, char[] sIP, int sizeIP)
{
    if (i >= g_Size) { LogError("GetInfo: OOB: (%d:%d)", i, g_Size); }

    int target = (g_Tracker + i) % sizeof(g_SteamIDs);

    strcopy(sName,      sizeName, g_Names[target]);
    strcopy(sAuthID,    sizeAuth, g_SteamIDs[target]);
    strcopy(sIP,        sizeIP,   g_IPs[target]);
}