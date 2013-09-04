#-------------------------------------------------
#
# Project created by QtCreator 2013-09-03T21:56:07
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = NavierStokesGPU
TEMPLATE = app

CONFIG += console
CONFIG += qt


SOURCES +=\
        src/mainwindow.cpp \
    src/inputParser.cpp \
    src/solver/navierStokesGPU.cpp \
    src/solver/navierStokesCPU.cpp \
    src/viewer/Viewer.cpp \
    src/viewer/SimplePGMWriter.cpp \
    src/fluidSimulation.cpp \
    src/viewer/SimpleQTViewer.cpp \
    src/main.cpp

HEADERS  += src/mainwindow.h \
    src/inputParser.h \
    src/solver/navierStokesCPU.h \
    src/solver/navierStokesSolver.h \
    src/solver/navierStokesGPU.h \
    src/viewer/Viewer.h \
    src/viewer/SimplePGMWriter.h \
    src/fluidSimulation.h \
    src/viewer/SimpleQTViewer.h

FORMS    += src/mainwindow.ui
