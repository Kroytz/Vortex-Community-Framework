/* 
Instruction:
Before compiling, include json and system2.
in json/array.inc, Line 739, 747, 748, 752, change variable name in order to avoid typename and funcname conflict.
*/

// define pwapi directly
// unnecessary to write convar.
#define COMMUNITY_KEY               "3c3c5f1706c7a73ba95602d0d22c1438"
#define COMMUNITY_ID                "Vector"

// #define _DEBUG_MODE

void PWSupportOnClientInit(int client)
{
    if (IsFakeClient(client)) return;

    PWSupportCheckIsPW(client);
}

void PWSupportOnClientDisconnect(int client)
{
    PWSupportDisconnectRequest(client);
}

void PWSupportCheckIsPW(int client)
{
    if (hasLength(gClientData[client].Steam64))
    {
        System2HTTPRequest httpRequest = new System2HTTPRequest(
            PWSupportCheckIsPWHTTPRequestCallback, "https://csgo.wanmei.com/api-user/isOnline?steamIds=%s", gClientData[client].Steam64
            );
        httpRequest.Timeout = 15;
        httpRequest.Any = GetClientUserId(client);
        httpRequest.SetHeader("Content-Type", "application/json;charset=utf-8");
        httpRequest.GET();

        delete httpRequest;
    }
}

void PWSupportCheckIsPWHTTPRequestCallback(bool success, const char[] error, System2HTTPRequest request, System2HTTPResponse response, HTTPRequestMethod method)
{
    #pragma unused method
    static char url[PLATFORM_LINE_LENGTH];
    request.GetURL(string(url));

    if (!success) {
        PrintToServer("ERROR: Couldn't retrieve URL %s. Error: %s", url, error);
        PrintToServer("");
        PrintToServer("INFO: Finished");
        PrintToServer("");

        return;
    }

    static char result[PLATFORM_LINE_LENGTH], steamid64[18], IsOnline[16];
    
    response.GetContent(string(result));
    JSON_Object obj = json_decode(result);
    JSON_Object data = obj.GetObject("result");
    StringMapSnapshot snap = data.Snapshot();
    snap.GetKey(0, string(steamid64));
    data.GetString(steamid64, string(IsOnline));

    int client = GetClientOfUserId(request.Any);
    if (client > 0)
    {
        gClientData[client].PerfectWorld = StrEqual(IsOnline, "online");

#if defined _DEBUG_MODE
        static char sName[NORMAL_LINE_LENGTH];
        GetClientName(client, string(sName));
        LogMessage("[PWSupportCheckIsPWHTTPRequestCallback] DEBUG: client %s(%s) zhCN Status: %s", sName, gClientData[client].Steam64, IsOnline);
#endif

        PWSupportConnectRequest(client);
    }

    delete snap; 
    delete data; 
    delete obj;

#if defined _DEBUG_MODE
    response.GetLastURL(url, sizeof(url));

    PrintToServer("INFO: Successfully retrieved URL %s in %.0f milliseconds", url, response.TotalTime * 1000.0);
    PrintToServer("");
    PrintToServer("INFO: HTTP Version: %s", (response.HTTPVersion == VERSION_1_0 ? "1.0" : "1.1"));
    PrintToServer("INFO: Status Code: %d", response.StatusCode);
    PrintToServer("INFO: Downloaded %d bytes with %d bytes/seconds", response.DownloadSize, response.DownloadSpeed);
    PrintToServer("INFO: Uploaded %d bytes with %d bytes/seconds", response.UploadSize, response.UploadSpeed);
    PrintToServer("");
    PrintToServer("INFO: Retrieved the following headers:");

    char name[128];
    char value[128];
    ArrayList headers = response.GetHeaders();

    for (int i = 0; i < headers.Length; i++) {
        headers.GetString(i, name, sizeof(name));
        response.GetHeader(name, value, sizeof(value));
        PrintToServer("\t%s: %s", name, value);
    }
    
    PrintToServer("");
    PrintToServer("INFO: Content (%d bytes):", response.ContentLength);
    PrintToServer("");
    
    char content[128];
    for (int found = 0; found < response.ContentLength;) {
        found += response.GetContent(content, sizeof(content), found);
        PrintToServer(content);
    }

    PrintToServer("");
    PrintToServer("INFO: Finished");
    PrintToServer("");
    
    delete headers;
#endif
}

