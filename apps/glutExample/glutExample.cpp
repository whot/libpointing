/* -*- mode: c++ -*-
 *
 * apps/glutExample/glutExample.cpp --
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

#include <pointing/transferfunctions/TransferFunction.h>

#include "PlatformSpecific.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace pointing ;

PointingDevice *input = 0 ;
DisplayDevice *output = 0 ;
TransferFunction *func = 0 ;

int sysX = 0, sysY = 0 ;
int ptrX = 0, ptrY = 0 ;
bool havePointer = false, syncPointers = true ;
int swapInterval = 0 ;

std::string inputinfo, outputinfo, funcinfo, eventinfo ;

void reshape(int width, int height) {
  glViewport(0,0,width,height) ;
  glMatrixMode(GL_PROJECTION) ;
  glLoadIdentity() ;
  glOrtho(0,width,height,0,-1,1) ;
  glMatrixMode(GL_MODELVIEW) ;
  glLoadIdentity() ;
}

static inline void
displayString(GLfloat x, GLfloat y, std::string s) {
  glRasterPos2f(x, y) ;
  for (std::string::iterator i=s.begin(); i!=s.end(); ++i)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *i) ;
}

void
display(void) {
  GLfloat hlpColor[3] = {1.0, 1.0, 1.0} ;
  GLfloat ptrColor[3] = {1.0, 1.0, 0.8} ;
  GLfloat ptrHalfSize = 10.0 ;
  GLfloat infoX=20, infoY=40, lineskip=20 ;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;

  if (havePointer) {	
    if (syncPointers) {
      ptrX = sysX ;
      ptrY = sysY ;
      syncPointers = false ;
    }
    glColor3fv(ptrColor) ;
    glPushMatrix() ;
    glTranslatef(ptrX, ptrY, 0) ;
    glBegin(GL_LINES) ;
    glVertex2f(-ptrHalfSize, 0) ;
    glVertex2f(ptrHalfSize, 0) ;
    glVertex2f(0, -ptrHalfSize) ;
    glVertex2f(0, ptrHalfSize) ;
    glEnd() ;
    glPopMatrix() ;
  }

  glColor3fv(ptrColor) ;
  displayString(infoX, infoY, inputinfo) ; infoY += lineskip ;
  displayString(infoX, infoY, funcinfo) ; infoY += lineskip ;
  displayString(infoX, infoY, outputinfo) ; infoY += lineskip ;
  infoY += lineskip ;
  displayString(infoX, infoY, eventinfo) ; infoY += lineskip ;

  glColor3fv(hlpColor) ;
  std::stringstream help ;
  help << "Press the spacebar to reset the custom pointer"
       << " or 'v' to toggle vsync (currently " << (swapInterval?"on":"off") << ")" ;
  displayString(infoX, glutGet(GLUT_SCREEN_HEIGHT)-lineskip, help.str()) ;

  glutSwapBuffers() ;
}

void
sysCallback(int x, int y) {
  sysX = x ;
  sysY = y ;
  havePointer = true ;
  glutPostRedisplay() ;
}

void
keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case ' ':
    syncPointers = true ;
    break;
  case 'v':
  case 'V':
    swapInterval=!swapInterval ;
    setSwapInterval(swapInterval) ;
    break;
  case 27:
    exit(0);
    break;
  }
  sysCallback(x, y) ;
  glutPostRedisplay() ;
}

void
ptrCallback(void * /*context*/, TimeStamp::inttime timestamp,
	    int input_dx, int input_dy, int buttons) {
  std::stringstream tmp ;
  tmp << "t=" << timestamp
      << ", dx=" << std::setw(3) << input_dx
      << ", dy=" << std::setw(3) << input_dy
      << ", btns=" << buttons ;
  eventinfo = tmp.str() ;

  int output_dx=0, output_dy=0 ;
  func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp) ;
  ptrX += output_dx ;
  ptrY += output_dy ;

  glutPostRedisplay() ;
}

int
main(int argc, char* argv[]) {

  // --- GLUT ---------------------------------------------------------------

  glutInit(&argc, argv) ;
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH) ;

  std::stringstream modestring ;
  modestring << glutGet(GLUT_SCREEN_WIDTH) << "x" << glutGet(GLUT_SCREEN_HEIGHT) ;
  glutGameModeString(modestring.str().c_str()) ;
  glutEnterGameMode() ;

  glLineWidth(1) ;

  setSwapInterval(swapInterval) ;

  glutReshapeFunc(reshape) ; // these need to be set up here because GLUT might want
  glutDisplayFunc(display) ; // to (re)display while looking for the pointing device

  // --- Pointing device ----------------------------------------------------

  input = PointingDevice::create(argc>1?argv[1]:"any:?debugLevel=1") ;

  for (TimeStamp reftime, now;
       !input->isActive() && now-reftime<15*TimeStamp::one_second; 
       now.refresh())
    PointingDevice::idle(500) ;

  std::stringstream inputinfostream ;
  inputinfostream << "PD: "
		  << input->getURI(true).asString()
		  << " [" << input->getResolution() << " cpi, "
		  << input->getUpdateFrequency() << " Hz, "
		  << (input->isActive()?"":"not ") << "active]" ;
  inputinfo = inputinfostream.str() ;
  std::cout << inputinfo << std::endl ;

  // --- Display device -----------------------------------------------------

  output = DisplayDevice::create(argc>2?argv[2]:"any:?debugLevel=1") ;

  double hdpi, vdpi;
  output->getResolution(&hdpi, &vdpi) ;
  DisplayDevice::Size size = output->getSize() ;
  DisplayDevice::Bounds bounds = output->getBounds() ;

  std::stringstream outputinfostream ;
  outputinfostream << "DD: "
		   << output->getURI(true).asString()
		   << " [" 
		   << size.width << " x " << size.height << " mm, "
		   << bounds.size.width << " x " << bounds.size.height << " pixels, "
		   << hdpi << " x " << vdpi << " PPI, "
		   << output->getRefreshRate() << " Hz]" ;
  outputinfo = outputinfostream.str() ;
  std::cout << outputinfo << std::endl ;

  // --- Transfer function --------------------------------------------------

  func = TransferFunction::create(argc>3?argv[3]:"sigmoid:?debugLevel=2", input, output) ;

  std::stringstream funcinfostream ;
  funcinfostream << "TF: " << func->getURI(true).asString() ;
  funcinfo = funcinfostream.str() ;
  std::cout << funcinfo << std::endl ;

  // --- Ready to go --------------------------------------------------------

  glutKeyboardFunc(keyboard) ;

  glutMotionFunc(sysCallback) ;
  glutPassiveMotionFunc(sysCallback) ;

  input->setPointingCallback(ptrCallback) ;

  glutMainLoop() ;

  return 0 ;
}
