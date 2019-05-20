#include <Windows.h>
#include <exception>
#include <iostream>
#include <intrin.h>
#include <string>
#include "detours.h"

using namespace std;

bool g_Unload = false;

INT (WINAPI* oMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT) = MessageBoxA;

INT WINAPI hkMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	int returnAddress = (int)_ReturnAddress();
	LPCSTR lpNewCaption = std::to_string(returnAddress).c_str();
	int iReturn = oMessageBoxA(hWnd, lpText, lpNewCaption, uType);
	return  iReturn;
}

INT (WINAPI* oMessageBoxW)(HWND, LPCWSTR, LPCWSTR, UINT) = MessageBoxW;

INT WINAPI hkMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	int returnAddress = (int)_ReturnAddress();
	LPCWSTR lpNewCaption = L"MessageBoxW";
	int iReturn = oMessageBoxW(hWnd, lpText, lpNewCaption, uType);
	return  iReturn;
}

DWORD WINAPI on_dll_attach(LPVOID base)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&reinterpret_cast<PVOID&>(oMessageBoxA), &hkMessageBoxA);
	DetourAttach(&reinterpret_cast<PVOID&>(oMessageBoxW), &hkMessageBoxW);

	DWORD error = DetourTransactionCommit();

	if (error == NO_ERROR)
		MessageBox(NULL, "Hooked MessageBox successfully", "hook", MB_OK);
	else
		MessageBox(NULL, "Failed hooking MessageBox", "Error", MB_OK);

	return 0;
}

DWORD WINAPI on_dll_detach()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourDetach(&reinterpret_cast<PVOID&>(oMessageBoxA), &hkMessageBoxA);
	DetourDetach(&reinterpret_cast<PVOID&>(oMessageBoxW), &hkMessageBoxW);

	DetourTransactionCommit();

	return 0;
}

BOOL WINAPI DllMain(
	_In_      HINSTANCE hinstDll,
	_In_      DWORD     fdwReason,
	_In_opt_  LPVOID    lpvReserved
)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDll);
		CreateThread(nullptr, 0, on_dll_attach, hinstDll, 0, nullptr);
		return TRUE;
	case DLL_PROCESS_DETACH:
		MessageBox(NULL, "Detaching MessageBox Hooks", "Detach", MB_OK);
		on_dll_detach();
		break;
	default:
		return TRUE;
	}
}