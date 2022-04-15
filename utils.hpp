#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <string>

Sandy64 Drive;



uintptr_t getBaseAddress(uintptr_t pid) {
	return Drive.GetModuleBase(pid, XorStr("FortniteClient-Win64-Shipping.exe").c_str());
}

template<typename T>
inline bool read_array(uintptr_t pid, uintptr_t address, T* array, size_t len) {
	return Drive.ReadPtr(pid, address, array, sizeof(T) * len);
}

template <typename T> T read(uintptr_t pid, uintptr_t address) {
	T t{};
	Drive.ReadPtr(pid, address, &t, sizeof(T));
	return t;
}

template<typename T>
inline bool write(int pid, uint64_t address, const T& value) {
	return Drive.WritePtr(pid, address, (PVOID)&value, sizeof(T));
}

std::string read_ascii(uintptr_t pid, const std::uintptr_t address, std::size_t size)
{
	std::unique_ptr<char[]> buffer(new char[size]);
	Drive.ReadPtr(pid, address, buffer.get(), size);
	return std::string(buffer.get());
}

std::wstring read_unicode(uintptr_t pid, const std::uintptr_t address, std::size_t size)
{
	const auto buffer = std::make_unique<wchar_t[]>(size);
	Drive.ReadPtr(pid, address, buffer.get(), size * 2);
	return std::wstring(buffer.get());
}

std::wstring read_wstr(uintptr_t pid, uintptr_t address)
{
	wchar_t buffer[1024 * sizeof(wchar_t)];
	Drive.ReadPtr(pid, address, &buffer, 64 * sizeof(wchar_t));
	return std::wstring(buffer);
}

wchar_t* readString(uintptr_t pid, DWORD64 Address, wchar_t* string)
{
	if (read<uint16_t>(pid, Address + 0x10) > 0)
	{
		for (int i = 0; i < read<uint16_t>(pid, Address + 0x10) * 2; i++)
		{
			string[i] = read<wchar_t>(pid, Address + 0x14 + (i * 2));
		}
	}
	return string;
}

namespace wndhide
{
	inline uintptr_t get_module_base(uint32_t process_id, const wchar_t* module_name)
	{
		uintptr_t base_address = 0;
		const HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
		if (snapshot_handle != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 module_entry;
			module_entry.dwSize = sizeof(module_entry);
			if (Module32First(snapshot_handle, &module_entry))
			{
				do
				{
					if (!_wcsicmp(module_entry.szModule, module_name))
					{
						base_address = reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
						break;
					}
				} while (Module32Next(snapshot_handle, &module_entry));
			}
		}
		CloseHandle(snapshot_handle);
		return base_address;
	}

