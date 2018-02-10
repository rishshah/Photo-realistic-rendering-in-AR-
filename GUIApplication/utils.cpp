#include "utils.h"

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

QVector3D pointOnPlane(QVector3D point, QVector3D plane){
    float t = (1 - point.x() + point.y() + point.z())/plane.lengthSquared();
    return point + t*plane;
}

//void LoadTexture(texture_filename, 256, 256){

//}
//    tex = LoadTexture(texture_filename, 256, 256);
