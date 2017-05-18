/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_PROJUTILS_H_
#define _CARTO_GEOCODING_PROJUTILS_H_

#include <cglib/vec.h>

namespace carto { namespace geocoding {
    namespace consts {
        static constexpr double PI = 3.1415926535897932;
        static constexpr double DEG_TO_RAD = 0.017453292519943295;
        static constexpr double RAD_TO_DEG = 57.295779513082323;
        static constexpr double EARTH_RADIUS = 6378137;
        static constexpr double METERS_PER_LAT_DEGREE = 110567;
    }

    cglib::vec2<double> webMercatorToWgs84(const cglib::vec2<double>& mercatorPoint);
    cglib::vec2<double> wgs84ToWebMercator(const cglib::vec2<double>& wgsPoint);

    cglib::vec2<double> wgs84Meters(const cglib::vec2<double>& wgsPoint);
    cglib::vec2<double> webMercatorMeters(const cglib::vec2<double>& wgsPoint);
} }

#endif
