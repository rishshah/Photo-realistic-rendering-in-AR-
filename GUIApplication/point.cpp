#include "point.h"
Point::Point()
{
    position = color = QVector3D(0,0,0);
}

Point::Point(QVector3D pos, QVector3D col)
{
    position = pos;
    color = col;
}


Point::Point(QVector3D pos)
{
    position = pos;
    color = QVector3D(0,0,0);
}
