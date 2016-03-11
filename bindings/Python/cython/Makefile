all: pointing
	python setup.py build_ext --inplace

pointing:
	cd ../../../ && qmake && make
	
clean:
	rm -rf build
	rm -f pylibpointing.so
