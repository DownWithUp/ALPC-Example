#include <Windows.h>
#include <winternl.h>
#include <stdio.h>
#include "ntalpcapi.h"
#pragma comment(lib, "ntdll.lib")

#define MSG_LEN 0x100

LPVOID CreateMsgMem(PPORT_MESSAGE PortMessage, SIZE_T MessageSize, LPVOID Message)
{
    /*
        It's important to understand that after the PORT_MESSAGE struct is the message data
    */
    LPVOID lpMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MessageSize + sizeof(PORT_MESSAGE));
    memmove(lpMem, PortMessage, sizeof(PORT_MESSAGE));
    memmove((BYTE*)lpMem + sizeof(PORT_MESSAGE), Message, MessageSize);
    return(lpMem);
}

void main()
{
    UNICODE_STRING  usPort;
    PORT_MESSAGE    pmSend;
    PORT_MESSAGE    pmReceive;
    NTSTATUS        ntRet;
    BOOLEAN         bBreak;
    SIZE_T          nLen;
    HANDLE          hPort;
    LPVOID          lpMem; 
    CHAR            szInput[MSG_LEN];

    printf("ALPC-Example Client\n");
    RtlInitUnicodeString(&usPort, L"\\RPC Control\\NameOfPort");
    RtlSecureZeroMemory(&pmSend, sizeof(pmSend));
    pmSend.u1.s1.DataLength = MSG_LEN;
    pmSend.u1.s1.TotalLength = pmSend.u1.s1.DataLength + sizeof(pmSend);
    lpMem = CreateMsgMem(&pmSend, MSG_LEN, L"Hello World!");
    ntRet = NtAlpcConnectPort(&hPort, &usPort, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL);

    printf("[i] NtAlpcConnectPort: 0x%X\n", ntRet);
    if (!ntRet)
    {
        printf("[i] type 'exit' to disconnect from the server\n");
        bBreak = TRUE;
        while (bBreak)
        {
            RtlSecureZeroMemory(&pmSend, sizeof(pmSend));
            RtlSecureZeroMemory(&szInput, sizeof(szInput));
            printf("[.] Enter Message > ");
            fgets(&szInput, MSG_LEN, stdin);
            pmSend.u1.s1.DataLength = strlen(szInput);
            pmSend.u1.s1.TotalLength = pmSend.u1.s1.DataLength + sizeof(PORT_MESSAGE);
            lpMem = CreateMsgMem(&pmSend, pmSend.u1.s1.DataLength, &szInput);
            ntRet = NtAlpcSendWaitReceivePort(hPort, 0, (PPORT_MESSAGE)lpMem, NULL, NULL, NULL, NULL, NULL);
            printf("[i] NtAlpcSendWaitReceivePort: 0x%X\n", ntRet);
            HeapFree(GetProcessHeap(), 0, lpMem);
        }
    }
    getchar();
    return;
}
