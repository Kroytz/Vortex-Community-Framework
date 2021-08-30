#include "vcfcore/manager/vtools/glow.cpp"
#include "vcfcore/manager/vtools/colorcorrection.cpp"
#include "vcfcore/manager/vtools/jumpboost.cpp"

void VToolsOnPluginStart()
{
    GlowOnPluginStart();
    JumpBoostOnPluginStart();
}

void VToolsOnMapStart()
{
    ColorCorrectionOnMapStart();
}

void VToolsOnPlayerSpawn(int client)
{
    GlowOnPlayerSpawn(client);
    ColorCorrectionOnPlayerSpawn(client);
}

void VToolsOnPlayerDeath(int client)
{
    GlowOnPlayerDeath(client);
    ColorCorrectionOnPlayerDeath(client);
}

void VToolsOnPlayerJump(int client)
{
    RequestFrame(JumpBoostOnClientJumpPost, GetClientUserId(client));
}

void VToolsOnClientUpdated(int client)
{
    VToolsOnPlayerDeath(client);
}

void VToolsOnNativeInit()
{
    GlowOnNativeInit();
}

void VToolsOnClientDisconnectPost(int client)
{
    GlowOnClientDisconnectPost(client);
    ColorCorrectionOnClientDisconnectPost(client);
}

void VToolsOnPlayerRunCmd(int client, int& oldbuttons, int& buttons, int& impulse, float vel[3], float angles[3], int& weapon, int& subtype, int& cmdnum, int& tickcount, int& seed, int mouse[2])
{
    // Prevent blaming warnings
    #pragma unused client, oldbuttons, buttons, impulse, vel, angles, weapon, subtype, cmdnum, tickcount, seed, mouse

    ColorCorrectionOnPlayerRunCmd(client, oldbuttons, buttons, impulse, vel, angles, weapon, subtype, cmdnum, tickcount, seed, mouse);
}