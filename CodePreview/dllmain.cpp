/****************************** Module Header ******************************\
The file implements DllMain, and the DllGetClassObject, DllCanUnloadNow,
DllRegisterServer, DllUnregisterServer functions that are necessary for a COM
DLL.

DllGetClassObject invokes the class factory defined in ClassFactory.h/cpp and
queries to the specific interface.

DllCanUnloadNow checks if we can unload the component from the memory.

DllRegisterServer registers the COM server and the preview handler in the
registry by invoking the helper functions defined in Register.h/cpp.
The preview handler is associated with the .cpp/.h file class.

DllUnregisterServer unregisters the COM server and the preview handler.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include <windows.h>
#include <Guiddef.h>
#include "ClassFactory.h"           // For the class factory
#include "Register.h"


// {DA832011-9B0F-40AE-892C-9494BB32B8B2}
const CLSID CLSID_CodePreviewHandler =
{ 0xDA832011, 0x9B0F, 0x40AE,{ 0x89, 0x2C, 0x94, 0x94, 0xBB, 0x32, 0xB8, 0xB2 } };

// {CD5C966F-C810-48C1-8B28-35FD17AFA5DA}
//const GUID APPID_CodePreviewHandler =
//{ 0xCD5C966F, 0xC810, 0x48C1,{ 0x8B, 0x28, 0x35, 0xFD, 0x17, 0xAF, 0xA5, 0xDA } };

#define SZ_CODEPREVIEWHANDLER         L"Code Preview Handler"
#define SZ_CLSID_CODEPREVIEWHANDLER   L"{DA832011-9B0F-40AE-892C-9494BB32B8B2}"
// the id of prevhost.exe that will host this preview handler (this is the surogate host)
#define SZ_CLSID_HOSTAPP              L"{6D2B5079-2F0B-48DD-AB7F-97CEC514D30B}"
// {DA832011-9B0F-40AE-892C-9494BB32B8B2}



#define SZ_CODEPREVIEWHANDLER         L"Code Preview Handler"
//#define SZ_CLSID_CODEPREVIEWHANDLER   L"{DA832011-9B0F-40AE-892C-9494BB32B8B2}"
// the id of prevhost.exe that will host this preview handler (this is the surogate host)
#define SZ_CLSID_HOSTAPP              L"{6D2B5079-2F0B-48DD-AB7F-97CEC514D30B}"


HINSTANCE   g_hInst = NULL;
long        g_cDllRef = 0;

#define TRACE

#ifdef TRACE
void DebugPrintf(const wchar_t *format, ...) {
	wchar_t buffer[2048];
	va_list pArguments;
	va_start(pArguments, format);
	wvsprintfW(buffer, format, pArguments);
	va_end(pArguments);
	::OutputDebugStringW(buffer);
}
#else
void DebugPrintf(const wchar_t *, ...) {
}
#endif

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	::DebugPrintf(L"CodePreview::DllMain(hModule = [0x%X], dwReason = [%d])\r\n", hModule, dwReason);
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Hold the instance of this DLL module, we will use it to get the 
		// path of the DLL to register the component.
		g_hInst = hModule;
		DisableThreadLibraryCalls(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//
//   FUNCTION: DllGetClassObject
//
//   PURPOSE: Create the class factory and query to the specific interface.
//
//   PARAMETERS:
//   * rclsid - The CLSID that will associate the correct data and code.
//   * riid - A reference to the identifier of the interface that the caller 
//     is to use to communicate with the class object.
//   * ppv - The address of a pointer variable that receives the interface 
//     pointer requested in riid. Upon successful return, *ppv contains the 
//     requested interface pointer. If an error occurs, the interface pointer 
//     is NULL. 
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(CLSID_CodePreviewHandler, rclsid))
	{
		hr = E_OUTOFMEMORY;

		ClassFactory *pClassFactory = new ClassFactory();
		if (pClassFactory)
		{
			hr = pClassFactory->QueryInterface(riid, ppv);
			pClassFactory->Release();
		}
	}

	return hr;
}


//
//   FUNCTION: DllCanUnloadNow
//
//   PURPOSE: Check if we can unload the component from the memory.
//
//   NOTE: The component can be unloaded from the memory when its reference 
//   count is zero (i.e. nobody is still using the component).
// 
STDAPI DllCanUnloadNow(void)
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}

/*
//
//   FUNCTION: DllRegisterServer
//
//   PURPOSE: Register the COM server and the context menu handler.
// 
STDAPI DllRegisterServer(void)
{
	HRESULT hr;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Register the component.
	hr = RegisterInprocServer(szModule, CLSID_CodePreviewHandler,
		L"corba.CodePreviewHandler Class",
		L"Apartment", APPID_CodePreviewHandler);
	if (SUCCEEDED(hr))
	{
		// Register the preview handler. The preview handler is associated
		// with the .code file class.
		hr = RegisterShellExtPreviewHandler(L".code", CLSID_CodePreviewHandler, L"Cornel's Code Preview Handler");
	}

	return hr;
}


//
//   FUNCTION: DllUnregisterServer
//
//   PURPOSE: Unregister the COM server and the context menu handler.
// 
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = S_OK;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Unregister the component.
	hr = UnregisterInprocServer(CLSID_CodePreviewHandler, APPID_CodePreviewHandler);
	if (SUCCEEDED(hr))
	{
		// Unregister the context menu handler.
		hr = UnregisterShellExtPreviewHandler(L".code", CLSID_CodePreviewHandler);
	}

	return hr;
}*/










