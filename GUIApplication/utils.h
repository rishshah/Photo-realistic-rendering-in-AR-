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

#define OUTPUT_BASE BASE_DIR "build/"
#define SLAM_POINTS_FILEPATH OUTPUT_BASE "abc.txt"
#define SLAM_POINTS_DESC_FILEPATH OUTPUT_BASE "abc.desc"
#define SLAM_KFS_FILEPATH OUTPUT_BASE "KeyFrameTrajectory.txt"
#define AUGMENT_FILEPATH OUTPUT_BASE "augmentations.txt"
#define MESH_FILEPATH BASE_DIR "sample_mesh.obj"
#define START_X 10
#define START_Y 10
#define SIZE_X 752
#define SIZE_Y 480

#include <QtOpenGL/QGLFunctions>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include "point.h"

#define BUFFER_OFFSET( offset ) ((GLvoid*) (offset))

#define ADJUST_SCENE 0
#define ADJUST_MESH 1
#define PLAYBACK 2
#define PLAY_FIRST 3
#define MATCH 4

#define ONLINE_IMAGES 0
#define ONLINE_WEBCAM 1
#define OFFLINE_IMAGES 2
#define OFFLINE_WEBCAM 3

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <eigen3/Eigen/Dense>
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

QMatrix4x4 convert2QMat(cv::Mat x);

QVector3D pointOnPlane(QVector3D point, QVector3D plane);

GLuint distorted_texture_load(cv::Mat img, bool grayscale);

GLuint LoadTexture(const char* filename, int width, int height);

#endif // UTILS_H
