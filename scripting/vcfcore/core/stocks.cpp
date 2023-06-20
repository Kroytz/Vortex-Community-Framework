bool AddMenuItemEx(Menu menu, int style, const char[] info, const char[] display, any ...)
{
    char m_szBuffer[256];
    VFormat(m_szBuffer, 256, display, 5);
    return menu.AddItem(info, m_szBuffer, style);
}

// Chat Helper
static char ChatPrefix[32] = "[\x04SM\x01]";
static char ChatSpaces[32] = " ";
static bool ChatConSnd     = true;
static bool SkipNextChatCS = false;
static bool SkipNextPrefix = false;
static bool ReplaceColorEx = false;
static UserMessageType UMType = UM_Protobuf;
static EngineVersion GameEngine = Engine_CSGO;

stock void InitUserMessage()
{
    UMType = GetUserMessageType();
    GameEngine = GetEngineVersion();

    if(GameEngine == Engine_Insurgency || GameEngine == Engine_Left4Dead2)
    {
        ReplaceColorEx = true;
    }
    else if(GameEngine == Engine_CSGO)
    {
        ReplaceColorEx = false;
    }
    else
    {
        SetFailState("Current Engine not suppoirted");
    }
}

stock int SetChatPrefix(const char[] prefix)
{
    return strcopy(ChatPrefix, 32, prefix);
}

stock int SetChatSpaces(const char[] spaces)
{
    return strcopy(ChatSpaces, 32, spaces);
}

// Set Chat string log in console and chat sound?
stock void SetChatConSnd(bool consnd)
{
    ChatConSnd = consnd;
}

// set next chat stop sound and log
stock void SetSkipNextChatCS()
{
    SkipNextChatCS = true;
}

// set next chat ignore prefix
stock void SetSkipNextPrefix()
{
    SkipNextPrefix = true;
}

stock void Chat(int client, const char[] buffer, any ...)
{
    char msg[256];
    VFormat(msg, 256, buffer, 3);
    Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
    ProcessColorString(msg, 256);
    UTIL_SayText2(client, msg);
    SkipNextChatCS = false;
    SkipNextPrefix = false;
}

stock void ChatEx(int[] clients, int numClients, const char[] buffer, any ...)
{
    char msg[256];
    VFormat(msg, 256, buffer, 4);
    Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
    ProcessColorString(msg, 256);
    
    if(UMType == UM_Protobuf)
    {
        Protobuf SayText2 = view_as<Protobuf>(StartMessage("SayText2", clients, numClients, USERMSG_RELIABLE|USERMSG_BLOCKHOOKS));
        SayText2.SetInt("ent_idx", 0);
        SayText2.SetBool("chat", SkipNextChatCS ? false : ChatConSnd);
        SayText2.SetString("msg_name", msg);
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        EndMessage();
    }
    else
    {
        BfWrite SayText = view_as<BfWrite>(StartMessage("SayText", clients, numClients, USERMSG_RELIABLE|USERMSG_BLOCKHOOKS));
        SayText.WriteByte(0);
        SayText.WriteByte(true);
        SayText.WriteString(msg);
        EndMessage();
    }

    SkipNextChatCS = false;
    SkipNextPrefix = false;
}

stock void ChatTeam(int team, bool spec = true, const char[] buffer, any ...)
{
    int[] clients = new int [MAXPLAYERS+1];
    int total = 0;
    int iTeam = 0;
    
    for(int client = 1; client <= MaxClients; client++)
        if(ClientIsValid(client))
        {
            iTeam = GetClientTeam(client);
            if(iTeam == team || (spec && iTeam == 1))
                clients[total++] = client;
        }

    char message[256];
    VFormat(message, 256, buffer, 4);
    ChatEx(clients, total, message);
}

stock void ChatAlive(bool alive, bool spec = true, const char[] buffer, any ...)
{
    int[] clients = new int [MAXPLAYERS+1];
    int total = 0;

    for(int client = 1; client <= MaxClients; client++)
        if(ClientIsValid(client))
        {
            if((alive == IsPlayerAlive(client)) || (spec && GetClientTeam(client) == 1))
                clients[total++] = client;
        }
        
    char message[256];
    VFormat(message, 256, buffer, 4);
    ChatEx(clients, total, message);
}

stock void ChatAll(const char[] buffer, any ...)
{
    char msg[256];
    VFormat(msg, 256, buffer, 2);
    Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
    ProcessColorString(msg, 256);

    if(UMType == UM_Protobuf)
    {
        Protobuf SayText2 = view_as<Protobuf>(StartMessageAll("SayText2", USERMSG_RELIABLE|USERMSG_BLOCKHOOKS));
        SayText2.SetInt("ent_idx", 0);
        SayText2.SetBool("chat", SkipNextChatCS ? false : ChatConSnd);
        SayText2.SetString("msg_name", msg);
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        EndMessage();
    }
    else
    {
        BfWrite SayText = view_as<BfWrite>(StartMessageAll("SayText", USERMSG_RELIABLE|USERMSG_BLOCKHOOKS));
        SayText.WriteByte(0);
        SayText.WriteByte(true);
        SayText.WriteString(msg);
        EndMessage();
    }

    SkipNextChatCS = false;
    SkipNextPrefix = false;
}

