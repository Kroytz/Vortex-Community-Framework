#define RAW_PATH "materials/vortex/cc/go2full.raw"

void ColorCorrectionOnPlayerSpawn(int client)
{
    ColorCorrectionRemoveLight(client);
    ColorCorrectionRemoveCC(client);
}

void ColorCorrectionOnPlayerDeath(int client)
{
    ColorCorrectionRemoveLight(client);
    ColorCorrectionRemoveCC(client);
}

void ColorCorrectionOnMapStart()
{
    AddFileToDownloadsTable(RAW_PATH);
}

void ColorCorrectionOnPlayerRunCmd(int client, int& oldbuttons, int& buttons, int& impulse, float vel[3], float angles[3], int& weapon, int& subtype, int& cmdnum, int& tickcount, int& seed, int mouse[2])
{
    // Prevent blaming warnings
    // 按需所取...
    #pragma unused client, oldbuttons, buttons, impulse, vel, angles, weapon, subtype, cmdnum, tickcount, seed, mouse

    static float flEnableDelay = 2.0; // 3.0 Seconds
    static float flEnableTick[MAXPLAYERS+1];

    float flTickCount = float(tickcount);
    if (!(oldbuttons & IN_RELOAD) && (buttons & IN_RELOAD))
    {
        // Start counting
        flEnableTick[client] = flTickCount + ((1.0 / GetTickInterval()) * flEnableDelay);
    }
    else if ((oldbuttons & IN_RELOAD) && (buttons & IN_RELOAD)) // Still holding 'R'?
    {
        // Reach the delayed time
        if (flTickCount >= flEnableTick[client])
        {
            // Toggle
            ColorCorrectionToggleNightVision(client);
            flEnableTick[client] = 999999.0; // Prevent double toggle
        }
    }
}

void ColorCorrectionToggleNightVision(int client)
{
    if (gClientData[client].CCRef == INVALID_ENT_REFERENCE)
    {
        // Set ownership & open it
        ToolsSetNightVision(client, true, true);
        ToolsSetNightVision(client, true);

        // Create CC
        // Note: Can expand to a configuration/option.
        ColorCorrectionCreateLight(client);
        ColorCorrectionCreateCC(client, RAW_PATH);

        LPrintToChatSingleLine(client, "nightvision on");
    }
    else
    {
        ToolsSetNightVision(client, false);
        ToolsSetNightVision(client, false, true);

        ColorCorrectionRemoveLight(client);
        ColorCorrectionRemoveCC(client);

        LPrintToChatSingleLine(client, "nightvision off");
    }
}

int ColorCorrectionCreateCC(int client, char[] sFilePath)
{
    int entity = CreateEntityByName("color_correction");

    if (entity != -1)
    {
        DispatchKeyValue(entity, "StartDisabled", "0");
        DispatchKeyValue(entity, "maxweight", "1.0");
        DispatchKeyValue(entity, "maxfalloff", "-1.0");
        DispatchKeyValue(entity, "minfalloff", "0.0");
        DispatchKeyValue(entity, "filename", sFilePath);

        DispatchSpawn(entity);
        ActivateEntity(entity);

        SetEntPropFloat(entity, Prop_Send, "m_flCurWeight", 1.0);
        SetEdictFlags(entity, GetEdictFlags(entity) & ~FL_EDICT_ALWAYS);
        SDKHook(entity, SDKHook_SetTransmit, ColorCorrectionOnCCSetTransmit);
        gClientData[client].CCRef = EntIndexToEntRef(entity);
    }
    
    return entity;
}

public Action ColorCorrectionOnCCSetTransmit(int entity, int client)
{
    SetEdictFlags(entity, GetEdictFlags(entity) & ~FL_EDICT_ALWAYS);

    if (EntRefToEntIndex(gClientData[client].CCRef) != entity)
        return Plugin_Handled;
    else
    {
        SetEdictFlags(entity, GetEdictFlags(entity) | FL_EDICT_DONTSEND);
        SetEntPropFloat(entity, Prop_Send, "m_flCurWeight", 1.0);
        return Plugin_Continue;
    }
}

void ColorCorrectionRemoveCC(int client)
{
    int ent = EntRefToEntIndex(gClientData[client].CCRef);
    if(ent != -1 && IsValidEntity(ent))
    {
        // Make sure this edict is still the 'CC' entity
        char sClassname[32];
        GetEdictClassname(ent, sClassname, sizeof(sClassname));

        if (!strcmp(sClassname, "color_correction", false))
        {
            RemoveEntity(ent);
        }
    }

    gClientData[client].CCRef = INVALID_ENT_REFERENCE;
}

void ColorCorrectionCreateLight(int client)
{
    // Gets parent position
    float vPosition[3];
    GetEntPropVector(client, Prop_Data, "m_vecAbsOrigin", vPosition);

    int iLight = UTIL_CreateLight(client, vPosition, _, _, _, _, _, _, _, "200 200 200 100", 400.0, 800.0);
    if (iLight != -1)
    {
        SDKHook(iLight, SDKHook_SetTransmit, ColorCorrectionOnLightSetTransmit);
        gClientData[client].LightRef = EntIndexToEntRef(iLight);
    }
}

void ColorCorrectionRemoveLight(int client)
{
    int ent = EntRefToEntIndex(gClientData[client].LightRef);
    if(ent != -1 && IsValidEntity(ent))
    {
        // Make sure this edict is still the 'CC' entity
        char sClassname[32];
        GetEdictClassname(ent, sClassname, sizeof(sClassname));

        if (!strcmp(sClassname, "light_dynamic", false))
        {
            RemoveEntity(ent);
        }
    }

    gClientData[client].LightRef = INVALID_ENT_REFERENCE;
}

public Action ColorCorrectionOnLightSetTransmit(int entity, int client)
{
    if (EntRefToEntIndex(gClientData[client].LightRef) != entity)
        return Plugin_Handled;
    else
    {
        return Plugin_Continue;
    }
}

void ColorCorrectionOnClientDisconnectPost(int client)
{
    ColorCorrectionRemoveLight(client);
    ColorCorrectionRemoveCC(client);
}