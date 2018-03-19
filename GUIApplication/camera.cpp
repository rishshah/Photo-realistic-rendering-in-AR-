#include "camera.h"

Camera::Camera(){
    m_fx = m_fy = 0;
    m_k1 = m_k2 = m_p1 = m_p2 = 0;
    m_cx = m_cy = 0;
    m_near = 0.01f;
    m_far = 10000.0f;
}

Camera::Camera(float fx, float fy, float cx, float cy):m_fx(fx), m_fy(fy), m_cx(cx), m_cy(cy), m_near(0.001), m_far(10000.0){
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
