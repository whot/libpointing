javac:
	javac -encoding UTF-8 org/libpointing/*.java

javah:
	javah -jni -d ./native/ org.libpointing.DisplayDevice
	javah -jni -d ./native/ org.libpointing.PointingDevice
	javah -jni -d ./native/ org.libpointing.PointingDeviceManager
	javah -jni -d ./native/ org.libpointing.DisplayDeviceManager
	javah -jni -d ./native/ org.libpointing.TransferFunction
	javah -jni -d ./native/ org.libpointing.TimeStamp

clean:
	rm org/libpointing/*.class
	rm native/org*.h
	cd pro/ && make clean

distclean: clean
	rm -rf build/
	rm libpointing.jar

jar:
	jar cvf libpointing.jar org/libpointing/*.class org/libpointing/event/*.class

java: javac javah

nat:
	cd pro/ && qmake && make

all: javac javah jar nat
