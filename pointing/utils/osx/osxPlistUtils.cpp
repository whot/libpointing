/* -*- mode: c++ -*-
 *
 * pointing/utils/osx/osxPlistUtils.cpp --
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

#include <pointing/utils/osx/osxPlistUtils.h>

#include <iostream>

namespace pointing {

  CFPropertyListRef
  getPropertyListFromXML(const char *xml) {
    CFDataRef cfdata = CFDataCreate(kCFAllocatorDefault, (const UInt8*)xml, strlen(xml)) ;
    CFPropertyListRef plist = CFPropertyListCreateWithData(kCFAllocatorDefault,
							   cfdata,
							   kCFPropertyListImmutable,
							   NULL, NULL) ;
    CFRelease(cfdata) ;
    return plist ;
  }

  CFPropertyListRef
  getPropertyListFromFile(const char *path) {
    CFStringRef cfpath = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault,
							 path,
							 kCFStringEncodingISOLatin1,
							 kCFAllocatorNull) ;
    CFURLRef cfurl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfpath, 
						   kCFURLPOSIXPathStyle, false) ;
    CFRelease(cfpath) ;
    CFDataRef xmldata = 0 ;
    SInt32 errorCode = 0 ;
    Boolean ok = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault,
							  cfurl,
							  &xmldata,
							  NULL,
							  NULL,
							  &errorCode) ;
    CFRelease(cfurl) ;
    if (!ok) {
      std::cerr << "getPropertyListFromFile: error " << errorCode << " when trying to create CFURL" << std::endl ;
      return 0 ;
    }

#if 1
    CFPropertyListRef plist = CFPropertyListCreateWithData(kCFAllocatorDefault,
							   xmldata,
							   kCFPropertyListImmutable,
							   NULL, NULL) ;
    CFRelease(xmldata) ;
#else
    CFStringRef errorString ;
    CFPropertyListRef plist = CFPropertyListCreateFromXMLData(kCFAllocatorDefault,
							      xmldata,
							      kCFPropertyListImmutable,
							      &errorString) ;
#endif

    return plist ;
  }

}
