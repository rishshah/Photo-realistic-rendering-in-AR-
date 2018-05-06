#include "myglwidget.h"

//-------------------------------------------------
/// Descriptor Matching Algorithm
//-------------------------------------------------
/**
 * @brief Uses Hamming distance between orb-descriptots to check for a match
 *
 * @param map_pts ORBSLAM2 MapPoints of current image
 * @param key_pts Opencv KeyPoints of current image
 * @param descriptors Descriptors to match for
 * @param matcher ORB Matcher module
 *
 * @return Number of matches among the given map points
 */
int descriptor_match(std::vector<ORB_SLAM2::MapPoint*> map_pts, std::vector<cv::KeyPoint> key_pts, std::vector<cv::Mat> descriptors, ORB_SLAM2::ORBmatcher* matcher) {
    int num_matches = 0;

    #pragma omp parallel for
    for (uint i = 0; i < descriptors.size(); i++) {
        int bestDist = 256;
        int bestLevel = -1;
        int bestDist2 = 256;
        int bestLevel2 = -1;
        int bestIdx = -1 ;

        #pragma omp parallel for
        for (uint j = 0; j < map_pts.size(); j++) {
            if (map_pts[j]) {
                cv::Mat desc = map_pts[j]->GetDescriptor();
                int dist = matcher->DescriptorDistance(desc , descriptors[i]);
                if (dist < bestDist) {
                    bestDist2 = bestDist;
                    bestDist = dist;
                    bestLevel2 = bestLevel;
                    bestLevel = key_pts[j].octave;
                    bestIdx = j;
                } else if (dist < bestDist2) {
                    bestLevel2 = key_pts[j].octave;
                    bestDist2 = dist;
                }
            }
        }
        if (bestDist <= 60) {
            if (bestLevel == bestLevel2 && bestDist > 0.9 * bestDist2)
                continue;
            num_matches++;
        }
    }
    return num_matches;
}

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_xPos(0.0f), m_yPos(0.0f), m_zPos(0.0f),
      m_xRot(0), m_yRot(0), m_zRot(0),
      m_i(0),
      m_selected_plane_for_removal(-1),
      m_curr_image_index(0),
      m_curr_keyframe_index(0),
      m_plane_1(-1),
      m_plane_2(-1),
      m_snap_plane(-1),
      m_mesh_point_selected(false), m_shift(false),
      m_program(0),
      m_slam(NULL) {
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

MyGLWidget::~MyGLWidget() {
    cleanup();
}

/**
 * @brief QTWidget class derived function
 */
QSize MyGLWidget::minimumSizeHint() const {
    return QSize(50, 50);
}
/**
 * @brief QTWidget class derived function
 */
QSize MyGLWidget::sizeHint() const {
    return QSize(400, 400);
}


//-------------------------------------------------
/// Helper functions (rotations and other transforms)
//-------------------------------------------------
/**
 * @brief Normalize angle above 360 deg or below 0
 *
 * @param angle output/input angle
 */
static void qNormalizeAngle(int &angle) {
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

/**
 * @brief set X rotaion of screen to given angle
 */
void MyGLWidget::setXRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

/**
 * @brief set Y rotaion of screen to given angle
 */
void MyGLWidget::setYRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

/**
 * @brief set Z rotaion of screen to given angle
 */
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
    for (uint i = 0; i < m_vMPs.size(); i++) {
        delete m_vMPs[i];
    }
    delete m_slam;
    delete m_matcher;
    delete m_cam;
    m_program = 0;
    doneCurrent();
}

/**
 * @brief set the world rotation matrix according to x,y,z member variable rotations 
 */
void MyGLWidget::adjustWorldRotationTransform() {
    m_worldRotation.setToIdentity();
    m_worldRotation.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_worldRotation.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_worldRotation.rotate(m_zRot / 16.0f, 0, 0, 1);
}

/**
 * @brief set the world translation matrix according to x,y,z member variable translations
 */
void MyGLWidget::adjustWorldTranslationTransform() {
    m_worldTranslation.setToIdentity();
    m_worldTranslation.translate(m_xPos, m_yPos, m_zPos);
}


//-------------------------------------------------
/// Default functions for QT OpenGL
//-------------------------------------------------
/**
 * @brief Recalculate the projection transform on resizing the windoe
 *
 * @param w new width
 * @param h new height
 */
void MyGLWidget::resizeGL(int w, int h) {
    m_proj = m_cam->getProjectionTransform(w, h);
}

/**
 * @brief Bunch of initialization of Gl window operation including opengl shader initializations
 */
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
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, m_vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, m_fragmentShaderSource);
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
    m_uIs_colored = m_program->uniformLocation("uIs_colored");

    m_vao.create();
    m_vao.bind();
    m_scene_vbo.create();
    m_key_vbo.create();
    init_background();
    m_vao.release();
    m_program->release();
}

