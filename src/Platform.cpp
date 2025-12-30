// ============================================================================
// DaktLib Core - Platform Implementation (C++23)
// ============================================================================

#include "dakt/core/Platform.hpp"

#include "dakt/core/String.hpp"

#include <chrono>
#include <cstdlib>
#include <format>
#include <thread>

#if defined(DAKT_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <ShlObj.h>
    #include <Shlwapi.h>
    #include <Windows.h>
    #pragma comment(lib, "Shlwapi.lib")
#elif defined(DAKT_PLATFORM_LINUX) || defined(DAKT_PLATFORM_MACOS)
    #include <dlfcn.h>
    #include <pwd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>

    #include <climits>
    #if defined(DAKT_PLATFORM_LINUX)
        #include <sys/sysinfo.h>
    #elif defined(DAKT_PLATFORM_MACOS)
        #include <mach-o/dyld.h>
        #include <sys/sysctl.h>
    #endif
#endif

namespace dakt::core
{

// ============================================================================
// System Information
// ============================================================================

SystemInfo getSystemInfo()
{
    SystemInfo info{};

#if defined(DAKT_PLATFORM_WINDOWS)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    info.processorCount = sysInfo.dwNumberOfProcessors;
    info.pageSize = sysInfo.dwPageSize;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus))
    {
        info.totalPhysicalMemory = memStatus.ullTotalPhys;
        info.availablePhysicalMemory = memStatus.ullAvailPhys;
    }

    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerNameA(computerName, &size))
    {
        info.computerName = computerName;
    }

    char userName[256];
    size = sizeof(userName);
    if (GetUserNameA(userName, &size))
    {
        info.userName = userName;
    }

    OSVERSIONINFOEXA osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    #pragma warning(suppress : 4996)
    if (GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&osvi)))
    {
        info.osVersion = std::format("Windows {}.{}.{}", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    }

#elif defined(DAKT_PLATFORM_LINUX)
    info.processorCount = static_cast<u32>(sysconf(_SC_NPROCESSORS_ONLN));
    info.pageSize = static_cast<u64>(sysconf(_SC_PAGESIZE));

    struct sysinfo si;
    if (sysinfo(&si) == 0)
    {
        info.totalPhysicalMemory = si.totalram * si.mem_unit;
        info.availablePhysicalMemory = si.freeram * si.mem_unit;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        info.computerName = hostname;
    }

    if (auto* pw = getpwuid(getuid()))
    {
        info.userName = pw->pw_name;
    }

    info.osVersion = "Linux";

#elif defined(DAKT_PLATFORM_MACOS)
    info.processorCount = static_cast<u32>(sysconf(_SC_NPROCESSORS_ONLN));
    info.pageSize = static_cast<u64>(sysconf(_SC_PAGESIZE));

    int mib[2] = {CTL_HW, HW_MEMSIZE};
    size_t len = sizeof(info.totalPhysicalMemory);
    sysctl(mib, 2, &info.totalPhysicalMemory, &len, nullptr, 0);

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        info.computerName = hostname;
    }

    if (auto* pw = getpwuid(getuid()))
    {
        info.userName = pw->pw_name;
    }

    char osVersion[256];
    size_t osVersionLen = sizeof(osVersion);
    if (sysctlbyname("kern.osproductversion", osVersion, &osVersionLen, nullptr, 0) == 0)
    {
        info.osVersion = std::format("macOS {}", osVersion);
    }
#endif

    return info;
}

// ============================================================================
// Environment
// ============================================================================

Option<String> getEnv(StringView name)
{
    String nameStr(name);
#if defined(DAKT_PLATFORM_WINDOWS)
    char buffer[32767];
    DWORD result = GetEnvironmentVariableA(nameStr.c_str(), buffer, sizeof(buffer));
    if (result > 0 && result < sizeof(buffer))
    {
        return some(String(buffer));
    }
    return none;
#else
    if (const char* value = std::getenv(nameStr.c_str()))
    {
        return some(String(value));
    }
    return none;
#endif
}

