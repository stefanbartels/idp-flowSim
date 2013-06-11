TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
	src/main.cpp \
    src/solver/NavierStokesCPU.cpp \
    src/solver/NavierStokesGPU.cpp

HEADERS += \
	src/solver/NavierStokesSolver.h \
    src/solver/NavierStokesCPU.h \
    src/solver/NavierStokesGPU.h