stock void tChat(int client, const char[] buffer, any ...)
{
    char msg[256];
    SetGlobalTransTarget(client);
    VFormat(msg, 256, buffer, 3);
    Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
    ProcessColorString(msg, 256);
    UTIL_SayText2(client, msg);
    SkipNextChatCS = false;
    SkipNextPrefix = false;
    SetGlobalTransTarget(LANG_SERVER);
}

stock void tChatAlive(bool alive, bool spec = true, const char[] buffer, any ...)
{
    char msg[256];
    for(int client = 1; client <= MaxClients; client++)
        if(ClientIsValid(client))
        {
            if((alive == IsPlayerAlive(client)) || (spec && GetClientTeam(client) == 1))
            {
                SetGlobalTransTarget(client);
                VFormat(msg, 256, buffer, 4);
                Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
                ProcessColorString(msg, 256);
                UTIL_SayText2(client, msg);
            }
        }
    
    SkipNextChatCS = false;
    SkipNextPrefix = false;
    SetGlobalTransTarget(LANG_SERVER);
}

stock void tChatTeam(int team, bool spec = true, const char[] buffer, any ...)
{
    int iTeam = 0;
    char msg[256];

    for(int client = 1; client <= MaxClients; client++)
        if(ClientIsValid(client))
        {
            iTeam = GetClientTeam(client);
            if(iTeam == team || (spec && iTeam == 1))
            {
                SetGlobalTransTarget(client);
                VFormat(msg, 256, buffer, 4);
                Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
                ProcessColorString(msg, 256);
                UTIL_SayText2(client, msg);
            }
        }

    SkipNextChatCS = false;
    SkipNextPrefix = false;
    SetGlobalTransTarget(LANG_SERVER);
}

stock void tChatAll(const char[] buffer, any ...)
{
    char msg[256];
    for(int client = 1; client <= MaxClients; ++client)
        if(ClientIsValid(client))
        {
            SetGlobalTransTarget(client);
            VFormat(msg, 256, buffer, 2);
            Format(msg, 256, "%s%s%s", !SkipNextPrefix ? ChatPrefix : "", ChatSpaces, msg);
            ProcessColorString(msg, 256);
            UTIL_SayText2(client, msg);
        }

    SkipNextChatCS = false;
    SkipNextPrefix = false;
    SetGlobalTransTarget(LANG_SERVER);
}

stock void ProcessColorString(char[] message, int maxLen)
{
    if(ReplaceColorEx)
    {
        ReplaceString(message, maxLen, "{normal}",      "\x07FFFFFF", false);
        ReplaceString(message, maxLen, "{default}",     "\x07FFFFFF", false);
        ReplaceString(message, maxLen, "{white}",       "\x07FFFFFF", false);
        ReplaceString(message, maxLen, "{darkred}",     "\x07FF0000", false);
        ReplaceString(message, maxLen, "{pink}",        "\x07D32CE6", false);
        ReplaceString(message, maxLen, "{green}",       "\x077FFF00", false);
        ReplaceString(message, maxLen, "{lime}",        "\x07BFFF00", false);
        ReplaceString(message, maxLen, "{yellow}",      "\x07FFD700", false);
        ReplaceString(message, maxLen, "{lightgreen}",  "\x0798FB98", false);
        ReplaceString(message, maxLen, "{lightred}",    "\x07EB4B4B", false);
        ReplaceString(message, maxLen, "{red}",         "\x07DC143C", false);
        ReplaceString(message, maxLen, "{gray}",        "\x07DCDCDC", false);
        ReplaceString(message, maxLen, "{grey}",        "\x07DCDCDC", false);
        ReplaceString(message, maxLen, "{olive}",       "\x07808000", false);
        ReplaceString(message, maxLen, "{orange}",      "\x07E4AE39", false);
        ReplaceString(message, maxLen, "{silver}",      "\x07B0C3D9", false);
        ReplaceString(message, maxLen, "{lightblue}",   "\x075E98D9", false);
        ReplaceString(message, maxLen, "{blue}",        "\x074B69FF", false);
        ReplaceString(message, maxLen, "{purple}",      "\x078847FF", false);
        ReplaceString(message, maxLen, "{darkorange}",  "\x07CF6A32", false);
    }
    else
    {
        ReplaceString(message, maxLen, "{normal}",      "\x01", false);
        ReplaceString(message, maxLen, "{default}",     "\x01", false);
        ReplaceString(message, maxLen, "{white}",       "\x01", false);
        ReplaceString(message, maxLen, "{darkred}",     "\x02", false);
        ReplaceString(message, maxLen, "{pink}",        "\x03", false);
        ReplaceString(message, maxLen, "{green}",       "\x04", false);
        ReplaceString(message, maxLen, "{lime}",        "\x05", false);
        ReplaceString(message, maxLen, "{yellow}",      "\x05", false);
        ReplaceString(message, maxLen, "{lightgreen}",  "\x06", false);
        ReplaceString(message, maxLen, "{lightred}",    "\x07", false);
        ReplaceString(message, maxLen, "{red}",         "\x07", false);
        ReplaceString(message, maxLen, "{gray}",        "\x08", false);
        ReplaceString(message, maxLen, "{grey}",        "\x08", false);
        ReplaceString(message, maxLen, "{olive}",       "\x09", false);
        ReplaceString(message, maxLen, "{orange}",      "\x10", false);
        ReplaceString(message, maxLen, "{silver}",      "\x0A", false);
        ReplaceString(message, maxLen, "{lightblue}",   "\x0B", false);
        ReplaceString(message, maxLen, "{blue}",        "\x0C", false);
        ReplaceString(message, maxLen, "{purple}",      "\x0E", false);
        ReplaceString(message, maxLen, "{darkorange}",  "\x0F", false);
    }
}

