/* -*- mode: c++ -*-
 *
 * pointing/utils/osx/osxTimeUtils.h --
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

#ifndef osxTimeUtils_h
#define osxTimeUtils_h

#include <pointing/utils/TimeStamp.h>
#include <pointing/input/osx/osxPrivateMultitouchSupport.h>

#include <CoreServices/CoreServices.h>
#include <mach/mach_time.h>

using namespace pointing ;

class MachAbsTimeConverter {

  uint64_t mach_epoch ;
  TimeStamp::inttime epoch ;

  uint64_t nanoseconds(uint64_t mabst) {
    Nanoseconds nanos = AbsoluteToNanoseconds(*(AbsoluteTime *)&mabst) ;
    return (*(uint64_t *)&nanos) ;
  }
  
public:

  MachAbsTimeConverter(void) {
    mach_epoch = mach_absolute_time() ;
    epoch = TimeStamp::createAsInt() ;
  }

  TimeStamp::inttime convert(uint64_t mabst) {
    return epoch + (TimeStamp::inttime)nanoseconds(mabst-mach_epoch) * TimeStamp::one_nanosecond ;
  }
  
} ;

class MTAbsTimeConverter {

  uint64_t mt_epoch ;
  TimeStamp::inttime epoch ;
  
public:

  MTAbsTimeConverter(void) {
    mt_epoch = MTAbsoluteTimeGetCurrent() ;
    epoch = TimeStamp::createAsInt() ;
  }

  TimeStamp::inttime convert(double mtabst) {
    return epoch + (TimeStamp::inttime)((mtabst-mt_epoch)*TimeStamp::one_second) ;
  }
  
} ;

#endif
