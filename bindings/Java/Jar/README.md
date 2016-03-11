# Java bindings

Read this to learn how to write bindings

## Installation

* Install java (For ubuntu, you can follow [this tutorial](http://tecadmin.net/install-oracle-java-8-jdk-8-ubuntu-via-ppa/)

## Editing and compilation

* Write Java part
* Compile Java classes
* Copy interface functions from org_libpointing_***.h into corresponding source files and implement them
* Recompile native parts using pro/libpointing.pro

You can use `make all` to compile all.

## Running

Run one of the examples from ../ with make && make run



# Windows

## Installation

* Install jdk

## Editing and compilation

* Please run compile.bat, it will generate *class** and header files.
* Compile the project which is located in msvc-folder.
* From obtained dll now you can generate `libpointing.jar` with makejar.bat.

## Running

* Corresponding .bat files are used inside the project