bool setEnv(StringView name, StringView value)
{
    String nameStr(name);
    String valueStr(value);
#if defined(DAKT_PLATFORM_WINDOWS)
    return SetEnvironmentVariableA(nameStr.c_str(), valueStr.c_str()) != 0;
#else
    return setenv(nameStr.c_str(), valueStr.c_str(), 1) == 0;
#endif
}

bool unsetEnv(StringView name)
{
    String nameStr(name);
#if defined(DAKT_PLATFORM_WINDOWS)
    return SetEnvironmentVariableA(nameStr.c_str(), nullptr) != 0;
#else
    return unsetenv(nameStr.c_str()) == 0;
#endif
}

// ============================================================================
// Paths
// ============================================================================

String getCurrentDirectory()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer))
    {
        return String(buffer);
    }
    return {};
#else
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)))
    {
        return String(buffer);
    }
    return {};
#endif
}

bool setCurrentDirectory(StringView path)
{
    String pathStr(path);
#if defined(DAKT_PLATFORM_WINDOWS)
    return SetCurrentDirectoryA(pathStr.c_str()) != 0;
#else
    return chdir(pathStr.c_str()) == 0;
#endif
}

String getExecutablePath()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(nullptr, buffer, MAX_PATH))
    {
        return String(buffer);
    }
    return {};
#elif defined(DAKT_PLATFORM_LINUX)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        return String(buffer);
    }
    return {};
#elif defined(DAKT_PLATFORM_MACOS)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
    {
        char realPath[PATH_MAX];
        if (realpath(buffer, realPath))
        {
            return String(realPath);
        }
        return String(buffer);
    }
    return {};
#endif
}

String getExecutableDirectory()
{
    String exePath = getExecutablePath();
    if (exePath.empty())
        return {};

    auto pos = exePath.find_last_of("/\\");
    if (pos != String::npos)
    {
        return exePath.substr(0, pos);
    }
    return exePath;
}

String getHomeDirectory()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, buffer)))
    {
        return String(buffer);
    }
    return {};
#else
    if (const char* home = std::getenv("HOME"))
    {
        return String(home);
    }
    if (auto* pw = getpwuid(getuid()))
    {
        return String(pw->pw_dir);
    }
    return {};
#endif
}

String getTempDirectory()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    if (GetTempPathA(MAX_PATH, buffer))
    {
        return String(buffer);
    }
    return {};
#else
    if (const char* tmp = std::getenv("TMPDIR"))
        return String(tmp);
    if (const char* tmp = std::getenv("TMP"))
        return String(tmp);
    if (const char* tmp = std::getenv("TEMP"))
        return String(tmp);
    return "/tmp";
#endif
}

String getAppDataDirectory()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, buffer)))
    {
        return String(buffer);
    }
    return {};
#elif defined(DAKT_PLATFORM_MACOS)
    String home = getHomeDirectory();
    if (!home.empty())
    {
        return home + "/Library/Application Support";
    }
    return {};
#else
    if (const char* xdg = std::getenv("XDG_DATA_HOME"))
    {
        return String(xdg);
    }
    String home = getHomeDirectory();
    if (!home.empty())
    {
        return home + "/.local/share";
    }
    return {};
#endif
}

// ============================================================================
// Process
// ============================================================================

u32 getProcessId()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    return GetCurrentProcessId();
#else
    return static_cast<u32>(getpid());
#endif
}

u32 getThreadId()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    return GetCurrentThreadId();
#else
    return static_cast<u32>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
#endif
}