	inline uintptr_t get_module_export(HANDLE process_handle, uintptr_t module_base, const char* export_name)
	{
		SIZE_T dummy_read_size;
		IMAGE_DOS_HEADER dos_header = { 0 };
		IMAGE_NT_HEADERS64 nt_headers = { 0 };

		if (!ReadProcessMemory(process_handle, reinterpret_cast<void*>(module_base), &dos_header, sizeof(dos_header), &dummy_read_size) || dos_header.e_magic != IMAGE_DOS_SIGNATURE ||
			!ReadProcessMemory(process_handle, reinterpret_cast<void*>(module_base + dos_header.e_lfanew), &nt_headers, sizeof(nt_headers), &dummy_read_size) || nt_headers.Signature != IMAGE_NT_SIGNATURE)
			return 0;

		const auto export_base = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		const auto export_base_size = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		if (!export_base || !export_base_size)
			return 0;

		const auto export_data = static_cast<PIMAGE_EXPORT_DIRECTORY>(VirtualAlloc(nullptr, export_base_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!export_data)
			return 0;

		if (!ReadProcessMemory(process_handle, reinterpret_cast<void*>(module_base + export_base), export_data, export_base_size, &dummy_read_size))
		{
			VirtualFree(export_data, 0, MEM_RELEASE);
			return 0;
		}

		const auto delta = reinterpret_cast<uint64_t>(export_data) - export_base;

		const auto name_table = reinterpret_cast<uint32_t*>(export_data->AddressOfNames + delta);
		const auto ordinal_table = reinterpret_cast<uint16_t*>(export_data->AddressOfNameOrdinals + delta);
		const auto function_table = reinterpret_cast<uint32_t*>(export_data->AddressOfFunctions + delta);

		for (auto i = 0u; i < export_data->NumberOfNames; ++i)
		{
			const std::string current_function_name = std::string(reinterpret_cast<char*>(name_table[i] + delta));

			if (!_stricmp(current_function_name.c_str(), export_name))
			{
				const auto function_ordinal = ordinal_table[i];
				if (function_table[function_ordinal] <= 0x1000)
					return 0;

				const auto function_address = module_base + function_table[function_ordinal];

				if (function_address >= module_base + export_base && function_address <= module_base + export_base + export_base_size)
				{
					// Forwarded export
					VirtualFree(export_data, 0, MEM_RELEASE);
					return 0;
				}

				VirtualFree(export_data, 0, MEM_RELEASE);
				return function_address;
			}
		}

		VirtualFree(export_data, 0, MEM_RELEASE);
		return 0;
	}

	inline bool hide_window(uint32_t process_id, HWND window_id, bool hide = true)
	{
		const HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
		if (!process_handle || process_handle == INVALID_HANDLE_VALUE)
			return false;

		const uintptr_t user32_base = get_module_base(process_id, L"user32.dll");
		if (!user32_base)
			return false;

		const uintptr_t function_pointer = get_module_export(process_handle, user32_base, "SetWindowDisplayAffinity");
		if (!function_pointer)
			return false;

		unsigned char shellcode_buffer[] = "\x48\x89\x4C\x24\x08\x48\x89\x54\x24\x10\x4C\x89\x44\x24\x18\x4C\x89\x4C\x24"
			"\x20\x48\x83\xEC\x38\x48\xB9\xED\xFE\xAD\xDE\xED\xFE\x00\x00\x48\xC7\xC2\xAD"
			"\xDE\x00\x00\x48\xB8\xAD\xDE\xED\xFE\xAD\xDE\x00\x00\xFF\xD0\x48\x83\xC4\x38"
			"\x48\x8B\x4C\x24\x08\x48\x8B\x54\x24\x10\x4C\x8B\x44\x24\x18\x4C\x8B\x4C\x24"
			"\x20\xC3";

		const uint32_t mask = hide ? 0x00000011 : 0x00000000; // CWDA_EXCLUDEFROMCAPTURE : CWDA_NONE

		*reinterpret_cast<uintptr_t*>(shellcode_buffer + 26) = reinterpret_cast<uintptr_t>(window_id); // window hwnd
		*reinterpret_cast<uint32_t*>(shellcode_buffer + 37) = mask; // flags
		*reinterpret_cast<uintptr_t*>(shellcode_buffer + 43) = function_pointer; // function ptr

		void* allocated_base = VirtualAllocEx(process_handle, 0x0, sizeof(shellcode_buffer), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!allocated_base)
			return false;

		SIZE_T dummy_size;
		if (!WriteProcessMemory(process_handle, allocated_base, shellcode_buffer, sizeof(shellcode_buffer), &dummy_size))
			return false;

		HANDLE thread_handle = CreateRemoteThread(process_handle, nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(allocated_base), nullptr, 0, nullptr);
		if (!thread_handle || thread_handle == INVALID_HANDLE_VALUE)
			return false;

		// wait for shellcode to execute
		Sleep(1000);

		memset(shellcode_buffer, 0, sizeof(shellcode_buffer));
		if (!WriteProcessMemory(process_handle, allocated_base, shellcode_buffer, sizeof(shellcode_buffer), &dummy_size))
			return false;

		return true;
	}
}

#define M_PI 3.14159265358979323846264338327950288419716939937510

typedef struct
{
	DWORD R;
	DWORD G;
	DWORD B;
	DWORD A;
} RGBA;
int acount = 500;

class Vector2
{
public:
	double X;
	double Y;
};

class FRotator
{
public:
	double Pitch;
	double Yaw;
	double Roll;
};


static class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	double x;
	double y;
	double z;

