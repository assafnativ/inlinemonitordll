#include "monitor_io.h"
#include "globals.h"

#include <mutex>

using namespace std;
__declspec(thread) uint8_t g_nesting = 0;

HANDLE WINAPI MyCreateFileW(
    __in      LPCWSTR lpFileName,
    __in      DWORD dwDesiredAccess,
    __in      DWORD dwShareMode,
    __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in      DWORD dwCreationDisposition,
    __in      DWORD dwFlagsAndAttributes,
    __in_opt  HANDLE hTemplateFile)
{
    auto original = reinterpret_cast <decltype(MyCreateFileW) **>
                    (hooked[reinterpret_cast<void *>(MyCreateFileW)]);
    HANDLE result = (*original)(lpFileName, dwDesiredAccess, dwShareMode,
                                lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
                                hTemplateFile);

    g_nesting++;
    try {
        do {
            if (1 < g_nesting) {
                break;
            }
            fprintf(g_log, "CreateFile of %S\n", lpFileName);
        } while (false);
    } catch (...) {

    }
    g_nesting--;
    return result;
}

HookingTarget & getTargetsIO()
{
    static HookingTarget hookingTargets{};
    static mutex criticalProc{};

    lock_guard<mutex> criticalProcLock(criticalProc);

    if (!hookingTargets.empty()) {
        return hookingTargets;
    }

    hookingTargets.push_back(make_tuple<void *, void *>(
                                 reinterpret_cast<void *>(CreateFileW),
                                 reinterpret_cast<void *>(MyCreateFileW)));

    return hookingTargets;
}