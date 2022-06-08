/**
 * @section: Admin flag bitwise
 */
#define ADMINFLAG_RESERVATION                         (1 << 0)    // Convenience macro for Admin_Reservation as a FlagBit
#define ADMINFLAG_GENERIC                             (1 << 1)    // Convenience macro for Admin_Generic as a FlagBit
#define ADMINFLAG_KICK                                (1 << 2)    // Convenience macro for Admin_Kick as a FlagBit
#define ADMINFLAG_BAN                                 (1 << 3)    // Convenience macro for Admin_Ban as a FlagBit
#define ADMINFLAG_SLAY                                (1 << 4)    // Convenience macro for Admin_Slay as a FlagBit
#define ADMINFLAG_MAP                                 (1 << 5)    // Convenience macro for Admin_Changemap as a FlagBit
#define ADMINFLAG_CONVARS                             (1 << 6)    // Convenience macro for Admin_Convars as a FlagBit
#define ADMINFLAG_CONFIG                              (1 << 7)    // Convenience macro for Admin_Config as a FlagBit
#define ADMINFLAG_RCON                                (1 << 8)    // Convenience macro for Admin_RCON as a FlagBit
#define ADMINFLAG_ROOT                                (1 << 9)   // Convenience macro for Admin_Root as a FlagBit
/**
 * END:SECTION
 */

#include "vcfcore/manager/admin/bans.cpp"
#include "vcfcore/manager/admin/adminmenu.cpp"
#include "vcfcore/manager/admin/basecommands.cpp"

void AdminOnPluginStart()
{
    RegConsoleCmd("sm_reloadadmins", AdminReloadAdminsOnCommandCatched, "sm_reloadadmins");

    BansOnPluginStart();
    AdminMenuOnPluginStart();
    BaseCommandsOnPluginStart();
}

void AdminOnNativeInit()
{
    CreateNative("VCF_AdminHasFlag",        API_AdminHasFlag);
    CreateNative("VCF_GetClientAccess",     API_GetClientAccess);
    CreateNative("VCF_GetAdminImmunity",    API_GetClientAccess);
    CreateNative("VCF_GetAdminFlagBits",    API_GetAdminFlagBits);
}

public Action AdminReloadAdminsOnCommandCatched(int client, int args)
{
    if (!AdminIsClientAdmin(client))
    {
        LPrintToChatSingleLine(client, "no access");
        return Plugin_Handled;
    }

    AdminReloadAdmins(client);
    return Plugin_Handled;
}

void AdminReloadAdmins(int admin)
{
    DatabaseInsertAdminLog(admin, "Reload Admins");

    LoopAllPlayers(i)
    {
        if (IsFakeClient(i))
            continue;

        AdminFetchUser(i);
    }
}

void AdminOnClientPostAdminCheck(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    // Sub module
    BansOnClientPostAdminCheck(client);
}

inline void AdminOnClientReady(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    // Late load prevent pid = -1
    AdminFetchUser(client);
}

void AdminFetchUser(int client)
{
    if (IsFakeClient(client) || !IsPlayerExist(client, false))
        return;

    static char szQuery[512];
    FormatEx(szQuery, sizeof(szQuery), "SELECT * FROM `vcf_admins` WHERE `pid` = '%d' AND (modid = '%d' OR modid = '0');", gClientData[client].PID, gServerData.ModID);
    gServerData.DBI.Query(AdminFetchUserCB, szQuery, client);
}

DBCallbackGeneral(AdminFetchUserCB)
{
    if(dbRs == null)
    {
        LogError("[AdminFetchUserCB] -> Error: (%s)", szError);
        if(StrContains(szError, "Lost connection to MySQL", false) != -1)
        {
            DatabaseTryReconnect();
        }
        return;
    }
    
    if (dbRs.FetchRow())
    {
        // 0: pid
        // 1: modid
        dbRs.FetchString(2, gClientData[client].Nick, sizeof(gClientData[].Nick));
        gClientData[client].Access = dbRs.FetchInt(3);
        char szFlags[32]; dbRs.FetchString(4, szFlags, sizeof(szFlags));
        gClientData[client].Flags = AdminConvertStringToBits(szFlags);
    }

    return;
}

bool AdminIsClientAdmin(int client)
{
    if (client == 0 || gClientData[client].Access > 0)
        return true;
    else
        return false;
}

