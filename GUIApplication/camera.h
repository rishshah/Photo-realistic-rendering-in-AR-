#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"

class Camera{
    float m_fx, m_fy;
    float m_cx, m_cy;
    float m_k1, m_k2, m_p1, m_p2;
    float m_near, m_far;

public:
    Camera();
    Camera(float fx, float fy, float cx, float cy);
    void set_distortion(float k1, float k2, float p1, float p2){
        m_k1 = k1; m_k2 = k2;
        m_p1 = p1; m_p2 = p2;
    }
    cv::Mat get_cam_parameter();
    cv::Mat get_cam_distortion();
    QMatrix4x4 getProjectionTransform(int w, int h);
};

#endif // CAMERA_H
