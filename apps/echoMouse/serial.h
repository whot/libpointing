/* -*- mode: c++ -*-
 *
 * apps/echoMouse/serial.h --
 *
 * Initial software
 * Authors: Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <string>

int openPort(std::string path);

void setOptions(int fd);

void writePort(int fd, std::string data);

void writeByte(int fd, char byte);

void closePort(int fd);


#endif // SERIAL_H
