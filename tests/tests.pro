TEMPLATE = app
CONFIG += console
CONFIG -= qt

INCLUDEPATH += /usr/include/nvidia-current

LIBS+= -lOpenCL

SOURCES += \
	test_main.cpp \
    CLTest.cpp \
    Test.cpp

HEADERS += \
	Test.h \
    AuxiliaryKernelsTest.h \
    CLTest.h
