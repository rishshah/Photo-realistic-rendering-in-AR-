#include "camera.h"

Camera::Camera(){
    m_fx = m_fy = 0;
    m_k1 = m_k2 = m_p1 = m_p2 = 0;
    m_cx = m_cy = 0;
    m_near = 0.01f;
    m_far = 10000.0f;
}
/**
 * @brief Camera custom constructor
 * 
 * @param fx focal distance along x axis 
 * @param fy focal distance along y axis 
 * @param cx principle point x coordinate 
 * @param cy principle point y coordinate
 */
Camera::Camera(float fx, float fy, float cx, float cy):m_fx(fx), m_fy(fy), m_cx(cx), m_cy(cy), m_near(0.001), m_far(10000.0){
}

/**
 * @brief Get the opencv intrinsic camera matrix 3 X 3
 */
cv::Mat Camera::get_cam_parameter(){
    cv::Mat K = cv::Mat::eye(3,3,CV_32F);
    K.at<float>(0,0) = m_fx;
    K.at<float>(1,1) = m_fy;
    K.at<float>(0,2) = m_cx;
    K.at<float>(1,2) = m_cy;
    return K;
}

/**
 * @brief Get camera distortion coefficients in 4 X 1 matrix form
 */
cv::Mat Camera::get_cam_distortion(){
    cv::Mat DistCoef = cv::Mat::zeros(4,1,CV_32F);
    DistCoef.at<float>(0) = m_k1;
    DistCoef.at<float>(1) = m_k2;
    DistCoef.at<float>(2) = m_p1;
    DistCoef.at<float>(3) = m_p2;
    return DistCoef;
}

/**
 * @brief get opengl projection matrix from opencv intrinsic camera
 * 
 * @param w width of image
 * @param h height of image
 * 
 * @return Q Matrix 4 X 4 (Projection transform)
 */
QMatrix4x4 Camera::getProjectionTransform(int w, int h){
    QMatrix4x4 proj;
    proj.setToIdentity();
    float A = - (m_near + m_far)/(m_far - m_near);
    float B = - (2.0f * m_near * m_far)/(m_far - m_near);
    proj.setRow(0,QVector4D((2 * m_fx) / w,   1 - 2 * m_cx/w,  0, 0));
    proj.setRow(1,QVector4D(0,   (2 * m_fy) / h,  1 - 2 * m_cy/h, 0));
    proj.setRow(2,QVector4D(0,    0,   A,   B));
    proj.setRow(3,QVector4D(0,    0,  -1,  0));
    return proj;
}
