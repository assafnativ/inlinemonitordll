#include <windows.h>
#include <cstdio>
#include <detours.h>
#include "monitor_io.h"
#include "globals.h"

void installHooks()
{
    auto targets = getTargetsIO();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    for (auto target : targets) {
        void ** targetProc = new (void *)(std::get<0>(target));
        DetourAttach(targetProc, std::get<1>(target));
        hooked[std::get<1>(target)] = targetProc;
    }
    DetourTransactionCommit();
}

void uninstallHooks()
{
    auto targets = getTargetsIO();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    for (auto target : targets) {
        void * targetProc = std::get<0>(target);
        DetourDetach(&targetProc, std::get<1>(target));
        delete hooked[std::get<1>(target)];
        hooked[std::get<1>(target)] = nullptr;
    }
    DetourTransactionCommit();
}

void foo_bar()
{

}

BOOL APIENTRY DllMain(HMODULE,
                      DWORD  ul_reason_for_call,
                      LPVOID)
{
    while (!IsDebuggerPresent()) {
        Sleep(0);
    }

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        if (nullptr == g_log) {
            char fileName[0x100];
            std::sprintf(fileName, "c:\\temp\\pid%08d.log", GetCurrentProcessId());
            g_log = std::fopen(fileName, "w");
        }
        installHooks();
        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH: {
        uninstallHooks();
        if (nullptr != g_log) {
            std::fclose(g_log);
            g_log = nullptr;
        }
        break;
    }
    }
    return TRUE;
}

