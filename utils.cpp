#include "utils.h"

void Utils::ToClipboard(const char* text)
{
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		char* clip_data = (char*)(GlobalAlloc(GMEM_FIXED, MAX_PATH));
		lstrcpy(clip_data, text);
		SetClipboardData(CF_TEXT, (HANDLE)(clip_data));
		LCID* lcid = (DWORD*)(GlobalAlloc(GMEM_FIXED, sizeof(DWORD)));
		*lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT);
		SetClipboardData(CF_LOCALE, (HANDLE)(lcid));
		CloseClipboard();
	}
}

bool Utils::IsProcessRun(const char * const processName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &pe);

	while (1) {
		if (strcmp(pe.szExeFile, processName) == 0) return true;
		if (!Process32Next(hSnapshot, &pe)) return false;
	}
}

std::string Utils::GetSteamDir()
{
	HKEY rKey;
	TCHAR Path[256] = { 0 };
	DWORD RegetPath = sizeof(Path);
	RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Valve\\Steam", NULL, KEY_READ, &rKey);
	RegQueryValueEx(rKey, "SteamPath", NULL, NULL, (LPBYTE)&Path, &RegetPath);
	return Path;
}

void Utils::ShowUpdateUrl()
{
	MessageBoxW(0, L"Please kill yourself! Thanks!", WCHEAT_NAME, MB_OK | MB_ICONINFORMATION);

	std::string UpdateUrl;

	UpdateUrl.append("http://");
	UpdateUrl.append(HOST);
	UpdateUrl.append(PATH);
	UpdateUrl.append("update.php?update=");
	UpdateUrl.append(License::GetSerial64());

	std::string url = "url.dll,FileProtocolHandler ";
	url.append(UpdateUrl);

	ShellExecute(NULL, "open", "rundll32.exe", url.c_str(), NULL, SW_SHOWNORMAL);
}
