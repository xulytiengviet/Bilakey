// Vikey for Windows: tray icon (V / E), menu, and the message loop the hooks need.
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <windowsx.h>
#include "Core.h"
#include "Prefs.h"
#include "Site.h"
#include "Settings.h"
#include "Updater.h"
#include "Emoji.h"
#include "../../Engine/Vietnamese.h"
#include "resource.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")

static const UINT WM_TRAY = WM_APP + 1;
static const UINT WM_VIKEY_SETLANG = WM_APP + 2;   // tests: wParam = language (0 English, 1 Vietnamese)
static const UINT WM_VIKEY_EMOJI = WM_APP + 5;     // the emoji hotkey, posted from the keyboard hook
static const UINT TIMER_FLUSH = 1, TIMER_SITE = 2;
enum { ID_VIET = 100, ID_ENG, ID_SETTINGS, ID_AUTOSWITCH, ID_UPDATE, ID_LOGTOGGLE, ID_LOG, ID_ABOUT, ID_QUIT, ID_FEEDBACK, ID_GUIDE, ID_CVNSS };

static HWND _hwnd; static NOTIFYICONDATAW _nid = {};
static HICON _iconV, _iconE;

/// Draws the menu-bar glyph: a rounded square with a V or E cut out, like the macOS icon.
static HICON makeIcon(wchar_t letter, bool dark) {
    const int S = 32;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi = {}; bi.bmiHeader.biSize = sizeof(bi.bmiHeader); bi.bmiHeader.biWidth = S; bi.bmiHeader.biHeight = -S; bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32;
    void* bits = nullptr; HBITMAP color = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, color);
    DWORD* px = (DWORD*)bits; for (int i = 0; i < S * S; i++) px[i] = 0;
    COLORREF fg = dark ? RGB(255, 255, 255) : RGB(0, 0, 0);
    HBRUSH b = CreateSolidBrush(fg); HPEN p = CreatePen(PS_NULL, 0, 0);
    SelectObject(dc, b); SelectObject(dc, p);
    RoundRect(dc, 2, 3, S - 2, S - 3, 10, 10);
    HFONT f = CreateFontW(-22, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    SelectObject(dc, f); SetBkMode(dc, TRANSPARENT); SetTextColor(dc, RGB(1, 2, 3));
    RECT r = {0, 1, S, S}; DrawTextW(dc, &letter, 1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    // punch the letter out: pixels drawn in the marker color become transparent; the rest opaque
    for (int i = 0; i < S * S; i++) { DWORD c = px[i] & 0xFFFFFF; if (c == 0x030201) px[i] = 0; else if (c != 0) px[i] |= 0xFF000000; else px[i] = 0; }
    SelectObject(dc, old); DeleteObject(f); DeleteObject(b); DeleteObject(p); DeleteDC(dc);
    HBITMAP mask = CreateBitmap(S, S, 1, 1, nullptr);
    ICONINFO ii = {TRUE, 0, 0, mask, color}; HICON h = CreateIconIndirect(&ii);
    DeleteObject(mask); DeleteObject(color); return h;
}
static bool darkTaskbar() {
    HKEY h; DWORD v = 1, s = sizeof(v);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &h) == ERROR_SUCCESS) { RegQueryValueExW(h, L"SystemUsesLightTheme", nullptr, nullptr, (LPBYTE)&v, &s); RegCloseKey(h); }
    return v == 0;
}
static void updateTray(int lang) {
    _nid.hIcon = lang ? _iconV : _iconE;
    #ifdef BILAKEY_BUILD
    wcscpy_s(_nid.szTip, lang ? L"BilaKey – Tiếng Việt" : L"BilaKey – English");
#else
    wcscpy_s(_nid.szTip, lang ? L"Vikey – Tiếng Việt" : L"Vikey – English");
#endif
    Shell_NotifyIconW(NIM_MODIFY, &_nid);
}
static void toggle(const wchar_t* key, int fallback) { vikey::prefs::setInt(key, !vikey::prefs::getInt(key, fallback)); vikey::core::reloadSettings(); }
static std::wstring hotkeyLabel() {
    int hk = vikey::prefs::getInt(L"SwitchKeyStatus", 0); if (!hk) hk = 0x7A000206;
    std::wstring t; if (hk & 0x100) t += L"Ctrl+"; if (hk & 0x200) t += L"Alt+"; if (hk & 0x800) t += L"Shift+"; if (hk & 0x400) t += L"Win+";
    int k = hk & 0xFF; if (k != 0xFE) { unsigned short ch = keyCodeToCharacter((unsigned int)(k | 0x10000)); if (k == 49) t += L"Space"; else if (ch) t += (wchar_t)ch; } else if (!t.empty()) t.pop_back();
    return t;
}
static void showMenu() {
    using namespace vikey;
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING | (core::language() ? MF_CHECKED : 0), ID_VIET, (L"Tiếng Việt\t" + hotkeyLabel()).c_str());
    AppendMenuW(m, MF_STRING | (prefs::getInt(L"AutoSwitchByTyping", 1) ? MF_CHECKED : 0), ID_AUTOSWITCH, L"Tự chuyển Anh/Việt");
