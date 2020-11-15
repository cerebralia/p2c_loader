#pragma once

#include "includes.h"

namespace Utils
{
	void ToClipboard(const char* text);
	bool IsProcessRun(const char * const processName);
	std::string GetSteamDir();
	void ShowUpdateUrl();
};