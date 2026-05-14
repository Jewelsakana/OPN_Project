#ifndef FILESYSTEMCOMPAT_H
#define FILESYSTEMCOMPAT_H

#if __has_include(<filesystem>) && (!defined(__GNUC__) || __GNUC__ >= 9)
#  include <filesystem>
   namespace fs = std::filesystem;
#  define HAS_FILESYSTEM 1
#elif __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
   namespace fs = std::experimental::filesystem;
#  define HAS_FILESYSTEM 1
#else
#  define HAS_FILESYSTEM 0
#endif

#endif // FILESYSTEMCOMPAT_H