void PWSupportConnectRequest(int client)
{
    // I need server.ip variable
    // ignore here.

    int timestamp = GetTime();
    static char buffer[PLATFORM_LINE_LENGTH], output[PLATFORM_LINE_LENGTH];

    JSON_Array arr = new JSON_Array();
    JSON_Object obj_topic = new JSON_Object();
    obj_topic.SetString("topic", "log_csgo_3rdparty");
    JSON_Object obj_client = new JSON_Object();
    obj_client.SetObject("headers", obj_topic);
    FormatEx(
        string(buffer),
        "{                                                  \
        \n    \"timestamp\"         :           \"%i\"      \
        \n    \"community ID\"      :           \"%s\"      \
        \n    \"server IP\"         :           \"%s\"      \
        \n    \"type\"              :           \"login\"   \
        \n    \"steamid\"           :           \"%s\"      \
        \n    \"zhCN\"              :           \"%s\"      \
        \n}",
        timestamp, COMMUNITY_ID, gServerData.PublicIP, gClientData[client].Steam64, gClientData[client].PerfectWorld ? "true" : "false"
    );

#if defined _DEBUG_MODE
    LogMessage("[PWSupportConnectRequest] DEBUG: User connected. Data sent: %s", buffer);
#endif

    obj_client.SetString("body", buffer);
    arr.PushObject(obj_client);
    json_encode(arr, output, sizeof(output));

    System2HTTPRequest httpRequest = new System2HTTPRequest(PWSupportHTTPResponseCallback, "https://log.pwesports.cn/csgo?key=%s", COMMUNITY_KEY);
    httpRequest.Timeout = 10;
    httpRequest.SetHeader("Content-Type", "application/json;charset=utf-8");
    httpRequest.SetData(output);
    httpRequest.POST();
    delete arr;
    delete obj_topic;
    delete obj_client;
    delete httpRequest;
}

void PWSupportDisconnectRequest(int client)
{
    int timestamp = GetTime();
    static char buffer[PLATFORM_LINE_LENGTH], output[PLATFORM_LINE_LENGTH];

    JSON_Array arr = new JSON_Array();
    JSON_Object obj_topic = new JSON_Object();
    obj_topic.SetString("topic", "log_csgo_3rdparty");
    JSON_Object obj_client = new JSON_Object();
    obj_client.SetObject("headers", obj_topic);
    FormatEx(
        string(buffer),
        "{                                                  \
        \n    \"timestamp\"         :           \"%i\"      \
        \n    \"community ID\"      :           \"%s\"      \
        \n    \"server IP\"         :           \"%s\"      \
        \n    \"type\"              :           \"login\"   \
        \n    \"steamid\"           :           \"%s\"      \
        \n    \"zhCN\"              :           \"%s\"      \
        \n}",
        timestamp, COMMUNITY_ID, gServerData.PublicIP, gClientData[client].Steam64, gClientData[client].PerfectWorld ? "true" : "false"
    );

#if defined _DEBUG_MODE
    LogMessage("[PWSupportConnectRequest] DEBUG: User connected. Data sent: %s", buffer);
#endif

    obj_client.SetString("body", buffer);
    arr.PushObject(obj_client);
    json_encode(arr, output, sizeof(output));

    System2HTTPRequest httpRequest = new System2HTTPRequest(PWSupportHTTPResponseCallback, "https://log.pwesports.cn/csgo?key=%s", COMMUNITY_KEY);
    httpRequest.Timeout = 10;
    httpRequest.SetHeader("Content-Type", "application/json;charset=utf-8");
    httpRequest.SetData(output);
    httpRequest.POST();
    delete arr;
    delete obj_topic;
    delete obj_client;
    delete httpRequest;
}

void PWSupportHTTPResponseCallback(bool success, const char[] error, System2HTTPRequest request, System2HTTPResponse response, HTTPRequestMethod method) 
{
    #pragma unused method

    char url[256];
    request.GetURL(url, sizeof(url));

    if (!success) 
    {
        PrintToServer("ERROR: Couldn't retrieve URL %s. Error: %s", url, error);
        PrintToServer("");
        PrintToServer("INFO: Finished");
        PrintToServer("");

        return;
    }

#if defined _DEBUG_MODE
    response.GetLastURL(url, sizeof(url));

    PrintToServer("INFO: Successfully retrieved URL %s in %.0f milliseconds", url, response.TotalTime * 1000.0);
    PrintToServer("");
    PrintToServer("INFO: HTTP Version: %s", (response.HTTPVersion == VERSION_1_0 ? "1.0" : "1.1"));
    PrintToServer("INFO: Status Code: %d", response.StatusCode);
    PrintToServer("INFO: Downloaded %d bytes with %d bytes/seconds", response.DownloadSize, response.DownloadSpeed);
    PrintToServer("INFO: Uploaded %d bytes with %d bytes/seconds", response.UploadSize, response.UploadSpeed);
    PrintToServer("");
    PrintToServer("INFO: Retrieved the following headers:");

    char name[128];
    char value[128];
    ArrayList headers = response.GetHeaders();

    for (int i = 0; i < headers.Length; i++) {
        headers.GetString(i, name, sizeof(name));
        response.GetHeader(name, value, sizeof(value));
        PrintToServer("\t%s: %s", name, value);
    }
    
    PrintToServer("");
    PrintToServer("INFO: Content (%d bytes):", response.ContentLength);
    PrintToServer("");
    
    char content[128];
    for (int found = 0; found < response.ContentLength;) {
        found += response.GetContent(content, sizeof(content), found);
        PrintToServer(content);
    }

    PrintToServer("");
    PrintToServer("INFO: Finished");
    PrintToServer("");
    
    delete headers;
#endif
}