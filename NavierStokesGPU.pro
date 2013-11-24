TEMPLATE = app
CONFIG += console
CONFIG -= qt

INCLUDEPATH += /usr/include/nvidia-current

LIBS+= -lOpenCL

SOURCES += \
	src/main.cpp \
	src/solver/navierStokesCPU.cpp \
	src/solver/navierStokesGPU.cpp \
    src/inputParser.cpp \
    src/viewer/Viewer.cpp \
    src/viewer/SimplePGMWriter.cpp \
    src/viewer/VTKWriter.cpp \
    src/solver/navierStokesSolver.cpp

HEADERS += \
	src/solver/navierStokesSolver.h \
	src/solver/navierStokesGPU.h \
	src/solver/navierStokesCPU.h \
    src/inputParser.h \
    src/viewer/Viewer.h \
    src/viewer/SimplePGMWriter.h \
    src/viewer/VTKWriter.h \
    src/Definitions.h

OTHER_FILES += \
    src/kernels/auxiliary.cl \
    src/kernels/updateUV.cl \
    src/kernels/rightHandSide.cl \
    src/kernels/deltaT.cl \
    src/kernels/computeFG.cl \
    src/kernels/boundaryConditions.cl

