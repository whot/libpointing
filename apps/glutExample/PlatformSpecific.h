/* -*- mode: c++ -*-
 *
 * apps/glutExample/PlatformSpecific.h --
 *
 * Initial software
 * Authors: Gery Casiez, Nicolas Roussel
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#ifndef PlatformSpecific_h
#define PlatformSpecific_h

#include <string.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#endif

#ifdef __linux__
#include <GL/glut.h>
#include <GL/glx.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>
#endif

static inline void
setSwapInterval(int n) {
#ifdef __APPLE__
  GLint sync = n ;
  CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync) ;
#else
  typedef void (*swapIntervalExtFunc) (int) ;
  swapIntervalExtFunc swapIntervalEXT = NULL ;
  char *extensions = (char*)glGetString(GL_EXTENSIONS) ;
#ifdef WIN32
  if (strstr(extensions,"WGL_EXT_swap_control"))
    swapIntervalEXT = (swapIntervalExtFunc)wglGetProcAddress("wglSwapIntervalEXT") ;
#endif
#ifdef __linux__
  if (strstr(extensions,"GLX_MESA_swap_control"))
    swapIntervalEXT = (swapIntervalExtFunc)glXGetProcAddress((const GLubyte*)"glXSwapIntervalMESA") ;
  else if (strstr(extensions,"GLX_SGI_swap_control"))
    swapIntervalEXT = (swapIntervalExtFunc)glXGetProcAddress((const GLubyte*)"glXSwapIntervalSGI") ;
#endif
  if (swapIntervalEXT!=NULL) swapIntervalEXT(n) ;
#endif
  glutPostRedisplay() ;
}

#endif
