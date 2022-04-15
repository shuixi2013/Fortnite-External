#pragma once
#include <Windows.h>
#include <fstream>
#include <filesystem>

#include "..\string.hpp"
#include "raw_driver.hpp"
#include "file_utils.hpp"
#include "service_utils.hpp"

#define SERVICE_NAME XorStr(L"SandyBridge").c_str()
#define DISPLAY_NAME XorStr(L"SandyKernel").c_str()

namespace VulnerableDriver
{
	namespace detail
	{
		inline SC_HANDLE service_handle;
	}

	bool Init();
	void Unload();
}