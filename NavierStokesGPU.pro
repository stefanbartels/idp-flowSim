TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
	src/main.cpp \
	src/solver/navierStokesCPU.cpp \
	src/solver/navierStokesGPU.cpp \
    src/inputParser.cpp

HEADERS += \
	src/solver/navierStokesSolver.h \
	src/solver/navierStokesGPU.h \
	src/solver/navierStokesCPU.h \
    src/inputParser.h

