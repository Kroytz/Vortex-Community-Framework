bool AddMenuItemEx(Menu menu, int style, const char[] info, const char[] display, any ...)
{
    char m_szBuffer[256];
    VFormat(m_szBuffer, 256, display, 5);
    return menu.AddItem(info, m_szBuffer, style);
}