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
#include "plane.h"
#include "point.h"
#include "utils.h"

#define DX 0.1
#define DY 0.1
#define DZ 0.1

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event, bool select_mode, bool add_mode);
    void keyPress(QKeyEvent *event);
    void add_plane();
    void remove_plane();
    void playback();
    void fill_image_data(std::string dir, std::string csv);
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
    void draw_scene();
    void draw_mesh();
    void draw_background();
    void init_background();

    int m_xRot; float m_xPos;
    int m_yRot; float m_yPos;
    int m_zRot; float m_zPos;
    int m_mode;

    QPoint m_lastPos;
    int m_selected_plane, m_curr_image_index;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_scene_vbo, m_mesh_vbo, m_bg_vbo;

    QVector<Plane> m_planes;
    QVector<Point> m_scene_points, m_mesh_points, m_bg_points;

    QOpenGLShaderProgram *m_program;

    int m_mvMatrixLoc, m_vColor, m_vPosition, m_uIs_tp, m_vTexCoord;
    GLuint bg_tex;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;

    std::vector<std::pair<std::string, double> > m_image_data;
    std::string m_image_dir;
    QTimer *m_timer;
};

#endif // MYGLWIDGET_H

