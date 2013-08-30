TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
	src/main.cpp \
	src/solver/navierStokesCPU.cpp \
	src/solver/navierStokesGPU.cpp \
    src/inputParser.cpp \
    src/viewer/Viewer.cpp \
    src/viewer/SimplePGMWriter.cpp \
    src/viewer/VTKWriter.cpp

HEADERS += \
	src/solver/navierStokesSolver.h \
	src/solver/navierStokesGPU.h \
	src/solver/navierStokesCPU.h \
    src/inputParser.h \
    src/viewer/Viewer.h \
    src/viewer/SimplePGMWriter.h \
    src/viewer/VTKWriter.h

