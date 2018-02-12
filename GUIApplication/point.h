#ifndef POINT_H
#define POINT_H
#include<QVector3D>
#include<QVector2D>

class Point
{
public:
    QVector3D position;
    QVector3D color;
    QVector2D texture;

    Point();
    Point(QVector3D pos, QVector3D col);
    Point(QVector3D pos, QVector2D tex);
};

#endif // POINT_H