#ifdef BILAKEY_BUILD
    AppendMenuW(m, MF_STRING | (prefs::getInt(L"BilaKey3AutoDetect", 1) ? MF_CHECKED : 0), ID_CVNSS, L"Tự nhận diện CVNSS4.0");
#endif
    AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m, MF_STRING, ID_SETTINGS, L"Mở Vikey…");
    AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m, MF_STRING, ID_GUIDE, L"Hướng dẫn…");
    AppendMenuW(m, MF_STRING, ID_FEEDBACK, L"Gửi góp ý…");
    #ifndef BILAKEY_BUILD
    AppendMenuW(m, MF_STRING, ID_UPDATE, L"Kiểm tra cập nhật…");
#endif
    AppendMenuW(m, MF_STRING, ID_ABOUT, L"Giới thiệu Vikey");
    HMENU dbg = CreatePopupMenu();
    AppendMenuW(dbg, MF_STRING | (prefs::getInt(L"TypingLogEnabled", 0) ? MF_CHECKED : 0), ID_LOGTOGGLE, L"Ghi nhật ký gõ");
    AppendMenuW(dbg, MF_STRING | (prefs::getInt(L"TypingLogEnabled", 0) ? 0 : MF_GRAYED), ID_LOG, L"Xem nhật ký gõ…");
    AppendMenuW(m, MF_POPUP, (UINT_PTR)dbg, L"Gỡ lỗi");
    AppendMenuW(m, MF_STRING, ID_QUIT, L"Thoát Vikey");
    POINT pt; GetCursorPos(&pt); SetForegroundWindow(_hwnd);
    TrackPopupMenu(m, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, _hwnd, nullptr);
    DestroyMenu(m);
}
static LRESULT CALLBACK wndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    using namespace vikey;
    switch (msg) {
    case WM_TRAY:
        if (LOWORD(lp) == WM_LBUTTONUP) ui::showSettings(GetModuleHandleW(nullptr), false);
        else if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_CONTEXTMENU) showMenu();
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_VIET: core::setLanguageByUser(!core::language()); break;
        case ID_AUTOSWITCH: toggle(L"AutoSwitchByTyping", 1); break;
    case ID_CVNSS: toggle(L"BilaKey3AutoDetect", 1); break;
        case ID_SETTINGS: ui::showSettings(GetModuleHandleW(nullptr), false); break;
        case ID_UPDATE: updater::checkNow(h); break;
        case ID_ABOUT: ui::showAbout(GetModuleHandleW(nullptr)); break;
        case ID_FEEDBACK: ShellExecuteW(nullptr, L"open", L"https://vikey.app/feedback?platform=windows&v=" VIKEY_VERSION_W, nullptr, nullptr, SW_SHOWNORMAL); break;
        case ID_GUIDE: ui::showOnboarding(GetModuleHandleW(nullptr)); break;
        case ID_LOGTOGGLE: toggle(L"TypingLogEnabled", 0); break;
        case ID_LOG: ui::showTypingLog(GetModuleHandleW(nullptr)); break;
        case ID_QUIT:
