#include <windows.h>
#include "MinHook.h"
#include <cstdio>
#include <vector>
#include <ctype.h>

#pragma comment(lib, "libMinHook.lib")

typedef BOOL (WINAPI* PEEKMESSAGEW)(LPMSG, HWND, UINT, UINT, UINT);

PEEKMESSAGEW fpPeekMessageW = NULL;

struct Message
{
    HWND hWnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
};

std::vector<Message> messages;

BOOL WINAPI DetourPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
    BOOL result = fpPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    if (result != 0) {

        if (lpMsg->message == WM_CHAR) {
            UINT chr = lpMsg->wParam & 65535;
            printf("WM_CHAR hWnd=%p char=%c wParam=0x%llx lParam=0x%llx", lpMsg->hwnd, chr, lpMsg->wParam, lpMsg->lParam);

            if (chr > 255) {

                UINT keycode = VK_PACKET;
                
                // keydown VK_PACKET
                Message msg1 = Message();
                msg1.hWnd = lpMsg->hwnd;
                msg1.message = WM_KEYDOWN;
                msg1.wParam = keycode;
                msg1.lParam = 0x1;
                messages.push_back(msg1);

                // real char
                Message msg2 = Message();
                msg2.hWnd = lpMsg->hwnd;
                msg2.message = WM_CHAR;
                msg2.wParam = chr;
                msg2.lParam = 0x1;
                messages.push_back(msg2);

                // keyup VK_PACKET
                Message msg3 = Message();
                msg3.hWnd = lpMsg->hwnd;
                msg3.message = WM_KEYUP;
                msg3.wParam = keycode;
                msg3.lParam = 0xC0000001;
                messages.push_back(msg3);

                printf(" CANCELLED\n");

                return 0;
            }
            printf("\n");
        }

        if (lpMsg->message == WM_KEYDOWN) {
            printf("WM_KEYDOWN hWnd=%p wParam=0x%llx lParam=0x%llx", lpMsg->hwnd, lpMsg->wParam, lpMsg->lParam);
            printf("\n");
        }

        if (lpMsg->message == WM_KEYUP) {
            printf("WM_KEYUP hWnd=%p wParam=0x%llx lParam=0x%llx", lpMsg->hwnd, lpMsg->wParam, lpMsg->lParam);
            printf("\n");
        }

    }
    else {
        if (messages.size() > 0) {
            Message msg = messages[0];
            lpMsg->hwnd = msg.hWnd;
            lpMsg->message = msg.message;
            lpMsg->wParam = msg.wParam;
            lpMsg->lParam = msg.lParam;

            printf("SEND hWnd=%p message=0x%x wParam=0x%llx lParam=0x%llx\n", lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);

            messages.erase(messages.begin());

            return 1;
        }
    }
    return result;
}

void hook() {

#if _DEBUG

    AllocConsole();
    FILE* fOut;
    freopen_s(&fOut, "conout$", "w", stdout);

#endif // DEBUG

    printf("LunarInputFix by duoduo\n");

    if (MH_Initialize() != MH_OK) {
        printf("initialize error");
        return;
    }

    if (MH_CreateHook(PeekMessageW, &DetourPeekMessageW,
        reinterpret_cast<LPVOID*>(&fpPeekMessageW)) != MH_OK)
    {
        printf("MH_CreateHook error");
        return;
    }

    if (MH_EnableHook(PeekMessageW) != MH_OK)
    {
        printf("MH_EnableHook error");
        return;
    }

    MessageBox(NULL, (LPCWSTR)L"注入成功", (LPCWSTR)L"LunarInputFix by duoduo", 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hook();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

