/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKTGEOMETRYPARSER_H_
#define _CARTO_WKTGEOMETRYPARSER_H_

#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "utils/Log.h"

#include <memory>
#include <functional>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace carto {
    namespace WKTGeometryParserImpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace repo = boost::spirit::repository::qi;
        namespace encoding = boost::spirit::iso8859_1;

        template <typename Iterator>
        struct Grammar : qi::grammar<Iterator, std::shared_ptr<Geometry>(), encoding::space_type> {
            Grammar() : Grammar::base_type(geometry) {
                using qi::_val;
                using qi::_1;
                using qi::_2;
                using qi::_3;

                point_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["point"]];
                linestring_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["linestring"]];
                polygon_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["polygon"]];
                multipoint_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["multipoint"]];
                multilinestring_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["multilinestring"]];
                multipolygon_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["multipolygon"]];
                geometrycollection_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["geometrycollection"]];
                z_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["z"]];
                zm_kw = repo::distinct(qi::char_("a-zA-Z0-9_"))[qi::no_case["zm"]];
                empty_kw = repo::distinct(qi::char_("a-zA-Z0-9_")) [qi::no_case["empty"]];

                pos = (qi::double_ >> qi::double_ >> -qi::double_ >> -qi::double_) [_val = phx::bind(&MakeMapPos, _1, _2, _3)];
                ring = (pos | '(' >> pos >> ')') % ',';
                rings = ('(' >> ring >> ')') % ',';
                type = z_kw | zm_kw;

                geometry =
                      point_kw           >> -type >> (empty_kw [_val = phx::bind(&MakePoint, nullptr)]         | ('(' >> pos >> ')')              [_val = phx::bind(&MakePoint, &_1)])
                    | linestring_kw      >> -type >> (empty_kw [_val = phx::bind(&MakeLine, nullptr)]          | ('(' >> ring >> ')')             [_val = phx::bind(&MakeLine, &_1)])
                    | polygon_kw         >> -type >> (empty_kw [_val = phx::bind(&MakePolygon, nullptr)]       | ('(' >> rings >> ')')            [_val = phx::bind(&MakePolygon, &_1)])
                    | multipoint_kw      >> -type >> (empty_kw [_val = phx::bind(&MakeMultiPoint, nullptr)]    | ('(' >> (pos % ',') >> ')')      [_val = phx::bind(&MakeMultiPoint, &_1)])
                    | multilinestring_kw >> -type >> (empty_kw [_val = phx::bind(&MakeMultiLine, nullptr)]     | ('(' >> (ring % ',') >> ')')     [_val = phx::bind(&MakeMultiLine, &_1)])
                    | multipolygon_kw    >> -type >> (empty_kw [_val = phx::bind(&MakeMultiPolygon, nullptr)]  | ('(' >> (rings % ',') >> ')')    [_val = phx::bind(&MakeMultiPolygon, &_1)])
                    | geometrycollection_kw       >> (empty_kw [_val = phx::bind(&MakeMultiGeometry, nullptr)] | ('(' >> (geometry % ',') >> ')') [_val = phx::bind(&MakeMultiGeometry, &_1)])
                    ;
            }

            qi::rule<Iterator, qi::unused_type()> point_kw, linestring_kw, polygon_kw, multipoint_kw, multilinestring_kw, multipolygon_kw, geometrycollection_kw, z_kw, zm_kw, empty_kw, type;
            qi::rule<Iterator, MapPos(), encoding::space_type> pos;
            qi::rule<Iterator, std::vector<MapPos>(), encoding::space_type> ring;
            qi::rule<Iterator, std::vector<std::vector<MapPos> >(), encoding::space_type> rings;
            qi::rule<Iterator, std::shared_ptr<Geometry>(), encoding::space_type> geometry;

        private:
            static MapPos MakeMapPos(double x, double y, boost::optional<double> z) {
                return MapPos(x, y, z ? *z : 0);
            }

            static std::shared_ptr<PointGeometry> MakePoint(const MapPos* pos) {
                if (!pos) {
                    Log::Error("WKTGeometryParser: Empty point not supported, using POINT(0, 0)");
                    return std::make_shared<PointGeometry>(MapPos(0, 0));
                }
                return std::make_shared<PointGeometry>(*pos);
            }

            static std::shared_ptr<LineGeometry> MakeLine(const std::vector<MapPos>* ring) {
                if (!ring) {
                    return std::make_shared<LineGeometry>(std::vector<MapPos>());
                }
                return std::make_shared<LineGeometry>(*ring);
            }

            static std::shared_ptr<PolygonGeometry> MakePolygon(const std::vector<std::vector<MapPos> >* rings) {
                if (!rings) {
                    return std::make_shared<PolygonGeometry>(std::vector<std::vector<MapPos> >());
                }
                return std::make_shared<PolygonGeometry>(*rings);
            }

            static std::shared_ptr<MultiPointGeometry> MakeMultiPoint(const std::vector<MapPos>* posList) {
                std::vector<std::shared_ptr<PointGeometry> > points;
                if (posList) {
                    std::transform(posList->begin(), posList->end(), std::back_inserter(points), [](const MapPos& pos) { return std::make_shared<PointGeometry>(pos); });
                }
                return std::make_shared<MultiPointGeometry>(points);
            }

            static std::shared_ptr<MultiLineGeometry> MakeMultiLine(const std::vector<std::vector<MapPos> >* ringList) {
                std::vector<std::shared_ptr<LineGeometry> > lines;
                if (ringList) {
                    std::transform(ringList->begin(), ringList->end(), std::back_inserter(lines), [](const std::vector<MapPos>& ring) { return std::make_shared<LineGeometry>(ring); });
                }
                return std::make_shared<MultiLineGeometry>(lines);
            }

            static std::shared_ptr<MultiPolygonGeometry> MakeMultiPolygon(const std::vector<std::vector<std::vector<MapPos> > >* ringsList) {
                std::vector<std::shared_ptr<PolygonGeometry> > polygons;
                if (ringsList) {
                    std::transform(ringsList->begin(), ringsList->end(), std::back_inserter(polygons), [](const std::vector<std::vector<MapPos> >& rings) { return std::make_shared<PolygonGeometry>(rings); });
                }
                return std::make_shared<MultiPolygonGeometry>(polygons);
            }

            static std::shared_ptr<Geometry> MakeMultiGeometry(const std::vector<std::shared_ptr<Geometry> >* geometries) {
                if (!geometries) {
                    return std::make_shared<MultiGeometry>(std::vector<std::shared_ptr<Geometry> >());
                }
                return std::make_shared<MultiGeometry>(*geometries);
            }
        };
    }

    template <typename Iterator> using WKTGeometryParser = WKTGeometryParserImpl::Grammar<Iterator>;
}

#endif
