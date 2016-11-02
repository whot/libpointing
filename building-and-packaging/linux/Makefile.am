lib_LTLIBRARIES = libpointing.la

include_HEADERS = 
    ../../pointing/pointing.h						\
    ../../pointing/utils/Base64.h					\
    ../../pointing/utils/ByteOrder.h					\
    ../../pointing/utils/Correlation.h				\
    ../../pointing/utils/FileUtils.h					\
    ../../pointing/utils/TimeStamp.h					\
    ../../pointing/utils/URI.h					\
    ../../pointing/utils/ConfigDict.h					\
    ../../pointing/utils/HIDReportParser.h				\
    ../../pointing/utils/HIDItem.h					\
    ../../pointing/utils/HIDTags.h					\
    ../../pointing/utils/PointingCursor.h				\
    ../../pointing/utils/FrequencyEstimator.h				\
    ../../pointing/input/DummyPointingDevice.h			\
    ../../pointing/input/PointingDevice.h				\
    ../../pointing/output/DisplayDevice.h				\
    ../../pointing/output/DisplayDeviceManager.h			\
    ../../pointing/output/DummyDisplayDevice.h			\
    ../../pointing/transferfunctions/Composition.h			\
    ../../pointing/transferfunctions/ConstantFunction.h		\
    ../../pointing/transferfunctions/NaiveConstantFunction.h		\
    ../../pointing/transferfunctions/SigmoidFunction.h		\
    ../../pointing/transferfunctions/TransferFunction.h		\
    ../../pointing/transferfunctions/SubPixelFunction.h		\
    ../../pointing/transferfunctions/Interpolation.h			\
    ../../pointing/input/PointingDeviceManager.h			\
    ../../pointing/input/SystemPointingDevice.h			\
    ../../pointing/input/linux/linuxPointingDevice.h			\
    ../../pointing/output/linux/xorgDisplayDevice.h			\
    ../../pointing/transferfunctions/linux/xorgSystemPointerAcceleration.h	\
    ../../pointing/input/linux/linuxPointingDeviceManager.h		\
    ../../pointing/output/linux/xorgDisplayDeviceManager.h		\
    ../../pointing/input/linux/XInputHelper.h				\
    ../../pointing-xorg/transferfunctions/XorgFunction.h		\

libpointing_la_SOURCES = \
    $(include_HEADERS) \
    ../../pointing/utils/Base64.cpp					\ 
    ../../pointing/utils/ByteOrder.cpp				\ 
    ../../pointing/utils/FileUtils.cpp				\ 
    ../../pointing/utils/TimeStamp.cpp				\ 
    ../../pointing/utils/URI.cpp					\ 
    ../../pointing/utils/ConfigDict.cpp				\ 
    ../../pointing/utils/HIDItem.cpp					\ 
    ../../pointing/utils/HIDReportParser.cpp				\ 
    ../../pointing/utils/PointingCursor.cpp				\ 
    ../../pointing/utils/FrequencyEstimator.cpp			\ 
    ../../pointing/input/PointingDevice.cpp				\ 
    ../../pointing/input/DummyPointingDevice.cpp			\ 
    ../../pointing/output/DisplayDevice.cpp				\ 
    ../../pointing/output/DisplayDeviceManager.cpp			\ 
    ../../pointing/output/DummyDisplayDevice.cpp			\ 
    ../../pointing/transferfunctions/Composition.cpp			\ 
    ../../pointing/transferfunctions/ConstantFunction.cpp		\ 
    ../../pointing/transferfunctions/NaiveConstantFunction.cpp	\ 
    ../../pointing/transferfunctions/SigmoidFunction.cpp		\ 
    ../../pointing/transferfunctions/TransferFunction.cpp		\ 
    ../../pointing/transferfunctions/SubPixelFunction.cpp		\ 
    ../../pointing/transferfunctions/Interpolation.cpp		\ 
    ../../pointing/input/PointingDeviceManager.cpp			\ 
    ../../pointing/input/SystemPointingDevice.cpp			\ 
    ../../pointing/input/linux/linuxPointingDevice.cpp		\ 
    ../../pointing/output/linux/xorgDisplayDevice.cpp			\ 
    ../../pointing/transferfunctions/linux/xorgSystemPointerAcceleration.cpp	\ 
    ../../pointing/input/linux/linuxPointingDeviceManager.cpp		\ 
    ../../pointing/output/linux/xorgDisplayDeviceManager.cpp		\ 
    ../../pointing/input/linux/XInputHelper.cpp			\
    ../../pointing-xorg/transferfunctions/XorgFunction.cpp		\
    ../../pointing-xorg/transferfunctions/XorgFunctionPrivate.cpp

libpointing_la_LIBADD = \
    $(LIBUDEV_LIBS) \
    $(LIBXRANDR_LIBS) \
    $(LIBXI_LIBS) \
    $(LIBX11_LIBS)

libpointing_la_CFLAGS = \
    -DPOINTING_XORG \
    $(LIBUDEV_CFLAGS) \
    $(LIBXRANDR_CFLAGS) \
    $(LIBXI_CFLAGS) \
    $(LIBX11_CFLAGS)
