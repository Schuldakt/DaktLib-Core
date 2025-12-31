// ============================================================================
// DaktLib Core - FileSystem Implementation
// ============================================================================

#include <dakt/core/FileSystem.hpp>

#include <chrono>
#include <fstream>
#include <system_error>

#if defined(DAKT_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace dakt::core
{

// ============================================================================
// Path Utilities (in dakt::core::path namespace)
// ============================================================================

namespace path
{

String normalize(StringView path)
{
    String result(path);
    for (auto& c : result)
    {
        if (c == '\\')
            c = '/';
    }
    return result;
}

String join(StringView a, StringView b)
{
    if (a.empty())
        return String(b);
    if (b.empty())
        return String(a);

    String result(a);
    if (result.back() != '/' && result.back() != '\\')
    {
        result += '/';
    }

    StringView bStart = b;
    if (!b.empty() && (b[0] == '/' || b[0] == '\\'))
    {
        bStart = b.substr(1);
    }
    result += bStart;
    return result;
}

String parent(StringView path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == StringView::npos || pos == 0)
        return String(path.substr(0, pos == StringView::npos ? 0 : 1));
    return String(path.substr(0, pos));
}

String filename(StringView path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == StringView::npos)
        return String(path);
    return String(path.substr(pos + 1));
}

String extension(StringView path)
{
    auto name = filename(path);
    auto pos = name.find_last_of('.');
    if (pos == StringView::npos || pos == 0)
        return "";
    return name.substr(pos);
}

String stem(StringView path)
{
    auto name = filename(path);
    auto pos = name.find_last_of('.');
    if (pos == StringView::npos || pos == 0)
        return name;
    return name.substr(0, pos);
}

String replaceExtension(StringView path, StringView newExt)
{
    auto pos = path.find_last_of('.');
    String result;
    if (pos == StringView::npos)
    {
        result = path;
    }
    else
    {
        result = path.substr(0, pos);
    }

    if (!newExt.empty() && newExt[0] != '.')
    {
        result += '.';
    }
    result += newExt;
    return result;
}

bool isAbsolute(StringView path)
{
    if (path.empty())
        return false;

#ifdef _WIN32
    if (path.size() >= 2 && path[1] == ':')
        return true;
    if (path.size() >= 2 && (path[0] == '\\' || path[0] == '/') && (path[1] == '\\' || path[1] == '/'))
        return true;
#else
    if (path[0] == '/')
        return true;
#endif
    return false;
}

bool isRelative(StringView path)
{
    return !isAbsolute(path);
}

String makeAbsolute(StringView path)
{
    if (isAbsolute(path))
        return String(path);
    return std::filesystem::absolute(path).string();
}

String makeRelative(StringView path, StringView base)
{
    return std::filesystem::relative(path, base).string();
}

String commonPrefix(StringView a, StringView b)
{
    usize len = std::min(a.size(), b.size());
    usize lastSep = 0;

    for (usize i = 0; i < len; ++i)
    {
        if (a[i] != b[i])
            break;
        if (a[i] == '/' || a[i] == '\\')
            lastSep = i + 1;
    }

    return String(a.substr(0, lastSep));
}

std::vector<String> split(StringView path)
{
    std::vector<String> result;
    usize start = 0;

    for (usize i = 0; i < path.size(); ++i)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            if (i > start)
            {
                result.emplace_back(path.substr(start, i - start));
            }
            start = i + 1;
        }
    }

    if (start < path.size())
    {
        result.emplace_back(path.substr(start));
    }

    return result;
}

bool hasExtension(StringView path, StringView ext)
{
    auto pathExt = extension(path);
    return pathExt == ext;
}

bool extensionEquals(StringView path, StringView ext)
{
    auto pathExt = extension(path);
    if (pathExt.size() != ext.size())
        return false;

    for (usize i = 0; i < pathExt.size(); ++i)
    {
        char a = pathExt[i];
        char b = ext[i];
        if (a >= 'A' && a <= 'Z')
            a += 32;
        if (b >= 'A' && b <= 'Z')
            b += 32;
        if (a != b)
            return false;
    }
    return true;
}

}  // namespace path

// ============================================================================
// File Operations (in dakt::core::fs namespace)
// ============================================================================

