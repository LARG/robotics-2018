#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <math/Point.h>
#include <map>
#include <stdint.h>

class Polygon {

    public:
        Polygon();
        Polygon(std::vector<Point>);
        std::vector<Point> enclosedPoints();
        bool enclosesPoint(int,int);
        int left_, right_, top_, bottom_;
        int getLeft();
        int getRight();
        int getTop();
        int getBottom();
        void setBounds();
        std::map<int,bool> pointCache_;

    private:
        int *xcoords_, *ycoords_, count_;
        std::vector<Point> vertices_;
        std::vector<Point> enclosed_;
        void addVertex(int,int);
        bool enclosesPoint(int,int*,int*,int,int);

};
#endif
