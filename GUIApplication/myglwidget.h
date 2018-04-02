#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <../ORB_SLAM2/include/System.h>
#include <../ORB_SLAM2/include/ORBmatcher.h>
#include <QtOpenGL/QGLFunctions>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <qt5/QtOpenGL/QGLShaderProgram>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTimer>
#include <QQuaternion>
#include <QtConcurrent/QtConcurrent>

#include "plane.h"
#include "mesh.h"
#include "point.h"
#include "utils.h"
#include "camera.h"

#define DX 0.01
#define DY 0.01
#define DZ 0.01

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

struct Keyframe{
    double timestamp;
    QVector3D position;
    QQuaternion orientation;
    Keyframe(){}
    Keyframe(double t, QVector3D pos, QQuaternion q):
        timestamp(t), position(pos), orientation(q){}
};

int descriptor_match(std::vector<ORB_SLAM2::MapPoint*> map_pts, std::vector<cv::Mat> descriptors, ORB_SLAM2::ORBmatcher* matcher);

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    // Images input at start
    void fill_image_data(std::string  camSettings); // Webcam Online
    void fill_image_data(string camSettings, string mapPoints); // Webcam Offline
    void fill_image_data(std::string  camSettings, std::string imageDirectory, std::string imageCSV); // Images Online
    void fill_image_data(std::string  camSettings, std::string imageDirectory, std::string imageCSV, std::string mapPoints); // Images Offline

    // Input Events
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event, bool select_mode, std::string insert_mode);
    void keyPress(QKeyEvent *event);

    // Comfirm Click
    void add_plane();
    void remove_plane();
    void adjust_planes();
    void adjust_mesh();

    // Playback Click
    void playback();
    void playfirst();

    // Add Mesh click
    void input_mesh(std::string f);


public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void cleanup();

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;

private:
    void init_background();

    void draw_scene();
    void draw_mesh();
    void draw_planes();
    void draw_background();
    void draw_background_webcam();
    void draw_background_images();

    void adjustWorldRotationTransform();
    void adjustWorldTranslationTransform();
    void adjustProjectionTransform(int w, int h);

    void readCamSettings(std::string file);
    void readMapPoints(std::string file);
    void readImageCSV(std::string file);

    float m_xPos, m_yPos, m_zPos;
    int m_xRot, m_yRot, m_zRot;
    int m_mode, m_playback_mode;

    int m_i, m_ij;
    int m_mvMatrixLoc, m_vColor, m_vPosition, m_uIs_tp, m_vTexCoord;
    GLuint bg_tex;

    int m_selected_plane_for_removal, m_curr_image_index, m_curr_keyframe_index;
    int m_plane_1, m_plane_2;
    int m_snap_plane;
    bool m_mesh_point_selected;

    QPoint m_lastPos;

    cv::VideoCapture m_webcam;

    Camera *m_cam;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_scene_vbo, m_bg_vbo, m_key_vbo;

    QVector<Plane> m_planes;
    Mesh m_mesh;

    QVector<Point> m_scene_points, m_bg_points;
    QOpenGLShaderProgram *m_program;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_worldTranslation;
    QMatrix4x4 m_worldRotation;
    QMatrix4x4 m_camera;

    ORB_SLAM2::System* m_slam;
    ORB_SLAM2::ORBmatcher* m_matcher;
    QFuture<int> m_match_thread;

    std::vector<std::pair<std::string, double> > m_image_data;
    std::vector<QMatrix4x4> m_image_rt;
    std::vector<Keyframe> m_keyframes;
    std::vector<ORB_SLAM2::MapPoint*> m_vMPs;
    std::vector<cv::Mat> m_scene_descriptors;
    std::string m_image_dir;
    QTimer *m_timer;
    int m_simulation_param;

    FILE* m_image_csv;
};

#endif // MYGLWIDGET_H

