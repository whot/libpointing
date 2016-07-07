javac org/libpointing/*.java

javah -jni -d ./native/ org.libpointing.DisplayDevice
javah -jni -d ./native/ org.libpointing.PointingDevice
javah -jni -d ./native/ org.libpointing.PointingDeviceManager
javah -jni -d ./native/ org.libpointing.DisplayDeviceManager
javah -jni -d ./native/ org.libpointing.TransferFunction
javah -jni -d ./native/ org.libpointing.TimeStamp