void sleepMs(u32 milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void sleepUs(u32 microseconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void yield()
{
    std::this_thread::yield();
}

// ============================================================================
// Console
// ============================================================================

void setConsoleColor(ConsoleColor foreground, ConsoleColor background)
{
#if defined(DAKT_PLATFORM_WINDOWS)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attr = 0;

    if (foreground != ConsoleColor::Default)
    {
        attr |= static_cast<WORD>(foreground);
    }
    else
    {
        attr |= 7;
    }

    if (background != ConsoleColor::Default)
    {
        attr |= static_cast<WORD>(background) << 4;
    }

    SetConsoleTextAttribute(hConsole, attr);
#else
    static const char* fgCodes[] = {"30", "34", "32", "36", "31", "35", "33", "37",
                                    "90", "94", "92", "96", "91", "95", "93", "97"};
    static const char* bgCodes[] = {"40",  "44",  "42",  "46",  "41",  "45",  "43",  "47",
                                    "100", "104", "102", "106", "101", "105", "103", "107"};

    if (foreground != ConsoleColor::Default && static_cast<u8>(foreground) < 16)
    {
        std::printf("\033[%sm", fgCodes[static_cast<u8>(foreground)]);
    }
    if (background != ConsoleColor::Default && static_cast<u8>(background) < 16)
    {
        std::printf("\033[%sm", bgCodes[static_cast<u8>(background)]);
    }
#endif
}

void resetConsoleColor()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    std::printf("\033[0m");
#endif
}

bool enableAnsiEscapeCodes()
{
#if defined(DAKT_PLATFORM_WINDOWS)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode))
        return false;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, mode) != 0;
#else
    return true;
#endif
}

// ============================================================================
// Memory
// ============================================================================

void* virtualAlloc(usize size)
{
#if defined(DAKT_PLATFORM_WINDOWS)
    return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (ptr == MAP_FAILED) ? nullptr : ptr;
#endif
}

void virtualFree(void* ptr, usize size)
{
#if defined(DAKT_PLATFORM_WINDOWS)
    DAKT_UNUSED(size);
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}

bool virtualProtect(void* ptr, usize size, MemoryProtection protection)
{
#if defined(DAKT_PLATFORM_WINDOWS)
    DWORD protect = 0;
    switch (protection)
    {
        case MemoryProtection::NoAccess:
            protect = PAGE_NOACCESS;
            break;
        case MemoryProtection::Read:
            protect = PAGE_READONLY;
            break;
        case MemoryProtection::ReadWrite:
            protect = PAGE_READWRITE;
            break;
        case MemoryProtection::Execute:
            protect = PAGE_EXECUTE;
            break;
        case MemoryProtection::ReadExecute:
            protect = PAGE_EXECUTE_READ;
            break;
        case MemoryProtection::ReadWriteExecute:
            protect = PAGE_EXECUTE_READWRITE;
            break;
        default:
            protect = PAGE_NOACCESS;
            break;
    }
    DWORD oldProtect;
    return VirtualProtect(ptr, size, protect, &oldProtect) != 0;
#else
    int prot = 0;
    if (static_cast<u32>(protection) & static_cast<u32>(MemoryProtection::Read))
        prot |= PROT_READ;
    if (static_cast<u32>(protection) & static_cast<u32>(MemoryProtection::Write))
        prot |= PROT_WRITE;
    if (static_cast<u32>(protection) & static_cast<u32>(MemoryProtection::Execute))
        prot |= PROT_EXEC;
    return mprotect(ptr, size, prot) == 0;
#endif
}

// ============================================================================
// Dynamic Library
// ============================================================================

LibraryHandle loadLibrary(StringView path)
{
    String pathStr(path);
#if defined(DAKT_PLATFORM_WINDOWS)
    return LoadLibraryA(pathStr.c_str());
#else
    return dlopen(pathStr.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}

void freeLibrary(LibraryHandle handle)
{
    if (!handle)
        return;
#if defined(DAKT_PLATFORM_WINDOWS)
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

void* getLibrarySymbol(LibraryHandle handle, StringView name)
{
    if (!handle)
        return nullptr;
    String nameStr(name);
#if defined(DAKT_PLATFORM_WINDOWS)
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), nameStr.c_str()));
#else
    return dlsym(handle, nameStr.c_str());
#endif
}

}  // namespace dakt::core