int AdminFindTarget(const char[] szIdentifier)
{
    static char sTemp[256];
    strcopy(sTemp, sizeof(sTemp), szIdentifier);
    int target = -1;
    switch (sTemp[0])
    {
        // STEAM_X:X:XXXXXXXX
        case 'S':
        {
            LoopAllPlayers(i)
            {
                static char szSteam32[33]; GetClientAuthId(i, AuthId_Steam2, szSteam32, sizeof(szSteam32));
                if (strcmp(szSteam32, sTemp, false) == 0)
                {
                    target = i;
                    break;
                }
            }
        }

        // #XX
        case '#':
        {
            static int iIdentifier;
            ReplaceString(sTemp, sizeof(sTemp), "#", "");
            iIdentifier = StringToInt(sTemp);

            LoopAllPlayers(i)
            {
                int iUserid = GetClientUserId(i);
                if (iUserid == iIdentifier)
                {
                    target = i;
                    break;
                }
            }
        }

        default: // Steam64?
        {
            static char szSteam32[33];
            ConvertSteam64ToSteam32(szIdentifier, szSteam32, sizeof(szSteam32));
            AdminFindTarget(szSteam32);
        }
    }

    return target;
}

int AdminConvertStringToBits(const char[] szFlagString)
{
    int iFlags = 0;

    if (StrContains(szFlagString, "a", false))
        iFlags |= ADMINFLAG_GENERIC;

    if (StrContains(szFlagString, "b", false))
        iFlags |= ADMINFLAG_RESERVATION;

    if (StrContains(szFlagString, "c", false))
        iFlags |= ADMINFLAG_KICK;

    if (StrContains(szFlagString, "d", false))
        iFlags |= ADMINFLAG_BAN;

    if (StrContains(szFlagString, "e", false))
        iFlags |= ADMINFLAG_SLAY;

    if (StrContains(szFlagString, "f", false))
        iFlags |= ADMINFLAG_CONVARS;

    if (StrContains(szFlagString, "g", false))
        iFlags |= ADMINFLAG_CONFIG;

    if (StrContains(szFlagString, "h", false))
        iFlags |= ADMINFLAG_RCON;

    if (StrContains(szFlagString, "z", false))
        iFlags |= ADMINFLAG_ROOT;

    return iFlags;
}

inline bool AdminHasClientFlag(int client, int iFlag)
{
    if (
        client == 0 || 
        ((gClientData[client].Flags > 0) && ((gClientData[client].Flags & iFlag) || (gClientData[client].Flags & ADMINFLAG_ROOT)))
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

public int API_AdminHasFlag(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);
    int flag = GetNativeCell(2);
    
    // Return the value
    return AdminHasClientFlag(client, flag);
}

public int API_GetClientAccess(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);
    
    // Return the value
    return gClientData[client].Access;
}

public int API_GetAdminFlagBits(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);
    
    // Return the value
    return gClientData[client].Flags;
}

stock int ConvertSteam64ToSteam32(const char[] steamId64, char[] steamId32, int maxLen)
{
    if(strlen(steamId64) != 17 || StrContains(steamId64, "765611", false) != 0)
    {
        ThrowNativeError(SP_ERROR_PARAM, "ConvertSteam64ToSteam32 -> Invalid steamId64 given. -> %s", steamId64);
        return -1;
    }

    char[] m_szBase = "76561197960265728";
    char m_szAuth[18], m_szAccount[18];
    int m_iBorrow, m_iTemp, m_iSteamId[2];

    strcopy(m_szAuth, 18, steamId64);

    if(CharToNumber(m_szAuth[16]) % 2 == 1)
    {
        m_iSteamId[0] = 1;
        m_szAuth[16] = NumberToChar(CharToNumber(m_szAuth[16]) - 1);
    }
    
    for(int k = 16; k >= 0; k--)
    {
        if(m_iBorrow > 0)
        {
            m_iTemp = CharToNumber(m_szAuth[k]) - 1;

            if(m_iTemp >= CharToNumber(m_szBase[k]))
            {
                m_iBorrow = 0;
                m_szAccount[k] = NumberToChar(m_iTemp - CharToNumber(m_szBase[k]));
            }
            else
            {
                m_iBorrow = 1;
                m_szAccount[k] = NumberToChar((m_iTemp + 10) - CharToNumber(m_szBase[k]));
            }
        }
        else
        {
            if(CharToNumber(m_szAuth[k]) >= CharToNumber(m_szBase[k]))
            {
                m_iBorrow = 0;
                m_szAccount[k] = NumberToChar(CharToNumber(m_szAuth[k]) - CharToNumber(m_szBase[k]));
            }
            else
            {
                m_iBorrow = 1;
                m_szAccount[k] = NumberToChar((CharToNumber(m_szAuth[k]) + 10) - CharToNumber(m_szBase[k]));
            }
        }
    }

    m_iSteamId[1] = StringToInt(m_szAccount);
    m_iSteamId[1] /= 2;

    return FormatEx(steamId32, maxLen, "STEAM_1:%d:%d", m_iSteamId[0], m_iSteamId[1]);
}

stock int NumberToChar(const int iNum)
{
    return '0' + ((iNum >= 0 && iNum <= 9) ? iNum : 0);
}

stock int CharToNumber(const int cNum)
{
    return (cNum >= '0' && cNum <= '9') ? (cNum - '0') : 0;
}