# Node bindings

Read this to learn how to write bindings

## Installation

1. You need to have:
	* Stable Node.js
	* npm (Node package manager) (For ubuntu: `sudo apt-get install npm`)
	* Python version > 2.7
	* node-gyp (`sudo npm install -g node-gyp`)

2. Install libpointing and link it in *libpointing/binding.gyp* 

3. In the current folder install nan with `npm install nan`

## Compilation

* Run `make`, which will execute `node-gyp configure build`

## Running

You can run *consoleExample.js* to test the bindings with `node consoleExample.js`

# Server app

You can use libpointing in your browser.
Configure *pointingserver* app or install it with:

	npm install pointingserver

# Windows app

Follow these instructions to build the nw-app for Windows:

1. Compile libpointing in Release mode (root/pointing/pointing.vcxproj)

1. Install everything to build with nw:

	* npm install nw -g
	* npm install nw-gyp -g
	* npm install nw-builder ncp

1. In *libpointing*-folder run `npm install nan`

1. Compile bindings with libpointing/nw/pointing.vcxproj in Release mode.
   Note that, if you compile with VS 2015, the output executable may not work on previous Windows versions.
   Compiling with VS 2012 worked well.
1. In *Node/nw*-folder run `npm install`
1. Run `node builder.js`

