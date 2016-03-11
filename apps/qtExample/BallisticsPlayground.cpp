/* -*- mode: c++ -*-
 *
 * apps/ballistics/BallisticsPlayground.h --
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

#include <pointing/utils/TimeStamp.h>

#include <QApplication>
#include <QPainter>
#include <QDateTime>
#include <QPoint>
#include <QDebug>

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#define USE_APPLE_CURSOR 0
#endif

// ----------------------------------------------------------------------------

static QPoint
GetCursorPosition(void) {
#if ! BALLISCTICSPLAYGROUND_USE_QTPOS
#ifdef __APPLE__
  CGPoint loc = CGEventGetLocation(CGEventCreate(NULL)) ;
  return QPoint(loc.x, loc.y) ;
#endif
#ifdef WIN32
  POINT loc;
  GetCursorPos(&loc) ;
  return QPoint(loc.x, loc.y) ;
#endif
#ifdef __linux__
  // FIXME
#endif
#endif
  return QCursor::pos() ;
}

// ----------------------------------------------------------------------------

BallisticsPlayground::Pointer::Pointer(const char *funcuri, BallisticsPlayground *playground) {
  master = playground ;
  func = TransferFunction::create(funcuri, master->input, master->output) ;
  URI uri = func->getURI() ;
  name = QString::fromStdString(uri.asString()) ;
}

BallisticsPlayground::Pointer::Pointer(TransferFunction *func, BallisticsPlayground *playground) {
  master = playground ;
  this->func = func ;
  URI uri = func->getURI() ;
  name = QString::fromStdString(uri.asString()) ;
}

void
BallisticsPlayground::Pointer::moveBy(int input_dx, int input_dy, TimeStamp::inttime timestamp) {
  int output_dx=0, output_dy=0 ;
  func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp) ;
  pos += QPoint(output_dx, output_dy) ;
}

void
BallisticsPlayground::Pointer::moveTo(QPoint p) {
  pos = p ;
}

BallisticsPlayground::Pointer::~Pointer(void) {
  delete func ;
}

// ----------------------------------------------------------------------------

BallisticsPlayground::BallisticsPlayground(PointingDevice *input, 
					   DisplayDevice *output, 
					   QWidget *parent)
  : BALLISCTICSPLAYGROUND_BASECLASS(parent),
    disptimer(this) {
  this->input = input ;
  this->output = output ;

#if 0 && BALLISCTICSPLAYGROUND_USE_OPENGL
  const QGLFormat fmt = format() ;
  std::cerr << "QGLFormat swapInterval: " << fmt.swapInterval() << std::endl ;
#endif

  input->setPointingCallback(pointingCallback, (void*)this) ;

  oDesc = QString("%1 [%2 PPI @ %3 Hz]")
    .arg(output->getURI().asString().c_str())
    .arg(output->getResolution())
    .arg(output->getRefreshRate()) ;

  pdDesc = QString("QPaintDevice: log=%1/%2 DPI, phy=%3/%4 DPI")
    .arg(this->logicalDpiX())
    .arg(this->logicalDpiY())
    .arg(this->physicalDpiX())
    .arg(this->physicalDpiY()) ;

#if BALLISCTICSPLAYGROUND_FORCE_UPDATES
  connect(&disptimer, SIGNAL(timeout()), this, SLOT(update())) ;
  disptimer.start(1000.0/output->getRefreshRate()) ;
#endif

  recordfile = 0 ;
  record = 0 ;

  artificialLatency = 0 ;

  buttons = 0 ;

  sysPointerViz = SYSTEM ;
  sysPointerLoc = GetCursorPosition() ;
  setMouseTracking(true) ;

  debugLevel = 3 ;

  reset() ;
}

void
BallisticsPlayground::addPointer(const char *function_uri) {
  std::cerr << "Adding pointer " << function_uri << std::endl ;
  Pointer *p = new Pointer(function_uri, this) ;
  p->moveTo(GetCursorPosition()) ;
  pointers.push_back(p) ;
  update() ;
}

void
BallisticsPlayground::addPointer(TransferFunction *function) {
  Pointer *p = new Pointer(function, this) ;
  p->moveTo(GetCursorPosition()) ;
  pointers.push_back(p) ;
  update() ;
}

void
BallisticsPlayground::pointingCallback(void *context, TimeStamp::inttime timestamp,
				       int input_dx, int input_dy, int buttons) {
  BallisticsPlayground *playground = (BallisticsPlayground*)context ;

  playground->pointingEvent(timestamp, input_dx, input_dy, buttons) ;
}

void
BallisticsPlayground::reset(void) {
  QPoint syspos = GetCursorPosition() ;
  for (std::list<Pointer*>::iterator p=pointers.begin();
       p!=pointers.end(); ++p)
    (*p)->moveTo(syspos) ;
  QCursor::setPos(syspos) ;
}

void
BallisticsPlayground::pointingEvent(TimeStamp::inttime timestamp,
				    int input_dx, int input_dy, 
                    int buttons) {
  this->buttons = buttons ;

  for (std::list<Pointer*>::const_iterator p=pointers.begin();
       p!=pointers.end(); ++p) {
    Pointer *ptr = *p ;
    if (input_dx || input_dy)
      ptr->moveBy(input_dx, input_dy, timestamp) ;
  }

  update() ; // scheduled
  // repaint() ; // immediate
}

void
BallisticsPlayground::keyPressEvent(QKeyEvent *event) {
  if (event->text()==" ") {
    reset() ;
    update() ;
  } else if (event->text()=="p") {
    sysPointerViz =( sysPointerVisualization)((sysPointerViz+1)%4) ;
    setCursor(sysPointerViz&SYSTEM ? Qt::ArrowCursor : Qt::BlankCursor) ;
    update() ;
  } else if (event->text()=="d") {
    debugLevel++;
    if (debugLevel == 3) debugLevel=0;
    update() ;
  }

}

void
BallisticsPlayground::mouseMoveEvent(QMouseEvent *event) {
  sysPointerLoc = event->pos() ;
}

void
BallisticsPlayground::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter(this) ;
  painter.setRenderHint(QPainter::Antialiasing, false) ;

  painter.setFont(QFont("Courier", 11)) ;

  if (iDesc.text().isEmpty() && input->isActive())
    iDesc = QString("%1 [%2 CPI @ %3 Hz]")
      .arg(input->getURI().asString().c_str())
      .arg(input->getResolution())
      .arg(input->getUpdateFrequency()) ;

  painter.setPen(Qt::black) ;
  painter.setBrush(Qt::red) ;

  if (sysPointerViz&PSEUDO)
    painter.drawRect(sysPointerLoc.x()-2, sysPointerLoc.y()-2, 4, 4) ;

  painter.setBrush(Qt::white) ;
  for (std::list<Pointer*>::const_iterator p=pointers.begin();
       p!=pointers.end(); ++p) {
    QPoint lp = mapFromGlobal((*p)->pos) ;
    int x = lp.x(), y = lp.y() ;
    painter.drawRect(x-2, y-2, 4, 4) ;
    if (debugLevel > 0) painter.drawStaticText(x+7,y+10, (*p)->name) ;
  }

  if (debugLevel > 1) {
    int width = painter.window().width(), x = 20, y = 20 ;

#if BALLISCTICSPLAYGROUND_SHOW_PAINTDEVICEDPI
    painter.drawStaticText(x,y, pdDesc) ;

    y += 20 ;
#endif

    painter.drawStaticText(x,y, oDesc) ;

    y += 20 ;

    painter.drawStaticText(x,y, iDesc) ;
  }
}

BallisticsPlayground::~BallisticsPlayground(void) {

  while (pointers.size()) {
    Pointer *p = pointers.front() ;
    pointers.pop_front() ;
    delete p ;
  }

  delete output ;
  delete input ;
}
