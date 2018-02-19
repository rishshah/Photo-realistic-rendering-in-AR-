#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

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
#include "plane.h"
#include "mesh.h"
#include "point.h"
#include "utils.h"

#define DX 0.1
#define DY 0.1
#define DZ 0.1

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

struct Keyframe{
    double timestamp;
    QVector3D position;
    QQuaternion orientation;
    Keyframe(){}
    Keyframe(double t, QVector3D pos, QQuaternion q):
        timestamp(t), position(pos), orientation(q){}
};

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    // Images input at start
    void fill_image_data(std::string dir, std::string csv);

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
    void read_points();
    int get_keyframe_index(int start, double val);
    void draw_scene();
    void draw_mesh();
    void draw_background();
    void init_background();

    int m_xRot; float m_xPos;
    int m_yRot; float m_yPos;
    int m_zRot; float m_zPos;
    int m_mode;

    int m_i;
    int m_mvMatrixLoc, m_vColor, m_vPosition, m_uIs_tp, m_vTexCoord;
    GLuint bg_tex;

    int m_selected_plane_for_removal, m_curr_image_index, m_curr_keyframe_index;
    int m_plane_1, m_plane_2;
    int m_snap_plane;
    bool m_mesh_point_selected;

    QPoint m_lastPos;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_scene_vbo, m_bg_vbo;

    QVector<Plane> m_planes;
    Mesh m_mesh;

    QVector<Point> m_scene_points, m_bg_points;
    QOpenGLShaderProgram *m_program;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;
    QMatrix4x4 m_keyframe_transform;

    std::vector<std::pair<std::string, double> > m_image_data;
    std::vector<Keyframe> m_keyframes;
    std::string m_image_dir;
    QTimer *m_timer;
    int m_simulation_time_ms;
};

#endif // MYGLWIDGET_H

