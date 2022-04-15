#define WIN32_LEAN_AND_MEAN  
#include "signed.h"
//DWORD Pid;
HANDLE hDrive;

BOOL Sandy64::Init()
{

	//Pid = d;

	hDrive = ::CreateFileA(XorStr("\\\\.\\Sandy64").c_str(), GENERIC_ALL, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL);
	//
	//hDrive = ::CreateFileA(XorStr("\\\\.\\Sandy64").c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);
	//hDrive = CreateFileW(L"\\\\.\\Sandy64", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	//::CloseHandle(hDrive);
	return hDrive != 0;
}
ULONG64 Sandy64::GetModuleBase(ULONG ProcessPid, LPCSTR ModuleName)
{
	READWRITE ReadWrite = { ProcessPid,0,0,(ULONG64)ModuleName };
	BYTE* Temp = new BYTE[8];
	::memset(Temp, 0, 8);
	BOOL bRet = ::DeviceIoControl(hDrive, 0x22200C, &ReadWrite, sizeof(READWRITE), Temp, 8, NULL, NULL);
	ULONG64 temp = 0;
	if (bRet == TRUE)
	{
		memcpy(&temp, Temp, 8);
	}
	delete[] Temp;
	return temp;
}
BOOL Sandy64::ReadPtr(ULONG ProcessPid,ULONG64 Address, PVOID pBuffer, DWORD Size)
{
	READWRITE ReadWrite = { ProcessPid,Address,Size,0 };
	BYTE* Temp = new BYTE[Size];
	memset(Temp, 0, Size);
	BOOL bRet = ::DeviceIoControl(hDrive, 0x222000, &ReadWrite, sizeof(READWRITE), Temp, Size, NULL, NULL);
	if (bRet)
	{
		memcpy(pBuffer, Temp, Size);
	}
	delete[] Temp;
	return bRet;
}
BOOL Sandy64::WritePtr(ULONG ProcessPid,ULONG64 Address, PVOID pBuffer, DWORD Size)
{
	READWRITE ReadWrite = { ProcessPid,Address,Size,(ULONG64)pBuffer };
	BOOL bRet = ::DeviceIoControl(hDrive, 0x222004, &ReadWrite, sizeof(READWRITE), NULL, NULL, NULL, NULL);
	return bRet;
}
ULONG64 Sandy64::ApplyMemory(ULONG ProcessPid, DWORD Size)
{
	READWRITE ReadWrite = { ProcessPid,0,Size,0 };
	BYTE* Temp = new BYTE[8];
	BOOL bRet = ::DeviceIoControl(hDrive, 0x222008, &ReadWrite, sizeof(READWRITE), Temp, 8, NULL, NULL);
	ULONG64 temp = 0;
	if (bRet == TRUE)
	{
		memcpy(&temp, Temp, 8);
	}
	delete[] Temp;
	return temp;
}

BOOL Sandy64::QueryMemory(ULONG ProcessPid, ULONG64 BaseAddress, MEMORY_BASIC_INFORMATIONA &pinfo)
{
	READWRITE ReadWrite = { ProcessPid,BaseAddress,0,(ULONG64)&pinfo.BaseAddress };

	BOOL bRet = ::DeviceIoControl(hDrive, 0x222010, &ReadWrite, sizeof(READWRITE), NULL, NULL, NULL, NULL);
	return bRet;
}