namespace fs
{

bool exists(StringView path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool isFile(StringView path)
{
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

bool isDirectory(StringView path)
{
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

FileType getFileType(StringView path)
{
    std::error_code ec;
    auto status = std::filesystem::status(path, ec);
    if (ec)
        return FileType::None;

    switch (status.type())
    {
        case std::filesystem::file_type::regular:
            return FileType::Regular;
        case std::filesystem::file_type::directory:
            return FileType::Directory;
        case std::filesystem::file_type::symlink:
            return FileType::Symlink;
        default:
            return FileType::Other;
    }
}

Option<FileInfo> getFileInfo(StringView path)
{
    std::error_code ec;
    auto status = std::filesystem::status(path, ec);
    if (ec)
        return std::nullopt;

    FileInfo info;
    info.path = path;
    info.name = dakt::core::path::filename(path);
    info.type = getFileType(path);

    if (info.type == FileType::Regular)
    {
        info.size = std::filesystem::file_size(path, ec);
    }

    return info;
}

Option<u64> getFileSize(StringView path)
{
    std::error_code ec;
    auto size = std::filesystem::file_size(path, ec);
    if (ec)
        return std::nullopt;
    return size;
}

Option<i64> getModifiedTime(StringView path)
{
    std::error_code ec;
    auto ftime = std::filesystem::last_write_time(path, ec);
    if (ec)
        return std::nullopt;

    // Convert file_time to system_clock
    auto duration = ftime.time_since_epoch();
    auto sysTime = std::chrono::duration_cast<std::chrono::seconds>(duration);
    return static_cast<i64>(sysTime.count());
}

// ============================================================================
// File Reading
// ============================================================================

GenericResult<Buffer> readFile(StringView path)
{
    std::ifstream file(String(path), std::ios::binary | std::ios::ate);
    if (!file)
    {
        return makeError(ErrorCode::NotFound, "Failed to open file");
    }

    auto size = file.tellg();
    if (size < 0)
    {
        return makeError(ErrorCode::IoError, "Failed to get file size");
    }

    file.seekg(0);

    Buffer buffer(static_cast<usize>(size));
    buffer.resize(static_cast<usize>(size));

    if (size > 0 && !file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        return makeError(ErrorCode::IoError, "Failed to read file");
    }

    return buffer;
}

GenericResult<String> readTextFile(StringView path)
{
    std::ifstream file(String(path), std::ios::in | std::ios::ate);
    if (!file)
    {
        return makeError(ErrorCode::NotFound, "Failed to open file");
    }

    auto size = file.tellg();
    if (size < 0)
    {
        return makeError(ErrorCode::IoError, "Failed to get file size");
    }

    file.seekg(0);

    String content(static_cast<usize>(size), '\0');
    if (size > 0 && !file.read(content.data(), size))
    {
        return makeError(ErrorCode::IoError, "Failed to read file");
    }

    return content;
}

GenericResult<std::vector<String>> readLines(StringView path)
{
    String pathStr(path);
    std::ifstream file(pathStr);
    if (!file)
    {
        return makeError(ErrorCode::NotFound, "Failed to open file");
    }

    std::vector<String> lines;
    std::string line;  // Use std::string for getline
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }

    return lines;
}

// ============================================================================
// File Writing
// ============================================================================

GenericResult<Unit> writeFile(StringView path, ConstByteSpan data)
{
    std::ofstream file(String(path), std::ios::binary);
    if (!file)
    {
        return makeError(ErrorCode::IoError, "Failed to create file");
    }

    if (!data.empty() && !file.write(reinterpret_cast<const char*>(data.data()), data.size()))
    {
        return makeError(ErrorCode::IoError, "Failed to write file");
    }

    return Unit{};
}

GenericResult<Unit> writeTextFile(StringView path, StringView text)
{
    String pathStr(path);
    std::ofstream file(pathStr);
    if (!file)
    {
        return makeError(ErrorCode::IoError, "Failed to create file");
    }

    if (!text.empty() && !file.write(text.data(), text.size()))
    {
        return makeError(ErrorCode::IoError, "Failed to write file");
    }

    return Unit{};
}

GenericResult<Unit> appendFile(StringView path, ConstByteSpan data)
{
    std::ofstream file(String(path), std::ios::binary | std::ios::app);
    if (!file)
    {
        return makeError(ErrorCode::IoError, "Failed to open file for append");
    }

    if (!data.empty() && !file.write(reinterpret_cast<const char*>(data.data()), data.size()))
    {
        return makeError(ErrorCode::IoError, "Failed to append to file");
    }

    return Unit{};
}

GenericResult<Unit> appendTextFile(StringView path, StringView text)
{
    std::ofstream file(String(path), std::ios::app);
    if (!file)
    {
        return makeError(ErrorCode::IoError, "Failed to open file for append");
    }

    if (!text.empty() && !file.write(text.data(), text.size()))
    {
        return makeError(ErrorCode::IoError, "Failed to append to file");
    }

    return Unit{};
}

// ============================================================================
// File Operations
// ============================================================================

GenericResult<Unit> copyFile(StringView src, StringView dst, bool overwrite)
{
    std::error_code ec;
    auto options = overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none;
    std::filesystem::copy_file(src, dst, options, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<Unit> moveFile(StringView src, StringView dst)
{
    std::error_code ec;
    std::filesystem::rename(src, dst, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<Unit> deleteFile(StringView path)
{
    std::error_code ec;
    if (!std::filesystem::remove(path, ec) && ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

// ============================================================================
// Directory Operations
// ============================================================================

GenericResult<Unit> createDirectory(StringView path)
{
    std::error_code ec;
    std::filesystem::create_directory(path, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<Unit> createDirectories(StringView path)
{
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<Unit> deleteDirectory(StringView path)
{
    std::error_code ec;
    std::filesystem::remove(path, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<Unit> deleteDirectoryRecursive(StringView path)
{
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<Unit> copyDirectory(StringView source, StringView dest)
{
    std::error_code ec;
    std::filesystem::copy(
        source, dest, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

// ============================================================================
// Directory Listing
// ============================================================================

GenericResult<std::vector<String>> listDirectory(StringView path)
{
    std::error_code ec;
    std::vector<String> result;

    for (const auto& entry : std::filesystem::directory_iterator(path, ec))
    {
        if (ec)
            break;
        result.push_back(entry.path().filename().string());
    }

    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }

    return result;
}

GenericResult<std::vector<FileInfo>> listDirectoryInfo(StringView path)
{
    std::error_code ec;
    std::vector<FileInfo> result;

    for (const auto& entry : std::filesystem::directory_iterator(path, ec))
    {
        if (ec)
            break;

        FileInfo info;
        info.path = entry.path().string();
        info.name = entry.path().filename().string();
        info.type = entry.is_directory()      ? FileType::Directory
                    : entry.is_regular_file() ? FileType::Regular
                    : entry.is_symlink()      ? FileType::Symlink
                                              : FileType::Other;

        if (info.type == FileType::Regular)
        {
            info.size = entry.file_size(ec);
        }

        result.push_back(std::move(info));
    }

    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }

    return result;
}

GenericResult<std::vector<String>> glob(StringView pattern)
{
    std::vector<String> result;

    String patternStr(pattern);
    auto starPos = patternStr.find('*');
    if (starPos == String::npos)
    {
        if (exists(pattern))
        {
            result.push_back(patternStr);
        }
        return result;
    }

    String dir = starPos > 0 ? dakt::core::path::parent(pattern) : ".";
    String filePattern = dakt::core::path::filename(pattern);

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
    {
        if (ec)
            break;
        if (!entry.is_regular_file())
            continue;

        String name = entry.path().filename().string();
        if (filePattern == "*" || (filePattern.starts_with("*.") && name.ends_with(filePattern.substr(1))))
        {
            result.push_back(entry.path().string());
        }
    }

    return result;
}

// ============================================================================
// Directory Traversal
// ============================================================================

GenericResult<Unit> walkDirectory(StringView path, TraversalCallback callback, bool recursive, int maxDepth)
{
    std::error_code ec;
    auto options = std::filesystem::directory_options::skip_permission_denied;

    std::function<TraversalAction(const std::filesystem::path&, int)> walk;
    walk = [&](const std::filesystem::path& p, int d) -> TraversalAction
    {
        if (maxDepth >= 0 && d > maxDepth)
            return TraversalAction::Skip;

        for (const auto& entry : std::filesystem::directory_iterator(p, options, ec))
        {
            if (ec)
                return TraversalAction::Stop;

            FileInfo info;
            info.path = entry.path().string();
            info.name = entry.path().filename().string();
            info.type = entry.is_directory()      ? FileType::Directory
                        : entry.is_regular_file() ? FileType::Regular
                        : entry.is_symlink()      ? FileType::Symlink
                                                  : FileType::Other;

            if (info.type == FileType::Regular)
            {
                info.size = entry.file_size(ec);
            }

            auto action = callback(info, d);
            if (action == TraversalAction::Stop)
                return TraversalAction::Stop;

            if (action != TraversalAction::Skip && entry.is_directory() && recursive)
            {
                auto subAction = walk(entry.path(), d + 1);
                if (subAction == TraversalAction::Stop)
                    return TraversalAction::Stop;
            }
        }
        return TraversalAction::Continue;
    };

    walk(std::filesystem::path(path), 0);

    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

GenericResult<std::vector<String>> findFiles(StringView path, std::function<bool(const FileInfo&)> predicate,
                                             bool recursive)
{
    std::vector<String> result;

    auto callback = [&](const FileInfo& info, int) -> TraversalAction
    {
        if (predicate(info))
        {
            result.push_back(info.path);
        }
        return TraversalAction::Continue;
    };

    auto walkResult = walkDirectory(path, callback, recursive);
    if (!walkResult)
    {
        return makeError(walkResult.error().code, walkResult.error().message);
    }

    return result;
}

GenericResult<std::vector<String>> findFilesByExtension(StringView path, StringView extension, bool recursive)
{
    return findFiles(
        path, [extension](const FileInfo& info)
        { return info.type == FileType::Regular && dakt::core::path::extensionEquals(info.path, extension); },
        recursive);
}

// ============================================================================
// Temporary Files
// ============================================================================

String getTempFilePath(StringView prefix, StringView extension)
{
    auto tempDir = std::filesystem::temp_directory_path();
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    String filename = String(prefix) + "_" + std::to_string(now) + String(extension);
    return (tempDir / filename).string();
}

GenericResult<String> createTempFile(StringView prefix, StringView extension)
{
    auto filePath = getTempFilePath(prefix, extension);
    std::ofstream file(filePath);
    if (!file)
    {
        return makeError(ErrorCode::IoError, "Failed to create temp file");
    }
    return filePath;
}

GenericResult<String> createTempDirectory(StringView prefix)
{
    auto tempDir = std::filesystem::temp_directory_path();
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    String dirname = String(prefix) + "_" + std::to_string(now);
    auto dirPath = (tempDir / dirname).string();

    std::error_code ec;
    std::filesystem::create_directory(dirPath, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }

    return dirPath;
}

// ============================================================================
// File Watching
// ============================================================================

bool hasBeenModified(StringView path, i64 sinceTime)
{
    auto modTime = getModifiedTime(path);
    if (!modTime)
        return false;
    return *modTime > sinceTime;
}

// ============================================================================
// Working Directory
// ============================================================================

String getCurrentDirectory()
{
    return std::filesystem::current_path().string();
}

GenericResult<Unit> setCurrentDirectory(StringView path)
{
    std::error_code ec;
    std::filesystem::current_path(path, ec);
    if (ec)
    {
        return makeError(ErrorCode::IoError, ec.message());
    }
    return Unit{};
}

// ============================================================================
// Memory Mapped File
// ============================================================================

MemoryMappedFile::~MemoryMappedFile()
{
    close();
}

void MemoryMappedFile::close()
{
    if (!m_data)
        return;

#if defined(DAKT_PLATFORM_WINDOWS)
    UnmapViewOfFile(m_data);
    if (m_mappingHandle)
        CloseHandle(m_mappingHandle);
    if (m_fileHandle)
        CloseHandle(m_fileHandle);
    m_mappingHandle = nullptr;
    m_fileHandle = nullptr;
#else
    munmap(m_data, m_size);
    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
#endif

    m_data = nullptr;
    m_size = 0;
}

#if defined(DAKT_PLATFORM_WINDOWS)

GenericResult<MemoryMappedFile> MemoryMappedFile::open(StringView path, bool writable)
{
    MemoryMappedFile mmf;

    DWORD access = GENERIC_READ | (writable ? GENERIC_WRITE : 0);
    DWORD share = FILE_SHARE_READ;

    mmf.m_fileHandle =
        CreateFileA(String(path).c_str(), access, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (mmf.m_fileHandle == INVALID_HANDLE_VALUE)
    {
        return makeError(ErrorCode::NotFound, "Failed to open file");
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(mmf.m_fileHandle, &fileSize))
    {
        CloseHandle(mmf.m_fileHandle);
        return makeError(ErrorCode::IoError, "Failed to get file size");
    }

    mmf.m_size = static_cast<u64>(fileSize.QuadPart);

    DWORD protect = writable ? PAGE_READWRITE : PAGE_READONLY;
    mmf.m_mappingHandle = CreateFileMappingA(mmf.m_fileHandle, nullptr, protect, 0, 0, nullptr);

    if (!mmf.m_mappingHandle)
    {
        CloseHandle(mmf.m_fileHandle);
        return makeError(ErrorCode::IoError, "Failed to create file mapping");
    }

    DWORD mapAccess = writable ? FILE_MAP_WRITE : FILE_MAP_READ;
    mmf.m_data = static_cast<byte*>(MapViewOfFile(mmf.m_mappingHandle, mapAccess, 0, 0, 0));

    if (!mmf.m_data)
    {
        CloseHandle(mmf.m_mappingHandle);
        CloseHandle(mmf.m_fileHandle);
        return makeError(ErrorCode::IoError, "Failed to map view of file");
    }

    mmf.m_writable = writable;
    return mmf;
}

GenericResult<MemoryMappedFile> MemoryMappedFile::create(StringView path, u64 size)
{
    MemoryMappedFile mmf;

    mmf.m_fileHandle = CreateFileA(String(path).c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);

    if (mmf.m_fileHandle == INVALID_HANDLE_VALUE)
    {
        return makeError(ErrorCode::IoError, "Failed to create file");
    }

    LARGE_INTEGER li;
    li.QuadPart = static_cast<LONGLONG>(size);
    if (!SetFilePointerEx(mmf.m_fileHandle, li, nullptr, FILE_BEGIN) || !SetEndOfFile(mmf.m_fileHandle))
    {
        CloseHandle(mmf.m_fileHandle);
        return makeError(ErrorCode::IoError, "Failed to set file size");
    }

    mmf.m_size = size;

    mmf.m_mappingHandle = CreateFileMappingA(mmf.m_fileHandle, nullptr, PAGE_READWRITE, static_cast<DWORD>(size >> 32),
                                             static_cast<DWORD>(size), nullptr);

    if (!mmf.m_mappingHandle)
    {
        CloseHandle(mmf.m_fileHandle);
        return makeError(ErrorCode::IoError, "Failed to create file mapping");
    }

    mmf.m_data = static_cast<byte*>(MapViewOfFile(mmf.m_mappingHandle, FILE_MAP_WRITE, 0, 0, 0));

    if (!mmf.m_data)
    {
        CloseHandle(mmf.m_mappingHandle);
        CloseHandle(mmf.m_fileHandle);
        return makeError(ErrorCode::IoError, "Failed to map view of file");
    }

    mmf.m_writable = true;
    return mmf;
}

GenericResult<Unit> MemoryMappedFile::flush()
{
    if (!m_data || !m_writable)
        return Unit{};

    if (!FlushViewOfFile(m_data, 0))
    {
        return makeError(ErrorCode::IoError, "Failed to flush mapped file");
    }
    return Unit{};
}

#else  // POSIX

GenericResult<MemoryMappedFile> MemoryMappedFile::open(StringView path, bool writable)
{
    MemoryMappedFile mmf;

    int flags = writable ? O_RDWR : O_RDONLY;
    mmf.m_fd = ::open(String(path).c_str(), flags);
    if (mmf.m_fd < 0)
    {
        return makeError(ErrorCode::NotFound, "Failed to open file");
    }

    struct stat st;
    if (fstat(mmf.m_fd, &st) < 0)
    {
        ::close(mmf.m_fd);
        return makeError(ErrorCode::IoError, "Failed to get file size");
    }

    mmf.m_size = static_cast<u64>(st.st_size);

    int prot = PROT_READ | (writable ? PROT_WRITE : 0);
    mmf.m_data = static_cast<byte*>(mmap(nullptr, mmf.m_size, prot, MAP_SHARED, mmf.m_fd, 0));
    if (mmf.m_data == MAP_FAILED)
    {
        mmf.m_data = nullptr;
        ::close(mmf.m_fd);
        return makeError(ErrorCode::IoError, "Failed to map file");
    }

    mmf.m_writable = writable;
    return mmf;
}

GenericResult<MemoryMappedFile> MemoryMappedFile::create(StringView path, u64 size)
{
    MemoryMappedFile mmf;

    mmf.m_fd = ::open(String(path).c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (mmf.m_fd < 0)
    {
        return makeError(ErrorCode::IoError, "Failed to create file");
    }

    if (ftruncate(mmf.m_fd, static_cast<off_t>(size)) < 0)
    {
        ::close(mmf.m_fd);
        return makeError(ErrorCode::IoError, "Failed to set file size");
    }

    mmf.m_size = size;

    mmf.m_data = static_cast<byte*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, mmf.m_fd, 0));
    if (mmf.m_data == MAP_FAILED)
    {
        mmf.m_data = nullptr;
        ::close(mmf.m_fd);
        return makeError(ErrorCode::IoError, "Failed to map file");
    }

    mmf.m_writable = true;
    return mmf;
}

GenericResult<Unit> MemoryMappedFile::flush()
{
    if (!m_data || !m_writable)
        return Unit{};

    if (msync(m_data, m_size, MS_SYNC) < 0)
    {
        return makeError(ErrorCode::IoError, "Failed to flush mapped file");
    }
    return Unit{};
}

#endif  // Platform

}  // namespace fs

}  // namespace dakt::core
