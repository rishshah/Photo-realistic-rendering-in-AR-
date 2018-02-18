#include "utils.h"
#include "plane.h"
#include <qt5/QtOpenGL/QGLShaderProgram>
#include <QCoreApplication>

Plane::Plane(){
    initializeOpenGLFunctions();
}

float Plane::num_between_corners(QMatrix4x4 transform, QVector3D c1, QVector3D c2){
    int num_bw_corners = 0;
    for(int i=0;i<m_points.size(); i++){
        num_bw_corners += between_corners(transform, m_points[i].position, c1, c2)?1:0;
    }
    return (float)(num_bw_corners/(float)m_points.size());
}

void Plane::recolor(float r, float g, float b){
    for(int i=0; i<m_points.size(); i++){
        m_points[i].color = QVector3D(r,g,b);
    }
}

Plane::Plane(QVector3D equation, QVector<QVector3D> points){
    initializeOpenGLFunctions();
    m_equation = equation;
    calculate_Points(points);
    setUpBuffer();
}

void Plane::setUpBuffer(){
    m_vbo.create();
    m_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_points.size() * sizeof(Point), &m_points[0], GL_STATIC_DRAW);
    m_vbo.release();
}

void Plane::adjust(QVector3D v){
    m_points[m_selected_point].position = v;
    m_points[3]= m_points[0] ;
    m_points[4]= m_points[2] ;
}

bool Plane::select_point(QMatrix4x4 transform, QVector3D c1, QVector3D c2){
    for(int i=0;i<m_points.size(); i++){
        if(between_corners(transform, m_points[i].position, c1, c2)){
            m_selected_point = i;
            return true;
        }
    }
    return false;
}

void Plane::recolor_selected(float r, float g, float b){
    m_points[m_selected_point].color = QVector3D(r,g,b);
    m_points[3]= m_points[0] ;
    m_points[4]= m_points[2] ;
}

void Plane::draw(QOpenGLShaderProgram *program, int vPosition, int vColor){
    m_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_points.size() * sizeof(Point), &m_points[0], GL_STATIC_DRAW);
    program->enableAttributeArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(0) );
    program->enableAttributeArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(sizeof(QVector3D)));
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_points.size());
    m_vbo.release();
}

void Plane::calculate_Points(QVector<QVector3D> points){
    m_points.clear();

    QVector3D unit_z = QVector3D(0,0,1);
    QVector3D unit_normal = m_equation;
    unit_normal.normalize();
    double theta = acos(unit_normal.dotProduct(unit_normal,unit_z));
    QVector3D rotation_axis = unit_normal.crossProduct(unit_normal, unit_z);
    QMatrix4x4 planeToXY;
    planeToXY.setToIdentity();
    planeToXY.rotate(theta * 180.0/3.14159 ,rotation_axis);

    float INF = 10000000;
    QVector3D tl(INF,-INF,0), br(-INF,INF,0), tr(-INF,-INF,0), bl(INF,INF,0);
    for(int i=0; i<points.size(); i++){
        QVector4D finalp = planeToXY * QVector4D(pointOnPlane(points[i], m_equation), 1.0f);
        if(finalp.x()/finalp.w() <= tl.x() and finalp.y()/finalp.w() >= tl.y()){
            tl = QVector3D(finalp.x()/finalp.w(),finalp.y()/finalp.w(),i);
        }
        if(finalp.x()/finalp.w() >= tr.x() and finalp.y()/finalp.w() >= tr.y()){
            tr = QVector3D(finalp.x()/finalp.w(),finalp.y()/finalp.w(),i);
        }
        if(finalp.x()/finalp.w() <= bl.x() and finalp.y()/finalp.w() <= bl.y()){
            bl = QVector3D(finalp.x()/finalp.w(),finalp.y()/finalp.w(),i);
        }
        if(finalp.x()/finalp.w() >= br.x() and finalp.y()/finalp.w() <= br.y()){
            br = QVector3D(finalp.x()/finalp.w(),finalp.y()/finalp.w(),i);
        }
    }
    m_points.append(Point(points[tl.z()], QVector3D(0,0,1)));
    m_points.append(Point(points[tr.z()], QVector3D(0,0,1)));
    m_points.append(Point(points[br.z()], QVector3D(0,0,1)));
    m_points.append(Point(points[tl.z()], QVector3D(0,0,1)));
    m_points.append(Point(points[br.z()], QVector3D(0,0,1)));
    m_points.append(Point(points[bl.z()], QVector3D(0,0,1)));
}
