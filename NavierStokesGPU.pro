TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
	src/main.cpp \
	src/solver/navierStokesCPU.cpp \
	src/solver/navierStokesGPU.cpp

HEADERS += \
	src/solver/navierStokesSolver.h \
	src/solver/navierStokesGPU.h \
	src/solver/navierStokesCPU.h

