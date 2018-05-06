#include "utils.h"
#include "plane.h"
#include <qt5/QtOpenGL/QGLShaderProgram>
#include <QCoreApplication>

Plane::Plane() {
    initializeOpenGLFunctions();
}

/**
 * @brief save plane corners
 *
 * @param fp file pointer to be uesd
 */
void Plane::save(FILE* fp) {
    for (uint i = 0; i < m_points.size(); i++) {
        fprintf(fp, "%f %f %f\n", m_points[i].position.x(), m_points[i].position.y(), m_points[i].position.z());
    }
}
/**
 * @brief load plane corners
 *
 * @param fp file pointer to be used
 */
void Plane::load(FILE* fp) {
    m_points.resize(6);
    for (uint i = 0; i < 6; i++) {
        float x, y, z;
        fscanf(fp, "%f %f %f\n", &x, &y, &z);
        m_points[i].position = QVector3D(x, y, z);
    }
}

/**
 * @brief get the fraction of points selected by a mouse select action
 *
 * @param transform world transform of the plane
 * @param c1 corner 1 of mouse drag
 * @param c2 corner opposite to corner 1 of mouse drag
 */
float Plane::num_between_corners(QMatrix4x4 transform, QVector3D c1, QVector3D c2) {
    int num_bw_corners = 0;
    for (int i = 0; i < m_points.size(); i++) {
        num_bw_corners += between_corners(transform, m_points[i].position, c1, c2) ? 1 : 0;
    }
    return (float)(num_bw_corners / (float)m_points.size());
}

/**
 * @brief Color the plane
 *
 * @param r Red component of color
 * @param g Green component of color
 * @param b Blue component of color
 */
void Plane::recolor(float r, float g, float b) {
    for (int i = 0; i < m_points.size(); i++) {
        m_points[i].color = QVector3D(r, g, b);
    }
}

/**
 * @brief Set up the plane using its equation and points
 *
 * @param equation equation of the plane
 * @param points points on the plane
 */
Plane::Plane(QVector3D equation, QVector<QVector3D> points) {
    initializeOpenGLFunctions();
    m_equation = equation;
    calculate_Points(points);
    setUpBuffer();
}

/**
 * @brief Set up the vertex buffer array for the plane 
 */
void Plane::setUpBuffer() {
    m_vbo.create();
    m_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_points.size() * sizeof(Point), &m_points[0], GL_STATIC_DRAW);
    m_vbo.release();
}

/**
 * @brief merge two planes at their selected points
 *
 * @param v selected point on the second plane
 */
void Plane::adjust(QVector3D v) {
    m_points[m_selected_point].position = v;
    m_points[3] = m_points[0] ;
    m_points[4] = m_points[2] ;
}

/**
 * @brief select point on plane using mouse select action
 *
 * @param transform world transform of the plane
 * @param c1 corner 1 of mouse drag
 * @param c2 corner opposite to corner 1 of mouse drag
 * @return True iff successful selection of point
 */
bool Plane::select_point(QMatrix4x4 transform, QVector3D c1, QVector3D c2) {
    for (int i = 0; i < m_points.size(); i++) {
        if (between_corners(transform, m_points[i].position, c1, c2)) {
            m_selected_point = i;
            return true;
        }
    }
    return false;
}

/**
 * @brief Color the plane selected vertex
 *
 * @param r Red component of color
 * @param g Green component of color
 * @param b Blue component of color
 */
void Plane::recolor_selected(float r, float g, float b) {
    m_points[m_selected_point].color = QVector3D(r, g, b);
    m_points[3] = m_points[0] ;
    m_points[4] = m_points[2] ;
}

/**
 * @brief Draw plane on the screen
 *
 * @param program opengl shader program used
 * @param vPosition shader location of vertex position 
 * @param vColor shader location of vertex color
 */
void Plane::draw(QOpenGLShaderProgram *program, int vPosition, int vColor) {
    m_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_points.size() * sizeof(Point), &m_points[0], GL_STATIC_DRAW);
    program->enableAttributeArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(0) );
    program->enableAttributeArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(sizeof(QVector3D)));
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_points.size());
    m_vbo.release();
}

/**
 * @brief get best 4 corners (saved as 6 points for drawing gltriangles) of plane from a vector of points around it
 *
 * @param points vector of points very close to the plane
 */
void Plane::calculate_Points(QVector<QVector3D> points) {
    m_points.clear();

    QVector3D unit_z = QVector3D(0, 0, 1);
    QVector3D unit_normal = m_equation;
    unit_normal.normalize();
    double theta = acos(unit_normal.dotProduct(unit_normal, unit_z));
    QVector3D rotation_axis = unit_normal.crossProduct(unit_normal, unit_z);
    QMatrix4x4 planeToXY;
    planeToXY.setToIdentity();
    planeToXY.rotate(theta * 180.0 / 3.14159 , rotation_axis);

    float INF = 10000000;
    QVector3D tl(INF, -INF, 0), br(-INF, INF, 0), tr(-INF, -INF, 0), bl(INF, INF, 0);
    for (int i = 0; i < points.size(); i++) {
        QVector4D finalp = planeToXY * QVector4D(pointOnPlane(points[i], m_equation), 1.0f);
        if (finalp.x() / finalp.w() <= tl.x() and finalp.y() / finalp.w() >= tl.y()) {
            tl = QVector3D(finalp.x() / finalp.w(), finalp.y() / finalp.w(), i);
        }
        if (finalp.x() / finalp.w() >= tr.x() and finalp.y() / finalp.w() >= tr.y()) {
            tr = QVector3D(finalp.x() / finalp.w(), finalp.y() / finalp.w(), i);
        }
        if (finalp.x() / finalp.w() <= bl.x() and finalp.y() / finalp.w() <= bl.y()) {
            bl = QVector3D(finalp.x() / finalp.w(), finalp.y() / finalp.w(), i);
        }
        if (finalp.x() / finalp.w() >= br.x() and finalp.y() / finalp.w() <= br.y()) {
            br = QVector3D(finalp.x() / finalp.w(), finalp.y() / finalp.w(), i);
        }
    }
    m_points.append(Point(points[tl.z()], QVector3D(0, 0, 1)));
    m_points.append(Point(points[tr.z()], QVector3D(0, 0, 1)));
    m_points.append(Point(points[br.z()], QVector3D(0, 0, 1)));
    m_points.append(Point(points[tl.z()], QVector3D(0, 0, 1)));
    m_points.append(Point(points[br.z()], QVector3D(0, 0, 1)));
    m_points.append(Point(points[bl.z()], QVector3D(0, 0, 1)));
}
