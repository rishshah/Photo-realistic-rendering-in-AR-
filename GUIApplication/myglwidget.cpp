#include "myglwidget.h"
#include "utils.h"
#include <QMouseEvent>
#include <qt5/QtOpenGL/QGLShaderProgram>
#include <QCoreApplication>
#include <QMessageBox>

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_xRot(0),m_yRot(0),m_zRot(0),
      m_mode(0),
      m_selected_plane(-1),
      m_program(0){
}

MyGLWidget::~MyGLWidget(){
    cleanup();
}

QSize MyGLWidget::minimumSizeHint() const{
    return QSize(50, 50);
}

QSize MyGLWidget::sizeHint() const{
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle){
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void MyGLWidget::setXRotation(int angle){
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void MyGLWidget::setYRotation(int angle){
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void MyGLWidget::setZRotation(int angle){
    qNormalizeAngle(angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void MyGLWidget::cleanup(){
    makeCurrent();
    m_scene_vbo.destroy();
    delete m_program;
    m_program = 0;
    doneCurrent();
}

static const char *vertexShaderSource =
    "#version 330\n"
    "in vec3 vPosition;\n"
    "in vec3 vColor;\n"
    "in vec2 vTexCoord;\n"

    "flat out int is_texture_present;\n"
    "out vec4 color;\n"
    "out vec2 tex;\n"

    "uniform int uIs_tp;\n"
    "uniform mat4 uModelViewMatrix;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = uModelViewMatrix * vec4(vPosition, 1.0f);\n"
    "   color = vec4(vColor,1.0f);\n"
    "   tex = vTexCoord;\n"
    "   is_texture_present = uIs_tp;\n"
    "}";

static const char *fragmentShaderSource =
    "#version 400\n"

    "in vec4 color;\n"
    "in vec2 tex;\n"
    "flat in int is_texture_present;\n"

    "out vec4 frag_color;\n"
    "uniform sampler2D sampler;\n"

    "void main ()\n"
    "{\n"
    "   if(is_texture_present == 1){\n"
    "       frag_color = texture2D(sampler, tex);\n"
    "   } else {\n"
    "       frag_color = color;\n"
    "   }\n"
    "}";

void MyGLWidget::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &MyGLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();

    m_program->bind();

    // Location of variables in shader files
    m_vColor = m_program->attributeLocation("vColor");
    m_program->enableAttributeArray(m_vColor);

    m_vPosition = m_program->attributeLocation("vPosition");
    m_program->enableAttributeArray(m_vPosition);

    m_vTexCoord = m_program->attributeLocation("vTexCoord");
    m_program->enableAttributeArray(m_vTexCoord);

    m_mvMatrixLoc = m_program->uniformLocation("uModelViewMatrix");
    m_uIs_tp = m_program->uniformLocation("uIs_tp");

    // Our camera never changes in this example.
    m_camera.setToIdentity();
    m_camera.translate(0, 0, -1);

    // Read Points from SLAM output
    read_points();

    m_vao.create();
    m_vao.bind();


    // Fill vbo in points obtained
    m_scene_vbo.create();
    m_scene_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_scene_points.size() * sizeof(Point), &m_scene_points[0], GL_STATIC_DRAW);
    m_scene_vbo.release();

    m_vao.release();
    m_program->release();
}

void MyGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_world.setToIdentity();
    m_world.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_world.rotate(m_zRot / 16.0f, 0, 0, 1);

    m_program->bind();
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_camera * m_world);
    m_program->setUniformValue(m_uIs_tp, 0);

    m_vao.bind();

    switch (m_mode) {
    case 1:
        draw_scene();
        for(int i=0;i < m_planes.size(); i++){
            m_planes[i].draw(m_program, m_vPosition, m_vColor);
        }
        break;

    case 2:
        draw_scene();
        for(int i=0;i < m_planes.size(); i++){
            m_planes[i].draw(m_program, m_vPosition, m_vColor);
        }
        draw_mesh();
        break;

    case 3:
        draw_background();
        draw_mesh();
        break;

    default:
        draw_scene();
        break;
    }

    m_vao.release();
    m_program->release();
}

void MyGLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void MyGLWidget::draw_mesh(){
    m_program->setUniformValue(m_uIs_tp, 0);
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_camera * m_world);

    m_mesh_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_mesh_points.size() * sizeof(Point), &m_mesh_points[0], GL_STATIC_DRAW);

    glVertexAttribPointer(m_vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(0) );
    m_program->enableAttributeArray(m_vPosition);

    glVertexAttribPointer(m_vColor, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(sizeof(QVector3D)));
    m_program->enableAttributeArray(m_vColor);

    glDrawArrays(GL_TRIANGLES, 0, m_mesh_points.size());

    m_mesh_vbo.release();
}