// A struct to hold the information required for a registry entry

struct REGISTRY_ENTRY
{
	HKEY   hkeyRoot;
	PCWSTR pszKeyName;
	PCWSTR pszValueName;
	PCWSTR pszData;
};

// Creates a registry key (if needed) and sets the default value of the key

HRESULT CreateRegKeyAndSetValue(const REGISTRY_ENTRY *pRegistryEntry)
{
	HKEY hKey;
	HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyExW(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL));
	if (SUCCEEDED(hr))
	{
		hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, pRegistryEntry->pszValueName, 0, REG_SZ,
			(LPBYTE)pRegistryEntry->pszData,
			((DWORD)wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR)));
		RegCloseKey(hKey);
	}
	return hr;
}

//
// Registers this COM server
//
STDAPI DllRegisterServer()
{
	HRESULT hr;

	WCHAR szModuleName[MAX_PATH];

	if (!GetModuleFileNameW(g_hInst, szModuleName, ARRAYSIZE(szModuleName)))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		// List of registry entries we want to create

		const REGISTRY_ENTRY rgRegistryEntries[] =
		{
			// RootKey            KeyName                                                                                            ValueName                       Data
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                                        NULL,                           SZ_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                                        L"DisplayName",                 L"Cornel's Code Preview Handler" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",                    NULL,                           szModuleName },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",                    L"ThreadingModel",              L"Apartment" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                                        L"AppID",                       SZ_CLSID_HOSTAPP },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                           NULL,                           SZ_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                           L"DisplayName",                 L"Cornel's Code Preview Handler" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",       NULL,                           szModuleName },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",       L"ThreadingModel",              L"Apartment" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                           L"AppID",                       SZ_CLSID_HOSTAPP },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\.code\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",                     NULL,                           SZ_CLSID_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\.cpp\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",                      NULL,                           SZ_CLSID_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\.h\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",                        NULL,                           SZ_CLSID_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\.txt\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",                        NULL,                           SZ_CLSID_CODEPREVIEWHANDLER },
			{ HKEY_LOCAL_MACHINE,  L"Software\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers",                                 SZ_CLSID_CODEPREVIEWHANDLER,    L"Cornel's Code Preview Handler" },
		};

		hr = S_OK;
		for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
		{
			hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);
		}
	}
	return hr;
}

//
// Unregisters this COM server
//
STDAPI DllUnregisterServer()
{
	HRESULT hr = S_OK;

	const PCWSTR rgpszKeys[] =
	{
		L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,
		L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,
		L"Software\\Classes\\.code\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",
		L"Software\\Classes\\.cpp\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",
		L"Software\\Classes\\.h\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",
		L"Software\\Classes\\.txt\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}"
	};

	// Delete the registry entries
	for (int i = 0; i < ARRAYSIZE(rgpszKeys) && SUCCEEDED(hr); i++)
	{
		hr = HRESULT_FROM_WIN32(RegDeleteTreeW(HKEY_CURRENT_USER, rgpszKeys[i]));
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			// If the registry entry has already been deleted, say S_OK.
			hr = S_OK;
		}
	}
	if (SUCCEEDED(hr))
	{
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
		{
			RegDeleteValue(hKey, SZ_CLSID_CODEPREVIEWHANDLER);
			RegCloseKey(hKey);
		}
	}
	return hr;
}























