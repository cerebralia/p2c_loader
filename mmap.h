#pragma once
#include <Windows.h>
#include <TlHelp32.h> //PROCESSENTRY

typedef HMODULE(WINAPI *pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI *pGetProcAddress)(HMODULE, LPCSTR);
typedef BOOL(WINAPI *PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT
{
	PVOID                    ImageBase;
	PIMAGE_NT_HEADERS        NtHeaders;
	PIMAGE_BASE_RELOCATION   BaseRelocation;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA            fnLoadLibraryA;
	pGetProcAddress          fnGetProcAddress;
}MANUAL_INJECT, *PMANUAL_INJECT;

DWORD WINAPI LoadDll(PVOID p)
{
	PMANUAL_INJECT ManualInject;

	HMODULE  hModule;
	DWORD i,
		Function,
		count,
		delta;

	PDWORD ptr;
	PWORD  list;

	PIMAGE_BASE_RELOCATION   pIBR;
	PIMAGE_IMPORT_DESCRIPTOR pIID;
	PIMAGE_IMPORT_BY_NAME    pIBN;
	PIMAGE_THUNK_DATA        FirstThunk,
		OrigFirstThunk;
	PDLL_MAIN                EntryPoint;

	ManualInject = (PMANUAL_INJECT)p;

	pIBR = ManualInject->BaseRelocation;
	delta = (DWORD)((LPBYTE)ManualInject->ImageBase - ManualInject->NtHeaders->OptionalHeader.ImageBase);

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			list = (PWORD)(pIBR + 1);

			for (i = 0; i<count; i++)
			{
				if (list[i])
				{
					ptr = (PDWORD)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}
		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}
	pIID = ManualInject->ImportDirectory;

	while (pIID->Characteristics)
	{
		OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

		hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

		if (!hModule)
			return FALSE;

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				Function = (DWORD)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}

			else
			{
				pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
				Function = (DWORD)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

				if (!Function)
					return FALSE;

				FirstThunk->u1.Function = Function;
			}

			OrigFirstThunk++;
			FirstThunk++;
		}

		pIID++;
	}

	if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		EntryPoint = (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
		int Reserved = 1337;
		return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, (void*)Reserved);
	}
	return TRUE;
}

DWORD WINAPI LoadDllEnd()
{
	return 0;
}

BOOL MMapMemory(DWORD ProcessId, PVOID  pBuffer)
{
	PIMAGE_DOS_HEADER     pIDH;
	PIMAGE_NT_HEADERS     pINH;
	PIMAGE_SECTION_HEADER pISH;

	HANDLE hProcess,
		hThread;
	PVOID  image,
		mem;
	DWORD  i,
		ExitCode;


	MANUAL_INJECT ManualInject;

	pIDH = (PIMAGE_DOS_HEADER)pBuffer;
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		//std::cout << "Error:: Invalid executable image" << std::endl;
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	pINH = (PIMAGE_NT_HEADERS)((LPBYTE)pBuffer + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
	{
		//std::cout << "Error:: Invalid PE header" << std::endl;
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	if (!(pINH->FileHeader.Characteristics & IMAGE_FILE_DLL))
	{
		//std::cout << "Error:: The image is not DLL" << std::endl;
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	if (!hProcess)
	{
		//std::cout << "Error::  Open process::" << GetLastError() << std::endl;
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return FALSE;
	}

	image = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!image)
	{
		//std::cout << "Error::  Allocate memory for the DLL::" << GetLastError() << std::endl;
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return FALSE;
	}

	if (!WriteProcessMemory(hProcess, image, pBuffer, pINH->OptionalHeader.SizeOfHeaders, NULL))
	{
		//std::cout << "Error::  Copy headers to process::" << GetLastError() << std::endl;
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);
	for (i = 0; i<pINH->FileHeader.NumberOfSections; i++)
	{
		WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)pBuffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
	}

	mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!mem)
	{
		//std::cout << "Error::  Allocate memory for the loader code::" << GetLastError() << std::endl;
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	memset(&ManualInject, 0, sizeof(MANUAL_INJECT));
	ManualInject.ImageBase = image;
	ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);
	ManualInject.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	ManualInject.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	ManualInject.fnLoadLibraryA = LoadLibraryA;
	ManualInject.fnGetProcAddress = GetProcAddress;

	WriteProcessMemory(hProcess, mem, &ManualInject, sizeof(MANUAL_INJECT), NULL);
	WriteProcessMemory(hProcess, (PVOID)((PMANUAL_INJECT)mem + 1), LoadDll, (DWORD)LoadDllEnd - (DWORD)LoadDll, NULL);

	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((PMANUAL_INJECT)mem + 1), mem, 0, NULL);

	if (!hThread)
	{
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, &ExitCode);

	if (!ExitCode)
	{
		VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
		VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);

		CloseHandle(hThread);
		CloseHandle(hProcess);

		VirtualFree(pBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	CloseHandle(hThread);
	VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
	CloseHandle(hProcess);


	VirtualFree(pBuffer, 0, MEM_RELEASE);

	return TRUE;
}
