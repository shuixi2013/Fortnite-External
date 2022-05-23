/*
	*Gloomy.cc
 	*https://github.com/Chase1803
	
	Copyright (c) 2022 Chase1803
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <string>

#include <iostream>
#include "GL/glew.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <tchar.h>
#include <intrin.h>
#include "driverloader/singed.hpp"
#include "driver.h"
#include "Controller.h"
#include <d3d9.h>
#include "d3dx9.h"
#include <thread>
#include <string>
#include <stdio.h>
#include <fstream>
#include "font.h"
#include "utils.hpp"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "structs.h"
#include "maths.h"

#include "globals.h"
#include "overlay.h"

#include "sdk.h"
#include <array>

enum InjectedInputMouseOptions
{
	Absolute = 32768,
	HWheel = 4096,
	LeftDown = 2,
	LeftUp = 4,
	MiddleDown = 32,
	MiddleUp = 64,
	Move = 1,
	MoveNoCoalesce = 8192,
	None = 0,
	RightDown = 8,
	RightUp = 16,
	VirtualDesk = 16384,
	Wheel = 2048,
	XDown = 128,
	XUp = 256
};

typedef struct _InjectedInputMouseInfo
{
	int DeltaX;
	int DeltaY;
	unsigned int MouseData;
	InjectedInputMouseOptions MouseOptions;
	unsigned int TimeOffsetInMilliseconds;
	void* ExtraInfo;
} InjectedInputMouseInfo;

typedef bool (WINAPI* InjectMouseInput_t)(InjectedInputMouseInfo* inputs, int count);

InjectMouseInput_t InjectMouseInput;

void aimbot(float x, float y)
{
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	int AimSpeed = AimbotSmoothnessValue;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	InjectedInputMouseInfo info = { 0 };
	info.DeltaX = TargetX;
	info.DeltaY = TargetY;
	InjectMouseInput(&info, 1);

	return;
}

RGBA ESPColor;
ImU32 ESPColorU;

std::uintptr_t find_signature(const char* sig, const char* mask)
{
	auto buffer = std::make_unique<std::array<std::uint8_t, 0x100000>>();
	auto data = buffer.get()->data();

	for (std::uintptr_t i = 0u; i < (2u << 25u); ++i)
	{
		Drive.ReadPtr(Globals::g_pid, Globals::g_base_address + i * 0x100000, data, 0x100000);

		if (!data)
			return 0;

		for (std::uintptr_t j = 0; j < 0x100000u; ++j)
		{
			if ([](std::uint8_t const* data, std::uint8_t const* sig, char const* mask)
				{
					for (; *mask; ++mask, ++data, ++sig)
					{
						if (*mask == 'x' && *data != *sig) return false;
					}
					return (*mask) == 0;
				}(data + j, (std::uint8_t*)sig, mask))
			{
				std::uintptr_t result = Globals::g_base_address + i * 0x100000 + j;
				std::uint32_t rel = 0;

				Drive.ReadPtr(Globals::g_pid, result + 3, &rel, sizeof(std::uint32_t));

				if (!rel)
					return 0;

				return result + rel + 7;
			}
		}
	}

	return 0;
}

std::wstring s2ws(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

int get_fn_processid() {
	BYTE target_name[] = { 'F','o','r','t','n','i','t','e','C','l','i','e','n','t','-','W','i','n','6','4','-','S','h','i','p','p','i','n','g','.','e','x','e', 0 };
	std::wstring process_name = s2ws(std::string((char*)target_name));
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);

	if (!Process32First(snapshot, &entry)) {
		return 0;
	}

	while (Process32Next(snapshot, &entry)) {
		if (std::wstring(entry.szExeFile) == process_name) {
			return entry.th32ProcessID;
		}
	}

	return 0;
}
