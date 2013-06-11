TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
	src/main.cpp \
	src/solver/NaverStokesGPU.cpp \
	src/solver/NaverStokesCPU.cpp

HEADERS += \
	src/solver/NavierStokesSolver.h \
	src/solver/NaverStokesGPU.h \
	src/solver/NaverStokesCPU.h

