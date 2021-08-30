/**
 * @section Struct of operation types for server arrays.
 */
enum struct ServerData
{
    /* Database */
    Database DBI;

    /* ServerInfo */
    int             ServerID;
    int             ModID;
    char            PublicIP[NORMAL_LINE_LENGTH];

    /* AdminMenu */
    ArrayList AdminMenu;
    StringMap ProtectedCvar;

    /* Core Plugins */
    bool            core_Store;

    /* Mods Plugins */
    bool            mod_ZombiePlague;

    // /* Sign */
    // float SignRatio;
}

/**
 * @brief Array to store the server data.
 */
ServerData gServerData;

enum struct CvarsList
{
    ConVar RESERVEDSLOT_ENABLE;
    ConVar RESERVEDSLOT_VISIBLESLOT;
    ConVar RESERVEDSLOT_VIPSLOTS;
    ConVar RESERVEDSLOT_ADMINSLOTS;

    ConVar GLOW_FORCE_TRANSMIT_PLAYERS;

    ConVar JUMPBOOST_ENABLE;
    ConVar JUMPBOOST_MULTIPLY;
    ConVar JUMPBOOST_HEIGHT_MULTIPLY;
    ConVar JUMPBOOST_MAX;
}
/**
 * @endsection
 **/
 
/**
 * Array to store cvar data in.
 **/
CvarsList gCvarList;

enum /* SayHookType */
{
    SAYHOOK_NONE,
    SAYHOOK_INVITE = 1,
    SAYHOOK_ADMINBAN
}

/**
 * @section Struct of operation types for client arrays.
 */
enum struct ClientData
{
    /* Init */
    char      Steam64[32];
    int       Cooldown;
    char      szTemp[32];
    int       SayHookType;

    /* Admin */
    int       Access;
    int       Flags;
    char      Nick[32];

    /* Users */
    int       PID;
    float     TotalPurchase;
    int       VIPLevel;
    int       WarningPoint;
    int       WarningPointClearTimes;

    int       HonorPointSteam;
    int       HonorPointRule;
    int       HonorPointOther;

    ArrayList Items;

    /* Glow */
    int       GlowRef;
    int       LightRef;

    /* ColorCorrection*/
    int       CCRef;

    /* Invite */
    bool      Entered;
    char      Code[32];
    int       Count;

    /* Sign */
    float     SignBasicRatio;
    float     SignBonusRatio;

    StringMap ClientSteamID;

    /**
     * @brief Resets all variables.
     */
    void ResetVars()
    {
        this.Steam64       = "";
        this.Cooldown      = 0;
        this.szTemp        = "";
        this.SayHookType   = SAYHOOK_NONE;

        this.Access        = 0;
        this.Nick          = "";

        this.PID           = -1;
        this.TotalPurchase = 0.0;
        this.VIPLevel      = 0;
        this.WarningPoint  = 0;
        this.WarningPointClearTimes = 0;
        this.HonorPointSteam = 0;
        this.HonorPointRule = 0;
        this.HonorPointOther = 0;

        this.Items.Clear();

        this.GlowRef       = INVALID_ENT_REFERENCE;

        this.CCRef         = INVALID_ENT_REFERENCE;
        this.LightRef      = INVALID_ENT_REFERENCE;

        this.SignBasicRatio = 1.0;
        this.SignBonusRatio = 0.0;

        this.Entered       = true;
        this.Code          = "";
        this.Count         = 0;

        delete this.ClientSteamID;
    }
}

ClientData gClientData[MAXPLAYERS+1];