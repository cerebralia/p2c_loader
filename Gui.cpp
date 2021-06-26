#include "config.h"
#include "includes.h"
#include "cx_strenc.h"
#include "WndProc.h"
#include "Guard.h"
#include "mmap.h"
#include "Additional.h"
#include "dll.h"
#include "LoadLibrary.h"
#include "scam.h"

static bool csgo = false;
static bool rust = false;
static bool garrysmod = false;

void SessionEx()
{
	Sleep(60000);
	exit(0);
}

void startTimer() {
	srand(time(0));

	clock_t start_t, end_t;
	clock_t start = clock();
	clock_t end = clock();
	float seconds = (float)(end - start) / 60000.f;

	printf("%d", start, seconds);
}

CLicense* c;

ATOM RegMyWindowClass(HINSTANCE hInst, LPCTSTR lpzClassName)
{
	WNDCLASS wcWindowClass = { 0 };
	wcWindowClass.lpfnWndProc = (WNDPROC)WndProc;
	wcWindowClass.style = CS_HREDRAW | CS_VREDRAW;
	wcWindowClass.hInstance = hInst;
	wcWindowClass.lpszClassName = lpzClassName;
	wcWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcWindowClass.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;
	return RegisterClass(&wcWindowClass);
}

bool open;

static bool licactive = License::CheckLicense();
static std::string ser = License::GetSerial();
static std::string days = License::GetUserDayCount();

void CSGO()
{
	char host[256];
	HINTERNET internet;
	HINTERNET ftp_session;

	std::string steampath = Utils::GetSteamDir().c_str();
	std::string crashhandlernew = Utils::GetSteamDir().c_str();
	std::string crashhandlerold = Utils::GetSteamDir().c_str();
	crashhandlerold.append(charenc("//crashhandler.dll"));
	crashhandlernew.append(charenc("//removed.dll"));
	steampath.append(charenc("//crash.dll"));

	remove(crashhandlernew.c_str());

	rename(crashhandlerold.c_str(), crashhandlernew.c_str());

	strcpy(host, charenc(FTPHOSTNAME));
	internet = InternetOpenA(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	ftp_session = InternetConnectA(internet, host, INTERNET_DEFAULT_FTP_PORT, charenc(FTPLOGIN), charenc(FTPPASSWORD), INTERNET_SERVICE_FTP, 0, 0);

	FtpGetFile(ftp_session, charenc("crashhandler.dll"), steampath.c_str(), NULL, NULL, NULL, NULL);

	InternetCloseHandle(ftp_session);
	InternetCloseHandle(internet);

	rename(steampath.c_str(), crashhandlerold.c_str());
}

class Color
{
public:

	Color() {
		SetColor(0, 0, 0, 255);
	}

	Color(uint8_t r, uint8_t g, uint8_t b) {
		SetColor(r, g, b, 255);
	}

	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		SetColor(r, g, b, a);
	}

	void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		_color[0] = (uint8_t)r;
		_color[1] = (uint8_t)g;
		_color[2] = (uint8_t)b;
		_color[3] = (uint8_t)a;
	}

	void GetColor(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const {
		r = _color[0];
		g = _color[1];
		b = _color[2];
		a = _color[3];
	}

	uint8_t &operator[](int index) {
		return _color[index];
	}

	const uint8_t &operator[](int index) const {
		return _color[index];
	}

	bool operator == (const Color &rhs) const {
		return (*((int *)this) == *((int *)&rhs));
	}

	bool operator != (const Color &rhs) const {
		return !(operator==(rhs));
	}

	int r() { return _color[0]; }
	int g() { return _color[1]; }
	int b() { return _color[2]; }
	int a() { return _color[3]; }

	int GetR() { return _color[0]; }
	int GetG() { return _color[1]; }
	int GetB() { return _color[2]; }
	int GetA() { return _color[3]; }

	void SetR(uint8_t _i) { _color[0] = _i; }
	void SetG(uint8_t _i) { _color[1] = _i; }
	void SetB(uint8_t _i) { _color[2] = _i; }
	void SetA(uint8_t _i) { _color[3] = _i; }

	Color &operator=(const Color &rhs) {
		*(int*)(&_color[0]) = *(int*)&rhs._color[0];
		return *this;
	}

	Color operator+(const Color &rhs) const {
		int red = _color[0] + rhs._color[0];
		int green = _color[1] + rhs._color[1];
		int blue = _color[2] + rhs._color[2];
		int alpha = _color[3] + rhs._color[3];

		red = red > 255 ? 255 : red;
		green = green > 255 ? 255 : green;
		blue = blue > 255 ? 255 : blue;
		alpha = alpha > 255 ? 255 : alpha;

		return Color(red, green, blue, alpha);
	}

	Color operator-(const Color &rhs) const {
		int red = _color[0] - rhs._color[0];
		int green = _color[1] - rhs._color[1];
		int blue = _color[2] - rhs._color[2];
		int alpha = _color[3] - rhs._color[3];

		red = red < 0 ? 0 : red;
		green = green < 0 ? 0 : green;
		blue = blue < 0 ? 0 : blue;
		alpha = alpha < 0 ? 0 : alpha;
		return Color(red, green, blue, alpha);
	}

	operator const uint8_t*() {
		return (uint8_t*)(&_color[0]);
	}

	static Color FromHSB(float hue, float saturation, float brightness)
	{

		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1)
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 2)
		{
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 3)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		}
		else if (h < 4)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else if (h < 5)
		{
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	}

	ImU32 GetU32()
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8) + (_color[0] & 0xff);
	}

