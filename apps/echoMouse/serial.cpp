/* -*- mode: c++ -*-
 *
 * apps/echoMouse/serial.cpp --
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

#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include "serial.h"

using namespace std;

// open serial port
int openPort(string path)
{
  int fd; //file descriptor for port
  fd = open(path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
    cerr << "Cannot open port" << endl;
  else
    fcntl(fd, F_SETFL, 0);
  return (fd);
}

//set options for an open serial port
void setOptions(int fd)
{
    /*
    struct termios options;

    tcgetattr(fd, &options);
    cfsetispeed(&options, B57600);
    cfsetospeed(&options, B57600);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &options);
  */
  struct termios options;
  tcgetattr(fd, &options);
  cfsetispeed(&options, B115200);
  cfsetospeed(&options, B115200);

  //No parity 8N1
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  //No flow control
  options.c_cflag &= ~CRTSCTS;

  //Turn off s/w flow control
  options.c_iflag &= ~(IXON | IXOFF | IXANY);

  //Turn on read and ignore ctrl lines
  options.c_cflag |= (CLOCAL | CREAD);

  if( tcsetattr(fd, TCSANOW, &options) < 0) {
    cerr << "Could not set attributes" << endl;
  }
}

//write to serial port
void writePort(int fd, string data)
{
  int n = write(fd, data.c_str(), data.length());
  if (n < 0)
    cerr << "Cannot write to port" << endl;
}

void writeByte(int fd, char byte)
{
    int n = write(fd, &byte, 1);
    if (n < 0)
      cerr << "Cannot write to port" << endl;
}

void closePort(int fd)
{
    close(fd);
}