/** Old registration

#include <objbase.h>
#include <shlwapi.h>
#include <new>

extern HRESULT CCodePreviewHandler_CreateInstance(REFIID riid, void **ppv);

#define SZ_CODEPREVIEWHANDLER         L"Code Preview Handler"
#define SZ_CLSID_CODEPREVIEWHANDLER   L"{DA832011-9B0F-40AE-892C-9494BB32B8B2}"
// the id of prevhost.exe that will host this preview handler (this is the surogate host)
#define SZ_CLSID_HOSTAPP              L"{6D2B5079-2F0B-48DD-AB7F-97CEC514D30B}"
// {DA832011-9B0F-40AE-892C-9494BB32B8B2}
const CLSID CLSID_CodePreviewHandler = { 0xDA832011, 0x9B0F, 0x40AE,{ 0x89, 0x2C, 0x94, 0x94, 0xBB, 0x32, 0xB8, 0xB2 } };

typedef HRESULT (*PFNCREATEINSTANCE)(REFIID riid, void **ppvObject);
struct CLASS_OBJECT_INIT
{
    const CLSID *pClsid;
    PFNCREATEINSTANCE pfnCreate;
};

// add classes supported by this module here
const CLASS_OBJECT_INIT c_rgClassObjectInit[] =
{
    { &CLSID_CodePreviewHandler, CCodePreviewHandler_CreateInstance }
};


long g_cRefModule = 0;

// Handle the the DLL's module
HINSTANCE g_hInst = NULL;

// Standard DLL functions
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    // Only allow the DLL to be unloaded after all outstanding references have been released
    return (g_cRefModule == 0) ? S_OK : S_FALSE;
}

void DllAddRef()
{
    InterlockedIncrement(&g_cRefModule);
}

void DllRelease()
{
    InterlockedDecrement(&g_cRefModule);
}

class CClassFactory : public IClassFactory
{
public:
    static HRESULT CreateInstance(REFCLSID clsid, const CLASS_OBJECT_INIT *pClassObjectInits, size_t cClassObjectInits, REFIID riid, void **ppv)
    {
        *ppv = NULL;
        HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
        for (size_t i = 0; i < cClassObjectInits; i++)
        {
            if (clsid == *pClassObjectInits[i].pClsid)
            {
                IClassFactory *pClassFactory = new (std::nothrow) CClassFactory(pClassObjectInits[i].pfnCreate);
                hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
                if (SUCCEEDED(hr))
                {
                    hr = pClassFactory->QueryInterface(riid, ppv);
                    pClassFactory->Release();
                }
                break; // match found
            }
        }
        return hr;
    }

    CClassFactory(PFNCREATEINSTANCE pfnCreate) : _cRef(1), _pfnCreate(pfnCreate)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CClassFactory, IClassFactory),
            { 0 }
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        return punkOuter ? CLASS_E_NOAGGREGATION : _pfnCreate(riid, ppv);
    }

    IFACEMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }

private:
    ~CClassFactory()
    {
        DllRelease();
    }

    long _cRef;
    PFNCREATEINSTANCE _pfnCreate;
};


STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    return CClassFactory::CreateInstance(clsid, c_rgClassObjectInit, ARRAYSIZE(c_rgClassObjectInit), riid, ppv);
}

// A struct to hold the information required for a registry entry

struct REGISTRY_ENTRY
{
    HKEY   hkeyRoot;
    PCWSTR pszKeyName;
    PCWSTR pszValueName;
    PCWSTR pszData;
};

// Creates a registry key (if needed) and sets the default value of the key

HRESULT CreateRegKeyAndSetValue(const REGISTRY_ENTRY *pRegistryEntry)
{
    HKEY hKey;
    HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyExW(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName,
                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL));
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, pRegistryEntry->pszValueName, 0, REG_SZ,
                            (LPBYTE) pRegistryEntry->pszData,
                            ((DWORD) wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR)));
        RegCloseKey(hKey);
    }
    return hr;
}

//
// Registers this COM server
//
STDAPI DllRegisterServer()
{
    HRESULT hr;

    WCHAR szModuleName[MAX_PATH];

    if (!GetModuleFileNameW(g_hInst, szModuleName, ARRAYSIZE(szModuleName)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        // List of registry entries we want to create

        const REGISTRY_ENTRY rgRegistryEntries[] =
        {
            // RootKey            KeyName                                                                                            ValueName                       Data
            { HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                                        NULL,                           SZ_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                                        L"DisplayName",                 L"Cornel's Code Preview Handler" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",                    NULL,                           szModuleName},
            { HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",                    L"ThreadingModel",              L"Apartment"},
            { HKEY_CURRENT_USER,   L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                                        L"AppID",                       SZ_CLSID_HOSTAPP },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                           NULL,                           SZ_CODEPREVIEWHANDLER },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                           L"DisplayName",                 L"Cornel's Code Preview Handler" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",       NULL,                           szModuleName },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER L"\\InProcServer32",       L"ThreadingModel",              L"Apartment" },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,                           L"AppID",                       SZ_CLSID_HOSTAPP },
			{ HKEY_CURRENT_USER,   L"Software\\Classes\\.code\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}",                     NULL,                           SZ_CLSID_CODEPREVIEWHANDLER },
			{ HKEY_LOCAL_MACHINE,  L"Software\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers",                                 SZ_CLSID_CODEPREVIEWHANDLER,    L"Cornel's Code Preview Handler" },
		};

        hr = S_OK;
        for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
        {
            hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);
        }
    }
    return hr;
}

//
// Unregisters this COM server
//
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    const PCWSTR rgpszKeys[] =
    {
		L"Software\\Classes\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,
		L"Software\\Classes\\Wow6432Node\\CLSID\\" SZ_CLSID_CODEPREVIEWHANDLER,
		L"Software\\Classes\\.code\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}"
	};

    // Delete the registry entries
    for (int i = 0; i < ARRAYSIZE(rgpszKeys) && SUCCEEDED(hr); i++)
    {
        hr = HRESULT_FROM_WIN32(RegDeleteTreeW(HKEY_CURRENT_USER, rgpszKeys[i]));
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            // If the registry entry has already been deleted, say S_OK.
            hr = S_OK;
        }
    }
    if (SUCCEEDED(hr))
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            RegDeleteValue(hKey, SZ_CLSID_CODEPREVIEWHANDLER);
            RegCloseKey(hKey);
        }
    }
    return hr;
}
*/
