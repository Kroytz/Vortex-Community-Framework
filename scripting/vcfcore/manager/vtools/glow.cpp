void GlowOnPluginStart()
{
    gCvarList.GLOW_FORCE_TRANSMIT_PLAYERS = FindConVar("sv_force_transmit_players");

    if (gCvarList.GLOW_FORCE_TRANSMIT_PLAYERS.IntValue == 0)
        gCvarList.GLOW_FORCE_TRANSMIT_PLAYERS.SetInt(1, true);
}

void GlowOnClientDisconnectPost(int client)
{
    if (gClientData[client].GlowRef == INVALID_ENT_REFERENCE)
        return;

    GlowRemoveGlow(client);
}

void GlowOnPlayerDeath(int client)
{
    if (gClientData[client].GlowRef == INVALID_ENT_REFERENCE)
        return;

    GlowRemoveGlow(client);
}

void GlowOnPlayerSpawn(int client)
{
    if (gClientData[client].GlowRef == INVALID_ENT_REFERENCE)
        return;

    GlowRemoveGlow(client);
}

int GlowCreateGlow(int client, int colors[3], float removedelay = 0.0)
{
    if (!IsPlayerExist(client))
        return -1;

    // Gets model
    char szModel[512]; GetClientModel(client, szModel, sizeof(szModel));

    // Gets position
    float vPosition[3]; GetEntPropVector(client, Prop_Data, "m_vecAbsOrigin", vPosition);

    // Gets angle
    float vAngles[3]; GetClientEyeAngles(client, vAngles);

    // Create glow
    int pGlow = -1;
    pGlow = UTIL_CreateDynamic("glow", vPosition, vAngles, szModel, _, _, _, false);

    // Create failed?
    if (pGlow == -1)
        return -1;

    // Block trans from client
    SetEntityRenderMode(pGlow, RENDER_TRANSALPHA);
    SetEntityRenderColor(pGlow, 0, 0, 0, 0);

    // Set effects
    SetEntProp(pGlow, Prop_Send, "m_fEffects", EF_BONEMERGE|EF_NOSHADOW|EF_NORECEIVESHADOW);

    // Set parent
    SetVariantString("!activator");
    AcceptEntityInput(pGlow, "SetParent", client, pGlow);

    // Animation fix
    SetVariantString("primary");
    AcceptEntityInput(pGlow, "SetParentAttachment", pGlow, pGlow, 0);

    // Create glowing
    UTIL_CreateGlowing(pGlow, true, 1, colors[0], colors[1], colors[2], 255);

    // Sets glow reference for transmit
    gClientData[client].GlowRef = EntIndexToEntRef(pGlow);

    // Hook failed?
    if (!SDKHookEx(pGlow, SDKHook_SetTransmit, GlowOnSetTransmit))
        return -1;

    // Remove Delay Exists -> Set
    if (removedelay > 0.0)
        UTIL_RemoveEntity(pGlow, removedelay);

    return pGlow;
}

public Action GlowOnSetTransmit(int entity, int client)
{
    // Glow exists
    if (gClientData[client].GlowRef != INVALID_ENT_REFERENCE)
    {
        // Gets entity ptr
        int pEntity = EntRefToEntIndex(gClientData[client].GlowRef);
        if (pEntity == INVALID_ENT_REFERENCE || !IsValidEntity(pEntity) || pEntity != entity) // invalid?
        {
            return Plugin_Continue;
        }
        else
        {
            return Plugin_Handled; // block transmit
        }
    }

    return Plugin_Continue;
}

public void GlowRemoveGlow(int client)
{
    // Glow exists?
    if (gClientData[client].GlowRef != INVALID_ENT_REFERENCE)
    {
        // Gets entity ptr
        int pEntity = EntRefToEntIndex(gClientData[client].GlowRef);
        if (pEntity != INVALID_ENT_REFERENCE && IsValidEntity(pEntity))
        {
            // Make sure this edict is still the 'glow' entity
            char sClassname[32];
            GetEdictClassname(pEntity, sClassname, sizeof(sClassname));
            if (!strcmp(sClassname, "prop_dynamic", false))
            {
                // Remove it
                UTIL_RemoveEntity(pEntity, 0.1);
            }
        }
    }

    gClientData[client].GlowRef = INVALID_ENT_REFERENCE;
}

void GlowOnNativeInit()
{
    CreateNative("VCF_CreateGlow",   API_CreateGlow);
    CreateNative("VCF_RemoveGlow",   API_RemoveGlow);
}

public int API_CreateGlow(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);

    // Gets colors
    int colors[3] = {0, 0, 0};
    GetNativeArray(2, colors, sizeof(colors));

    // Gets remove delay
    float flDelay = view_as<float>(GetNativeCell(3));

    return GlowCreateGlow(client, colors, flDelay);
}

public int API_RemoveGlow(Handle hPlugin, int iNumParams)
{
    // Gets real player index from native cell 
    int client = GetNativeCell(1);

    GlowRemoveGlow(client);

    return 1;
}