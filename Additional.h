#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

using namespace std;
namespace Project
{
	CLicense lic;
	class Additional
	{
	public:
		string GetWinUserName()
		{
			string res = "";
			char buffer[256];
			DWORD size = 256;
			GetComputerNameA(buffer, &size);
			res = buffer;
			return res;
		}
		void ExecuteFile(char* path_to, int SW = 0)
		{
			ShellExecute(0, 0, path_to, 0, 0, SW);
		}
		void OpenWebPage(const char* url)
		{
			ShellExecute(0, 0, url, 0, 0, SW_SHOW);
		}
		void DownLoadFile(char* from, char* to)
		{
			HRESULT hr;
			LPCTSTR Url = _T(from), File = _T(to);
			hr = URLDownloadToFile(0, Url, File, 0, 0);
		}	 
		DWORD GetProcessPID(char* ProcName)
		{
			PROCESSENTRY32 lppe;
			long PID = 0, Result = 0;
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (hSnap)
			{
				lppe.dwSize = sizeof(PROCESSENTRY32);
				Result = Process32First(hSnap, &lppe);
				while (Result)
				{
					if (strcmp(lppe.szExeFile, ProcName) == NULL)
					{
						PID = lppe.th32ProcessID;
						break;
					}
					Result = Process32Next(hSnap, &lppe);
				}
				CloseHandle(hSnap);
			}
			return PID;
		}
		int DownLoadFile(LPCSTR lpszURL, BYTE **ppBuffer, DWORD *pdwSize) //CREDITS: XVII
		{
			LPSTREAM lpStream = 0;
			BYTE *pResult = 0;
			int nResult = 0;

			if (lpszURL && SUCCEEDED(URLOpenBlockingStream(NULL, lpszURL, &lpStream, 0, NULL)))
			{
				STATSTG statStream;
				if (SUCCEEDED(lpStream->Stat(&statStream, STATFLAG_NONAME)))
				{
					DWORD dwSize = statStream.cbSize.LowPart;
					pResult = (BYTE *)malloc(dwSize);
					if (pResult)
					{
						LARGE_INTEGER liPos;
						ZeroMemory(&liPos, sizeof(liPos));
						lpStream->Seek(liPos, STREAM_SEEK_SET, NULL);
						lpStream->Read(pResult, dwSize, NULL);
						*ppBuffer = pResult;
						*pdwSize = dwSize;
						nResult = 1;
					}
				}
				lpStream->Release();
			}

			return nResult;
		}
	};
	Additional* misc;

	namespace Settings
	{
		//string LoaderName = XorStr("");
		//string client_ver = XorStr("0");
		//string server_ver = lic.GetVer();
		//bool Lic = lic.CheckLicense();
		//bool CheatOK = lic.CheatEnabled();
		//string days = lic.GetDays();
		//string user = lic.Username();
		//bool admin = lic.GetOnlineAdmin(XorStr("admin"));
		bool bStatus;
	}
}