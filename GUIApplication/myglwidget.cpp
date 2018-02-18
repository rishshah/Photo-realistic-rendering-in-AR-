#include "myglwidget.h"

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_xPos(0.0f), m_yPos(0.0f), m_zPos(0.0f),
      m_xRot(0), m_yRot(0), m_zRot(0),
      m_mode(0),
      m_i(0),
      m_selected_plane_for_removal(-1),
      m_curr_image_index(0),
      m_plane_1(-1),
      m_plane_2(-1),
      m_snap_plane(-1),
      m_mesh_point_selected(false),
      m_program(0) {
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

MyGLWidget::~MyGLWidget() {
    cleanup();
}

QSize MyGLWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize MyGLWidget::sizeHint() const {
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle) {
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void MyGLWidget::setXRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void MyGLWidget::setYRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void MyGLWidget::setZRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void MyGLWidget::cleanup() {
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
    "       frag_color = vec4(frag_color.x, frag_color.x, frag_color.x, 1);\n"
    "   } else {\n"
    "       frag_color = color;\n"
    "   }\n"
    "}";

void MyGLWidget::initializeGL() {
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

    // Read Points from SLAM output
    read_points();

    m_vao.create();
    m_vao.bind();
    m_scene_vbo.create();
    m_vao.release();
    m_program->release();
}

void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);

    m_world.setToIdentity();
    m_world.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_world.rotate(m_zRot / 16.0f, 0, 0, 1);

    m_camera.setToIdentity();
    m_camera.translate(m_xPos, m_yPos, m_zPos);

    m_program->bind();
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_world * m_camera);
    m_program->setUniformValue(m_uIs_tp, 0);

    m_vao.bind();

    switch (m_mode) {
    case 1:
        draw_scene();
        for (int i = 0; i < m_planes.size(); i++) {
            m_planes[i].draw(m_program, m_vPosition, m_vColor);
        }
        break;

    case 2:
        draw_scene();
        for (int i = 0; i < m_planes.size(); i++) {
            m_planes[i].draw(m_program, m_vPosition, m_vColor);
        }
        draw_mesh();
        break;

    case 3:
        draw_background();
        draw_scene();
        draw_mesh();
        break;

    default:
        draw_scene();
        break;
    }

    m_vao.release();
    m_program->release();
}

void MyGLWidget::resizeGL(int w, int h) {
    m_proj.setToIdentity();
    m_proj.perspective(60.0f, GLfloat(w) / h, 0.01f, 100000.0f);
}

void MyGLWidget::draw_mesh() {
    m_program->setUniformValue(m_uIs_tp, 0);
    m_keyframe_transform.setToIdentity();

    if(m_mode == 3){
        int curr_keyframe_index = (m_keyframes.size() * m_curr_image_index)/m_image_data.size();
        if(curr_keyframe_index >= m_keyframes.size() - 1){
            curr_keyframe_index = m_keyframes.size() - 2;
        }

        float curr_kf_eq_index = ((m_image_data.size() * curr_keyframe_index)/(float)m_keyframes.size());
        float next_kf_eq_index = ((m_image_data.size() * (curr_keyframe_index + 1))/(float)m_keyframes.size());
        float t = (m_curr_image_index - curr_kf_eq_index)/(next_kf_eq_index - curr_kf_eq_index);
        QQuaternion slerp = QQuaternion::slerp(
                    m_keyframes[curr_keyframe_index].orientation.conjugate(),
                    m_keyframes[curr_keyframe_index + 1].orientation.conjugate(),
                    t
                );

        QVector3D curr_pos = -m_keyframes[curr_keyframe_index].position;
        QVector3D next_pos = -m_keyframes[curr_keyframe_index + 1].position;
        QVector3D slerp_pos = curr_pos + t * (next_pos - curr_pos);

        m_keyframe_transform.rotate(slerp);
        m_keyframe_transform.translate(slerp_pos);
        m_mesh.draw(m_program, m_proj * m_keyframe_transform, m_mvMatrixLoc, m_vPosition, m_vColor);
    } else {
        if(m_i >= m_keyframes.size()){
            m_i = 0;
        }
        m_keyframe_transform.rotate(m_keyframes[m_i].orientation.conjugate());
        m_keyframe_transform.translate(-m_keyframes[m_i].position);
        m_mesh.draw(m_program, m_proj * m_keyframe_transform * m_world * m_camera, m_mvMatrixLoc, m_vPosition, m_vColor);
    }
}