	inline double Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline Vector3 operator/(double v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	inline double Distance(Vector3 v)
	{
		return double(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}

	inline double Length() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(double flNum) { return Vector3(x * flNum, y * flNum, z * flNum); }
};



struct FPlane : Vector3
{
	double W = 0;


	Vector3 ToVector3()
	{
		Vector3 value;
		value.x = this->x;
		value.y = this->y;
		value.z = this->y;

		return value;
	}
};

struct FMatrix
{
	FPlane XPlane;
	FPlane YPlane;
	FPlane ZPlane;
	FPlane WPlane;
};

struct FTransform
{
	FPlane  rot;
	Vector3 translation;
	char    pad[8];
	Vector3 scale;

	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		double x2 = rot.x + rot.x;
		double y2 = rot.y + rot.y;
		double z2 = rot.z + rot.z;

		double xx2 = rot.x * x2;
		double yy2 = rot.y * y2;
		double zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		double yz2 = rot.y * z2;
		double wx2 = rot.W * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		double xy2 = rot.x * y2;
		double wz2 = rot.W * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		double xz2 = rot.x * z2;
		double wy2 = rot.W * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};
static D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}
static D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0)) {
	float radPitch = (rot.x * double(M_PI) / 180.f);
	float radYaw = (rot.y * double(M_PI) / 180.f);
	float radRoll = (rot.z * double(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

namespace Globals {
	uintptr_t LocalPlayer = 0, LocalPawn = 0, LocalPawnRootComponent = 0;
	int selectedbone = 0, hitbox = 0;


	bool menuIsOpen = false;

	Vector3 LocalPlayerRelativeLocation = Vector3(0.0f, 0.0f, 0.0f);

	float Width = GetSystemMetrics(SM_CXSCREEN), Height = GetSystemMetrics(SM_CYSCREEN), ScreenCenterX = 0.0f, ScreenCenterY = 0.0f;
}

RGBA ESPColor = { 255, 255, 255, 255 };
RGBA MenuColor = { 255, 165, 0, 255 };
RGBA ESPColor2 = { 255, 215, 0, 255 };


/*
__int64 __fastcall sub_7FF6119D0ED0(unsigned __int16 *a1, _WORD *a2)
{
  _WORD *v2; // rdi
  unsigned __int16 *v3; // rbx
  int v4; // ebx
  unsigned int v5; // eax
  unsigned int v6; // er9
  unsigned int v7; // er8
  __int64 result; // rax
  unsigned int v9; // er9
  int v10; // ecx

  v2 = a2;
  v3 = a1;
  ((void (__fastcall *)(_WORD *, unsigned __int16 *, unsigned __int64))unk_7FF60EA8CC72)(
	a2,
	a1 + 2,
	2 * ((unsigned __int64)*a1 >> 6));
  v4 = *v3 >> 6;
  v5 = sub_7FF60EAA60C0();
  v6 = v5;
  v7 = v5 ^ 0x9C677CC5;
  result = 1041204193 * v5;
  v9 = v6 % 0x21;
  v10 = 0;
  if ( v4 )
  {
	do
	{
	  result = v10++ + v9;
	  v7 += result;
	  *v2 ^= v7;
	  ++v2;
	}
	while ( v10 < v4 );
  }
  return result;
}
*/

static std::string RealGetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(g_pid, g_base_address + 0xC42D200 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	uint16_t nameEntry = read<uint16_t>(g_pid, NamePoolChunk);

	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = read<char>(g_pid, NamePoolChunk + 4 + x);
		}
		char* v2 = buff; // rbx
		unsigned int v4 = nameLength;
		unsigned int v5; // eax
		__int64 result; // rax
		int v7; // ecx
		unsigned int v8; // kr00_4
		__int64 v9; // ecx

		v5 = 0;
		result = 28i64;
		if (v4)
		{
			do
			{
				++v2;
				v7 = v5++ | 0xB000;
				v8 = v7 + result;
				BYTE(v7) = v8 ^ ~*(BYTE*)(v2 - 1);
				result = v8 >> 2;
				*(BYTE*)(v2 - 1) = v7;
			} while (v5 < v4);
		}
		buff[nameLength] = '\0';
		return std::string(buff);
	}
	else
	{
		return "";
	}
}

static std::string GetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = read<uint64_t>(g_pid, g_base_address + 0xC42D200 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	if (read<uint16_t>(g_pid, NamePoolChunk) < 64)
	{
		auto a1 = read<DWORD>(g_pid, NamePoolChunk + 4);
		return RealGetNameFromFName(a1);
	}
	else
	{
		return RealGetNameFromFName(key);
	}
}

namespace camera
{
	float m_FovAngle;
	Vector3 m_CameraRotation, m_CameraLocation;
}

namespace g_functions
{
	Vector3 ConvertWorld2Screen(Vector3 WorldLocation) {

		camera::m_CameraRotation.z = 0;

		if (camera::m_CameraRotation.y < 0)
			camera::m_CameraRotation.y += 360;

		D3DMATRIX matrix = Matrix(camera::m_CameraRotation);

		auto dist = WorldLocation - camera::m_CameraLocation;

		auto transform = Vector3(
			dist.Dot(Vector3(
				matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]
			)),
			dist.Dot(Vector3(
				matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]
			)),
			dist.Dot(Vector3(
				matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]
			))
		);

		if (transform.z < 1.f)
			transform.z = 1.f;


		float ScreenCenterX = Globals::Width / 2;
		float ScreenCenterY = Globals::Height / 2;

		Vector3 Screenlocation;
		Screenlocation.x = ScreenCenterX + transform.x * (ScreenCenterX / tanf(camera::m_FovAngle * (float)M_PI / 360.f)) / transform.z;
		Screenlocation.y = ScreenCenterY - transform.y * (ScreenCenterX / tanf(camera::m_FovAngle * (float)M_PI / 360.f)) / transform.z;

		return Screenlocation;
	}

	FTransform f_boneIndex(uint64_t mesh, int index)
	{
		uint64_t bonearray = read<uint64_t>(g_pid, mesh + 0x590);
		if (!bonearray) 
		bonearray = read<DWORD_PTR>(g_pid, mesh + 0x5a0);
	
		return read<FTransform>(g_pid, bonearray + (index * 0x60));
	}

	Vector3 f_getbonewithIndex(DWORD_PTR mesh, int id)
	{
		FTransform bone = f_boneIndex(mesh, id);
		FTransform ComponentToWorld = read<FTransform>(g_pid, mesh + 0x240);

		D3DMATRIX Matrix;
		Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	}


}


