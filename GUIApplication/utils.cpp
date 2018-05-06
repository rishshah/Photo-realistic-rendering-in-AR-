#include "utils.h"
/**
 * @brief get 3 different numbers between 0 and n
 * 
 * @param x output 1st number
 * @param y output 2nd number
 * @param z output 3rd number
 */
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

/**
 * @brief get distance from point to plane  
 */
double dist2plane(QVector3D point, QVector3D plane){
    double num = abs(plane.x()*point.x() + plane.y()*point.y() + plane.z()*point.z() - 1);
    double den = sqrt(plane.x()*plane.x() + plane.y()*plane.y() + plane.z()*plane.z());
    return num/den;
}

/**
 * @brief Convert cv Matric 4 X 4 to Q Matrix 4 X 4
 * 
 * @param x cv Matrix
 * @return Q Matrix
 */
QMatrix4x4 convert2QMat(cv::Mat x){
    QMatrix4x4 out;
    out.setToIdentity();
    if (!x.empty()){
        for(int i=0;i<4;i++){
            out.setRow(i, QVector4D(x.at<float>(i,0), x.at<float>(i,1), x.at<float>(i,2), x.at<float>(i,3)));
        }
    }
    return out;
}

/**
 * @brief fit plane to bunch of points and get error in fit
 * 
 * @param points vector of input points
 * @param maybe_model output model plane equation of best fit 
 * @param error output error of fit
 */
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

/**
 * @brief check if point and plane are close enough
 * 
 * @param error_limit distance should be less than this limit
 * @return true iff close enough point and plane
 */
bool close_enough(QVector3D point, QVector3D plane, double error_limit){
    return dist2plane(point, plane) < error_limit;
}

/**
 * @brief check whether the fit is good enough
 * 
 * @param points points in the fit
 * @param good_num_points threshold limit 
 * 
 * @return true iff number of points > threshold
 */
bool good_enough(QVector<QVector3D> points, int good_num_points){
    return points.size() > good_num_points;
}

/**
 * @brief check if a point is between corners of mouse drag
 * 
 * @param transform world transform of the point
 * @param point point to be checked
 * @param c1 corner 1 of mouse drag
 * @param c2 corner opposite to corner 1 of mouse drag
 * @return true if point really in the mouse drag window
 */
bool between_corners(QMatrix4x4 transform, QVector3D point, QVector3D c1, QVector3D c2){
    QVector4D tp = transform * QVector4D(point, 1.0f);
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

/**
 * @brief Get the point closest to given point on given plane
 * 
 * @param point given point 
 * @param plane given plane
 * 
 * @return point on the plane 
 */
QVector3D pointOnPlane(QVector3D point, QVector3D plane){
    float t = (1 - point.x() + point.y() + point.z())/plane.lengthSquared();
    return point + t*plane;
}

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

/**
 * @brief Load texture from cv image matrix
 * 
 * @param img cv image matrix
 * @param grayscale true iff texture needed is grayscale
 * 
 * @return texture Glint 
 */
GLuint distorted_texture_load(cv::Mat img, bool grayscale){
    GLuint textureTrash;
    cv::flip(img, img, 0);
    glGenTextures(1, &textureTrash);
    glBindTexture(GL_TEXTURE_2D, textureTrash);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    GLint format, internal_format;
    if(grayscale){
        format = internal_format = GL_RED;
    } else {
        cv::cvtColor(img, img, CV_BGR2RGB);
        format = internal_format = GL_RGB;
    }
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 internal_format,            // Internal colour format to convert to
                 img.cols,          // Image width  i.e. 640 for Kinect in standard mode
                 img.rows,          // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 format, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 img.ptr());        // The actual image data itself

    glGenerateMipmap(GL_TEXTURE_2D);
    return textureTrash;
}
