#ifndef POINT_H
#define POINT_H
#include<QVector3D>

class Point
{
public:
    QVector3D position;
    QVector3D color;

    Point();
    Point(QVector3D pos, QVector3D col);
    Point(QVector3D pos);
};

#endif // POINT_H