void MyGLWidget::draw_background() {
    if(m_curr_image_index < m_image_data.size()){
        std::string s = m_image_dir + "/" + m_image_data[m_curr_image_index].first;
        bg_tex = png_texture_load(s.c_str());
        m_curr_image_index++;
    } else {
        m_curr_image_index = 0;
        m_timer->stop();
        m_mode = 1;
        return;
    }
    printf("Image: %d\n", m_curr_image_index);
    glDisable(GL_DEPTH_TEST);
    m_program->setUniformValue(m_uIs_tp, 1);
    QMatrix4x4 m_ortho;
    m_ortho.setToIdentity();
    m_ortho.ortho(-1,1,-1,1,-1,1);
    m_program->setUniformValue(m_mvMatrixLoc, m_ortho);
    m_bg_vbo.bind();

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
    glEnable(GL_DEPTH_TEST);
}

void MyGLWidget::fill_image_data(std::string image_dir, std::string image_info_csv) {
    m_image_dir = image_dir;
    FILE *fp_input = fopen(image_info_csv.c_str(), "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "file corrupt");
    } else {
        m_image_data.clear();
        char img_file[200];
        float timestamp;
        while (fscanf (fp_input, "%f, %s", &timestamp, img_file) != EOF) {
            m_image_data.push_back(std::make_pair(img_file, timestamp));
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

void MyGLWidget::draw_scene() {
    if(m_i >= m_keyframes.size()){
        m_i = 0;
    }
    if (m_mode != 3 and m_i > 0){
        m_keyframe_transform.setToIdentity();
        m_keyframe_transform.rotate(m_keyframes[m_i].orientation.conjugate());
        m_keyframe_transform.translate(-m_keyframes[m_i].position);
        m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_keyframe_transform * m_world * m_camera);
    }
    if(m_mode == 3){
        m_keyframe_transform.setToIdentity();
        int curr_keyframe_index = (m_keyframes.size() * m_curr_image_index)/m_image_data.size();
        if(curr_keyframe_index >= m_keyframes.size() - 1){
            curr_keyframe_index = m_keyframes.size() - 2;
        }

        float curr_kf_eq_index = ((m_image_data.size() * curr_keyframe_index)/(float)m_keyframes.size());
        float next_kf_eq_index = ((m_image_data.size() * (curr_keyframe_index + 1))/(float)m_keyframes.size());
        float t = (m_curr_image_index - curr_kf_eq_index)/(next_kf_eq_index - curr_kf_eq_index);
        QQuaternion slerp = QQuaternion::slerp(
                    m_keyframes[curr_keyframe_index].orientation.conjugate(),
                    m_keyframes[curr_keyframe_index + 1].orientation.conjugate(),
                    t
                );

        QVector3D curr_pos = -m_keyframes[curr_keyframe_index].position;
        QVector3D next_pos = -m_keyframes[curr_keyframe_index + 1].position;
        QVector3D slerp_pos = curr_pos + t * (next_pos - curr_pos);

        m_keyframe_transform.rotate(slerp);
        m_keyframe_transform.translate(slerp_pos);
        m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_keyframe_transform);
    }
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
        QMessageBox::critical(this, "Error", SLAM_POINTS_FILEPATH " corrupt");
    } else {
        m_scene_points.clear();
        float x, y, z;
        while (fscanf (fp_input, "%f %f %f", &x, &y, &z) != EOF) {
            m_scene_points.append(Point(QVector3D(x, y, z), QVector3D(1, 1, 1)));
        }
        fclose(fp_input);
    }
}

