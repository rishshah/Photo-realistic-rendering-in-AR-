#ifndef UTILS_H
#define UTILS_H

#define BASE_DIR "/home/rishshah/Downloads/Sem6/Study_Material/RnD/Photo-realistic-rendering-in-AR-/"
#define PANGOLIN_LIB BASE_DIR "Pangolin/build/src/"
#define ORB_SLAM_LIB BASE_DIR "ORB_SLAM2/lib/"
#define THIRDPARTY1_LIB BASE_DIR "Thirdparty/DBoW2/lib/"
#define THIRDPARTY2_LIB BASE_DIR "Thirdparty/g2o/lib/"
#define LD_LIBRARY_PATH "/usr/local/lib/:" BASE_DIR ":" ORB_SLAM_LIB ":" PANGOLIN_LIB ":" THIRDPARTY1_LIB ":" THIRDPARTY2_LIB ";"

#define VOCABULARY BASE_DIR "ORB_SLAM2/Vocabulary/ORBvoc.txt"
#define CAM_SETTING BASE_DIR "ORB_SLAM2/Examples/Monocular/EuRoC.yaml"

#define SLAM_POINTS_FILEPATH BASE_DIR "abc.txt"
#define SLAM_KFS_FILEPATH BASE_DIR "KeyFrameTrajectory.txt"

#define START_X 10
#define START_Y 10
#define SIZE_X 460
#define SIZE_Y 460

#include <QtOpenGL/QGLFunctions>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#include <stdio.h>
#include <Eigen/Dense>
#include <cmath>
#include <QVector3D>
#include <QVector>
#include <libpng/png.h>

void get3Points(int n,int&x,int&y,int&z);

double dist2plane(QVector3D point, QVector3D plane);

void fit_plane(QVector<QVector3D> points, QVector3D& maybe_model, double& error);

bool close_enough(QVector3D point, QVector3D plane, double error_limit);

bool good_enough(QVector<QVector3D> points, int good_num_points);

bool between_corners(QMatrix4x4 transform, QVector3D point, QVector3D c1, QVector3D c2);

QVector3D pointOnPlane(QVector3D point, QVector3D plane);

GLuint png_texture_load(const char * file_name);

GLuint LoadTexture(const char* filename, int width, int height);

#endif // UTILS_H