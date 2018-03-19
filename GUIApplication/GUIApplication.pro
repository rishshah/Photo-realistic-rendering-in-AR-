#-------------------------------------------------
#
# Project created by QtCreator 2018-02-03T11:15:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

TARGET = GUIApplication
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imagemode.cpp \
    myglwidget.cpp \
    openglwindow.cpp \
    utils.cpp \
    plane.cpp \
    point.cpp \
    mesh.cpp \
    camera.cpp

HEADERS  += mainwindow.h \
    imagemode.h \
    utils.h \
    myglwidget.h \
    openglwindow.h \
    plane.h \
    point.h \
    mesh.h \
    camera.h

ORB_INC = ../ORB_SLAM2/ ../ORB_SLAM2/include/
PANG_INC = ../Pangolin/include/ ../Pangolin/ ../Pangolin/build/src/include/
EIGEN_INC = ../ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/

OPENGL_LIB= -lGLEW -lglfw -lGL
OPENCV_LIB= `pkg-config opencv --cflags --libs`
ORB_LIB= -L../ORB_SLAM2/lib/ -lORB_SLAM2
PANG_LIB= -L../Pangolin/build/src/ -lpangolin

LIBS += $$ORB_LIB \
        $$PANG_LIB \
        $$OPENCV_LIB \
        $$OPENGL_LIB \
        -lpng

INCLUDEPATH +=  $$ORB_INC \
                $$PANG_INC \
                $$EIGEN_INC

FORMS    += mainwindow.ui \
    imagemode.ui \
    openglwindow.ui

unix:!macx: LIBS += -L$$PWD/../ORB_SLAM2/lib/ -lORB_SLAM2

INCLUDEPATH += $$PWD/../ORB_SLAM2/include
DEPENDPATH += $$PWD/../ORB_SLAM2/include

unix:!macx: LIBS += -L$$PWD/../Pangolin/build/src/ -lpangolin

INCLUDEPATH += $$PWD/../Pangolin/build/src
DEPENDPATH += $$PWD/../Pangolin/build/src