void MyGLWidget::input_mesh(std::string fileName) {

    m_mesh = Mesh(fileName);
    m_mode = 2;
    FILE* fp_input = fopen(SLAM_KFS_FILEPATH, "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "file corrupt");
        return;
    } else {
        float t, x, y, z, q1, q2, q3, q4;
        while (fscanf (fp_input, "%f %f %f %f %f %f %f %f", &t, &x, &y, &z, &q1, &q2, &q3 ,&q4) != EOF) {
            m_keyframes.push_back(Keyframe(t, QVector3D(x, y, z), QQuaternion(q4, q1, q2, q3)));
        }
        fclose(fp_input);
    }

    update();
}

void MyGLWidget::playback() {
    float i = 1, j = 1, z = -1;
    m_bg_points.append(Point(QVector3D(-1*i, i, z), QVector2D(0, j)));
    m_bg_points.append(Point(QVector3D(-1*i, -1*i, z), QVector2D(0, 0)));
    m_bg_points.append(Point(QVector3D(i, i, z), QVector2D(j, j)));
    m_bg_points.append(Point(QVector3D(i, i, z), QVector2D(j, j)));
    m_bg_points.append(Point(QVector3D(-1*i, -1*i, z), QVector2D(0, 0)));
    m_bg_points.append(Point(QVector3D(i, -1*i, z), QVector2D(j, 0)));
    m_bg_vbo.create();
    m_mode = 3;
    m_curr_image_index = 0;

    m_timer->start(1);
    update();
}

void MyGLWidget::mousePress(QMouseEvent *event) {
    m_lastPos = event->pos();
}

void MyGLWidget::mouseMove(QMouseEvent *event, bool select_mode,  std::string insert_mode) {
    if (!select_mode) { // PAN mode : changing rotation about origin
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
        QMatrix4x4 transform = m_proj * m_world * m_camera;
        if (insert_mode == "ADD_PLANE") { // Show selected points
            for (int i = 0; i < m_scene_points.size(); i++) {
                if (between_corners(transform, m_scene_points[i].position, corner1, QVector3D(event->x(), event->y(), 0))) {
                    m_scene_points[i].color = QVector3D(1, 0, 0);
                } else {
                    m_scene_points[i].color = QVector3D(1, 1, 1);
                }
            }
        } else if(insert_mode == "REMOVE_PLANE"){ // Show selected plane
            m_selected_plane_for_removal = -1;
            float fraction_inside = 0;
            for (int i = 0; i < m_planes.size(); i++) {
                float curr_fraction = m_planes[i].num_between_corners(transform, corner1, QVector3D(event->x(), event->y(), 0));
                if (curr_fraction > fraction_inside) {
                    fraction_inside = curr_fraction;
                    m_selected_plane_for_removal = i;
                }
            }
            if (m_selected_plane_for_removal != -1) {
                for (int i = 0; i < m_planes.size(); i++) {
                    if (m_selected_plane_for_removal != i) {
                        m_planes[i].recolor(0, 0, 1);
                    }
                }
                m_planes[m_selected_plane_for_removal].recolor(0, 1, 0);
            }
        } else if(insert_mode == "ADJUST_PLANE"){ // Show selected plane point
            if(m_plane_1 == -1){
                for (int i = 0; i < m_planes.size(); i++) {
                    if(m_planes[i].select_point(transform, corner1, QVector3D(event->x(), event->y(), 0))){
                        m_planes[i].recolor_selected(1,0,0);
                        m_plane_1 = i;
                        break;
                    }
                }
            } else if(m_plane_2 == -1){
                for (int i = 0; i < m_planes.size(); i++) {
                    if(m_planes[i].select_point(transform, corner1, QVector3D(event->x(), event->y(), 0))){
                        m_planes[i].recolor_selected(1,0,0);
                        if(m_plane_1 != i)
                            m_plane_2 = i;
                        break;
                    }
                }
            }
        } else if(insert_mode == "ADJUST_MESH"){ // Show selected plane point and mesh point
            if(m_snap_plane == -1){
                for (int i = 0; i < m_planes.size(); i++) {
                    if(m_planes[i].select_point(transform, corner1, QVector3D(event->x(), event->y(), 0))){
                        m_planes[i].recolor_selected(1,1,0);
                        m_snap_plane = i;
                        break;
                    }
                }
            } else {
                m_mesh_point_selected = m_mesh.select_point(m_keyframe_transform * transform, corner1, QVector3D(event->x(), event->y(), 0));
            }
        }
    }
    update();
}

