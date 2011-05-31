/* gvwlang.c */
/* put some code in the DLL to keep the linker happy */

#include <windows.h>
#ifdef _MSC_VER
#define _export
#endif

#ifdef __WIN32__
BOOL WINAPI _export
DllEntryPoint(HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}


#else
int WINAPI _export
LibMain(HINSTANCE hInstance, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine)
{
	return 1;
}

int WINAPI _export
WEP(int nParam)
{
	return 1;
}
#endif

