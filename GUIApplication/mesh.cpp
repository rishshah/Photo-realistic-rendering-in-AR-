#include "mesh.h"

/**
 * @brief Save the mesh translation and scale
 * 
 * @param fp open file pointer in which the save is made
 */
void Mesh::save(FILE* fp){
    fprintf(fp, "%f %f %f\n", m_scale.x(), m_scale.y(), m_scale.z());
    fprintf(fp, "%f %f %f\n", m_translation.x(), m_translation.y(), m_translation.z());
}

/**
 * @brief Load the mesh translation and scale
 * 
 * @param fp open file pointer from which we load
 */
void Mesh::load(FILE* fp){
    float x,y,z;
    fscanf(fp, "%f %f %f\n", &x, &y, &z);
    m_scale = QVector3D(x,y,z);
    fscanf(fp, "%f %f %f\n", &x, &y, &z);
    m_translation = QVector3D(x,y,z);
}

/**
 * @brief Move the mesh to join its selected point to a plane's selected point
 * 
 * @param v point on the plane mesh should join to
 * @param transform1 world transform of plane
 * @param transform2 world transform of mesh
 */
void Mesh::adjust(QVector3D v, QMatrix4x4 transform1, QMatrix4x4 transform2){
    QMatrix4x4 model_transform;
    model_transform.setToIdentity();
    model_transform.scale(m_scale);
    model_transform.translate(m_translation);
    QVector4D s = transform1 * QVector4D(v,1.0f);
    QVector4D e = transform2 * model_transform * QVector4D(m_points[m_selected_point].position,1.0f);
    m_translation =  QVector3D(model_transform.inverted() * transform2.inverted() * (s - e));
    m_points[m_selected_point].color = m_selected_point_color;
}

/**
 * @brief Select point on mesh
 * 
 * @param transform mesh world transform
 * @param c1 corner 1 of mouse drag
 * @param c2 corner opposite to corner 1 of mouse drag
 * @return True if selection successful
 */
bool Mesh::select_point(QMatrix4x4 transform, QVector3D c1, QVector3D c2){
    QMatrix4x4 model_transform;
    model_transform.setToIdentity();
    model_transform.scale(m_scale);
    model_transform.translate(m_translation);
    for(int i=0;i<m_points.size(); i++){
        if(between_corners(transform * model_transform, m_points[i].position, c1, c2)){
            m_selected_point = i;
            m_selected_point_color = m_points[m_selected_point].color;
            return true;
        }
    }
    return false;
}

/**
 * @brief Recolor the mesh selected vertex
 */
void Mesh::recolor_selected(float r, float g, float b){
    m_points[m_selected_point].color = QVector3D(r,g,b);
}

Mesh::Mesh(){
}
/**
 * @brief Load mesh from given file 
 */
Mesh::Mesh(std::string fileName){
    initializeOpenGLFunctions();
    FILE *fp_input = fopen(fileName.c_str(), "r" );
    if (fp_input ==  NULL) {
        printf("Error file corrupt");
        return;
    } else {
        float x, y, z, c1, c2, c3;
        fscanf (fp_input, "%f %f %f", &x, &y, &z);
        m_scale = QVector3D(x,y,z);
        m_translation = QVector3D(0,0,0);
        m_points.clear();
        while (fscanf (fp_input, "%f %f %f %f %f %f", &x, &y, &z, &c1, &c2, &c3) != EOF) {
            m_points.append(Point(QVector3D(x, y, z), QVector3D(c1, c2, c3)));
        }
        fclose(fp_input);
    }
    m_vbo.create();
}

/**
 * @brief Draw mesh on the screen
 * 
 * @param program opengl shader program 
 * @param transform world transform to be applied before drawing
 * @param uMatrix shader location of overall transform
 * @param vPosition shader location of vertex position 
 * @param vColor shader location of vertex color
 */
void Mesh::draw(QOpenGLShaderProgram *program, QMatrix4x4 transform, int uMatrix, int vPosition, int vColor){
    QMatrix4x4 model_transform;
    model_transform.setToIdentity();
    model_transform.scale(m_scale);
    model_transform.translate(m_translation);

    program->setUniformValue(uMatrix,transform *  model_transform);

    m_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_points.size() * sizeof(Point), &m_points[0], GL_STATIC_DRAW);

    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(0) );
    program->enableAttributeArray(vPosition);

    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(sizeof(QVector3D)));
    program->enableAttributeArray(vColor);

    glDrawArrays(GL_TRIANGLES, 0, m_points.size());

    m_vbo.release();
}
