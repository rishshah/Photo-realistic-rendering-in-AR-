#include "myglwidget.h"

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_xPos(0.0f), m_yPos(0.0f), m_zPos(0.0f),
      m_xRot(0), m_yRot(0), m_zRot(0),
      m_mode(ADJUST_SCENE),
      m_i(0),m_ij(0),
      m_selected_plane_for_removal(-1),
      m_curr_image_index(0),
      m_curr_keyframe_index(0),
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
    delete m_cam;
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

void MyGLWidget::adjustCameraTransform(){
    m_camera.setToIdentity();
    if (m_mode != PLAYBACK){
        m_camera.rotate(m_keyframes[m_i].orientation.conjugate());
        m_camera.translate(-m_keyframes[m_i].position);
    } else {
        double curr_image_ts = m_image_data[m_curr_image_index].second;
        m_curr_keyframe_index = get_keyframe_index(0, curr_image_ts);
        if (m_curr_keyframe_index != -1 and m_curr_keyframe_index != -2){
            double t = (curr_image_ts - m_keyframes[m_curr_keyframe_index].timestamp)
                    /(m_keyframes[m_curr_keyframe_index+1].timestamp- m_keyframes[m_curr_keyframe_index].timestamp);
            QQuaternion slerp = QQuaternion::slerp(
                        m_keyframes[m_curr_keyframe_index].orientation.conjugate(),
                        m_keyframes[m_curr_keyframe_index + 1].orientation.conjugate(),
                        t
                    );

            QVector3D curr_pos = -m_keyframes[m_curr_keyframe_index].position;
            QVector3D next_pos = -m_keyframes[m_curr_keyframe_index + 1].position;
            QVector3D slerp_pos = curr_pos + t * (next_pos - curr_pos);

            m_camera.rotate(slerp.normalized());
            m_camera.translate(slerp_pos);
        }
    }
}

cv::Mat Camera::get_cam_parameter(){
    cv::Mat K = cv::Mat::eye(3,3,CV_32F);
    K.at<float>(0,0) = m_fx;
    K.at<float>(1,1) = m_fy;
    K.at<float>(0,2) = m_cx;
    K.at<float>(1,2) = m_cy;
    return K;
}

cv::Mat Camera::get_cam_distortion(){
    cv::Mat DistCoef = cv::Mat::zeros(4,1,CV_32F);
    DistCoef.at<float>(0) = m_k1;
    DistCoef.at<float>(1) = m_k2;
    DistCoef.at<float>(2) = m_p1;
    DistCoef.at<float>(3) = m_p2;
    return DistCoef;
}

QMatrix4x4 Camera::getProjectionTransform(int w, int h){
    QMatrix4x4 proj;
    proj.setToIdentity();
    float A = - (m_near + m_far)/(m_far - m_near);
    float B = - (2.0f * m_near * m_far)/(m_far - m_near);
    proj.setRow(0,QVector4D((2 * m_fx) / w,   1 - 2 * m_cx/w,  0, 0));
    proj.setRow(1,QVector4D(0,   (2 * m_fy) / h,  1 - 2 * m_cy/h, 0));
    proj.setRow(2,QVector4D(0,    0,   A,   B));
    proj.setRow(3,QVector4D(0,    0,  -1,  0));
//    float fovy = 2*std::atan(0.5*h/m_fy)*180.00/3.14159;
//    float aspect = (w*m_fy)/(h*m_fx);
//    double L = -(m_cx) * m_near / m_fx;
//    double R = +(w-m_cx) * m_near / m_fx;
//    double T = -(m_cy) * m_near / m_fy;
//    double B = +(h-m_cy) * m_near / m_fy;

//    proj.setRow(0,QVector4D(2 * m_near / (R-L), 0, 0, 0));
//    proj.setRow(1,QVector4D(0, 2 * m_near / (T-B),  0, 0));
//    proj.setRow(2,QVector4D((R+L)/(L-R), (T+B)/(B-T),   (m_far +m_near) / (m_far - m_near),   1));
//    proj.setRow(3,QVector4D(0, 0, (2*m_far*m_near)/(m_near - m_far), 0));
//    proj.perspective(61.0, aspect, m_near, m_far);

    return proj;
}

