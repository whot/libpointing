/* -*- mode: c++ -*-
 *
 * apps/ballistics/ballistics.cpp --
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

#include "BallisticsPlayground.h"

#include <QApplication>
#include <QStyleFactory>

#include <iostream>
#include <stdexcept>

#ifdef _MSC_VER // Visual Studio C++
#  define USE_GETOPT 0
int optind = 1 ;
#else
#  include <getopt.h>
#  define USE_GETOPT 1
#endif

int
main(int argc, char **argv) {
  BallisticsPlayground *playground = 0 ;

  try {
    QApplication::setStyle(QStyleFactory::create("plastique")) ;

    QApplication app(argc, argv) ;

    char *input_uri = 0 ;
    char *output_uri = 0 ;
    bool transparent = false ;

#if USE_GETOPT
    int ch ;
    while ((ch = getopt(argc, argv, "i:o:t")) != -1) {
      switch (ch) {
      case 'i': input_uri = optarg ; break ;
      case 'o': output_uri = optarg ; break ;
      case 't': transparent = true ; break ;
      default: 
	std::cerr << "Usage: " << argv[0] << " [-i pointing-device-uri] [-o display-device-uri] [-t(ransparent window)]" << std::endl ;
	return -1 ;
      }
    }
#endif

    argc -= optind ;
    argv += optind ;

#if BALLISCTICSPLAYGROUND_USE_OPENGL
    std::cerr << "OpenGL-based BallisticsPlayground will use a swap interval of 0" << std::endl ;
    QGLFormat fmt ;
    fmt.setDirectRendering(true) ; 
    fmt.setSwapInterval(0) ;
    QGLFormat::setDefaultFormat(fmt) ;
#endif

    PointingDevice *input = PointingDevice::create(input_uri ? input_uri : "any:?") ;
    for (TimeStamp reftime, now;
     !input->isActive() && now-reftime<15*TimeStamp::one_second;
     now.refresh())
    PointingDevice::idle(500) ;

    DisplayDevice *output = DisplayDevice::create(output_uri ? output_uri : "any:?") ;

    playground = new BallisticsPlayground(input, output) ;
    if (argc==0)
      playground->addPointer("sigmoid:") ;
    else
      for (int i=0; i<argc; ++i)
        playground->addPointer(argv[i]) ;

    if (transparent) {
#if BALLISCTICSPLAYGROUND_USE_OPENGL
      std::cerr << "Warning: can't set transparent background with OpenGL-based BallisticsPlayground" << std::endl ;
#else
      playground->setAttribute(Qt::WA_TranslucentBackground, true) ;
      playground->setWindowFlags(Qt::WindowStaysOnTopHint) ;
      playground->setFocusPolicy(Qt::NoFocus) ;
#endif
    }

    playground->showFullScreen() ;
    if (!transparent) playground->raise() ;

    int result = app.exec() ;
    delete playground ;
    return result ;
  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Exception: " << e.what() << std::endl ;
  }

  delete playground ;
  return -1 ;
}
