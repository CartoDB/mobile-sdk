/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CONST_H_
#define _CARTO_CONST_H_

namespace carto {

    class Const {
    public:
        // Predefined PI value
        static const double PI;
    
        // Values used for converting between degrees and radians
        static const double DEG_TO_RAD;
        static const double RAD_TO_DEG;

        // Other often-needed constants
        static const double LOGE_2;
        static const double SQRT_2;

        // World size. World bounds are from left -HALF_WORLD_SIZE to right +HALF_WORLD_SIZE
        // and from top +HALF_WORLD_SIZE to bottom -HALF_WORLD_SIZE
        static const int WORLD_SIZE = 1 << 20;
        static const float HALF_WORLD_SIZE;
    
        // Maximum supported zoom level
        enum { MAX_SUPPORTED_ZOOM_LEVEL = 24 };
    
        // If device dpi = UNSCALED_DPI then 1 dp = 1 px
        static const float UNSCALED_DPI;
    
        // Min and max supported tilt angles
        static const float MIN_SUPPORTED_TILT_ANGLE;
        // Maximum building height, taller buildings will get clipped, if the camera
        // gets lower than the acutal building height, then it will get clipped anyway
        // for world map: Const.UNIT_SIZE / 16667. For SF map this looks usable: Const.UNIT_SIZE / 833
        static const float MAX_HEIGHT;
        // Minimum near plane distance, used to avoid visual artifacts on large zoom levels
        static const float MIN_NEAR;
        // Maximum near plane distance
        static const float MAX_NEAR;
    };
    
}

#endif
