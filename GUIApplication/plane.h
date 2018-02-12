#ifndef PLANE_H
#define PLANE_H
#include "point.h"

#include <QtOpenGL/QGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector>
#include <QVector3D>

class Plane : protected QOpenGLFunctions
{
private:
    QOpenGLBuffer m_vbo;
    QVector3D m_equation;
    QVector<Point> m_points;
    void calculate_Points(QVector<QVector3D> points);
    void setUpBuffer();

public:
    Plane();
    float num_between_corners(QMatrix4x4 transform, QVector3D c1, QVector3D c2);
    void recolor(float r, float g, float b);
    Plane(QVector3D equation, QVector<QVector3D> points);
    void draw(QOpenGLShaderProgram *program, int vPosition, int vColor);
};
#endif // PLANE_H
