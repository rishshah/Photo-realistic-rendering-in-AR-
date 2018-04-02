#ifndef MESH_H
#define MESH_H

#include "point.h"
#include "utils.h"

#include <QtOpenGL/QGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector>
#include <QVector3D>

class Mesh : protected QOpenGLFunctions
{
private:
    QOpenGLBuffer m_vbo;
    QVector<Point> m_points;

    QVector3D m_scale;
    QVector3D m_translation;

    int m_selected_point;
    QVector3D m_selected_point_color;

public:
    void adjust(QVector3D v, QMatrix4x4 transform1, QMatrix4x4 transform2);
    bool select_point(QMatrix4x4 transform, QVector3D c1, QVector3D c2);
    void recolor_selected(float r, float g, float b);
    Mesh();
    Mesh(std::string);
    void draw(QOpenGLShaderProgram *program, QMatrix4x4 transform, int uMatrix, int vPosition, int vColor);
    void save(FILE* fp);
    void load(FILE* fp);
};

#endif // MESH_H
