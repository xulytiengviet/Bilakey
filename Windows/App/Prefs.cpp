#include "Prefs.h"
#include <shlobj.h>

namespace vikey { namespace prefs {

#ifdef BILAKEY_BUILD
static const wchar_t* ROOT = L"Software\\BilaKey";
static const wchar_t* APP = L"BilaKey";
#else
static const wchar_t* ROOT = L"Software\\Vikey";
static const wchar_t* APP = L"Vikey";
#endif

static HKEY open(REGSAM access) {
    HKEY h = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, ROOT, 0, nullptr, REG_OPTION_NON_VOLATILE, access, nullptr, &h, nullptr) != ERROR_SUCCESS) return nullptr;
    return h;
}

bool has(const wchar_t* key) {
    HKEY h = open(KEY_READ); if (!h) return false;
    DWORD type = 0, size = 0;
    bool ok = RegQueryValueExW(h, key, nullptr, &type, nullptr, &size) == ERROR_SUCCESS;
    RegCloseKey(h); return ok;
}

int getInt(const wchar_t* key, int fallback) {
    HKEY h = open(KEY_READ); if (!h) return fallback;
    DWORD v = 0, size = sizeof(v), type = 0;
    int r = fallback;
    if (RegQueryValueExW(h, key, nullptr, &type, (LPBYTE)&v, &size) == ERROR_SUCCESS && type == REG_DWORD) r = (int)v;
    RegCloseKey(h); return r;
}

void setInt(const wchar_t* key, int value) {
    HKEY h = open(KEY_WRITE); if (!h) return;
    DWORD v = (DWORD)value;
    RegSetValueExW(h, key, 0, REG_DWORD, (const BYTE*)&v, sizeof(v));
    RegCloseKey(h);
}

std::vector<BYTE> getBinary(const wchar_t* key) {
    std::vector<BYTE> out;
    HKEY h = open(KEY_READ); if (!h) return out;
    DWORD size = 0, type = 0;
    if (RegQueryValueExW(h, key, nullptr, &type, nullptr, &size) == ERROR_SUCCESS && size > 0) {
        out.resize(size);
        if (RegQueryValueExW(h, key, nullptr, &type, out.data(), &size) != ERROR_SUCCESS) out.clear();
    }
    RegCloseKey(h); return out;
}

void setBinary(const wchar_t* key, const BYTE* data, DWORD size) {
    HKEY h = open(KEY_WRITE); if (!h) return;
    RegSetValueExW(h, key, 0, REG_BINARY, data, size);
    RegCloseKey(h);
}

std::wstring getString(const wchar_t* key, const std::wstring& fallback) {
    HKEY h = open(KEY_READ); if (!h) return fallback;
    wchar_t buf[1024]; DWORD size = sizeof(buf), type = 0;
    std::wstring r = fallback;
    if (RegQueryValueExW(h, key, nullptr, &type, (LPBYTE)buf, &size) == ERROR_SUCCESS && type == REG_SZ) r.assign(buf, size / 2 ? size / 2 - 1 : 0);
    RegCloseKey(h); return r;
}

void setString(const wchar_t* key, const std::wstring& value) {
    HKEY h = open(KEY_WRITE); if (!h) return;
    RegSetValueExW(h, key, 0, REG_SZ, (const BYTE*)value.c_str(), (DWORD)((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(h);
}

void remove(const wchar_t* key) {
    HKEY h = open(KEY_WRITE); if (!h) return;
    RegDeleteValueW(h, key); RegCloseKey(h);
}

std::wstring appDataDir() {
    wchar_t path[MAX_PATH] = {0};
    SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);
    std::wstring dir = std::wstring(path) + (std::wstring(L"\\") + APP);
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir;
}

static const wchar_t* RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

bool launchAtLogin() {
    HKEY h; if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_READ, &h) != ERROR_SUCCESS) return false;
    DWORD size = 0; bool ok = RegQueryValueExW(h, APP, nullptr, nullptr, nullptr, &size) == ERROR_SUCCESS;
    RegCloseKey(h); return ok;
}

void setLaunchAtLogin(bool on) {
    HKEY h; if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_WRITE, &h) != ERROR_SUCCESS) return;
    if (on) {
        wchar_t exe[MAX_PATH]; GetModuleFileNameW(nullptr, exe, MAX_PATH);
        std::wstring cmd = L"\"" + std::wstring(exe) + L"\"";
        RegSetValueExW(h, APP, 0, REG_SZ, (const BYTE*)cmd.c_str(), (DWORD)((cmd.size() + 1) * sizeof(wchar_t)));
    } else RegDeleteValueW(h, APP);
    RegCloseKey(h);
}

void migrateFromOpenKey() {
    if (has(L"MigratedFromOpenKey")) return;
    setInt(L"MigratedFromOpenKey", 1);
    HKEY h; if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\TuyenMai\\OpenKey", 0, KEY_READ, &h) != ERROR_SUCCESS) return;
    struct { const wchar_t* from; const wchar_t* to; } map[] = {
        {L"vLanguage", L"InputMethod"}, {L"vCheckSpelling", L"Spelling"}, {L"vRestoreIfWrongSpelling", L"RestoreIfInvalidWord"},
        {L"vUseModernOrthography", L"ModernOrthography"}, {L"vUseSmartSwitchKey", L"UseSmartSwitchKey"}, {L"vSwitchKeyStatus", L"SwitchKeyStatus"},
    };
    for (auto& m : map) {
        DWORD v = 0, size = sizeof(v), type = 0;
        if (RegQueryValueExW(h, m.from, nullptr, &type, (LPBYTE)&v, &size) == ERROR_SUCCESS && type == REG_DWORD && !has(m.to)) setInt(m.to, (int)v);
    }
    DWORD size = 0, type = 0;
    if (RegQueryValueExW(h, L"smartSwitchKey", nullptr, &type, nullptr, &size) == ERROR_SUCCESS && size > 0 && !has(L"smartSwitchKey")) {
        std::vector<BYTE> d(size);
        if (RegQueryValueExW(h, L"smartSwitchKey", nullptr, &type, d.data(), &size) == ERROR_SUCCESS) setBinary(L"smartSwitchKey", d.data(), size);
    }
    RegCloseKey(h);
}

}} // namespace