bool CheckInScreen(uintptr_t CurrentActor, int Width, int Height) {
	//Vector3 Pos;
	//functions::get_bone(CurrentActor, 66, &Pos);
	//Pos = g_functions::ConvertWorld2Screen(Pos);

	Vector3 Posx = g_functions::f_getbonewithIndex(CurrentActor, 66);
	Vector3 vPos = g_functions::ConvertWorld2Screen(Posx);

	return true;
	/*
	if (CurrentActor)
	{
		if (((vPos.x <= 0 or vPos.x > Width) and (vPos.y <= 0 or vPos.y > Height)) or ((vPos.x <= 0 or vPos.x > Width) or (vPos.y <= 0 or vPos.y > Height))) {
			return false;
		}
		else {
			return true;
		}

	}*/
}


class Color
{
public:
	RGBA red = { 255,0,0,255 };
	RGBA Magenta = { 255,0,255,255 };
	RGBA yellow = { 255,255,0,255 };
	RGBA grayblue = { 128,128,255,255 };
	RGBA green = { 128,224,0,255 };
	RGBA darkgreen = { 0,224,128,255 };
	RGBA brown = { 192,96,0,255 };
	RGBA pink = { 255,168,255,255 };
	RGBA DarkYellow = { 216,216,0,255 };
	RGBA SilverWhite = { 236,236,236,255 };
	RGBA purple = { 144,0,255,255 };
	RGBA Navy = { 88,48,224,255 };
	RGBA skyblue = { 0,136,255,255 };
	RGBA graygreen = { 128,160,128,255 };
	RGBA blue = { 0,96,192,255 };
	RGBA orange = { 255,128,0,255 };
	RGBA peachred = { 255,80,128,255 };
	RGBA reds = { 255,128,192,255 };
	RGBA darkgray = { 96,96,96,255 };
	RGBA Navys = { 0,0,128,255 };
	RGBA darkgreens = { 0,128,0,255 };
	RGBA darkblue = { 0,128,128,255 };
	RGBA redbrown = { 128,0,0,255 };
	RGBA purplered = { 128,0,128,255 };
	RGBA greens = { 0,255,0,255 };
	RGBA envy = { 0,255,255,255 };
	RGBA black = { 0,0,0,255 };
	RGBA gray = { 128,128,128,255 };
	RGBA white = { 255,255,255,255 };
	RGBA blues = { 30,144,255,255 };
	RGBA lightblue = { 135,206,250,160 };
	RGBA Scarlet = { 220, 20, 60, 160 };
	RGBA white_ = { 255,255,255,200 };
	RGBA gray_ = { 128,128,128,200 };
	RGBA black_ = { 0,0,0,200 };
	RGBA red_ = { 255,0,0,200 };
	RGBA Magenta_ = { 255,0,255,200 };
	RGBA yellow_ = { 255,255,0,200 };
	RGBA grayblue_ = { 128,128,255,200 };
	RGBA green_ = { 128,224,0,200 };
	RGBA darkgreen_ = { 0,224,128,200 };
	RGBA brown_ = { 192,96,0,200 };
	RGBA pink_ = { 255,168,255,200 };
	RGBA darkyellow_ = { 216,216,0,200 };
	RGBA silverwhite_ = { 236,236,236,200 };
	RGBA purple_ = { 144,0,255,200 };
	RGBA Blue_ = { 88,48,224,200 };
	RGBA skyblue_ = { 0,136,255,200 };
	RGBA graygreen_ = { 128,160,128,200 };
	RGBA blue_ = { 0,96,192,200 };
	RGBA orange_ = { 255,128,0,200 };
	RGBA pinks_ = { 255,80,128,200 };
	RGBA Fuhong_ = { 255,128,192,200 };
	RGBA darkgray_ = { 96,96,96,200 };
	RGBA Navy_ = { 0,0,128,200 };
	RGBA darkgreens_ = { 0,128,0,200 };
	RGBA darkblue_ = { 0,128,128,200 };
	RGBA redbrown_ = { 128,0,0,200 };
	RGBA purplered_ = { 128,0,128,200 };
	RGBA greens_ = { 0,255,0,200 };
	RGBA envy_ = { 0,255,255,200 };

