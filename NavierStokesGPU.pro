TEMPLATE = app
CONFIG += console gui opengl thread

INCLUDEPATH += /usr/include/nvidia-current

LIBS+= -lOpenCL -lQtOpenGL

SOURCES += \
	src/main.cpp \
	src/solver/navierStokesCPU.cpp \
	src/solver/navierStokesGPU.cpp \
    src/inputParser.cpp \
    src/viewer/Viewer.cpp \
    src/viewer/SimplePGMWriter.cpp \
    src/viewer/VTKWriter.cpp \
    src/solver/navierStokesSolver.cpp \
    src/Simulation.cpp \
	src/ui/MainWindow.cpp \
    src/viewer/GLViewer.cpp \
    src/CLManager.cpp

HEADERS += \
	src/solver/navierStokesSolver.h \
	src/solver/navierStokesGPU.h \
	src/solver/navierStokesCPU.h \
    src/inputParser.h \
    src/viewer/Viewer.h \
    src/viewer/SimplePGMWriter.h \
    src/viewer/VTKWriter.h \
    src/Definitions.h \
    src/Simulation.h \
	src/ui/MainWindow.h \
    src/Parameters.h \
    src/viewer/GLViewer.h \
    src/CLManager.h

OTHER_FILES += \
    src/kernels/auxiliary.cl \
    src/kernels/updateUV.cl \
    src/kernels/rightHandSide.cl \
    src/kernels/deltaT.cl \
    src/kernels/computeFG.cl \
    src/kernels/boundaryConditions.cl \
    src/kernels/pressure.cl

