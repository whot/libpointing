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

#include <string>
#include <stdexcept>
#include <iostream>

#ifndef _MSC_VER // Not include it on Windows
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>

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

}
