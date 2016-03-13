# Libpointing

[![Build Status](https://travis-ci.org/INRIA/libpointing.svg?branch=master)](https://travis-ci.org/INRIA/libpointing)

An open-source cross-platform library to get raw events from pointing devices and master transfer functions.

## Description

Libpointing is an open-source cross-platform library that provides direct access to HID pointing devices and supports the design of pointing transfer functions.

## Installation

Visit [installation page](http://libpointing.org/user-guide/install/) or build it from source.

## Features

* runs on Windows, Mac OS X and Linux,
* makes it easy to choose the devices at run-time through the use of URIs,
* provides raw information from input devices,
* provides resolution and frequency information for the available pointing and display devices,
* supports hot-plugging
* allows to bypass the system's transfer functions to receive raw asynchronous events from one or more pointing devices.
* replicates as faithfully as possible the transfer functions used by **Microsoft Windows**, **Apple OS X** and **Xorg** (the X.Org Foundation server).
* running on these three platforms, it makes it possible to compare the replicated functions to the genuine ones as well as custom ones.
* provides the functionality to use existing transfer functions, custom ones or even build your own functions.
* supports subpixel interaction.
* Java, Python, Node.js bindings are available.

## Minimalistic example

Once you correctly installed *libpointing* and linked it to your project, simply write `#include <pointing/pointing.h>` and start coding. Here is the console example which applies system specific transfer function to an input device and outputs the results to console:

	#include <iostream>
	#include <pointing/pointing.h>

	using namespace pointing;

	TransferFunction *func = 0;

    // context is user data, timestamp is a moment at which the event was received
    // input_dx, input_dy are displacements in horizontal and vertical directions
    // buttons is a variable indicating which buttons of the pointing device were pressed.
	void pointingCallback(void *, TimeStamp::inttime timestamp, int input_dx, int input_dy, int buttons) {
	    if (!func) return;

	    int output_dx = 0, output_dy = 0;
	    // In order to use a particular transfer function, its applyi method must be called.
	    func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp);

	    std::cout << "Displacements in x and y: " << input_dx << " " << input_dy << std::endl;
	    std::cout << "Corresponding pixel displacements: " << output_dx << " " << output_dy << std::endl;
	}

	int main() {
		// Basically, to start using main functionality of libpointing
		// one needs to create objects of PointingDevice, DisplayDevice classes,
		// connect them passing to TransferFunction class object.

		// Any available pointing and display devices
		// if debugLevel > 0, the list of available devices
		// and extended information will be output.
	    PointingDevice *input = PointingDevice::create("any:?debugLevel=1");
	    DisplayDevice *output = DisplayDevice::create("any:?debugLevel=1");

	    func = TransferFunction::create("sigmoid:?debugLevel=2", input, output);

	    // To receive events from PointingDevice object, a callback function must be set.
	    input->setPointingCallback(pointingCallback);
	    while (1)
	        PointingDevice::idle(100); // milliseconds

	    delete input;
	    delete output;
	    delete func;

	    return 0;
	}

## Contributors

* [Nicolas Roussel](http://interaction.lille.inria.fr/~roussel)
* [Géry Casiez](http://www.lifl.fr/~casiez/)
* Izzatbek Mukhanov

## License

This software may be used and distributed according to the terms of the GNU General Public License version 2 or any later version.