#include "security.h"

BOOL WINAPI IsDebug()
{
	HANDLE hProcess = NULL;
	PROCESSENTRY32 Pe32 = { 0 };
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	Pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hProcessSnap, &Pe32))
	{
		do
		{
			if (_stricmp("csrss.exe", Pe32.szExeFile) == 0)
			{
				HANDLE hProcess = OpenProcess(
					PROCESS_ALL_ACCESS,
					FALSE,
					Pe32.th32ProcessID
				);

				if (hProcess)
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
				CloseHandle(hProcess);
			}
		} while (Process32Next(hProcessSnap, &Pe32));
	}
	CloseHandle(hProcessSnap);
}

PVOID AntiRevers(HMODULE dwModule)
{
	PVOID pEntry = NULL;
	PIMAGE_DOS_HEADER pId = (PIMAGE_DOS_HEADER)dwModule;
	PIMAGE_NT_HEADERS pInt = (PIMAGE_NT_HEADERS)(dwModule + pId->e_lfanew);
	pEntry = dwModule + pInt->OptionalHeader.BaseOfCode;
	return pEntry;
}

void AntiHeaders(HINSTANCE hModule)
{
	DWORD dwPEB_LDR_DATA = 0;
	_asm
	{
		pushad;
		pushfd;
		mov eax, fs:[30h]
			mov eax, [eax + 0Ch]
			mov dwPEB_LDR_DATA, eax

			InLoadOrderModuleList :
		mov esi, [eax + 0Ch]
			mov edx, [eax + 10h]

			LoopInLoadOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 18h]
			cmp ecx, hModule
			jne SkipA
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp InMemoryOrderModuleList
			SkipA :
		cmp edx, esi
			jne LoopInLoadOrderModuleList

			InMemoryOrderModuleList :
		mov eax, dwPEB_LDR_DATA
			mov esi, [eax + 14h]
			mov edx, [eax + 18h]

			LoopInMemoryOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 10h]
			cmp ecx, hModule
			jne SkipB
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp InInitializationOrderModuleList
			SkipB :
		cmp edx, esi
			jne LoopInMemoryOrderModuleList

			InInitializationOrderModuleList :
		mov eax, dwPEB_LDR_DATA
			mov esi, [eax + 1Ch]
			mov edx, [eax + 20h]

			LoopInInitializationOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 08h]
			cmp ecx, hModule
			jne SkipC
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp Finished
			SkipC :
		cmp edx, esi
			jne LoopInInitializationOrderModuleList

			Finished :
		popfd;
		popad;
	}
}


void HideModule(HINSTANCE hModule)
{
	DWORD dwPEB_LDR_DATA = 0;
	_asm
	{
		pushad;
		pushfd;
		mov eax, fs:[30h]
			mov eax, [eax + 0Ch]
			mov dwPEB_LDR_DATA, eax
			InLoadOrderModuleList :
		mov esi, [eax + 0Ch]
			mov edx, [eax + 10h]
			LoopInLoadOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 18h]
			cmp ecx, hModule
			jne SkipA
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp InMemoryOrderModuleList
			SkipA :
		cmp edx, esi
			jne LoopInLoadOrderModuleList
			InMemoryOrderModuleList :
		mov eax, dwPEB_LDR_DATA
			mov esi, [eax + 14h]
			mov edx, [eax + 18h]
			LoopInMemoryOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 10h]
			cmp ecx, hModule
			jne SkipB
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp InInitializationOrderModuleList
			SkipB :
		cmp edx, esi
			jne LoopInMemoryOrderModuleList
			InInitializationOrderModuleList :
		mov eax, dwPEB_LDR_DATA
			mov esi, [eax + 1Ch]
			mov edx, [eax + 20h]
			LoopInInitializationOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 08h]
			cmp ecx, hModule
			jne SkipC
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp Finished
			SkipC :
		cmp edx, esi
			jne LoopInInitializationOrderModuleList
			Finished :
		popfd;
		popad;
	}
}

inline bool Int2DCheck()
{
	__try
	{
		__asm
		{
			int 0x2d
			xor eax, eax
			add eax, 2
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return true;
}

DWORD __forceinline IsInsideVPC_exceptionFilter(LPEXCEPTION_POINTERS ep)
{
	PCONTEXT ctx = ep->ContextRecord;

	ctx->Ebx = -1;
	ctx->Eip += 4;
	return EXCEPTION_CONTINUE_EXECUTION;
}

bool IsInsideVPC()
{
	bool rc = false;

	__try
	{
		_asm push ebx
		_asm mov  ebx, 0
		_asm mov  eax, 1


		_asm __emit 0Fh
		_asm __emit 3Fh
		_asm __emit 07h
		_asm __emit 0Bh

		_asm test ebx, ebx
		_asm setz[rc]
			_asm pop ebx
	}

	__except (IsInsideVPC_exceptionFilter(GetExceptionInformation()))
	{
	}

	return rc;
}

bool IsInsideVMWare()
{
	bool rc = true;

	__try
	{
		__asm
		{
			push   edx
			push   ecx
			push   ebx

			mov    eax, 'VMXh'
			mov    ebx, 0
			mov    ecx, 10
			mov    edx, 'VX'

			in     eax, dx

			cmp    ebx, 'VMXh'
			setz[rc]

			pop    ebx
			pop    ecx
			pop    edx
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		rc = false;
	}

	return rc;
}

bool VMCheck()
{
	unsigned int time1 = 0;
	unsigned int time2 = 0;
	__asm
	{
		RDTSC
		MOV time1, EAX
		RDTSC
		MOV time2, EAX

	}
	if ((time2 - time1) > 100)
	{
		return true;
	}

	return false;
}

BOOL AD_CheckRemoteDebuggerPresent()
{
	FARPROC Func_addr;
	HMODULE hModule = GetModuleHandle("kernel32.dll");

	if (hModule == INVALID_HANDLE_VALUE)
		return false;

	(FARPROC&)Func_addr = GetProcAddress(hModule, "CheckRemoteDebuggerPresent");

	if (Func_addr != NULL) {
		__asm {
			push  eax;
			push  esp;
			push  0xffffffff;
			call  Func_addr;
			test  eax, eax;
			je    choke_false;
			pop    eax;
			test  eax, eax
				je    choke_false;
			jmp    choke_true;
		}
	}

choke_true:
	return true;

choke_false:
	return false;
}

BOOL AD_PEB_NtGlobalFlags()
{
	__asm {
		mov eax, fs:[30h]
		mov eax, [eax + 68h]
		and eax, 0x70
	}
}


BOOL AD_PEB_IsDebugged()
{
	__asm {
		xor eax, eax
		mov ebx, fs:[30h]
		mov al, byte ptr[ebx + 2]
	}
}

bool SecurityCheck()
{
	if (IsDebuggerPresent())
		return false;

	if (IsDebug())
		return false;

	if (Int2DCheck())
		return false;

	if (AD_PEB_IsDebugged())
		return false;

	if (AD_PEB_NtGlobalFlags())
		return false;

	if (AD_CheckRemoteDebuggerPresent())
		return false;

	if (IsInsideVPC())
		return false;

	if (IsInsideVMWare())
		return false;

	if (VMCheck())
		return false;

	return true;
}

void Security::SecurityLoop()
{
	while (true)
	{
		if (!SecurityCheck())
			exit(0);
		Sleep(1000);
	}
}