void MyGLWidget::keyPress(QKeyEvent *event){
    if(event->key() == Qt::Key_S){
        m_yPos -= DY;
    }
    else if(event->key() == Qt::Key_W){
        m_yPos += DY;
    }

    if(event->key() == Qt::Key_A){
        m_xPos -= DX;
    }
    else if(event->key() == Qt::Key_D){
        m_xPos += DX;
    }

    if(event->key() == Qt::Key_X){
        m_zPos -= DZ;
    }
    else if(event->key() == Qt::Key_Z){
        m_zPos += DZ;
    }

    if(event->key() == Qt::Key_P){
        m_i++;
    }
    update();
}

void MyGLWidget::add_plane() {
    QVector<QVector3D> selected_points;
    for (int i = 0; i < m_scene_points.size(); i++) {
        if (m_scene_points[i].color.y() != 1) {
            selected_points.append(m_scene_points[i].position);
        }
    }
    // printf("S : %d\n", selected_points.size());
    int iterations = 0, num_total_iterations = 50, good_num_points = selected_points.size() / 5;
    double best_error = 10E5, error_limit = 0.3;
    QVector3D best_model;
    QVector<QVector3D> best_consensus_set;

    // Algortithm RANSAC for one plane
    while (iterations < num_total_iterations) {
        int x, y, z;
        get3Points(selected_points.size(), x, y, z);

        QVector<QVector3D> consensus_set;
        consensus_set.append(selected_points[x]);
        consensus_set.append(selected_points[y]);
        consensus_set.append(selected_points[z]);

        QVector3D maybe_model;
        double this_error;
        fit_plane(consensus_set, maybe_model, this_error);

        for (int i = 0; i < selected_points.size(); i++) {
            if (i != x and i != y and i != z and close_enough(selected_points[i], maybe_model, error_limit)) {
                consensus_set.append(selected_points[i]);
            }
        }

        if (good_enough(consensus_set, good_num_points)) {
            fit_plane(consensus_set, maybe_model, this_error);
            // printf("T %d %d %f\n", iterations, consensus_set.size(), this_error);
            if (this_error < best_error) {
                best_model = maybe_model;
                best_consensus_set = consensus_set;
                best_error = this_error;
            }
        }
        iterations++;
        // printf("F %d %d %f\n", iterations, best_consensus_set.size(), best_error);
    }
    printf("PLANE %f %f %f\n", best_model.x(), best_model.y(), best_model.z());

    m_vao.bind();
    m_planes.append(Plane(best_model, best_consensus_set));
    m_vao.release();
    m_mode = 1;
    update();
}

void MyGLWidget::remove_plane() {
    if (m_selected_plane_for_removal != -1) {
        m_planes.remove(m_selected_plane_for_removal);
        m_selected_plane_for_removal = -1;
        update();
    }
}

void MyGLWidget::adjust_planes() {
    if(m_plane_1 != -1 and m_plane_2 != -1){
        m_planes[m_plane_1].adjust(m_planes[m_plane_2].get_selected_point());
        m_planes[m_plane_1].recolor_selected(0,0,1);
        m_planes[m_plane_2].recolor_selected(0,0,1);
    } else {
        if(m_plane_1 != -1)
            m_planes[m_plane_1].recolor_selected(0,0,1);
        if(m_plane_2 != -1)
            m_planes[m_plane_2].recolor_selected(0,0,1);
    }
    m_plane_1 = m_plane_2 = -1;
    update();
}

void MyGLWidget::adjust_mesh(){
    if(m_mesh_point_selected and m_snap_plane != -1){
        QMatrix4x4 transform = m_world * m_camera;
        m_mesh.adjust(m_planes[m_snap_plane].get_selected_point(), transform, m_keyframe_transform * transform);
    }
    if(m_snap_plane != -1)
        m_planes[m_snap_plane].recolor_selected(0,0,1);
    m_snap_plane = -1;
    update();
}
