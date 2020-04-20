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

        // World size. All internal coordinates are between -0.5 * WORLD_SIZE and 0.5 * WORLD_SIZE.
        static const int WORLD_SIZE;

        // Earth size
        static const double EARTH_RADIUS;
        static const double EARTH_CIRCUMFERENCE;

        // If device dpi = UNSCALED_DPI then 1 dp = 1 px
        static const float UNSCALED_DPI;
    
        // Maximum supported zoom level
        static const int MAX_SUPPORTED_ZOOM_LEVEL;
    
        // Min and max supported tilt angles
        static const float MIN_SUPPORTED_TILT_ANGLE;
        // Minimum height (negative value). This is needed for globe view as the globe is tesselated and surface may be 'below'
        // zero level. Should be approximately -WORLD_SIZE * cos(PI / TESSELATION_LEVEL) + WORLD_SIZE.
        static const float MIN_HEIGHT;
        // Maximum building height, taller buildings will get clipped, if the camera
        // gets lower than the acutal building height, then it will get clipped anyway
        // for world map: Const.UNIT_SIZE / 16667. For SF map this looks usable: Const.UNIT_SIZE / 833
        static const float MAX_HEIGHT;
        // Minimum near plane distance, used to avoid visual artifacts on large zoom levels
        static const float MIN_NEAR;
    };
    
}

#endif
