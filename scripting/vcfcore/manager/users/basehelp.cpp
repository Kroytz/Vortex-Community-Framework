/* 
=================== General ==========================
!?					Gets all commands prompts.
!vuser				Opens the user center
!sign, !qd			Sign in the game, to earn reward.
!roll				Roll a dice.
!flip				Flip a coin.
R(Holds on)			Enables nightvision
!store				Opens store.
!inv				Check what store item you own.
!invite				Invite friends.
!bag				Inventory.
================= Zombie Plague ======================
!zmenu				Opens the main menu.
!guns				Select weapons.
!zclass				Selects zombie classes.
!hclass				Selects human classes.
[Shift]				Opens equipment menu.
[F]					Toggle flashlight.
 */
void BaseHelpDisplay(int client)
{
    if (!IsPlayerExist(client, false))
        return;

    LPrintToChatSingleLine(client, "basehelp check console");

    PrintToConsole(client, "=================== General ==========================");
    PrintToConsole(client, "/?                              获取指令集 - Command sets");
    PrintToConsole(client, "!vuser                          打开用户中心 - User center");
    PrintToConsole(client, "!qd, !sign                      签到 - Sign");
    PrintToConsole(client, "!roll                           摇骰子 - Roll");
    PrintToConsole(client, "!flip                           抛硬币 - Flip");
    PrintToConsole(client, "R[长按]                         开启夜视仪 - NightVision");
    PrintToConsole(client, "!store                          开启商店 - Store");
    PrintToConsole(client, "!inv                            查看你已购买的物品 - Bought items");
    PrintToConsole(client, "!invite                         邀请友伴, 输入邀请码 - Invite friends");
    PrintToConsole(client, "!bag                            打开库存 - Inventory");
    PrintToConsole(client, "!dj                             关闭地图音乐 - MapMusic");
}