TEMPLATE = app
CONFIG += console
CONFIG -= qt

INCLUDEPATH += /usr/include/nvidia-current

LIBS+= -lOpenCL

SOURCES += \
	test_main.cpp \
	Test.cpp \
	cltests/CLTest.cpp

HEADERS += \
	Test.h \
	cltests/AuxiliaryKernelsTest.h \
	cltests/CLTest.h \
    cltests/TimestepKernelTest.h \
    cltests/BoundaryKernelsTest.h \
    cltests/FGKernelsTest.h \
    cltests/RHSKernelTest.h \
    cltests/UpdateUVKernelTest.h
