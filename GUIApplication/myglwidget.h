#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include<QtOpenGL/QGLFunctions>
#include<QtWidgets/QOpenGLWidget>
#include<QtGui/QOpenGLVertexArrayObject>
#include<QtGui/QOpenGLBuffer>
#include<QtGui/QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void pan_mousePressEvent(QMouseEvent *event);
    void pan_mouseMoveEvent(QMouseEvent *event);
    void select_mousePressEvent(QMouseEvent *event);
    void select_mouseMoveEvent(QMouseEvent *event);
    void best_plane();

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
    void setUpPositionBuffer();
    void setUpColorBuffer();
    void setUpGridBuffer();
    bool between_corners(QVector3D point, QVector3D c1, QVector3D c2);
    void draw_plane(QVector3D plane, QVector<QVector3D> points);

    QVector<QVector3D> m_points_pos, m_points_col, m_grid_pos, m_grid_col;

    int m_xRot;
    int m_yRot;
    int m_zRot;
    int m_mode;

    QPoint m_lastPos;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo_pos, m_vbo_col, m_vbo_grid_pos, m_vbo_grid_col;

    QOpenGLShaderProgram *m_program;

    int m_mvMatrixLoc;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;
};

#endif // MYGLWIDGET_H