	RGBA glassblack = { 0, 0, 0, 160 };
	RGBA GlassBlue = { 65,105,225,80 };
	RGBA glassyellow = { 255,255,0,160 };
	RGBA glass = { 200,200,200,60 };

	RGBA filled = { 0, 0, 0, 150 };

	RGBA Plum = { 221,160,221,160 };

};
Color Col;


static const char* Hitbox[] =
{
	"Head",
	"Neck",
	"Chest"
};

int select_hitbox()
{
	int hitbox = 0;

	if (Globals::hitbox == 0)
		hitbox = 66;
	else if (Globals::hitbox == 2)
		hitbox = 65;
	else if (hitbox == 1)
		hitbox = 7;
	return hitbox;
}

namespace hotkeys
{
	int aimkey;
	int airstuckey;
	int instares;
}

static int keystatus = 0;
static int realkey = 0;

bool GetKey(int key)
{
	realkey = key;
	return true;
}
void ChangeKey(void* blank)
{
	keystatus = 1;
	while (true)
	{
		for (int i = 0; i < 0x87; i++)
		{
			if (GetKeyState(i) & 0x8000)
			{
				hotkeys::aimkey = i;
				keystatus = 0;
				return;
			}
		}
	}
}




static const char* keyNames[] =
{
	"Keybind",
	"Left Mouse",
	"Right Mouse",
	"Cancel",
	"Middle Mouse",
	"Mouse 5",
	"Mouse 4",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
};

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}
void HotkeyButton(int aimkey, void* changekey, int status)
{
	const char* preview_value = NULL;
	if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
		Items_ArrayGetter(keyNames, aimkey, &preview_value);

	std::string aimkeys;
	if (preview_value == NULL)
		aimkeys = "Select Key";
	else
		aimkeys = preview_value;

	if (status == 1)
	{

		aimkeys = "Press the Key";
	}
	if (ImGui::Button(aimkeys.c_str(), ImVec2(125, 20)))
	{
		if (status == 0)
		{
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
		}
	}
}

