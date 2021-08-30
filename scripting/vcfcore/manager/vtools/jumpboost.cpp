void JumpBoostOnPluginStart()
{
    // Convar initialize
    gCvarList.JUMPBOOST_ENABLE          = CreateConVar("vcf_jumpboost_enable", "1", "Enable the jump boost feature? 0 = Disable, 1 = Enable", 0, true, 0.0, true, 1.0);
    gCvarList.JUMPBOOST_MULTIPLY        = CreateConVar("vcf_jumpboost_multiply", "1.1", "The multiplier per jump", 0, true, 0.0);
    gCvarList.JUMPBOOST_HEIGHT_MULTIPLY = CreateConVar("vcf_jumpboost_height_multiply", "1.0", "The multiplier for height per jump", 0, true, 0.0);
    gCvarList.JUMPBOOST_MAX             = CreateConVar("vcf_jumpboost_speedmax", "380.0", "The speed max for jump boost?", 0, true, 0.0);
}

public void JumpBoostOnClientJumpPost(int userID)
{
    if (gCvarList.JUMPBOOST_ENABLE.IntValue == 0)
        return;

    // Gets client index from the user ID
    int client = GetClientOfUserId(userID);

    // Validate client
    if (IsPlayerExist(client))
    {
        // Gets client velocity
        static float vVelocity[3];
        ToolsGetVelocity(client, vVelocity);
        
        // Only apply horizontal multiplier if it not a bhop
        if (GetVectorLength(vVelocity) < gCvarList.JUMPBOOST_MAX.FloatValue)
        {
            // Apply horizontal multipliers to jump vector
            vVelocity[0] *= gCvarList.JUMPBOOST_MULTIPLY.FloatValue;
            vVelocity[1] *= gCvarList.JUMPBOOST_MULTIPLY.FloatValue;
        }

        // Apply height multiplier to jump vector
        vVelocity[2] *= gCvarList.JUMPBOOST_HEIGHT_MULTIPLY.FloatValue;

        // Sets new velocity
        ToolsSetVelocity(client, vVelocity, true, false);
    }
}