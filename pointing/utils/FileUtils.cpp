/* -*- mode: c++ -*-
 *
 * pointing/utils/FileUtils.cpp --
 *
 * Initial software
 * Authors: Nicolas Roussel
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/utils/FileUtils.h>

#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <cstring>

#ifndef _MSC_VER // Not include it on Windows
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#define SLASH '/'
#endif

#ifdef _WIN32
#include <windows.h>
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define SLASH '\\'
#endif

#define PATH_LENGTH 256

namespace pointing {

  bool
  fileExists(const char *filename) {
#ifndef _MSC_VER // Visual Studio C++
    struct stat statinfo ;
    if (stat(filename, &statinfo)==-1) return false ;
    return true ;
#else
#pragma warning(disable:4100)
    std::cerr << "fileExists (FileUtils) is not implemented on this platform" << std::endl ;
    return false ;
#endif
  }

  uint64_t
  getFileSize(const char *filename) {
#ifndef _MSC_VER // Visual Studio C++
    struct stat statinfo ;
    if (stat(filename, &statinfo)==-1) return 0 ;
    return statinfo.st_size ;
#else
    std::cerr << "getFileSize (FileUtils) is not implemented on this platform" << std::endl ;
    return 0 ;
#endif
  }

  void
  readFromFile(const char *filename, char *data, unsigned int size) {
#ifndef _MSC_VER // Visual Studio C++
    int fd = open(filename, O_RDONLY) ;
    if (fd==-1) {
      std::string msg("can't open ") ;
      msg.append(filename) ;
      msg.append(" (readFromFile)") ;
      throw std::runtime_error(msg) ;
    }
    if (read(fd, data, size)!=(int)size) {
      std::string msg("can't read from ") ;
      msg.append(filename) ;
      msg.append(" (readFromFile)") ;
      throw std::runtime_error(msg) ;
    }
    close(fd) ;
#else
    std::cerr << "FIXME: readFromFile (FileUtils) is not implemented on this platform" << std::endl ;
#endif
  }

  bool pointingDirExists(const char *path)
  {
    const char pointingDir[] = "/pointing";
    char tmp[PATH_LENGTH];
    sprintf(tmp, "%s%s", path, pointingDir);
    struct stat sb;
    return stat(tmp, &sb) == 0 && S_ISDIR(sb.st_mode);
  }

  bool getModulePath(char *path)
  {
#ifdef _WIN32
    HMODULE hm = NULL;

    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR) &moduleHeadersPath,
            &hm))
    {
      fprintf(stderr, "GetModuleHandle returned %d\n", GetLastError());
      return false;
    }
    if (!GetModuleFileNameA(hm, path, PATH_LENGTH))
    {
      fprintf(stderr, "GetModuleFileName returned %d\n", GetLastError());
      return false;
    }
    return true;
#else
    Dl_info info;
    if (dladdr((void *)moduleHeadersPath, &info))
    {
      strcpy(path, info.dli_fname);
      return true;
    }
    return false;
#endif
  }

  std::string moduleHeadersPath()
  {
    char path[PATH_LENGTH];
    if (getModulePath(path))
    {
      for (int i = 0; i < 5; i++) // Check 5 levels max
      {
        char *lastSlash = strrchr(path, SLASH);
        if (!lastSlash)
          break;

        sprintf(lastSlash, "%s", "/include");
        if (pointingDirExists(path))
          return std::string(path);

        // Cut the path (go to path/..)
        *lastSlash = 0;
        if (pointingDirExists(path))
          return std::string(path);
      }
    }
    return "";
  }
}
