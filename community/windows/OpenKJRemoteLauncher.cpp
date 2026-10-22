// OpenKJ Offline Remote portable launcher (Windows x64)
// Copyright (c) 2026 - MIT-licensed small bootstrap utility.
// This program changes environment variables ONLY in its own process and the
// OpenKJ child process. It does not change Windows machine/user settings.
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

namespace {

bool exists(const std::wstring &p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}
bool isDir(const std::wstring &p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

void error(const std::wstring &message) {
    MessageBoxW(nullptr, message.c_str(), L"OpenKJ Remote — Startup problem", MB_ICONERROR | MB_OK);
}

std::wstring getEnv(const wchar_t *key) {
    DWORD count = GetEnvironmentVariableW(key, nullptr, 0);
    if (count == 0) return L"";
    std::vector<wchar_t> val(count + 1, L'\0');
    DWORD got = GetEnvironmentVariableW(key, val.data(), static_cast<DWORD>(val.size()));
    if (got == 0 || got >= val.size()) return L"";
    return std::wstring(val.data(), got);
}

bool setEnv(const wchar_t *key, const std::wstring &val) {
    return SetEnvironmentVariableW(key, val.c_str()) != 0;
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    std::vector<wchar_t> module(32768, L'\0');
    DWORD count = GetModuleFileNameW(nullptr, module.data(), static_cast<DWORD>(module.size()));
    if (count == 0 || count >= module.size()) {
        error(L"Unable to locate this launcher on disk.");
        return 1;
    }
    std::wstring launcherPath(module.data(), count);
    const size_t cut = launcherPath.find_last_of(L"\\/");
    if (cut == std::wstring::npos) {
        error(L"Could not determine the application directory.");
        return 1;
    }
    const std::wstring root = launcherPath.substr(0, cut + 1);
    const std::wstring app = root + L"openkj.exe";
    const std::wstring qtPlatform = root + L"platforms";
    const std::wstring gstPlugins = root + L"gst-plugins";
    const std::wstring gstScanner = root + L"gst-plugin-scanner.exe";
    const std::wstring gioModules = root + L"gio-modules";

    if (!exists(app)) {
        error(L"openkj.exe was not found in the same folder as OpenKJ-Remote.exe.\n\n"
              L"Copy the ENTIRE portable OpenKJ folder, not just the launcher.");
        return 2;
    }
    if (!exists(root + L"Qt5Core.dll") || !exists(root + L"libgstreamer-1.0-0.dll") ||
        !exists(qtPlatform + L"\\qwindows.dll") || !isDir(gstPlugins)) {
        error(L"Some packaged Qt or GStreamer components are missing.\n\n"
              L"Make sure you're using the OpenKJ-Portable folder built by the portable packager.\n"
              L"Do not copy just openkj.exe or OpenKJ-Remote.exe.");
        return 3;
    }

    // Put bundled DLLs ahead of any other Qt/GStreamer installation on this PC.
    // Win32 resolves the app's load-time DLLs from its own directory; the PATH
    // also lets any *runtime-loaded plugins* find their DLL dependencies.
    const std::wstring path = root + L";" + getEnv(L"PATH");
    if (!setEnv(L"PATH", path) ||
        !setEnv(L"QT_QPA_PLATFORM_PLUGIN_PATH", qtPlatform) ||
        !setEnv(L"QT_PLUGIN_PATH", root) ||
        !setEnv(L"GST_PLUGIN_PATH_1_0", gstPlugins) ||
        !setEnv(L"GST_PLUGIN_SYSTEM_PATH_1_0", gstPlugins)) {
        error(L"Unable to configure the portable runtime environment.");
        return 4;
    }

    if (exists(gstScanner)) {
        setEnv(L"GST_PLUGIN_SCANNER", gstScanner);
        setEnv(L"GST_PLUGIN_SCANNER_1_0", gstScanner);
    }
    if (isDir(gioModules)) setEnv(L"GIO_EXTRA_MODULES", gioModules);

    // Passing the application path separately from the command line avoids
    // errors with installation paths that contain spaces.
    std::wstring cmdline = L"\"" + app + L"\"";
    std::vector<wchar_t> mutableCmd(cmdline.begin(), cmdline.end());
    mutableCmd.push_back(L'\0');
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(app.c_str(), mutableCmd.data(), nullptr, nullptr,
                             FALSE, 0, nullptr, root.c_str(), &si, &pi);
    if (!ok) {
        error(L"Windows could not start the packaged OpenKJ application.\n\n"
              L"Error code: " + std::to_wstring(GetLastError()) +
              L"\n\nCheck that the full portable folder was copied.");
        return 5;
    }

    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD childResult = 0;
    GetExitCodeProcess(pi.hProcess, &childResult);
    CloseHandle(pi.hProcess);
    return static_cast<int>(childResult);
}