#ifndef BILAKEY_BUILD
            updater::installPendingAndQuit();
#endif
            DestroyWindow(h); break;
        }
        return 0;
    case WM_APP + 3: ui::showSettings(GetModuleHandleW(nullptr), false); return 0;
    case updater::WM_UPDATER: updater::onEvent(wp); return 0;
    case WM_VIKEY_SETLANG: core::setLanguageQuiet((int)wp); return 1;
    case WM_VIKEY_EMOJI: emoji::toggle(); return 0;
    case WM_TIMER: if (wp == TIMER_FLUSH) core::flushTypingLog(); else if (wp == TIMER_SITE) vikey::site::poll(); return 0;
    case WM_SETTINGCHANGE: { bool dark = darkTaskbar(); DestroyIcon(_iconV); DestroyIcon(_iconE); _iconV = makeIcon(L'V', dark); _iconE = makeIcon(L'E', dark); updateTray(core::language()); return 0; }
    case WM_DESTROY: core::stop(); Shell_NotifyIconW(NIM_DELETE, &_nid); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
    HANDLE single = CreateMutexW(nullptr, TRUE, 
#ifdef BILAKEY_BUILD
        L"Local\\BilaKeyRunning"
#else
        L"Local\\VikeyRunning"
#endif
        );
    if (GetLastError() == ERROR_ALREADY_EXISTS) {   // already running: opening the app again shows its window, like on macOS
        HWND t = FindWindowW(L"VikeyTray", nullptr);
        if (t) { DWORD pid = 0; GetWindowThreadProcessId(t, &pid); AllowSetForegroundWindow(pid); PostMessageW(t, WM_APP + 3, 0, 0); }   // our foreground right goes to the running Vikey: its window comes to the front
        return 0;
    }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    #ifndef BILAKEY_BUILD
    if (vikey::updater::installPendingAtLaunch()) return 0;   // an update downloaded earlier installs now and relaunches Vikey
#endif
    vikey::prefs::migrateFromOpenKey();
    WNDCLASSW wc = {}; wc.lpfnWndProc = wndProc; wc.hInstance = inst; wc.lpszClassName = L"VikeyTray";
    RegisterClassW(&wc);
    _hwnd = CreateWindowW(L"VikeyTray", L"Vikey", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, inst, nullptr);
    bool dark = darkTaskbar(); _iconV = makeIcon(L'V', dark); _iconE = makeIcon(L'E', dark);
    _nid.cbSize = sizeof(_nid); _nid.hWnd = _hwnd; _nid.uID = 1; _nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; _nid.uCallbackMessage = WM_TRAY; _nid.hIcon = _iconV;
    #ifdef BILAKEY_BUILD
    wcscpy_s(_nid.szTip, L"BilaKey");
#else
    wcscpy_s(_nid.szTip, L"Vikey");
#endif
    Shell_NotifyIconW(NIM_ADD, &_nid);
    vikey::core::setLanguageObserver([](int lang) { updateTray(lang); vikey::ui::languageChanged(lang); });
    if (!vikey::core::start()) { MessageBoxW(nullptr, L"Không cài được bộ bắt phím (SetWindowsHookEx).", L"Vikey", MB_ICONERROR); return 1; }
    updateTray(vikey::core::language());
    vikey::emoji::install(inst);
    vikey::core::setEmojiHotkeyHandler([] { PostMessageW(_hwnd, WM_VIKEY_EMOJI, 0, 0); });   // not inside the hook: opening can take a moment
    if (!vikey::prefs::has(L"FirstRunDone")) vikey::ui::showSettings(inst, true);
    SetTimer(_hwnd, TIMER_FLUSH, 10000, nullptr);
    #ifndef BILAKEY_BUILD
    vikey::updater::start(_hwnd);
#endif
    SetTimer(_hwnd, TIMER_SITE, 1000, nullptr);   // per-website memory: the active tab can change without any event we see
    MSG msg; while (GetMessageW(&msg, nullptr, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    ReleaseMutex(single); CloseHandle(single);
    return 0;
}