/**
 * @brief Draw call of Qt that will be called again if anything changes
 */
void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    adjustWorldRotationTransform();
    adjustWorldTranslationTransform();
    m_camera.setToIdentity();


    m_program->bind();
    m_vao.bind();
    switch (m_playback_mode) {
    case ONLINE_WEBCAM:
        switch (m_mode) {
        case PLAY_FIRST:
            draw_background_webcam();
            break;
        case MATCH:
            draw_background_webcam();
            draw_scene();
            draw_planes();
            draw_mesh();
            break;
        default:
            break;
        }
        break;
    case ONLINE_IMAGES:
        switch (m_mode) {
        case PLAY_FIRST:
            draw_background_images();
            break;
        case MATCH:
            draw_background_images();
            draw_scene();
            draw_planes();
            draw_mesh();
            break;
        default:
            break;
        }
        break;
    case OFFLINE_WEBCAM:
        switch (m_mode) {
        case PLAY_FIRST:
            draw_background_webcam();
            break;
        case ADJUST_SCENE:
            draw_background_images();
            draw_scene();
            draw_planes();
            break;

        case ADJUST_MESH:
            draw_background_images();
            draw_scene();
            draw_planes();
            draw_mesh();
            break;

        case PLAYBACK:
            draw_background_images();
            draw_scene();
            draw_planes();
            draw_mesh();
            break;
        default:
            break;
        }
        break;
    case OFFLINE_IMAGES:
        switch (m_mode) {
        case PLAY_FIRST:
            draw_background_images();
            break;
        case ADJUST_SCENE:
            draw_background_images();
            draw_scene();
            draw_planes();
            break;
        case ADJUST_MESH:
            draw_background_images();
            draw_scene();
            draw_planes();
            draw_mesh();
            break;
        case PLAYBACK:
            draw_background_images();
            draw_scene();
            draw_planes();
            draw_mesh();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    m_vao.release();
    m_program->release();
}


//-------------------------------------------------
/// Draw elements on the screen
//-------------------------------------------------

/**
 * @brief Draw user created planes on the screen
 */
void MyGLWidget::draw_planes() {
    m_program->setUniformValue(m_uIs_tp, 0);
    m_program->setUniformValue(m_uIs_colored, 1);
    m_program->setUniformValue(m_mvMatrixLoc, m_proj * m_worldRotation * m_camera * m_worldTranslation);
    for (int i = 0; i < m_planes.size(); i++) {
        m_planes[i].draw(m_program, m_vPosition, m_vColor);
    }
}

/**
 * @brief Draw user uploaded mesh on the screen
 */
void MyGLWidget::draw_mesh() {
    m_program->setUniformValue(m_uIs_tp, 0);
    m_program->setUniformValue(m_uIs_colored, 1);
    m_mesh.draw(m_program, m_proj * m_worldRotation * m_camera * m_worldTranslation, m_mvMatrixLoc, m_vPosition, m_vColor);
}

/**
 * @brief Draw the sparse point cloud obtained from SLAM on the screen
 */
void MyGLWidget::draw_scene() {
    m_program->setUniformValue(m_uIs_tp, 0);
    m_program->setUniformValue(m_uIs_colored, 1);
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

/**
 * @brief Draw the background texture on the screen
 */
void MyGLWidget::draw_background() {
    glDisable(GL_DEPTH_TEST);
    m_program->setUniformValue(m_uIs_tp, 1);
    QMatrix4x4 m_ortho;
    m_ortho.setToIdentity();
    m_ortho.ortho(-1, 1, -1, 1, -1, 1);
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

/**
 * @brief Function called just prior to texture drawing in image input mode
 * @details This function takes care of various stages of application and draws different stuff required for each
 */
void MyGLWidget::draw_background_images() {
    cv::Mat imu;
    cv::Mat im;
    if (m_mode == ADJUST_SCENE or m_mode == ADJUST_MESH) {
        int image_index = get_image_index(0, m_keyframes[m_i].timestamp);
        if (image_index == -1 or image_index == -2)
            printf("ERRR %d: %d", m_i, image_index);

        std::string s = m_image_dir + "/" + m_image_data[image_index].first;
        im = cv::imread(s, cv::IMREAD_GRAYSCALE);
        cv::undistort(im, imu, m_cam->get_cam_parameter(), m_cam->get_cam_distortion());

        m_camera.rotate(m_keyframes[m_i].orientation.conjugate());
        m_camera.translate(-m_keyframes[m_i].position);
    }
    else {
        if (m_curr_image_index < m_image_data.size()) {
            std::string s = m_image_dir + "/" + m_image_data[m_curr_image_index].first;
            im = cv::imread(s, cv::IMREAD_GRAYSCALE);
            cv::undistort(im, imu, m_cam->get_cam_parameter(), m_cam->get_cam_distortion());

            if (m_mode == PLAYBACK or m_mode == MATCH) {
                m_camera =  convert2QMat(m_slam->TrackMonocular(im, m_image_data[m_curr_image_index].second));
            }
            else if (m_mode == PLAY_FIRST) {

                m_slam->TrackMonocular(im, m_image_data[m_curr_image_index].second);
                m_vMPs = m_slam->GetTrackedMapPoints();
                std::vector<cv::KeyPoint> keyPoints = m_slam->GetTrackedKeyPointsUn();
                for (uint i = 0; i < keyPoints.size(); i++) {
                    if (m_vMPs[i])
                        cv::circle(imu, keyPoints[i].pt, 3, cv::Scalar(0, 255, 0), -1);
                }

                if (m_playback_mode == ONLINE_IMAGES) {
                    if (m_curr_image_index % 50 == 0) {
                        if (m_curr_image_index  != 0)
                            m_match_thread.waitForFinished();
                        m_match_thread = QtConcurrent::run(descriptor_match, m_vMPs, keyPoints, m_scene_descriptors, m_matcher);
                        printf("ImgNo-> %d | vMPs-> %d | matches-> %d\n", m_curr_image_index, m_vMPs.size(), m_match_thread.result());
                        /// MATCHING
                        if (m_match_thread.result() > m_vMPs.size() / 3.0) {
                            m_mode = MATCH;
                            loadAll();
                        }
                    }
                }
            }
            m_curr_image_index++;

        } else {
            m_curr_image_index = 0;
            m_curr_keyframe_index = 0;
            m_timer->stop();
            m_slam->Shutdown();
            if (m_mode == PLAY_FIRST and m_playback_mode == OFFLINE_IMAGES) {
                m_mode = ADJUST_SCENE;
                m_slam->SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
                // Load map points // change path
                readMapPoints("./abc.txt");
                readKeyFrames("./KeyFrameTrajectory.txt");
            }
            return;
        }
    }
    bg_tex = distorted_texture_load(imu, true);
    if (im.channels() == 3) {
        m_program->setUniformValue(m_uIs_colored, 1);
    } else {
        m_program->setUniformValue(m_uIs_colored, 0);
    }
    draw_background();
}

/**
 * @brief Function called just prior to texture drawing in webcam input mode
 * @details This function takes care of various stages of application and draws different stuff required for each
 */
void MyGLWidget::draw_background_webcam() {
    cv::Mat imu;
    cv::Mat im;
    if (m_curr_image_index < m_simulation_param) {
        m_webcam >> im;
        cv::undistort(im, imu, m_cam->get_cam_parameter(), m_cam->get_cam_distortion());
        if (m_mode == PLAY_FIRST) {

            m_slam->TrackMonocular(im, m_curr_image_index);
            m_vMPs = m_slam->GetTrackedMapPoints();
            std::vector<cv::KeyPoint> keyPoints = m_slam->GetTrackedKeyPointsUn();
            for (uint i = 0; i < keyPoints.size(); i++) {
                if (m_vMPs[i])
                    cv::circle(imu, keyPoints[i].pt, 3, cv::Scalar(0, 255, 0), -1);
            }

            if (m_playback_mode == ONLINE_WEBCAM) { // Check for matching
                if (m_curr_image_index % 50 == 0) {
                    if (m_curr_image_index  != 0)
                        m_match_thread.waitForFinished();
                    m_match_thread = QtConcurrent::run(descriptor_match, m_vMPs, keyPoints, m_scene_descriptors, m_matcher);
                    printf("ImgNo-> %d | vMPs-> %d | matches-> %d\n", m_curr_image_index, m_vMPs.size(), m_match_thread.result());
                    /// MATCHING
                    if (m_match_thread.result() > m_vMPs.size() / 2.0) {
                        printf("ImgNo-> %d | vMPs-> %d | matches-> %d\n", m_curr_image_index, m_vMPs.size(), m_match_thread.result());
                        m_mode = MATCH;
                        loadAll();
                    }
                }
            } else { // SAVE Webcam Stream
                string image_png = "image_";
                image_png += std::to_string(m_curr_image_index);
                image_png += ".png";
                cv::imwrite(m_image_dir + "/" + image_png, im);
                fprintf(m_image_csv, "%d,%s\n", m_curr_image_index, image_png.c_str());
            }

        }
        else if (m_mode == MATCH) {
            m_camera =  convert2QMat(m_slam->TrackMonocular(im, m_curr_image_index));
        }

        m_curr_image_index++;
    }
    else {
        m_curr_image_index = 0;
        m_timer->stop();
        m_slam->Shutdown();
        if (m_webcam.isOpened())
            m_webcam.release();

        if (m_mode == PLAY_FIRST and m_playback_mode == OFFLINE_WEBCAM) {
            fclose(m_image_csv);
            m_slam->SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
            m_mode = ADJUST_SCENE;
            // Load images, map points
            readMapPoints("./abc.txt");
            readImageCSV("./imageinfo.csv");
            readKeyFrames("KeyFrameTrajectory.txt");
        }
        return;
    }
    bg_tex = distorted_texture_load(imu, false);
    if (im.channels() == 3) {
        m_program->setUniformValue(m_uIs_colored, 1);
    } else {
        m_program->setUniformValue(m_uIs_colored, 0);
    }
    draw_background();
}


//-------------------------------------------------
/// Read input files (CamSettings, MapPoints, Descriptors, Images)
//-------------------------------------------------

/**
 * @brief Read camera settings from file and set the camera member variable
 */
void MyGLWidget::readCamSettings(string file) {
    cv::FileStorage fSettings;
    fSettings.open(file, cv::FileStorage::READ);
    if (!fSettings.isOpened()) {
        QMessageBox::critical(this, "Error", "readCamSettings: file corrupt");
        return;
    } else {
        m_cam = new Camera(fSettings["Camera.fx"], fSettings["Camera.fy"], fSettings["Camera.cx"], fSettings["Camera.cy"]);
        m_cam->set_distortion(fSettings["Camera.k1"], fSettings["Camera.k2"], fSettings["Camera.p1"], fSettings["Camera.p2"]);
    }
}

/**
 * @brief Read map points(for sparse cloud) from file
 */
void MyGLWidget::readMapPoints(std::string file) {
    FILE* fp_input = fopen(file.c_str(), "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", " readMapPoints: file corrupt");
    } else {
        m_scene_points.clear();
        float x, y, z;
        while (fscanf (fp_input, "%f %f %f", &x, &y, &z) != EOF) {
            m_scene_points.append(Point(QVector3D(x, y, z), QVector3D(1, 1, 1)));
        }
        fclose(fp_input);
    }

    fp_input = fopen(SLAM_POINTS_DESC_FILEPATH, "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", " readMapPoints: file corrupt");
    } else {
        m_scene_descriptors.resize(m_scene_points.size());
        char garb;
        for (int i = 0; i < m_scene_points.size(); i++) {
            fscanf(fp_input, "%c", &garb); // [
            m_scene_descriptors[i] = cv::Mat::zeros(1, 32, CV_8UC1);
            for (int j = 0; j < 32; j++) {
                uint8_t a; fscanf(fp_input, "%d", &a);
                m_scene_descriptors[i].at<uint8_t>(0, j) = a;
                fscanf(fp_input, "%c", &garb); // , or ]
            }
            fscanf(fp_input, "%c", &garb); // \n
        }
        fclose(fp_input);
    }
}

/**
 * @brief Read image names and timestamps from image csv file
 */
void MyGLWidget::readImageCSV(string file) {
    FILE* fp_input = fopen(file.c_str(), "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "readImageCSV: file corrupt");
    } else {
        m_image_data.clear();
        char img_file[200];
        double timestamp;
        while (fscanf (fp_input, "%lf, %s", &timestamp, img_file) != EOF) {
            m_image_data.push_back(std::make_pair(img_file, timestamp));
        }
        fclose(fp_input);
    }
}

/**
 * @brief Read keyframes saved by orbslam image keyframetrajectory file
 */
void MyGLWidget::readKeyFrames(string file) {
    FILE* fp_input = fopen(file.c_str(), "r" );
    if (fp_input ==  NULL) {
        QMessageBox::critical(this, "Error", "resdkeyframes: file corrupt");
        return;
    } else {
        m_keyframes.clear();
        double t; float x, y, z, q1, q2, q3, q4;
        while (fscanf (fp_input, "%lf %f %f %f %f %f %f %f", &t, &x, &y, &z, &q1, &q2, &q3 , &q4) != EOF) {
            m_keyframes.push_back(Keyframe(t, QVector3D(x, y, z), QQuaternion(q4, q1, q2, q3)));
        }
        fclose(fp_input);
    }

    double minTimeStamp = std::min(m_keyframes[0].timestamp, m_image_data[0].second);
    double maxTimeStamp = std::max(m_keyframes[m_keyframes.size() - 1].timestamp, m_image_data[m_image_data.size() - 1].second);
    for (uint i = 0; i < m_image_data.size(); i++) {
        m_image_data[i].second = (m_image_data[i].second - minTimeStamp) / (maxTimeStamp - minTimeStamp);
    }
    for (uint i = 0; i < m_keyframes.size(); i++) {
        m_keyframes[i].timestamp = (m_keyframes[i].timestamp - minTimeStamp) / (maxTimeStamp - minTimeStamp);
    }
}


//-------------------------------------------------
/// Initialize widget according to 4 different modes
//-------------------------------------------------

/**
 * @brief Initialization stuff for OFFLINE WEBCAM mode called when creating this window
 */
void MyGLWidget::fill_image_data(string camSettings) {
    m_playback_mode = OFFLINE_WEBCAM; // Play webcam for a brand new video and record mappoints and video too
    readCamSettings(camSettings);
    m_camera_settings = camSettings;
    m_image_dir = "./images";

    m_image_csv = fopen("./imageinfo.csv", "w" );
    if (m_image_csv ==  NULL) {
        QMessageBox::critical(this, "Error", "readImageCSV: file corrupt");
    }

    m_webcam = cv::VideoCapture(0);
    if (!m_webcam.isOpened()) {
        QMessageBox::critical(this, "Error", "Webcam Problem");
        return;
    }
    playfirst(2000);
}

/**
 * @brief Initialization stuff for ONLINE WEBCAM mode called when creating this window
 */
void MyGLWidget::fill_image_data(string camSettings, string mapPoints) {
    m_playback_mode = ONLINE_WEBCAM; //  Play webcam for a related video and use matching to render
    readCamSettings(camSettings);
    m_camera_settings = camSettings;
    readMapPoints(mapPoints);
    m_webcam = cv::VideoCapture(0);
    if (!m_webcam.isOpened()) {
        QMessageBox::critical(this, "Error", "Webcam Problem");
        return;
    }
    playfirst(2000);
}

/**
 * @brief Initialization stuff for OFFLINE IMAGES mode called when creating this window
 */
void MyGLWidget::fill_image_data(string camSettings, string imageDirectory, string imageCSV) {
    m_playback_mode = OFFLINE_IMAGES; // Play imageStream for a brand new video and record mappoints
    m_image_dir = imageDirectory;
    m_camera_settings = camSettings;
    readCamSettings(camSettings);
    readImageCSV(imageCSV);
    playfirst(30000);
}

/**
 * @brief Initialization stuff for ONLINE IMAGES mode called when creating this window
 */
void MyGLWidget::fill_image_data(string camSettings, string imageDirectory, string imageCSV, string mapPoints) {
    m_playback_mode = ONLINE_IMAGES; // Play new related image stream and use matching to render
    m_image_dir = imageDirectory;
    m_camera_settings = camSettings;
    readCamSettings(camSettings);
    readImageCSV(imageCSV);
    readMapPoints(mapPoints);
    playfirst(30000);
}


/**
 * @brief Initialization stuff for Debug mode called when creating this window using "Direct Images" option in main window
 */
void MyGLWidget::fill_image_data() {
    m_playback_mode = OFFLINE_IMAGES;
    m_mode = ADJUST_SCENE;

   // Debug options for IMAGE MODE 
    string imageDirectory = BASE_DIR "V1_01_easy/mav0/cam0/data";
    string imageCSV  = BASE_DIR "V1_01_easy/mav0/cam0/data.csv";
    string camSettings  = BASE_DIR "ORB_SLAM2/Examples/Monocular/EuRoC.yaml";

   // Debug options for WEBCAM MODE 
   // string imageDirectory = OUTPUT_BASE "images";
   // string imageCSV  = OUTPUT_BASE "imageinfo.csv";
   // string camSettings  = BASE_DIR "webcam.yaml";

    string mapPoints = OUTPUT_BASE "abc.txt";
    string keyframes = OUTPUT_BASE "KeyFrameTrajectory.txt";

    m_camera_settings = camSettings;
    m_image_dir = imageDirectory;
    readCamSettings(camSettings);
    readImageCSV(imageCSV);
    readMapPoints(mapPoints);
    readKeyFrames(keyframes);
}


//-------------------------------------------------
/// Initialize mesh and background quad
//-------------------------------------------------

/**
 * @brief Initialize the quad for background texture
 */
void MyGLWidget::init_background() {
    float i = 1, j = 1, z = -1;
    m_bg_points.append(Point(QVector3D(-1 * i, i, z), QVector2D(0, j)));
    m_bg_points.append(Point(QVector3D(-1 * i, -1 * i, z), QVector2D(0, 0)));
    m_bg_points.append(Point(QVector3D(i, i, z), QVector2D(j, j)));
    m_bg_points.append(Point(QVector3D(i, i, z), QVector2D(j, j)));
    m_bg_points.append(Point(QVector3D(-1 * i, -1 * i, z), QVector2D(0, 0)));
    m_bg_points.append(Point(QVector3D(i, -1 * i, z), QVector2D(j, 0)));
    m_bg_vbo.create();
}

/**
 * @brief Initializes the mesh(called on click of "Add Mesh")
 *
 * @param fileName mesh file path
 */
void MyGLWidget::input_mesh(std::string fileName) {
    m_program->bind();
    m_vao.bind();

    m_mesh = Mesh(fileName);
    m_vao.release();
    m_program->release();
    m_mode = ADJUST_MESH;
    update();
}

/**
 * @brief get image index after start which is closest to val from image csv timestamps
 */
int MyGLWidget::get_image_index(int start, double val) {
    for (uint i = start; i < m_image_data.size() - 1; i++) {
        if (m_image_data[i].second == val)
            return i;
        if (val < m_image_data[i].second)
            return -1;
    }
    return -2;
}

//-------------------------------------------------
/// Initlialize function for PLAYFIRST and PLAYBACK mode
//-------------------------------------------------

/**
 * @brief Initialize the SLAM system and timer (called for ONLINE MODE)
 *
 * @param param total time of simulation
 */
void MyGLWidget::playfirst(int param) {
    m_mode = PLAY_FIRST;
    m_matcher = new ORB_SLAM2::ORBmatcher(0.9, true);
    m_slam = new ORB_SLAM2::System(VOCABULARY, m_camera_settings, ORB_SLAM2::System::MONOCULAR, false);
    m_simulation_param = param;
    m_curr_image_index = 0;
    m_timer->start(1);
}

/**
 * @brief Initialize SLAM system and timer (called for OFFLINE MODE)
 */
void MyGLWidget::playback() {
    m_mode = PLAYBACK;
    if (m_slam != NULL)
        delete m_slam;
    m_slam = new ORB_SLAM2::System(VOCABULARY, m_camera_settings, ORB_SLAM2::System::MONOCULAR, false);
    m_simulation_param = 30000;
    m_curr_image_index = 0;
    m_timer->start(1);
}


//-------------------------------------------------
/// Save/Load mesh and planes
//-------------------------------------------------

/**
 * @brief Save all the augmentations created by the user
 */
void MyGLWidget::saveAll() {
    FILE* fp_output = fopen(AUGMENT_FILEPATH, "w" );
    if (fp_output ==  NULL) {
        QMessageBox::critical(this, "Error", "cannot save output");
    }
    fprintf(fp_output, "%d\n", m_planes.size());
    for (uint i = 0; i < m_planes.size(); i++) {
        m_planes[i].save(fp_output);
    }
    m_mesh.save(fp_output);

    fclose(fp_output);
}

/**
 * @brief Load all augmentations saved by user
 */
void MyGLWidget::loadAll() {
    FILE* fp = fopen(AUGMENT_FILEPATH, "r" );
    if (fp ==  NULL) {
        QMessageBox::critical(this, "Error", "cannot load planes and meshes");
    }
    int x;
    fscanf(fp, "%d\n", &x);
    for (uint i = 0; i < x; i++) {
        Plane p;
        p.load(fp);
        m_planes.push_back(p);
    }
    std::string mesh_file = MESH_FILEPATH;
    m_mesh = Mesh(mesh_file);
    m_mesh.load(fp);
    fclose(fp);
}


//-------------------------------------------------
/// Keyboards and Mouse Events
//-------------------------------------------------

/**
 * @brief handle mouse press event to save the press location
 */
void MyGLWidget::mousePress(QMouseEvent *event) {
    m_lastPos = event->pos();
}

/**
 * @brief Handle mouse move event to show user the outcomes
 * @detaisl These are selections made in case of select mode, screen rotations in case of pan mode
 *
 * @param select_mode true iff select mode
 * @param insert_mode One of the four radiobutton oprions indication exact behaviour on mouse move
 */
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
        } else if (insert_mode == "REMOVE_PLANE") { // Show selected plane
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
        } else if (insert_mode == "ADJUST_PLANE") { // Show selected plane point
            if (m_plane_1 == -1) {
                for (int i = 0; i < m_planes.size(); i++) {
                    if (m_planes[i].select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0))) {
                        m_planes[i].recolor_selected(1, 0, 0);
                        m_plane_1 = i;
                        break;
                    }
                }
            } else if (m_plane_2 == -1) {
                for (int i = 0; i < m_planes.size(); i++) {
                    if (m_planes[i].select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0))) {
                        m_planes[i].recolor_selected(1, 0, 0);
                        if (m_plane_1 != i)
                            m_plane_2 = i;
                        break;
                    }
                }
            }
        } else if (insert_mode == "ADJUST_MESH") { // Show selected plane point and mesh point
            if (m_snap_plane == -1) {
                for (int i = 0; i < m_planes.size(); i++) {
                    if (m_planes[i].select_point(m_proj * transform, corner1, QVector3D(event->x(), event->y(), 0))) {
                        m_planes[i].recolor_selected(1, 1, 0);
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
/**
 * @brief Handle keyevents for ["Shift", "Alt", "WASDZX", "OP"]
 */
void MyGLWidget::keyPress(QKeyEvent *event) {
    if (event->key() == Qt::Key_S) {
        if (m_shift)
            m_yPos -= DY / 10.0;
        else
            m_yPos -= DY;
    }
    else if (event->key() == Qt::Key_W) {
        if (m_shift)
            m_yPos += DY / 10.0;
        else
            m_yPos += DY;
    }

    if (event->key() == Qt::Key_A) {
        if (m_shift)
            m_xPos -= DX / 10.0;
        else
            m_xPos -= DX;
    }
    else if (event->key() == Qt::Key_D) {
        if (m_shift)
            m_xPos += DX / 10.0;
        else
            m_xPos += DX;
    }

    if (event->key() == Qt::Key_X) {
        if (m_shift)
            m_zPos -= DZ / 10.0;
        else
            m_zPos -= DZ;
    }
    else if (event->key() == Qt::Key_Z) {
        if (m_shift)
            m_zPos += DZ / 10.0;
        else
            m_zPos += DZ;
    }

    if (event->key() == Qt::Key_Shift) {
        m_shift = true;
    }
    else if (event->key() == Qt::Key_Alt) {
        m_shift = false;
    }

    if ((m_mode == ADJUST_SCENE or m_mode == ADJUST_MESH)  and event->key() == Qt::Key_P) {
        m_i = (m_i + DM) % m_keyframes.size();
    }
    else if ((m_mode == ADJUST_SCENE or m_mode == ADJUST_MESH) and event->key() == Qt::Key_O) {
        m_i = (m_i - DM) % m_keyframes.size();
    }
    update();
}


//-------------------------------------------------
/// GLWidget interface functions
//-------------------------------------------------

/**
 * @brief Add plane to the scene based on user selected points (Called on "Add Mode" & "Confirm" click)
 */
void MyGLWidget::add_plane() {
    QVector<QVector3D> selected_points;
    for (int i = 0; i < m_scene_points.size(); i++) {
        if (m_scene_points[i].color.y() != 1) {
            selected_points.append(m_scene_points[i].position);
        }
    }
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


/**
 * @brief Remove plane from the scene based on user selected plane (Called on "Remove Mode" & "Confirm" click)
 */
void MyGLWidget::remove_plane() {
    if (m_selected_plane_for_removal != -1) {
        m_planes.remove(m_selected_plane_for_removal);
        m_selected_plane_for_removal = -1;
        update();
    }
}


/**
 * @brief Merge planes in the scene based on user selected points on two plane (Called on "Adjust Planes Mode" & "Confirm" click)
 */
void MyGLWidget::adjust_planes() {
    if (m_plane_1 != -1 and m_plane_2 != -1) {
        m_planes[m_plane_1].adjust(m_planes[m_plane_2].get_selected_point());
        m_planes[m_plane_1].recolor_selected(0, 0, 1);
        m_planes[m_plane_2].recolor_selected(0, 0, 1);
    } else {
        if (m_plane_1 != -1)
            m_planes[m_plane_1].recolor_selected(0, 0, 1);
        if (m_plane_2 != -1)
            m_planes[m_plane_2].recolor_selected(0, 0, 1);
    }
    m_plane_1 = m_plane_2 = -1;
    update();
}


/**
 * @brief Join mesh to plane in the scene based on user selected points (Called on "Adjust Mesh Mode" & "Confirm" click)
 */
void MyGLWidget::adjust_mesh() {
    if (m_mesh_point_selected and m_snap_plane != -1) {
        QMatrix4x4 transform = m_worldRotation * m_camera * m_worldTranslation;
        m_mesh.adjust(m_planes[m_snap_plane].get_selected_point(), transform, transform);
    }
    if (m_snap_plane != -1)
        m_planes[m_snap_plane].recolor_selected(0, 0, 1);
    m_snap_plane = -1;
    update();
}
