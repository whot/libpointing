# Pointing Server

[Libpointing](http://libpointing.org) server application

## Description

Libpointing is an open-source cross-platform library to get raw events from pointing devices and master transfer functions.

With this server you can use it in your browser.

## Installation

* Install [libpointing](http://libpointing.org)
* npm install pointingserver -g

Make sure, that installed package can be found on your path.
For **homebrew**, you might need to run
`npm config set prefix /usr/local` before installing the package which will symlink executable scripts into your `/usr/local`.

Note that, `pointingserver` with npm can be installed only on Mac OS and Linux.

## Usage

* `pointingserver start` starts the server.
* `pointingserver stop` stops it.

Download [client-side libpointing](https://raw.githubusercontent.com/INRIA/libpointing/master/bindings/Node/client/pointing.js) and you can start using it. Read the [documentation](https://github.com/INRIA/libpointing/wiki/Javascript-Bindings) to learn how to use it.