stock void RemoveAllColors(char[] message, int maxLen)
{
    ReplaceString(message, maxLen, "{normal}",      "", false);
    ReplaceString(message, maxLen, "{default}",     "", false);
    ReplaceString(message, maxLen, "{white}",       "", false);
    ReplaceString(message, maxLen, "{darkred}",     "", false);
    ReplaceString(message, maxLen, "{pink}",        "", false);
    ReplaceString(message, maxLen, "{green}",       "", false);
    ReplaceString(message, maxLen, "{lime}",        "", false);
    ReplaceString(message, maxLen, "{yellow}",      "", false);
    ReplaceString(message, maxLen, "{lightgreen}",  "", false);
    ReplaceString(message, maxLen, "{lightred}",    "", false);
    ReplaceString(message, maxLen, "{red}",         "", false);
    ReplaceString(message, maxLen, "{gray}",        "", false);
    ReplaceString(message, maxLen, "{grey}",        "", false);
    ReplaceString(message, maxLen, "{olive}",       "", false);
    ReplaceString(message, maxLen, "{orange}",      "", false);
    ReplaceString(message, maxLen, "{silver}",      "", false);
    ReplaceString(message, maxLen, "{lightblue}",   "", false);
    ReplaceString(message, maxLen, "{blue}",        "", false);
    ReplaceString(message, maxLen, "{purple}",      "", false);
    ReplaceString(message, maxLen, "{darkorange}",  "", false);
    ReplaceString(message, maxLen, "\x01",          "", false);
    ReplaceString(message, maxLen, "\x02",          "", false);
    ReplaceString(message, maxLen, "\x03",          "", false);
    ReplaceString(message, maxLen, "\x04",          "", false);
    ReplaceString(message, maxLen, "\x05",          "", false);
    ReplaceString(message, maxLen, "\x06",          "", false);
    ReplaceString(message, maxLen, "\x07",          "", false);
    ReplaceString(message, maxLen, "\x08",          "", false);
    ReplaceString(message, maxLen, "\x09",          "", false);
    ReplaceString(message, maxLen, "\x10",          "", false);
    ReplaceString(message, maxLen, "\x0A",          "", false);
    ReplaceString(message, maxLen, "\x0B",          "", false);
    ReplaceString(message, maxLen, "\x0C",          "", false);
    ReplaceString(message, maxLen, "\x0D",          "", false);
    ReplaceString(message, maxLen, "\x0E",          "", false);
    ReplaceString(message, maxLen, "\x0F",          "", false);
}

stock void UTIL_SayText2(int client, const char[] msg)
{
    if(UMType == UM_Protobuf)
    {
        Protobuf SayText2 = view_as<Protobuf>(StartMessageOne("SayText2", client, USERMSG_RELIABLE|USERMSG_BLOCKHOOKS));
        SayText2.SetInt("ent_idx", 0);
        SayText2.SetBool("chat", SkipNextChatCS ? false : ChatConSnd);
        SayText2.SetString("msg_name", msg);
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        SayText2.AddString("params", "");
        EndMessage();
    }
    else
    {
        BfWrite SayText = view_as<BfWrite>(StartMessageOne("SayText", client, USERMSG_RELIABLE|USERMSG_BLOCKHOOKS));
        SayText.WriteByte(0);
        SayText.WriteByte(true);
        SayText.WriteString(msg);
        EndMessage();
    }
}