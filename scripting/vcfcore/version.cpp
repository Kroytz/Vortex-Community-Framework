#include "vcfcore/version_auto.cpp"

void VersionOnPluginStart()
{
    RegConsoleCmd("vcf_version", VersionOnCommandCatched);
}

void VersionOnMapStart()
{
    VersionOnCommandCatched(LANG_SERVER, LANG_SERVER);
}

public Action VersionOnCommandCatched(int client, int params)
{
    // Initialize variables
    static char sBuffer[HUGE_LINE_LENGTH]; sBuffer[0] = NULL_STRING[0];
    static char sLine[BIG_LINE_LENGTH]; sLine[0] = NULL_STRING[0];

    /// Format strings
    FormatEx(sLine, sizeof(sLine), "\n%s\n", PLUGIN_NAME);
    StrCat(sBuffer, sizeof(sBuffer), sLine);
    FormatEx(sLine, sizeof(sLine), "%s\n\n", PLUGIN_COPYRIGHT);
    StrCat(sBuffer, sizeof(sBuffer), sLine);
    FormatEx(sLine, sizeof(sLine), "%24s: %s\n", "Version", PLUGIN_VERSION);
    StrCat(sBuffer, sizeof(sBuffer), sLine);
    FormatEx(sLine, sizeof(sLine), "%24s: %s\n", "Last edit", PLUGIN_DATE);
    StrCat(sBuffer, sizeof(sBuffer), sLine);
    FormatEx(sLine, sizeof(sLine), "%24s: %s\n", "Link+", PLUGIN_LINK);
    StrCat(sBuffer, sizeof(sBuffer), sLine);

    // Send information into the console
    ReplyToCommand(client, sBuffer);
    return Plugin_Handled;
}