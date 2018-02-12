#include "point.h"
Point::Point()
{
    position = color = QVector3D(0,0,0);
    texture = QVector2D(0,0);
}

Point::Point(QVector3D pos, QVector3D col)
{
    position = pos;
    color = col;
    texture = QVector2D(0,0);
}


Point::Point(QVector3D pos, QVector2D tex)
{
    position = pos;
    texture = tex;
    color = QVector3D(1,1,1);
}
