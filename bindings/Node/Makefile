all:
	cd libpointing/ && node-gyp configure build

pointing:
	cd ../../ && qmake && make
	
clean:
	node-gyp clean