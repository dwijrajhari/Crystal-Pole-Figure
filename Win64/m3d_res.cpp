#include "resource.h"
#include <Windows.h>

HMODULE fGetCurrentModule()
{
    HMODULE hmodule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)fGetCurrentModule, &hmodule);
    return hmodule;
}

unsigned long fGetResourceSize(int resname_, int restype_)
{
    HRSRC fileRes = FindResource(fGetCurrentModule(), MAKEINTRESOURCE(resname_), MAKEINTRESOURCE(restype_));
    if (fileRes)
    {
        return SizeofResource(fGetCurrentModule(), fileRes);
    }
    else return 0;
}

void fGetResourceData(int resname_, int restype_, char* data_)
{
    HRSRC fileRes = FindResource(fGetCurrentModule(), MAKEINTRESOURCE(resname_), MAKEINTRESOURCE(restype_));
    if (fileRes)
    {
        HGLOBAL file = LoadResource(fGetCurrentModule(), fileRes);
        if (file)
        {
            char* filedata = (char*)LockResource(file);
            DWORD filesize = SizeofResource(fGetCurrentModule(), fileRes);
            memcpy(data_, filedata, filesize);
        }
    }
}