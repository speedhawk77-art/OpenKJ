// SPDX-License-Identifier: GPL-3.0-or-later
// Launch the official yt-dlp zip application with its private, unmodified CPython.
// Keep this process alive and all descendants inside OpenKJ's cancellation Job Object.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cstdio>
static std::wstring quote(const wchar_t *value) {
    std::wstring out=L"\"";unsigned slashes=0;
    for (const wchar_t *p=value;*p;++p) {
        if (*p==L'\\') {++slashes;continue;}
        out.append(slashes*( *p==L'"' ? 2 : 1),L'\\');slashes=0;
        if (*p==L'"') out+=L'\\';out+=*p;
    }
    out.append(slashes*2,L'\\');return out+L'"';
}
int wmain() {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::vector<wchar_t> path(32768);
    const DWORD length=GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));
    if (!length||length>=path.size()) return 126;
    const std::wstring module(path.data(),length);const auto root=module.substr(0,module.find_last_of(L"\\/"));
    const auto python=root+L"\\python\\python.exe", app=root+L"\\yt-dlp.pyz";
    int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(!argv)return 126;
    std::wstring command=quote(python.c_str())+L" -I "+quote(app.c_str());
    for(int i=1;i<argc;++i)command+=L" "+quote(argv[i]);LocalFree(argv);
    STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESTDHANDLES;
    startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    PROCESS_INFORMATION process{};
    if(!CreateProcessW(python.c_str(),command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)) {
        std::fprintf(stderr,"Could not start bundled Python (Windows error %lu). Reinstall the YouTube helpers.\n",GetLastError());return 126;
    }
    CloseHandle(process.hThread);WaitForSingleObject(process.hProcess,INFINITE);
    DWORD code=126;GetExitCodeProcess(process.hProcess,&code);CloseHandle(process.hProcess);return static_cast<int>(code);
}