std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}
std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}
void DrawString(float fontSize, int x, int y, RGBA* color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 4;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), text.c_str());
}

void DrawFilledRect(int x, int y, int w, int h, RGBA* color)
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), 0, 0);
}

void DrawCornerBox(int x, int y, int w, int h, int borderPx, RGBA* color, RGBA* color2)
{
	DrawFilledRect(x + borderPx, y, w / 3, borderPx, color); //top 
	DrawFilledRect(x + w - w / 3 + borderPx, y, w / 3, borderPx, color); //top 
	DrawFilledRect(x, y, borderPx, h / 3, color); //left 
	DrawFilledRect(x, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color2); //left 
	DrawFilledRect(x + borderPx, y + h + borderPx, w / 3, borderPx, color); //bottom 
	DrawFilledRect(x + w - w / 3 + borderPx, y + h + borderPx, w / 3, borderPx, color); //bottom 
	DrawFilledRect(x + w + borderPx, y, borderPx, h / 3, color);//right 
	DrawFilledRect(x + w + borderPx, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color2);//right 
}
void DrawCircleFilled(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments);
}
void DrawNormalBox(int x, int y, int w, int h, int borderPx, RGBA* color, RGBA* color2)
{
	DrawFilledRect(x + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x + w - w + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x, y, borderPx, h, color); //left 
	DrawFilledRect(x, y + h - h + borderPx * 2, borderPx, h, color2); //left 
	DrawFilledRect(x + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w - w + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w + borderPx, y, borderPx, h, color);//right 
	DrawFilledRect(x + w + borderPx, y + h - h + borderPx * 2, borderPx, h, color2);//right 
}
void DrawCircle(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), segments);
}

void DrawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness)
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), thickness);
}



float bA1mb0tF0VV4lue = 150.0f, bA1mb0tSm00th1ngV4lue = 2.0f;

int bD1st4nce, bH1tb0x = 0, bE5pD1st4nce = 280, bA1mD1st4nce = 280, bB0xS1ze = 2.0f, bAut0L0ckD1st = 1, bLootRendering = 40;
static  float FOVChangerValue = 100.0f;



Vector3 AimbotCorrection(float bulletVelocity, float bulletGravity, float targetDistance, Vector3 targetPosition, Vector3 targetVelocity) {
	Vector3 recalculated = targetPosition;
	float gravity = fabs(bulletGravity);
	float time = targetDistance / fabs(bulletVelocity);
	/* Bullet drop correction */
	//float bulletDrop = (gravity / 250) * time * time;
	//recalculated.z += bulletDrop * 120;
	/* Player movement correction */
	//recalculated.x += time * (targetVelocity.x);
	//recalculated.y += time * (targetVelocity.y);
	//recalculated.z += time * (targetVelocity.z);
	return recalculated;
}

float powf_(float _X, float _Y) {
	return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
}
float sqrtf_(float _X) {
	return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X))));
}

double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

double GetDistance(double x1, double y1, double z1, double x2, double y2) {
	//return sqrtf(powf_((x2 - x1), 2) + powf_((y2 - y1), 2));
	return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
}