void MyGLWidget::adjustWorldRotationTransform(){
    m_worldRotation.setToIdentity();
    m_worldRotation.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_worldRotation.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_worldRotation.rotate(m_zRot / 16.0f, 0, 0, 1);
}

void MyGLWidget::adjustWorldTranslationTransform(){
    m_worldTranslation.setToIdentity();
    m_worldTranslation.translate(m_xPos, m_yPos, m_zPos);
}

void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    adjustCameraTransform();
    adjustWorldRotationTransform();
    adjustWorldTranslationTransform();

    m_program->bind();
    m_vao.bind();

    switch (m_mode) {
    case ADJUST_SCENE:
        draw_scene();
        draw_planes();
        break;

    case ADJUST_MESH:
        draw_background();
        draw_scene();
        draw_planes();
//        draw_mesh();
        break;

    case PLAYBACK:
        draw_background();
        draw_scene();
        draw_planes();
//        draw_mesh();
        break;

    default:
        break;
    }

    m_vao.release();
    m_program->release();
}

void MyGLWidget::resizeGL(int w, int h) {
    m_proj = m_cam->getProjectionTransform(w,h);
}

void MyGLWidget::draw_planes(){
    m_program->setUniformValue(m_uIs_tp, 0);
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_worldRotation * m_camera * m_worldTranslation);
    for (int i = 0; i < m_planes.size(); i++) {
        m_planes[i].draw(m_program, m_vPosition, m_vColor);
    }
}

void MyGLWidget::draw_mesh() {
    m_program->setUniformValue(m_uIs_tp, 0);
    m_mesh.draw(m_program, m_proj * m_worldRotation * m_camera * m_worldTranslation, m_mvMatrixLoc, m_vPosition, m_vColor);
}

