#define WARNING_POINT_MAX			12 	// maximum = 12
										// it is necessary for making some preparation to avoid some stupid players fuck up our brain.

void WarningSysOnInit()
{
	RegConsoleCmd("sm_warning_point_set", Command_SetWarningPoint);
}

public Action Command_SetWarningPoint(int client, int params)
{
	if (params < 2) 
	{
		ReplyToCommand(client, "Usage: sm_warning_point_set [client] [type<0 = set | 1 = increase  | 2 = decrease | 3 = clear>] [points] [reason<Optional>]");
		return Plugin_Handled;
	}

	static bool bReason = false;
	if (params == 4) bReason = true;

	static char sTarget[NORMAL_LINE_LENGTH];
	GetCmdArg(1, string(sTarget));
	int target = FindTarget(client, sName, true, false);
	if (target < 0) return Plugin_Handled;

	static char sMode[SMALL_LINE_LENGTH];
	GetCmdArg(2, string(sMode));
	int mode = StringToInt(string(sMode));
	if (mode > 3 || mode < 0)
	{
		ReplyToCommand(client, "Invalid type %d.", mode);
		return Plugin_Handled;
	}

	static char sPoints[NORMAL_LINE_LENGTH];
	GetCmdArg(3, string(sPoints));
	int iPoints = StringToInt(string(sPoints));
	if (iPoints < 0 || iPoints > 12)
	{
		ReplyToCommand(client, "Invalid point value %d. (Range allowed: [0, 12])", iPoints);
		return Plugin_Handled;
	}

	static char sReason[NORMAL_LINE_LENGTH];
	(bReason) ? GetCmdArg(4, string(sReason)) : FormatEx(string(sReason), "No reason.");	

	return Plugin_Handled;
}

// > 0 => tips that have point.
// [ 1,  5] basic tips, with little limit.
// [ 6,  9] most limited.
// [10, 11] alert the target to pay the fee.
// 12 -> kick, until pay the fee
void WarningPointCheck(int client)
{
	if (gClientData[client].WarningPoint > 0)
	{
		PrintToChat(client, "WARNING: You now have %d warning points.", gClientData[client].WarningPoint);
	}

	if (gClientData[client].WarningPoint >= 1 && gClientData[client].WarningPoint <= 5)
	{
		PrintToChat(client, "Several parts will be limited. <TBD> <TBD> <TBD>");
	}

	if (gClientData[client].WarningPoint >= 6 && gClientData[client].WarningPoint <= 9)
	{
		PrintToChat(client, " Major of rights are limited.");
	}

	if (gClientData[client].WarningPoint >= 10 && gClientData[client].WarningPoint <= 11)
	{
		PrintToChatAll(client, "WARNING: %d's warning point is now %d", client, gClientData[client].WarningPoint);
	}

	if (gClientData[client].WarningPoint >= 12)
	{
		KickClient(client, "Your warning point is now >= 12, you must pay all of your fine or you cannot join the community");
	}
}