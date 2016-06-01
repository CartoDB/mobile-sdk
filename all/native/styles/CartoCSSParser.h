/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSSPARSER_H_
#define _CARTO_CARTOCSSPARSER_H_

#include "graphics/Color.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

namespace carto {
    class PointStyleBuilder;
    class LineStyleBuilder;
    class PolygonStyleBuilder;
    class MarkerStyleBuilder;

    /**
     * A CartoCSS style parser. Geometry and marker styles can be parsed from simple human-readable format.
     * Only a subset of the full CartoCSS tags are supported. Also, some Nutiteq-specific tags are added not included
     * in original CartoCSS specification.
     * Note: parsers return style builder objects instead of style objects.
     * This allows easy customization (overriding) of some parameters in the code.
     */
    class CartoCSSParser {
    public:
        /**
         * Read point style parameters from CartoCSS description. The following tags are supported:
         * point-size, point-fill, point-opacity, point-file.
         * @param cartoCSS The CartoCSS to parse. For example, "point-size: 20.5; point-fill:#f8f"
         * @return Point style builder object with parameters set from the specified CartoCSS.
         */
        static std::shared_ptr<PointStyleBuilder> CreatePointStyleBuilder(const std::string& cartoCSS);
        /**
         * Read line style parameters from CartoCSS description. The following tags are supported:
         * line-width, line-fill, line-opacity, line-join, line-cap, line-dasharray.
         * @param cartoCSS The CartoCSS to parse. For example, "line-fill:#f0f; line-cap:round, line-dasharray:5,0.5"
         * @return Line style builder object with parameters set from the specified CartoCSS.
         */
        static std::shared_ptr<LineStyleBuilder> CreateLineStyleBuilder(const std::string& cartoCSS);
        /**
         * Read polygon style parameters from CartoCSS description. The following tags are supported:
         * polygon-fill, polygon-opacity, line-* (all line tags).
         * @param cartoCSS The CartoCSS to parse. For example, "polygon-fill:#ff00ff; polygon-opacity:0.3"
         * @return Polygon style builder object with parameters set from the specified CartoCSS.
         */
        static std::shared_ptr<PolygonStyleBuilder> CreatePolygonStyleBuilder(const std::string& cartoCSS);
        /**
         * Read marker style parameters from CartoCSS description. The following tags are supported:
         * marker-width, marker-fill, marker-opacity, marker-file, allow-overlap.
         * @param cartoCSS The CartoCSS to parse. For example, "marker-size:30; marker-file:url('//assets/marker.png')"
         * @return Marker style builder object with parameters set from the specified CartoCSS.
         */
        static std::shared_ptr<MarkerStyleBuilder> CreateMarkerStyleBuilder(const std::string& cartoCSS);

    private:
        struct CartoCSSKeyValues {
            CartoCSSKeyValues(const std::string& method, const std::map<std::string, std::string>& keyValueMap);
            ~CartoCSSKeyValues();
            
            std::string getString(const std::string& key, const std::string& defaultValue) const;
            std::string getFileName(const std::string& key, const std::string& defaultValue) const;
            Color getColor(const std::string& key, const Color& defaultValue) const;
            Color getColor(const std::string& key, const Color& defaultValue, float opacity) const;
            bool getBool(const std::string& key, bool defaultValue) const;
            float getFloat(const std::string& key, float defaultValue) const;

        private:
            const std::string _method;
            std::map<std::string, std::string> _keyValueMap;
            mutable std::set<std::string> _usedKeys;
        };
        
        static std::map<std::string, std::string> ParseCSSKeyValues(const std::string& cartoCSS);

        static std::shared_ptr<PointStyleBuilder> CreatePointStyleBuilder(const CartoCSSKeyValues& keyValues);
        static std::shared_ptr<LineStyleBuilder> CreateLineStyleBuilder(const CartoCSSKeyValues& keyValues);
        static std::shared_ptr<PolygonStyleBuilder> CreatePolygonStyleBuilder(const CartoCSSKeyValues& keyValues);
        static std::shared_ptr<MarkerStyleBuilder> CreateMarkerStyleBuilder(const CartoCSSKeyValues& keyValues);
    };
}

#endif
