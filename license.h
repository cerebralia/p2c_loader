#pragma once

#include "includes.h"
#include "config.h"

namespace License
{
	bool CheckVersion();
	std::string GetSerial();
	std::string GetUserDayCount();
	bool CheckLicense();
	std::string GetSerial64();
}