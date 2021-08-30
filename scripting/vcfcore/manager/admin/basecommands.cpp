#include "vcfcore/manager/admin/basecommands/slay.cpp"
#include "vcfcore/manager/admin/basecommands/slap.cpp"
#include "vcfcore/manager/admin/basecommands/kick.cpp"
#include "vcfcore/manager/admin/basecommands/map.cpp"
#include "vcfcore/manager/admin/basecommands/rcon.cpp"
#include "vcfcore/manager/admin/basecommands/cvar.cpp"
#include "vcfcore/manager/admin/basecommands/exec.cpp"
#include "vcfcore/manager/admin/basecommands/who.cpp"
#include "vcfcore/manager/admin/basecommands/rounddraw.cpp"

void BaseCommandsOnPluginStart()
{
    SlayOnPluginStart();
    SlapOnPluginStart();
    KickOnPluginStart();
    MapOnPluginStart();
    RconOnPluginStart();
    CvarOnPluginStart();
    ExecOnPluginStart();
    WhoOnPluginStart();
    RoundDrawOnPluginStart();
}