void MyGLWidget::draw_background(){
    m_program->setUniformValue(m_uIs_tp, 1);
    m_program->setUniformValue(m_mvMatrixLoc, QMatrix4x4());

    m_bg_vbo.bind();
    std::string s = m_image_dir + "/" + m_image_data[0].first;
    bg_tex = png_texture_load(s.c_str());

    glBindTexture(GL_TEXTURE_2D, bg_tex);

    glBufferData (GL_ARRAY_BUFFER, m_bg_points.size() * sizeof(Point), &m_bg_points[0], GL_STATIC_DRAW);

    glVertexAttribPointer(m_vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(2 * sizeof(QVector3D)));
    m_program->enableAttributeArray(m_vTexCoord);

    glVertexAttribPointer(m_vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(0) );
    m_program->enableAttributeArray(m_vPosition);

    glVertexAttribPointer(m_vColor, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(sizeof(QVector3D)));
    m_program->enableAttributeArray(m_vColor);

    glDrawArrays(GL_TRIANGLES, 0, m_bg_points.size());

    m_bg_vbo.release();
}

void MyGLWidget::fill_image_data(std::string image_dir, std::string image_info_csv){
    m_image_dir = image_dir;
    FILE *fp_input = fopen(image_info_csv.c_str(), "r" );
    if (fp_input ==  NULL) {
         QMessageBox::critical(this,"Error", "file corrupt");
    } else {
        m_image_data.clear();
        char img_file[200];
        float timestamp;
        while(fscanf (fp_input, "%f, %s", &timestamp, img_file) != EOF){
            m_image_data.push_back(std::make_pair(img_file,timestamp));
        }
        fclose(fp_input);
    }

    //    fp_input = fopen(SLAM_KFS_FILEPATH, "r" );
    //    if (fp_input ==  NULL) {
    //         QMessageBox::critical(this,"Error", "file keyframes corrupt");
    //    } else {
    //        m_image_data.clear();
    //        char img_file[200];
    //        float timestamp;
    //        while(fscanf (fp_input, "%f, %s", &timestamp, img_file) != EOF){
    //            m_image_data.push_back(std::make_pair(img_file,timestamp));
    //        }
    //        fclose(fp_input);
    //    }
}

void MyGLWidget::draw_scene(){
    m_scene_vbo.bind();
    glBufferData (GL_ARRAY_BUFFER, m_scene_points.size() * sizeof(Point), &m_scene_points[0], GL_STATIC_DRAW);

    glVertexAttribPointer(m_vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(0) );
    m_program->enableAttributeArray(m_vPosition);

    glVertexAttribPointer(m_vColor, 3, GL_FLOAT, GL_FALSE, sizeof(Point), BUFFER_OFFSET(sizeof(QVector3D)));
    m_program->enableAttributeArray(m_vColor);

    glDrawArrays(GL_POINTS, 0, m_scene_points.size());

    m_scene_vbo.release();
}

void MyGLWidget::read_points() {
    FILE *fp_input = fopen(SLAM_POINTS_FILEPATH, "r" );
    if (fp_input ==  NULL) {
         QMessageBox::critical(this,"Error",SLAM_POINTS_FILEPATH " corrupt");
    } else {
        m_scene_points.clear();
        float x, y, z;
        while(fscanf (fp_input, "%f %f %f", &x, &y, &z) != EOF){
            m_scene_points.append(Point(QVector3D(x,y,z),QVector3D(1,1,1)));
        }
        fclose(fp_input);
    }
}

void MyGLWidget::input_mesh(std::string fileName){
    FILE *fp_input = fopen(fileName.c_str(), "r" );
    if (fp_input ==  NULL) {
         QMessageBox::critical(this,"Error","file corrupt");
         return;
    } else {
        m_mesh_points.clear();
        float x, y, z, c1, c2, c3;
        while(fscanf (fp_input, "%f %f %f %f %f %f", &x, &y, &z, &c1, &c2, &c3) != EOF){
            m_mesh_points.append(Point(QVector3D(x,y,z),QVector3D(c1,c2,c3)));
        }
        fclose(fp_input);
    }
    m_mesh_vbo.create();
    m_mode = 2;
    update();
}

