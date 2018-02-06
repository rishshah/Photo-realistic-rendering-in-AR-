#include "myglwidget.h"
#include "utils.h"
#include <QMouseEvent>
#include <qt5/QtOpenGL/QGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <QMessageBox>
#include <stdio.h>
#include <Eigen/Dense>
#include <cmath>

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_xRot(0),
      m_yRot(0),
      m_zRot(0),
      m_mode(0),
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
    m_vbo_pos.destroy();
    delete m_program;
    m_program = 0;
    doneCurrent();
}

static const char *vertexShaderSource =
    "#version 130\n"
    "in vec3 vPosition;\n"
    "in vec4 vColor;\n"
    "out vec4 color;\n"
    "uniform mat4 uModelViewMatrix;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = uModelViewMatrix * vec4(vPosition, 1.0f);\n"
    "   color = vColor;\n"
    "}";

static const char *fragmentShaderSource =
    "#version 130\n"

    "in vec4 color;\n"
    "out vec4 frag_color;\n"

    "void main ()\n"
    "{\n"
    "  frag_color = color;\n"
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
    m_mvMatrixLoc = m_program->uniformLocation("uModelViewMatrix");


    // Our camera never changes in this example.
    m_camera.setToIdentity();
    m_camera.translate(0, 0, -1);

    // Read Points from SLAM output
    read_points();

    m_vao.create();
    m_vao.bind();

    m_vbo_pos.create();
    setUpPositionBuffer();

    m_vbo_col.create();
    setUpColorBuffer();

    m_vbo_grid_col.create();
    m_vbo_grid_pos.create();
    setUpGridBuffer();

    m_vao.release();
    m_program->release();
}

void MyGLWidget::setUpColorBuffer(){
    m_vbo_col.bind();

    m_vbo_col.allocate(m_points_col.constData(), m_points_col.size() * sizeof(QVector3D));

    m_program->enableAttributeArray("vColor");
    m_program->setAttributeBuffer("vColor", GL_FLOAT, 0, 3);

    m_vbo_col.release();
}

void MyGLWidget::setUpGridBuffer(){
    m_vbo_grid_col.bind();
    m_vbo_grid_col.allocate(m_grid_col.constData(), m_grid_col.size() * sizeof(QVector3D));
    m_program->enableAttributeArray("vColor");
    m_program->setAttributeBuffer("vColor", GL_FLOAT, 0, 3);
    m_vbo_grid_col.release();

    m_vbo_grid_pos.bind();
    m_vbo_grid_pos.allocate(m_grid_pos.constData(), m_grid_pos.size() * sizeof(QVector3D));
    m_program->enableAttributeArray("vPosition");
    m_program->setAttributeBuffer("vPosition", GL_FLOAT, 0, 3);
    m_vbo_grid_pos.release();
}

void MyGLWidget::setUpPositionBuffer(){
    m_vbo_pos.bind();

    m_vbo_pos.allocate(m_points_pos.constData(), m_points_pos.size() * sizeof(QVector3D));

    m_program->enableAttributeArray("vPosition");
    m_program->setAttributeBuffer("vPosition", GL_FLOAT, 0, 3);

    m_vbo_pos.release();
}

void MyGLWidget::read_points() {
    FILE *fp_input = fopen(SLAM_POINTS_FILEPATH, "r" );
    if (fp_input ==  NULL) {
         QMessageBox::critical(this,"Error",SLAM_POINTS_FILEPATH " corrupt");
    } else {
        m_points_pos.clear();
        m_points_col.clear();
        float x, y, z;
        while(fscanf (fp_input, "%f %f %f", &x, &y, &z) != EOF){
            m_points_pos.append(QVector3D(x,y,z));
            m_points_col.append(QVector3D(1,1,1));
        }
        fclose(fp_input);
    }
}

bool MyGLWidget::between_corners(QVector3D point, QVector3D c1, QVector3D c2){
    QMatrix4x4 curr_transform = m_proj * m_camera * m_world;
    QVector4D tp = curr_transform * QVector4D(point, 1.0f);
    QVector3D point3D = QVector3D(tp.x()/tp.z(), tp.y()/tp.z(), tp.y()/tp.z());
    c1 = QVector2D(((c1.x()-START_X) - SIZE_X/2.0)/(SIZE_X/2.0), -((c1.y()-START_Y) - SIZE_Y/2.0)/(SIZE_Y/2.0));
    c2 = QVector2D(((c2.x()-START_X) - SIZE_X/2.0)/(SIZE_X/2.0), -((c2.y()-START_Y) - SIZE_Y/2.0)/(SIZE_Y/2.0));

    if(c1.x() <= c2.x() and c1.y() <= c2.y()){ // c1 TL : c2 BR
        return (c1.x() <= point3D.x() and
                point3D.x() <= c2.x() and
                c1.y() <= point3D.y() and
                point3D.y() <= c2.y());

    } else if(c1.x() <= c2.x() and c1.y() >= c2.y()){ // c1 BL : c2 TR
        return (c1.x() <= point3D.x() and
                point3D.x() <= c2.x() and
                c2.y() <= point3D.y() and
                point3D.y() <= c1.y());

    } else if(c1.x() >= c2.x() and c1.y() <= c2.y()){ // c1 TR : c2 BL
        return (c2.x() <= point3D.x() and
                point3D.x() <= c1.x() and
                c1.y() <= point3D.y() and
                point3D.y() <= c2.y());

    } else { // c1 BR : c2 TL
        return (c2.x() <= point3D.x() and
                point3D.x() <= c1.x() and
                c2.y() <= point3D.y() and
                point3D.y() <= c1.y());
    }
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
    m_vao.bind();
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_camera * m_world);

    if(m_mode == 1){
        setUpGridBuffer();
        glDrawArrays(GL_QUAD_STRIP , 0, m_grid_pos.size());
    }

    setUpColorBuffer();
    setUpPositionBuffer();
    glDrawArrays(GL_POINTS, 0, m_points_pos.size());

    m_vao.release();
    m_program->release();
}

void MyGLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void MyGLWidget::pan_mousePressEvent(QMouseEvent *event){
    m_lastPos = event->pos();
}

void MyGLWidget::pan_mouseMoveEvent(QMouseEvent *event){
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
}

void MyGLWidget::select_mousePressEvent(QMouseEvent *event){
    m_lastPos = event->pos();
}

void MyGLWidget::select_mouseMoveEvent(QMouseEvent *event){
    QVector3D corner1 = QVector3D(m_lastPos.x(), m_lastPos.y(), 0);
    for(int i=0; i<m_points_pos.size(); i++){
        if (between_corners(m_points_pos[i], corner1, QVector3D(event->x(), event->y(), 0))){
            m_points_col[i] = QVector3D(1,0,0);
        } else {
            m_points_col[i] = QVector3D(1,1,1);
        }
    }
    update();
}

void get3Points(int n,int&x,int&y,int&z){
    x=rand()%n;
    do{
        y=rand()%n;
    }
    while (y==x);

    do{
        z=rand()%n;
    }
    while(z==x ||z==y);
}

double dist2plane(QVector3D point, QVector3D plane){
    double num = abs(plane.x()*point.x() + plane.y()*point.y() + plane.z()*point.z() - 1);
    double den = sqrt(plane.x()*plane.x() + plane.y()*plane.y() + plane.z()*plane.z());
    return num/den;
}

void fit_plane(QVector<QVector3D> points, QVector3D& maybe_model, double& error){
    int num_points = points.size();

    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixCustom;
    MatrixCustom A(num_points, 3), B(num_points, 1);

    for (int i = 0; i < num_points; ++i){
        A(i,0) = points[i].x();
        A(i,1) = points[i].y();
        A(i,2) = points[i].z();
        B(i,0) = 1;
    }
    MatrixCustom X = (A.transpose() * A).inverse() * A.transpose() * B;
    maybe_model = QVector3D(X(0,0), X(1,0), X(2,0));

    MatrixCustom E = B - A * X;
    error = 0;
    for (int i=0; i<num_points; i++){
        error += E(i,0) * E(i,0);
    }
}

bool close_enough(QVector3D point, QVector3D plane, double error_limit){
    return dist2plane(point, plane) < error_limit;
}

bool good_enough(QVector<QVector3D> points, int good_num_points){
    return points.size() > good_num_points;
}

void MyGLWidget::draw_plane(QVector3D plane, QVector<QVector3D> points){
    m_grid_pos.clear();
    m_grid_col.clear();

    float tl_x = 10000, tl_y = 10000;
    float br_x = -10000, br_y = -10000;
    for(int i=0; i<points.size(); i++){
        if(points[i].x() <= tl_x and points[i].y() <= tl_y){
            tl_x = points[i].x();
            tl_y = points[i].y();
        }
        if(points[i].x() >= br_x and points[i].y() >= br_y){
            br_x = points[i].x();
            br_y = points[i].y();
        }

    }
/*
        min_t = std::min(std::min(min_t, points[i].x()), points[i].y());
        max_t = std::max(std::min(max_t, points[i].x()), points[i].y());
*/
    for(float i=tl_x;i<br_x;i+=(br_x - tl_x)/10.0){
        for(float j=tl_y;j<br_y;j+=(br_y - tl_y)/10.0){
            m_grid_pos.append(QVector3D(i,j,(1 - plane.x()*i - plane.y()*j)/plane.z()));
            m_grid_col.append(QVector3D(0,0,1));
        }
    }

//    for(int i=0;i<points.size();i++){
//        m_grid_pos.append(points[i]);
//        m_grid_col.append(QVector3D(0,0,1));
//    }

    m_mode = 1;
    update();
}

void MyGLWidget::best_plane(){
    QVector<QVector3D> selected_points;
    for(int i=0; i<m_points_pos.size(); i++){
        if(m_points_col[i].y() != 1){
            selected_points.append(m_points_pos[i]);
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
        printf("IP %f\n", this_error);

        for(int i=0; i< selected_points.size(); i++){
            if (i != x and i != y and i != z and close_enough(selected_points[i], maybe_model, error_limit)){
                consensus_set.append(selected_points[i]);
            }
        }

        if (good_enough(consensus_set, good_num_points)){
            fit_plane(consensus_set, maybe_model, this_error);
            printf("T %d %d %f\n", iterations, consensus_set.size(), this_error);
            if (this_error < best_error){
                best_model = maybe_model;
                best_consensus_set = consensus_set;
                best_error = this_error;
            }
        }
        iterations++;
        printf("F %d %d %f\n", iterations, best_consensus_set.size(), best_error);
    }
    printf("PLANE %f %f %f\n", best_model.x(), best_model.y(), best_model.z());
    draw_plane(best_model, best_consensus_set);
}

