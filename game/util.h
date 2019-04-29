// Some utility macros/functions
// =---------------------------=

#ifndef UTIL_H
#define UTIL_H

#define AABB_Test(x1, y1, w1, h1, x2, y2, w2, h2) (!(x1 + w1 < x2 || x1 > x2 + w2 || y1 + h1 < y2 || y1 > y2 + h2))

#endif