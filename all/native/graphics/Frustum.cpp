#include "Frustum.h"
#include "core/MapPos.h"
#include "core/MapBounds.h"
#include "utils/Log.h"

#include <cmath>

namespace carto {
    
    Frustum::Frustum() :
        _planes()
    {
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 4; j++) {
                _planes[i][j] = 0;
            }
        }
    }
    
    Frustum::Frustum(const cglib::mat4x4<double>& mvpMatrix) :
        _planes()
    {
        // Right
        _planes[0][0] = mvpMatrix(3, 0) - mvpMatrix(0, 0);
        _planes[0][1] = mvpMatrix(3, 1) - mvpMatrix(0, 1);
        _planes[0][2] = mvpMatrix(3, 2) - mvpMatrix(0, 2);
        _planes[0][3] = mvpMatrix(3, 3) - mvpMatrix(0, 3);
        // Left
        _planes[1][0] = mvpMatrix(3, 0) + mvpMatrix(0, 0);
        _planes[1][1] = mvpMatrix(3, 1) + mvpMatrix(0, 1);
        _planes[1][2] = mvpMatrix(3, 2) + mvpMatrix(0, 2);
        _planes[1][3] = mvpMatrix(3, 3) + mvpMatrix(0, 3);
        // Bottom
        _planes[2][0] = mvpMatrix(3, 0) + mvpMatrix(1, 0);
        _planes[2][1] = mvpMatrix(3, 1) + mvpMatrix(1, 1);
        _planes[2][2] = mvpMatrix(3, 2) + mvpMatrix(1, 2);
        _planes[2][3] = mvpMatrix(3, 3) + mvpMatrix(1, 3);
        // Top
        _planes[3][0] = mvpMatrix(3, 0) - mvpMatrix(1, 0);
        _planes[3][1] = mvpMatrix(3, 1) - mvpMatrix(1, 1);
        _planes[3][2] = mvpMatrix(3, 2) - mvpMatrix(1, 2);
        _planes[3][3] = mvpMatrix(3, 3) - mvpMatrix(1, 3);
        // Far
        _planes[4][0] = mvpMatrix(3, 0) - mvpMatrix(2, 0);
        _planes[4][1] = mvpMatrix(3, 1) - mvpMatrix(2, 1);
        _planes[4][2] = mvpMatrix(3, 2) - mvpMatrix(2, 2);
        _planes[4][3] = mvpMatrix(3, 3) - mvpMatrix(2, 3);
        // Near
        _planes[5][0] = mvpMatrix(3, 0) + mvpMatrix(2, 0);
        _planes[5][1] = mvpMatrix(3, 1) + mvpMatrix(2, 1);
        _planes[5][2] = mvpMatrix(3, 2) + mvpMatrix(2, 2);
        _planes[5][3] = mvpMatrix(3, 3) + mvpMatrix(2, 3);
        
        normalize();
        
    }
    
    bool Frustum::pointInside(const MapPos& point) const {
        for (int p = 0; p < 6; p++) {
            if (_planes[p][0] * point.getX() + _planes[p][1] * point.getY() + _planes[p][2] * point.getZ()
                + _planes[p][3] <= 0) {
                return false;
            }
        }
        return true;
    }
    
    bool Frustum::circleIntersects(const MapPos& center, double radius) const {
        for (int p = 0; p < 6; p++) {
            double d = _planes[p][0] * center.getX() + _planes[p][1] * center.getY() + _planes[p][3];
            if (d <= -radius) {
                return false;
            }
        }
        return true;
    }
    
    bool Frustum::sphereIntersects(const MapPos& center, double radius) const {
        for (int p = 0; p < 6; p++) {
            double d = _planes[p][0] * center.getX() + _planes[p][1] * center.getY() + _planes[p][2] * center.getZ() + _planes[p][3];
            if (d <= -radius) {
                return false;
            }
        }
        return true;
    }
    
    bool Frustum::squareIntersects(const MapBounds& square) const {
        int c;
        for (int p = 0; p < 6; p++) {
            c = 0;
            if (_planes[p][0] * square.getMin().getX() + _planes[p][1] * square.getMin().getY() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * square.getMax().getX() + _planes[p][1] * square.getMin().getY() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * square.getMin().getX() + _planes[p][1] * square.getMax().getY() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * square.getMax().getX() + _planes[p][1] * square.getMax().getY() + _planes[p][3] > 0) {
                c++;
            }
            if (c == 0) {
                return false;
            }
        }
        return true;
    }
    
    bool Frustum::cuboidIntersects(const MapBounds& cuboid) const {
        int c;
        for (int p = 0; p < 6; p++) {
            c = 0;
            if (_planes[p][0] * cuboid.getMin().getX() + _planes[p][1] * cuboid.getMin().getY() + _planes[p][2] * cuboid.getMin().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMax().getX() + _planes[p][1] * cuboid.getMin().getY() + _planes[p][2] * cuboid.getMin().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMin().getX() + _planes[p][1] * cuboid.getMax().getY() + _planes[p][2] * cuboid.getMin().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMax().getX() + _planes[p][1] * cuboid.getMax().getY() + _planes[p][2] * cuboid.getMin().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMin().getX() + _planes[p][1] * cuboid.getMin().getY() + _planes[p][2] * cuboid.getMax().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMax().getX() + _planes[p][1] * cuboid.getMin().getY() + _planes[p][2] * cuboid.getMax().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMin().getX() + _planes[p][1] * cuboid.getMax().getY() + _planes[p][2] * cuboid.getMax().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (_planes[p][0] * cuboid.getMax().getX() + _planes[p][1] * cuboid.getMax().getY() + _planes[p][2] * cuboid.getMax().getZ() + _planes[p][3] > 0) {
                c++;
            }
            if (c == 0) {
                return false;
            }
        }
        return true;
    }
    
    void Frustum::normalize() {
        double t = 1.0 / std::sqrt((long double) (_planes[0][0] * _planes[0][0] + _planes[0][1] * _planes[0][1] + _planes[0][2] * _planes[0][2]));
        _planes[0][0] *= t;
        _planes[0][1] *= t;
        _planes[0][2] *= t;
        _planes[0][3] *= t;
        
        t = 1.0 / std::sqrt((long double) (_planes[1][0] * _planes[1][0] + _planes[1][1] * _planes[1][1] + _planes[1][2] * _planes[1][2]));
        _planes[1][0] *= t;
        _planes[1][1] *= t;
        _planes[1][2] *= t;
        _planes[1][3] *= t;
        
        t = 1.0 / std::sqrt((long double) (_planes[2][0] * _planes[2][0] + _planes[2][1] * _planes[2][1] + _planes[2][2] * _planes[2][2]));
        _planes[2][0] *= t;
        _planes[2][1] *= t;
        _planes[2][2] *= t;
        _planes[2][3] *= t;
        
        t = 1.0 / std::sqrt((long double) (_planes[3][0] * _planes[3][0] + _planes[3][1] * _planes[3][1] + _planes[3][2] * _planes[3][2]));
        _planes[3][0] *= t;
        _planes[3][1] *= t;
        _planes[3][2] *= t;
        _planes[3][3] *= t;
        
        t = 1.0 / std::sqrt((long double) (_planes[4][0] * _planes[4][0] + _planes[4][1] * _planes[4][1] + _planes[4][2] * _planes[4][2]));
        _planes[4][0] *= t;
        _planes[4][1] *= t;
        _planes[4][2] *= t;
        _planes[4][3] *= t;
        
        t = 1.0 / std::sqrt((long double) (_planes[5][0] * _planes[5][0] + _planes[5][1] * _planes[5][1] + _planes[5][2] * _planes[5][2]));
        _planes[5][0] *= t;
        _planes[5][1] *= t;
        _planes[5][2] *= t;
        _planes[5][3] *= t;
    }
}
