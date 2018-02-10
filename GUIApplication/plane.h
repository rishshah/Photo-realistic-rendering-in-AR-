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
    Plane(QVector3D equation, QVector<QVector3D> points);
    void draw(QOpenGLShaderProgram *program, int vPosition, int vColor);
};
#endif // PLANE_H