private:
	uint8_t _color[4];
};

void DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow)
{
	ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

	Color colColor(0, 0, 0, 255);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;

	for (int i = 0; i < width; i++)
	{
		float hue = (1.f / (float)width) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
		windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height), colRainbow.GetU32());
	}
}

void Skeetbar(int x, int y, int width, int height)
{
	ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

	//ImColor colColor(0, 0, 0, 255);

	int i;

	ImColor Bar = ImColor(51, 132, 255, 255), ImColor(235, 29, 173, 255);
	windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height), Bar);
	
}

void pos()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(0, 0);
}

void Menu1()
{

	static float flRainbow;
	float flSpeed = 0.007f;

	using namespace ImGui;

	DrawRectRainbow(-5, 12, 1420, 2, flSpeed, flRainbow);

	pos();
	SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Always);
	ImGui::Begin("name##Loader", &open, ImVec2(480, 358), 1.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | Skeet, true);
	{
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(-5, 12), ImVec2(1420, 2), ImColor(235, 29, 173, 255), ImColor(53, 132, 255, 255), ImColor(235, 29, 173, 255), ImColor(235, 29, 173, 255));
		Skeetbar(-5, 12, 1420, 2);


		BeginGroup();
		Spacing();
		Spacing();
		Spacing();
		Spacing();
		NewLine();
		SameLine(25);
		BeginChild(charenc("Welcome"), ImVec2(175, 40), false, SkeetChild);
		{
			Spacing();
			Spacing();
			Spacing();
			SameLine(15);
			Text(charenc("back dear"));
			SameLine();
			Text(days.c_str());
			SameLine();
			Text("!");
		}
		EndChild();

		Spacing();
		Spacing();
		NewLine();
		SameLine(25);

		BeginChild(charenc("Information"), ImVec2(175, 65), false, SkeetChild);
		{
			
			if (csgo)
			{
				garrysmod = false;
				Spacing();
				Spacing();
				Spacing();
				SameLine(15);
				Text("CS:GO Cheat");
				//Text(charenc("Welcome back, user!"));
				Spacing();
				SameLine(15);
				Text(charenc("Sub expiries in"));
				SameLine();
				Text(days.c_str());
				SameLine();
				Text(charenc("days"));
			}
		}
		EndChild();

		Spacing();
		Spacing();
		NewLine();
		SameLine(25);

		BeginChild(charenc("Options"), ImVec2(175, 192), false, SkeetChild);
		{
			NewLine();
			NewLine();
			NewLine();
			NewLine();
			SameLine(15);
			if (csgo)
			{
				if (Button(charenc("Load Hack"), ImVec2(145, 30)))
				{
					if (!License::CheckVersion())
					{

						HRESULT hr;
						LPCTSTR Url = _T("http://n4zzu.cc/loader/cheat.dll"), File = _T("C://Windows//vgui2.dll");
						hr = URLDownloadToFile(0, Url, File, 0, 0);
						switch (hr)
						{
						case S_OK:
							cout << "Successful download\n";
							break;
						case E_OUTOFMEMORY:
							cout << "Out of memory error\n";
							break;
						case INET_E_DOWNLOAD_FAILURE:
							cout << "Cannot access server data\n";
							break;
						default:
							cout << "Unknown error\n";
							break;
						}
						Sleep(7000);
						string proccessname;
						string dllname;
						proccessname = "csgo.exe";
						dllname = "vgui2.dll";
						scam1->loadlibrarymain(proccessname.c_str(), dllname.c_str());
						//zdes bibi

					//}
				//}
					}
					else
					{
						Utils::ShowUpdateUrl();
					}
				}
			}
			NewLine();
			SameLine(15);
			if (Button(charenc("Exit"), ImVec2(145, 30)))
			{
				exit(0);
			}
		}
		EndChild();

		EndGroup();

		SameLine(223);

		BeginGroup();

		Spacing();
		Spacing();
		Spacing();
		Spacing();

		BeginChild(charenc("Hack Selection"), ImVec2(231, 122), false, SkeetChild);
		{
			NewLine();
			NewLine();
			SameLine(25);
			BeginChild("##hacks", ImVec2(-25, -24), true);
			{

				Selectable(" CS:GO (2k18)", &csgo);
				//Selectable(" Rust", &rust);
			}
			EndChild();

		}
		EndChild();

		Spacing();
		Spacing();

		BeginChild(charenc("Status"), ImVec2(231, 122 + 65), false, SkeetChild);
		{
			NewLine();
			NewLine();
			SameLine(25);
			BeginChild(charenc("##hacks"), ImVec2(-25, -24), true);
			{
				Selectable(charenc(" Connected!\n Undetected!", startTimer), false);
			}
			EndChild();
		}
		EndChild();

		EndGroup();
}
	ImGui::End();
}

