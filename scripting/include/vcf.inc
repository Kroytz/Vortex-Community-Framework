#if defined _vcf_included
  #endinput
#endif
#define _vcf_included

// Core
forward void   VCF_OnClientLoaded(int client);

// User
native int     VCF_GetClientAccess(int client); // Deprecated
native int     VCF_GetClientPID(int client);
native int     VCF_GetClientVIPLevel(int client);
native float   VCF_GetClientTotalPurcahse(int client);
native int     VCF_GetClientWarningPoint(int client);
native void    VCF_SetClientWarningPoint(int client, int value);
native int     VCF_GetClientHonorPointSteam(int client);
native void    VCF_SetClientHonorPointSteam(int client, int value);
native int     VCF_GetClientHonorPointRule(int client);
native void    VCF_SetClientHonorPointRule(int client, int value);
native int     VCF_GetClientHonorPointOther(int client);
native void    VCF_SetClientHonorPointOther(int client, int value);
native int     VCF_GetWarningPointClearTimes(int client);
native void    VCF_GetServerPublicIP(char[] ip, int maxlen);
native bool    VCF_IsPlayerFromCN(int client);

// Admin
native int     VCF_GetAdminImmunity(int client);
native int     VCF_GetAdminFlagBits(int client);

// Glow
native int     VCF_CreateGlow(int client, int colors[3] = {0, 0, 0}, float removedelay = 0.0);
native void    VCF_RemoveGlow(int client);

// Inventory
forward Action VCF_OnBagItemShowPre(int client, const char[] sItemCode); // Continue -> 正常显示 | 正常扣除道具
forward Action VCF_OnBagItemUsePost(int client, const char[] sItemCode); // Handled ->  不可用显示 | 不扣除道具

native int     VCF_GetBagItem(int client, const char[] sItemCode);
native int     VCF_SetBagItem(int client, const char[] sItemCode, const char[] sSymbol, int iAmount);
native bool    VCF_ForceClientUseItem(int client, const char[] sItemCode);

// Stats       :: UNUSED ::
native int     VCF_GetStat(int client, int slot);
native int     VCF_SetStat(int client, int slot, const char[] sSymbol, int iAmount);

enum /* ZP_STATS_SLOT */
{
    ZP_STATS_KILL = 1, 
    ZP_STATS_DEATH, 
    ZP_STATS_INFECT, 
    ZP_STATS_KILLSI1, 
    ZP_STATS_KILLSI2, 
    ZP_STATS_ESCAPE
}

// Hide       :: UNUSED ::
native bool Hide_ShouldHide(int target, int client);      // True -> target not transmit to client
forward void Hide_OnHideTransmit(int target, int client); // !! 仅在 target 对 client 隐藏渲染时 call !!