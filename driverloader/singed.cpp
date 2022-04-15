#include "singed.hpp"

void system_no_output(std::string command)
{
    command.insert(0, "/C ");

    SHELLEXECUTEINFOA ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = "cmd.exe";
    ShExecInfo.lpParameters = command.c_str();
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;

    if (ShellExecuteExA(&ShExecInfo) == FALSE)

        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

    DWORD rv;
    GetExitCodeProcess(ShExecInfo.hProcess, &rv);
    CloseHandle(ShExecInfo.hProcess);
}

bool VulnerableDriver::Init()
{
    const std::string placement_path = XorStr("C:\\Windows\\System32\\drivers\\vmbusraid.sys").c_str();

    if (std::filesystem::exists(placement_path))
    {
        std::remove(placement_path.c_str());
    }

    if (!file_utils::create_file_from_buffer(
        placement_path,
        (void*)resource::raw_driver,
        sizeof(resource::raw_driver)
    ))
    {
        return false;
    }

    detail::service_handle = service_utils::create_service(placement_path);

    if (!CHECK_HANDLE(detail::service_handle))
    {
        return false;
    }


    if (!service_utils::start_service(detail::service_handle))
    {
        system_no_output(XorStr("sc start SandyBridge").c_str());
        return true;
    }

    return true;
}

void VulnerableDriver::Unload()
{
    system_no_output(XorStr("sc stop SandyBridge").c_str());
    system_no_output(XorStr("cls").c_str());
}