void MyGLWidget::draw_background() {
    cv::Mat imu;
    cv::Mat im;

    if(m_mode == PLAYBACK){
        if(m_curr_image_index < m_image_data.size()){
            std::string s = m_image_dir + "/" + m_image_data[m_curr_image_index].first;
            im = cv::imread(s, cv::IMREAD_GRAYSCALE);
            cv::undistort(im,imu,m_cam->get_cam_parameter(),m_cam->get_cam_distortion());
            bg_tex = distorted_texture_load(imu);

            m_timer->stop();
            delete m_timer;
            m_timer =new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
            double time_next = m_image_data[m_curr_image_index+1].second - m_image_data[m_curr_image_index].second;
            m_curr_image_index++;
            m_timer->start(int(m_simulation_time_ms * std::max(time_next,0.0)));
        } else {
            m_curr_image_index = 0;
            m_curr_keyframe_index = 0;
            m_i = 0;
            m_mode = ADJUST_MESH;
            m_timer->stop();
            return;
        }


//        m_ij = get_image_index(0, m_keyframes[m_i].timestamp);
//        if(m_ij != -1 and m_ij != -2){
//            std::string s = m_image_dir + "/" + m_image_data[m_ij].first;
//            im = cv::imread(s, cv::IMREAD_GRAYSCALE);
//            cv::undistort(im,imu,m_cam->get_cam_parameter(),m_cam->get_cam_distortion());
//            bg_tex = distorted_texture_load(imu);
//        }

//        std::string s = m_image_dir + "/" + m_image_data[m_curr_image_index].first;
//        im = cv::imread(s, cv::IMREAD_GRAYSCALE);
//        cv::undistort(im,imu,m_cam->get_cam_parameter(),m_cam->get_cam_distortion());
//        bg_tex = distorted_texture_load(imu);

    } else if (m_mode == ADJUST_MESH){
        int x = get_image_index(0, m_keyframes[0].timestamp);
        std::string s = m_image_dir + "/" + m_image_data[x].first;
        im = cv::imread(s, cv::IMREAD_GRAYSCALE);
        cv::undistort(im,imu,m_cam->get_cam_parameter(),m_cam->get_cam_distortion());
        bg_tex = distorted_texture_load(imu);
    }

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

void MyGLWidget::fill_image_data(std::string image_dir, std::string image_info_csv, std::string settings) {
    m_image_dir = image_dir;
    FILE *fp_input = fopen(image_info_csv.c_str(), "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "file corrupt");
    } else {
        m_image_data.clear();
        char img_file[200];
        double timestamp;
        while (fscanf (fp_input, "%lf, %s", &timestamp, img_file) != EOF) {
            m_image_data.push_back(std::make_pair(img_file, timestamp));
        }
        fclose(fp_input);
    }

    fp_input = fopen(SLAM_KFS_FILEPATH, "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "file corrupt");
        return;
    } else {
        double t;float x, y, z, q1, q2, q3, q4;
        while (fscanf (fp_input, "%lf %f %f %f %f %f %f %f", &t, &x, &y, &z, &q1, &q2, &q3 ,&q4) != EOF) {
            m_keyframes.push_back(Keyframe(t, QVector3D(x, y, z), QQuaternion(q4, q1, q2, q3)));
        }
        fclose(fp_input);
    }

    double minTimeStamp = std::min(m_keyframes[0].timestamp, m_image_data[0].second);
    double maxTimeStamp = std::max(m_keyframes[m_keyframes.size()-1].timestamp, m_image_data[m_image_data.size()-1].second);
    for(uint i=0; i< m_image_data.size();i++){
        m_image_data[i].second = (m_image_data[i].second - minTimeStamp)/(maxTimeStamp - minTimeStamp);
    }
    for(uint i=0; i< m_keyframes.size();i++){
        m_keyframes[i].timestamp = (m_keyframes[i].timestamp - minTimeStamp)/(maxTimeStamp - minTimeStamp);
    }

    cv::FileStorage fSettings;
    fSettings.open(settings, cv::FileStorage::READ);
    if (!fSettings.isOpened()){
        QMessageBox::critical(this, "Error", "file corrupt");
        return;
    } else {
        m_cam = new Camera(fSettings["Camera.fx"], fSettings["Camera.fy"], fSettings["Camera.cx"], fSettings["Camera.cy"]);
        m_cam->set_distortion(fSettings["Camera.k1"], fSettings["Camera.k2"], fSettings["Camera.p1"], fSettings["Camera.p2"]);
    }

    fp_input = fopen(SLAM_RT_MTX_FILEPATH , "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "file corrupt");
        return;
    } else {
        m_image_rt.clear();
        double r1, r2, r3 ,r4;
        char garb; fscanf(fp_input, " %c", &garb);
        while(garb == '-'){
            QMatrix4x4 temp;
            temp.setToIdentity();
            m_image_rt.push_back(temp);
            fscanf(fp_input, " %c", &garb);
        }

        while(garb == '['){
            QMatrix4x4 temp;
            temp.setToIdentity();

            fscanf (fp_input, "%lf, %lf, %lf, %lf;", &r1, &r2, &r3, &r4);
            temp.setRow(0,QVector4D(r1,r2,r3,0));
            fscanf (fp_input, " %lf, %lf, %lf, %lf;", &r1, &r2, &r3, &r4);
            temp.setRow(1,QVector4D(r1,r2,r3,0));
            fscanf (fp_input, " %lf, %lf, %lf, %lf;", &r1, &r2, &r3, &r4);
            temp.setRow(2,QVector4D(r1,r2,r3,0));
            fscanf (fp_input, " %lf, %lf, %lf, %lf]", &r1, &r2, &r3, &r4);
            temp.setRow(3,QVector4D(r1,r2,r3,1));

            m_image_rt.push_back(temp);
            if(fscanf(fp_input, " %c", &garb) == EOF)
                break;
        }
        fclose(fp_input);
    }

}

void MyGLWidget::draw_scene() {
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_worldRotation * m_camera * m_worldTranslation);
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

int MyGLWidget::get_keyframe_index(int start, double val){
    for(uint i=start;i<m_keyframes.size()-1;i++){
        if(m_keyframes[i].timestamp <= val and val <= m_keyframes[i+1].timestamp)
            return i;
        if (val < m_keyframes[i].timestamp)
            return -1;
    }
    return -2;
}

int MyGLWidget::get_image_index(int start, double val){
    for(uint i=start;i<m_image_data.size()-1;i++){
//        if(m_image_data[i].second <= val and val <= m_image_data[i+1].second)
//            return i+1;
        if(m_image_data[i].second == val)
            return i;
        if (val < m_image_data[i].second)
            return -1;
    }
    return -2;
}

void MyGLWidget::input_mesh(std::string fileName) {
    m_mesh = Mesh(fileName);
    m_mode = ADJUST_MESH;
    float i = 1, j = 1, z = -1;
    m_bg_points.append(Point(QVector3D(-1*i, i, z), QVector2D(0, j)));
    m_bg_points.append(Point(QVector3D(-1*i, -1*i, z), QVector2D(0, 0)));
    m_bg_points.append(Point(QVector3D(i, i, z), QVector2D(j, j)));
    m_bg_points.append(Point(QVector3D(i, i, z), QVector2D(j, j)));
    m_bg_points.append(Point(QVector3D(-1*i, -1*i, z), QVector2D(0, 0)));
    m_bg_points.append(Point(QVector3D(i, -1*i, z), QVector2D(j, 0)));
    m_bg_vbo.create();
    update();
}

void MyGLWidget::playback() {
    m_mode = PLAYBACK;
    m_simulation_time_ms = 100000;
    m_curr_image_index = 0;
    m_timer->start(int(m_simulation_time_ms * m_image_data[m_curr_image_index+1].second));
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
        QMatrix4x4 transform = m_worldRotation * m_camera * m_worldTranslation;
        if (insert_mode == "ADD_PLANE") { // Show selected points
            for (int i = 0; i < m_scene_points.size(); i++) {
                if (between_corners(m_proj * transform, m_scene_points[i].position, corner1, QVector3D(event->x(), event->y(), 0))) {
                    m_scene_points[i].color = QVector3D(1, 0, 0);
                } else {
                    m_scene_points[i].color = QVector3D(1, 1, 1);
                }
            }
        } else if(insert_mode == "REMOVE_PLANE"){ // Show selected plane
            m_selected_plane_for_removal = -1;
            float fraction_inside = 0;
            for (int i = 0; i < m_planes.size(); i++) {
                float curr_fraction = m_planes[i].num_between_corners(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0));
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
                    if(m_planes[i].select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0))){
                        m_planes[i].recolor_selected(1,0,0);
                        m_plane_1 = i;
                        break;
                    }
                }
            } else if(m_plane_2 == -1){
                for (int i = 0; i < m_planes.size(); i++) {
                    if(m_planes[i].select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0))){
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
                    if(m_planes[i].select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0))){
                        m_planes[i].recolor_selected(1,1,0);
                        m_snap_plane = i;
                        break;
                    }
                }
            } else {
                m_mesh_point_selected = m_mesh.select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0));
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
        m_curr_image_index++;
    }
    if(event->key() == Qt::Key_O){
        m_i--;
        m_curr_image_index--;
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
    m_mode = ADJUST_SCENE;
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
        QMatrix4x4 transform = m_worldRotation * m_camera * m_worldTranslation;
        m_mesh.adjust(m_planes[m_snap_plane].get_selected_point(), transform, transform);
    }
    if(m_snap_plane != -1)
        m_planes[m_snap_plane].recolor_selected(0,0,1);
    m_snap_plane = -1;
    update();
}
