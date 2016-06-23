/* -*- mode: c++ -*-
 *
 * apps/transferFunctionEditor/main.cpp --
 *
 * Initial software
 * Authors: Izzatbek Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char **argv)
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
