#include "gvc.h"
/* viewonly.cpp */


HINSTANCE hDllInstance;

/* DLL entry point for Borland C++ */
__declspec(dllexport) BOOL WINAPI
DllEntryPoint(HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
{
    hDllInstance = hInst;
    return TRUE;
}

/* DLL entry point for Microsoft Visual C++ */
__declspec(dllexport) BOOL WINAPI
DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
{
    return DllEntryPoint(hInst, fdwReason, lpReserved);
}


/* Exported API */
__declspec(dllexport) BOOL WINAPI
gsview_display_memory(const char *buffer, int len)
{
    psfile.file.SetMemory(buffer, len);
    return gsview_main(hDllInstance, "gsview dummy");
}

