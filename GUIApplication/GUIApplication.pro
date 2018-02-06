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
    openglwindow.cpp

HEADERS  += mainwindow.h \
    imagemode.h \
    utils.h \
    myglwidget.h \
    openglwindow.h

INCLUDEPATH += "/home/rishshah/Downloads/Sem6/Study_Material/RnD/Photo-realistic-rendering-in-AR-/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8"

FORMS    += mainwindow.ui \
    imagemode.ui \
    openglwindow.ui
