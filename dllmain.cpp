﻿#include "pch.h"

//////////// Hook Data ////////////
HANDLE hConsole;

BOOL(WINAPI* WriteFile_Ptr)(HANDLE hFile, LPCVOID lpBuffer,
    DWORD   nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped) = WriteFile;


//////////// Process ////////////
extern bool OutputProcess(std::string& line);
std::ostringstream buffer;

bool inline SendLine(HANDLE hOutput)
{
    DWORD dwOutput;
    std::string line = buffer.str();
    bool success = true;
    if (OutputProcess(line))
        success = WriteFile_Ptr(hOutput, line.data(), line.size(), &dwOutput, NULL);
    buffer.str("");
    return success;
}

void inline AddText(const char *text, size_t len)
{
    buffer.write(text, len);
}

BOOL WINAPI WriteFile_Process(HANDLE hFile, LPCVOID lpBuffer,
    DWORD   nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped)
{
    if (hFile == hConsole)
    {
        char *fromPos = (char*)lpBuffer;
        int nowLen = 0;
        for (size_t i = 0; i < nNumberOfBytesToWrite; ++i)
        {
            if (fromPos[nowLen++] == '\n')
            {
                AddText(fromPos, nowLen);
                SendLine(hFile);
                fromPos += nowLen;
                nowLen = 0;
            }
        }
        if (nowLen > 0)
            AddText(fromPos, nowLen);
        *lpNumberOfBytesWritten = nNumberOfBytesToWrite;
        return TRUE;
    }
    else
        return WriteFile_Ptr(hFile, lpBuffer, nNumberOfBytesToWrite,
            lpNumberOfBytesWritten, lpOverlapped);
}

//////////// Init ////////////
bool Hook()
{
    bool success = true;
    if ((hConsole = GetStdHandle(STD_OUTPUT_HANDLE)) <= 0)
        success = false;
    else
    {
        DetourRestoreAfterWith();
        if (DetourTransactionBegin() != NO_ERROR)
            success = false;
        else if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR)
            success = false;
        else if (DetourAttach((PVOID*)&WriteFile_Ptr, WriteFile_Process) != NO_ERROR)
            success = false;
        else if (DetourTransactionCommit() != NO_ERROR)
            success = false;
    }
    return success;
}

bool UnHook() {
    bool success = true;
    if (DetourTransactionBegin() != NO_ERROR)
        success = false;
    else if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR)
        success = false;
    else if (DetourDetach((PVOID*)&WriteFile_Ptr, WriteFile_Process) != NO_ERROR)
        success = false;
    else if (DetourTransactionCommit() != NO_ERROR)
        success = false;
    return success;
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (!Hook())
            return false;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        UnHook();
        break;
    }
    return TRUE;
}