void MyGLWidget::playback(){
    int i = 1 , z= -1;
    m_bg_points.append(Point(QVector3D(-1,1,z), QVector2D(0,i)));
    m_bg_points.append(Point(QVector3D(-1,-1,z), QVector2D(0,0)));
    m_bg_points.append(Point(QVector3D(1,1,z), QVector2D(i,i)));
    m_bg_points.append(Point(QVector3D(1,1,z), QVector2D(i,i)));
    m_bg_points.append(Point(QVector3D(-1,-1,z), QVector2D(0,0)));
    m_bg_points.append(Point(QVector3D(1,-1,z), QVector2D(i,0)));
    m_bg_vbo.create();
    m_mode = 3;
    update();
}

void MyGLWidget::mousePress(QMouseEvent *event){
    m_lastPos = event->pos();
}

void MyGLWidget::mouseMove(QMouseEvent *event, bool select_mode, bool add_mode){
    if (!select_mode){ // PAN mode : changing rotation about origin
        int dx = event->x() - m_lastPos.x();
        int dy = event->y() - m_lastPos.y();

        if (event->buttons() & Qt::LeftButton) {
            setXRotation(m_xRot + 8 * dy);
            setYRotation(m_yRot + 8 * dx);
        } else if (event->buttons() & Qt::RightButton) {
            setXRotation(m_xRot + 8 * dy);
            setZRotation(m_zRot + 8 * dx);
        }
        m_lastPos = event->pos();
    } else {
        QVector3D corner1 = QVector3D(m_lastPos.x(), m_lastPos.y(), 0);
        if(add_mode){ // ADD Mode : Show selected points
            for(int i=0; i<m_scene_points.size(); i++){
                if (between_corners(m_proj * m_camera * m_world, m_scene_points[i].position, corner1, QVector3D(event->x(), event->y(), 0))){
                    m_scene_points[i].color = QVector3D(1,0,0);
                } else {
                    m_scene_points[i].color = QVector3D(1,1,1);
                }
            }
        } else { // REMOVE Mode : Show selected plane
            m_selected_plane = -1;
            float fraction_inside = 0;
            for(int i=0; i<m_planes.size(); i++){
                float curr_fraction = m_planes[i].num_between_corners(m_proj * m_camera * m_world, corner1, QVector3D(event->x(), event->y(),0));
                if(curr_fraction > fraction_inside){
                    fraction_inside = curr_fraction;
                    m_selected_plane = i;
                }
            }
            if (m_selected_plane != -1){
                for(int i=0; i<m_planes.size(); i++){
                    if(m_selected_plane != i){
                        m_planes[i].recolor(0,0,1);
                    }
                }
                m_planes[m_selected_plane].recolor(0,1,0);
            }
        }
    }
    update();
}

void MyGLWidget::add_plane(){
    QVector<QVector3D> selected_points;
    for(int i=0; i<m_scene_points.size(); i++){
        if(m_scene_points[i].color.y() != 1){
            selected_points.append(m_scene_points[i].position);
        }
    }
    printf("S : %d\n", selected_points.size());
    int iterations = 0, num_total_iterations = 50, good_num_points = selected_points.size()/5;
    double best_error = 10E5, error_limit = 0.3;
    QVector3D best_model;
    QVector<QVector3D> best_consensus_set;

    // Algortithm RANSAC for one plane
    while(iterations < num_total_iterations){
        int x, y, z;
        get3Points(selected_points.size(),x,y,z);

        QVector<QVector3D> consensus_set;
        consensus_set.append(selected_points[x]);
        consensus_set.append(selected_points[y]);
        consensus_set.append(selected_points[z]);

        QVector3D maybe_model;
        double this_error;
        fit_plane(consensus_set, maybe_model, this_error);
//        printf("IP %f\n", this_error);

        for(int i=0; i< selected_points.size(); i++){
            if (i != x and i != y and i != z and close_enough(selected_points[i], maybe_model, error_limit)){
                consensus_set.append(selected_points[i]);
            }
        }

        if (good_enough(consensus_set, good_num_points)){
            fit_plane(consensus_set, maybe_model, this_error);
//            printf("T %d %d %f\n", iterations, consensus_set.size(), this_error);
            if (this_error < best_error){
                best_model = maybe_model;
                best_consensus_set = consensus_set;
                best_error = this_error;
            }
        }
        iterations++;
//        printf("F %d %d %f\n", iterations, best_consensus_set.size(), best_error);
    }
    printf("PLANE %f %f %f\n", best_model.x(), best_model.y(), best_model.z());

    m_vao.bind();
    m_planes.append(Plane(best_model, best_consensus_set));
    m_vao.release();
    m_mode = 1;
    update();

}

void MyGLWidget::remove_plane(){
    if(m_selected_plane != -1){
        m_planes.remove(m_selected_plane);
        m_selected_plane = -1;
        update();
    }
}
