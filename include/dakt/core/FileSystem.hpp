#pragma once

// ============================================================================
// DaktLib Core - FileSystem
// Basic file system operations using C++23 features
// (This is the low-level API - VFS is a separate module)
// ============================================================================

#include "Buffer.hpp"
#include "Macros.hpp"
#include "Types.hpp"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace dakt::core
{

// ============================================================================
// File System Types
// ============================================================================

// Re-export std::filesystem path
using Path = std::filesystem::path;

enum class FileType : u8
{
    None,
    Regular,
    Directory,
    Symlink,
    Other
};

struct FileInfo
{
    String path;
    String name;
    FileType type = FileType::None;
    u64 size = 0;
    i64 createdTime = 0;   // Unix timestamp
    i64 modifiedTime = 0;  // Unix timestamp
    i64 accessedTime = 0;  // Unix timestamp
    bool isReadOnly = false;
    bool isHidden = false;

    [[nodiscard]] auto operator<=>(const FileInfo&) const = default;
};

// ============================================================================
// Path Utilities (wrapping std::filesystem)
// ============================================================================

namespace path
{

// Normalize path separators (convert \ to / on all platforms)
[[nodiscard]] DAKT_API String normalize(StringView path);

// Join path components
[[nodiscard]] DAKT_API String join(StringView a, StringView b);

template <typename... Args>
    requires(std::convertible_to<Args, StringView> && ...)
[[nodiscard]] String join(StringView first, StringView second, Args... rest)
{
    return join(join(first, second), rest...);
}

// Get parent directory
[[nodiscard]] DAKT_API String parent(StringView path);

// Get filename (last component)
[[nodiscard]] DAKT_API String filename(StringView path);

// Get file extension (including dot)
[[nodiscard]] DAKT_API String extension(StringView path);

// Get filename without extension
[[nodiscard]] DAKT_API String stem(StringView path);

// Replace extension
[[nodiscard]] DAKT_API String replaceExtension(StringView path, StringView newExt);

// Check if path is absolute
[[nodiscard]] DAKT_API bool isAbsolute(StringView path);

// Check if path is relative
[[nodiscard]] DAKT_API bool isRelative(StringView path);

// Make path absolute
[[nodiscard]] DAKT_API String makeAbsolute(StringView path);

// Make path relative to base
[[nodiscard]] DAKT_API String makeRelative(StringView path, StringView base);

// Get common prefix of two paths
[[nodiscard]] DAKT_API String commonPrefix(StringView a, StringView b);

// Split path into components
[[nodiscard]] DAKT_API std::vector<String> split(StringView path);

// Check if path has extension
[[nodiscard]] DAKT_API bool hasExtension(StringView path, StringView ext);

// Compare extensions (case-insensitive)
[[nodiscard]] DAKT_API bool extensionEquals(StringView path, StringView ext);

// Convert to std::filesystem::path
[[nodiscard]] inline Path toPath(StringView p)
{
    return Path(p);
}

}  // namespace path

// ============================================================================
// File Operations
// ============================================================================

namespace fs
{

// Check if path exists
[[nodiscard]] DAKT_API bool exists(StringView path);

// Check if path is a file
[[nodiscard]] DAKT_API bool isFile(StringView path);

// Check if path is a directory
[[nodiscard]] DAKT_API bool isDirectory(StringView path);

// Get file type
[[nodiscard]] DAKT_API FileType getFileType(StringView path);

// Get file info
[[nodiscard]] DAKT_API Option<FileInfo> getFileInfo(StringView path);

// Get file size
[[nodiscard]] DAKT_API Option<u64> getFileSize(StringView path);

// Get file modification time
[[nodiscard]] DAKT_API Option<i64> getModifiedTime(StringView path);

// ============================================================================
// File Reading
// ============================================================================

// Read entire file into buffer
[[nodiscard]] DAKT_API GenericResult<Buffer> readFile(StringView path);

// Read entire file as string
[[nodiscard]] DAKT_API GenericResult<String> readTextFile(StringView path);

// Read file lines
[[nodiscard]] DAKT_API GenericResult<std::vector<String>> readLines(StringView path);

// ============================================================================
// File Writing
// ============================================================================

// Write buffer to file
[[nodiscard]] DAKT_API GenericResult<Unit> writeFile(StringView path, ConstByteSpan data);

// Write string to file
[[nodiscard]] DAKT_API GenericResult<Unit> writeTextFile(StringView path, StringView text);

// Append to file
[[nodiscard]] DAKT_API GenericResult<Unit> appendFile(StringView path, ConstByteSpan data);
[[nodiscard]] DAKT_API GenericResult<Unit> appendTextFile(StringView path, StringView text);

// ============================================================================
// File Operations
// ============================================================================

// Copy file
[[nodiscard]] DAKT_API GenericResult<Unit> copyFile(StringView source, StringView dest, bool overwrite = false);

// Move/rename file
[[nodiscard]] DAKT_API GenericResult<Unit> moveFile(StringView source, StringView dest);

// Delete file
[[nodiscard]] DAKT_API GenericResult<Unit> deleteFile(StringView path);

// ============================================================================
// Directory Operations
// ============================================================================

// Create directory
[[nodiscard]] DAKT_API GenericResult<Unit> createDirectory(StringView path);

// Create directory and all parent directories
[[nodiscard]] DAKT_API GenericResult<Unit> createDirectories(StringView path);

// Delete directory (must be empty)
[[nodiscard]] DAKT_API GenericResult<Unit> deleteDirectory(StringView path);

// Delete directory and all contents
[[nodiscard]] DAKT_API GenericResult<Unit> deleteDirectoryRecursive(StringView path);

// Copy directory recursively
[[nodiscard]] DAKT_API GenericResult<Unit> copyDirectory(StringView source, StringView dest);

// ============================================================================
// Directory Listing
// ============================================================================

// List directory contents
[[nodiscard]] DAKT_API GenericResult<std::vector<String>> listDirectory(StringView path);

// List directory with full file info
[[nodiscard]] DAKT_API GenericResult<std::vector<FileInfo>> listDirectoryInfo(StringView path);

// List files matching pattern (glob-style)
[[nodiscard]] DAKT_API GenericResult<std::vector<String>> glob(StringView pattern);

// ============================================================================
// Directory Traversal
// ============================================================================

enum class TraversalAction
{
    Continue,  // Continue traversal
    Skip,      // Skip current directory (don't descend)
    Stop       // Stop traversal completely
};

using TraversalCallback = std::function<TraversalAction(const FileInfo& info, int depth)>;

// Walk directory tree
[[nodiscard]] DAKT_API GenericResult<Unit> walkDirectory(StringView path, TraversalCallback callback,
                                                         bool recursive = true, int maxDepth = -1);

// Find files matching predicate
[[nodiscard]] DAKT_API GenericResult<std::vector<String>>
findFiles(StringView path, std::function<bool(const FileInfo&)> predicate, bool recursive = true);

// Find files by extension
[[nodiscard]] DAKT_API GenericResult<std::vector<String>> findFilesByExtension(StringView path, StringView extension,
                                                                               bool recursive = true);

// ============================================================================
// Temporary Files
// ============================================================================

// Get temp file path
[[nodiscard]] DAKT_API String getTempFilePath(StringView prefix = "dakt", StringView extension = ".tmp");

// Create temp file and return path
[[nodiscard]] DAKT_API GenericResult<String> createTempFile(StringView prefix = "dakt", StringView extension = ".tmp");

// Create temp directory and return path
[[nodiscard]] DAKT_API GenericResult<String> createTempDirectory(StringView prefix = "dakt");

// ============================================================================
// File Watching (basic - for advanced use the VFS module)
// ============================================================================

// Check if file has been modified since given time
[[nodiscard]] DAKT_API bool hasBeenModified(StringView path, i64 sinceTime);

// ============================================================================
// Memory Mapped Files
// ============================================================================

class MemoryMappedFile
{
public:
    MemoryMappedFile() = default;
    ~MemoryMappedFile();

    DAKT_NON_COPYABLE(MemoryMappedFile);
    DAKT_DEFAULT_MOVE(MemoryMappedFile);

    // Open file for mapping
    [[nodiscard]] static GenericResult<MemoryMappedFile> open(StringView path, bool writable = false);

    // Create and open new file
    [[nodiscard]] static GenericResult<MemoryMappedFile> create(StringView path, u64 size);

    // Close the mapping
    void close();

    // Access data
    [[nodiscard]] byte* data() noexcept { return m_data; }
    [[nodiscard]] const byte* data() const noexcept { return m_data; }
    [[nodiscard]] u64 size() const noexcept { return m_size; }
    [[nodiscard]] bool isOpen() const noexcept { return m_data != nullptr; }
    [[nodiscard]] bool isWritable() const noexcept { return m_writable; }

    // Get as span
    [[nodiscard]] ByteSpan span() noexcept { return ByteSpan(m_data, m_size); }
    [[nodiscard]] ConstByteSpan span() const noexcept { return ConstByteSpan(m_data, m_size); }

    // Flush changes to disk
    [[nodiscard]] GenericResult<Unit> flush();

private:
    byte* m_data = nullptr;
    u64 m_size = 0;
    bool m_writable = false;

#if defined(DAKT_PLATFORM_WINDOWS)
    void* m_fileHandle = nullptr;
    void* m_mappingHandle = nullptr;
#else
    int m_fd = -1;
#endif
};

}  // namespace fs

}  // namespace dakt::core