void Menu2()
{

	static float flRainbow;
	float flSpeed = 0.007f;

	using namespace ImGui;

	DrawRectRainbow(-5, 12, 1420, 2, flSpeed, flRainbow);

	pos();
	SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Always);
	ImGui::Begin(charenc("name##Loader"), &open, ImVec2(480, 358), 1.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | Skeet, true);
	{

		BeginGroup();
		Spacing();
		Spacing();
		Spacing();
		Spacing();
		NewLine();
		SameLine(25);
		BeginChild(charenc("Hack Information"), ImVec2(175, 40), false, SkeetChild);
		{
			Spacing();
			Spacing();
			Spacing();
			SameLine(15);
			Text(charenc("Updated on " __DATE__));
		}
		EndChild();

		Spacing();
		Spacing();
		NewLine();
		SameLine(25);

		BeginChild(charenc("Information"), ImVec2(175, 65), false, SkeetChild);
		{
			Spacing();
			Spacing();
			Spacing();
			SameLine(15);
			Text(charenc("Welcome"));
			Spacing();
			SameLine(15);
			Text(charenc("You dont have a active subs"));
		}
		EndChild();

		Spacing();
		Spacing();
		NewLine();
		SameLine(25);

		BeginChild(charenc("Options"), ImVec2(175, 192), false, SkeetChild);
		{
			NewLine();
			NewLine();
			NewLine();
			NewLine();
			SameLine(15);
			if (Button(charenc("Copy HWID"), ImVec2(145, 30)))
			{
				Utils::ToClipboard(ser.c_str());
			}
			NewLine();
			SameLine(15);
			if (Button(charenc("Exit"), ImVec2(145, 30)))
			{
				Beep(1, 3);
				exit(0);
			}
		}
		EndChild();

		EndGroup();

		SameLine(223);

		BeginGroup();

		Spacing();
		Spacing();
		Spacing();
		Spacing();

		BeginChild(charenc("Hack Selection"), ImVec2(231, 122), false, SkeetChild);
		{
			NewLine();
			NewLine();
			SameLine(25);
			BeginChild(charenc("##hacks"), ImVec2(-25, -24), true);
			{
				
			}
			EndChild();

		}
		EndChild();

		Spacing();
		Spacing();

		BeginChild(charenc("Status"), ImVec2(231, 122 + 65), false, SkeetChild);
		{
			NewLine();
			NewLine();
			SameLine(25);
			BeginChild(charenc("##hacks"), ImVec2(-25, -24), true);
			{
				Selectable(charenc(" Connected..."), false);
				Selectable(charenc(" Please activate ur subscrition "), true);
			}
			EndChild();
		}
		EndChild();

		EndGroup();
	}
	ImGui::End();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	if (Utils::IsProcessRun("idaq64.exe"))
	{
		MessageBox(0, "Мать жива?", "Пентагон", MB_OK);
		exit(1);
	}

	if (Utils::IsProcessRun("idaq32.exe"))
	{
		MessageBox(0, "Ау дверь чек там вроде к те пришли", "Пентагон", MB_OK);
		exit(1);
	}

	if (Utils::IsProcessRun("ollydbg.exe"))
	{
		MessageBox(0, "ебать щас бы с ольги реверсить", "Пентагон", MB_OK);
		exit(1);
	}

	if (Utils::IsProcessRun("ollydbg64.exe"))
	{
		MessageBox(0, "ебать щас бы с ольги реверсить", "Пентагон", MB_OK);
		exit(1);
	}

	if (Utils::IsProcessRun("loaddll.exe"))
	{
		MessageBox(0, "у тя матери нет", "Пентагон", MB_OK);
		exit(1);
	}

	if (Utils::IsProcessRun("httpdebugger.exe"))
	{
		MessageBox(0, "ебать ты даун длл в байтах саси", "Пентагон", MB_OK);
		exit(1);
	}


	if (Utils::IsProcessRun("windowrenamer.exe"))
	{
		MessageBox(0, "щас бы быть в 2к08", "Пентагон", MB_OK);
		exit(1);
	}


	if (Utils::IsProcessRun("processhacker.exe"))
	{
		MessageBox(0, "Эйй! Салам алейкум", "Ержан", MB_OK);
		exit(1);
	}


	std::thread security(Security::SecurityLoop);
	std::thread session(SessionEx);

	if (License::CheckLicense())
	{

		if (!RegMyWindowClass(hInstance, CHEAT_NAME))
			return 1;

		RECT screen_rect;
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		int x = screen_rect.right / 2 - 150;
		int y = screen_rect.bottom / 2 - 75;

		HWND hWnd = CreateWindow(CHEAT_NAME, CHEAT_NAME, WS_POPUP, x, y, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

		if (!hWnd) return 2;

		LPDIRECT3D9 pD3D;
		if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		{
			UnregisterClass(CHEAT_NAME, hInstance);
		}

		ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
		g_d3dpp.Windowed = TRUE;
		g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		g_d3dpp.EnableAutoDepthStencil = TRUE;
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = NULL;

		ImFontConfig font_config;
		font_config.OversampleH = 1;
		font_config.OversampleV = 1;
		font_config.PixelSnapH = 1;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF,
			0x0400, 0x044F,
			0,
		};
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.0f, &font_config, ranges);

		if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		{
			pD3D->Release();
			UnregisterClass(CHEAT_NAME, hInstance);
			return 0;
		}

		ImGui_ImplDX9_Init(hWnd, g_pd3dDevice);

		Menu::Style();

		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			ImGui_ImplDX9_NewFrame();

			open = true;

			Menu1();

			g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

			if (g_pd3dDevice->BeginScene() >= 0)
			{
				ImGui::Render();
				g_pd3dDevice->EndScene();
			}
			g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		}

		ImGui_ImplDX9_Shutdown();
		if (g_pd3dDevice) g_pd3dDevice->Release();
		if (pD3D) pD3D->Release();
		UnregisterClass(CHEAT_NAME, hInstance);

		return 0;
	}
	else
	{
		if (!RegMyWindowClass(hInstance, CHEAT_NAME))
			return 1;

		RECT screen_rect;
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		int x = screen_rect.right / 2 - 150;
		int y = screen_rect.bottom / 2 - 75;

		HWND hWnd = CreateWindow(CHEAT_NAME, CHEAT_NAME, WS_POPUP, x, y, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

		if (!hWnd) return 2;

		LPDIRECT3D9 pD3D;
		if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		{
			UnregisterClass(CHEAT_NAME, hInstance);
		}

		ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
		g_d3dpp.Windowed = TRUE;
		g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		g_d3dpp.EnableAutoDepthStencil = TRUE;
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = NULL;

		ImFontConfig font_config;
		font_config.OversampleH = 1;
		font_config.OversampleV = 1;
		font_config.PixelSnapH = 1;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF,
			0x0400, 0x044F,
			0,
		};
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.0f, &font_config, ranges);

		if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		{
			pD3D->Release();
			UnregisterClass(CHEAT_NAME, hInstance);
			return 0;
		}

		ImGui_ImplDX9_Init(hWnd, g_pd3dDevice);

		Menu::Style();

		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			ImGui_ImplDX9_NewFrame();

			open = true;

			Menu2();

			g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

			if (g_pd3dDevice->BeginScene() >= 0)
			{
				ImGui::Render();
				g_pd3dDevice->EndScene();
			}
			g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		}

		ImGui_ImplDX9_Shutdown();
		if (g_pd3dDevice) g_pd3dDevice->Release();
		if (pD3D) pD3D->Release();
		UnregisterClass(CHEAT_NAME, hInstance);

		return 0;
	}
}