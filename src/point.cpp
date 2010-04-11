#include "point.h"

void Point::reset() {
    x = 0;
    y = 0;
}

bool Point::zero() {
    return (x == 0 && y == 0);
}

bool Point::intersects(Point top_left, Point bottom_right) {
    return (
        x >= top_left.x &&
        y >= top_left.y &&
        x <= bottom_right.x &&
        y <= bottom_right.y
    );
}