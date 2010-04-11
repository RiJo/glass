/*******************************************************************************
 * Description:
 *   A representation of a coordinate.
 ******************************************************************************/

#ifndef _POINT_H_
#define _POINT_H_

#include <stdio.h>

class Point {

public:
    int x;
    int y;

public:
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}

    void reset();
    bool zero();
    bool intersects(Point, Point);
    
};

#endif