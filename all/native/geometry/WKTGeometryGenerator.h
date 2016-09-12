/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKTGEOMETRYGENERATOR_H_
#define _CARTO_WKTGEOMETRYGENERATOR_H_

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

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_alternative.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace carto {
    namespace WKTGeometryGeneratorImpl {
        namespace phx = boost::phoenix;
        namespace karma = boost::spirit::karma;
        namespace encoding = boost::spirit::iso8859_1;

        typedef std::function<std::shared_ptr<Geometry>(bool)> Func;

        template <typename Iterator>
        struct Grammar : karma::grammar<Iterator, std::shared_ptr<Geometry>(bool)> {
            Grammar() : Grammar::base_type(geometry) {
                using karma::_pass;
                using karma::_val;
                using karma::_1;
                using karma::_2;
                using karma::_3;
                using karma::_r1;

                pos = (karma::double_ << ' ' << karma::double_ << -(karma::lit(" ") << karma::double_)) [_pass = phx::bind(&GetMapPos, _val, _r1, _1, _2, _3)];
                ring = pos(_r1) % ',';
                rings = '(' << (ring(_r1) % ',') << ')';
                type = karma::lit(" Z") [_pass = _r1];
                empty = karma::lit(" EMPTY");

                geometry =
                      karma::lit("POINT")           << -type(_r1) << (empty [_pass = phx::bind(&GetPoint, _val, nullptr)]         | ('(' << pos(_r1) << ')')              [_pass = phx::bind(&GetPoint, _val, &_1)])
                    | karma::lit("LINESTRING")      << -type(_r1) << (empty [_pass = phx::bind(&GetLine, _val, nullptr)]          | ('(' << ring(_r1) << ')')             [_pass = phx::bind(&GetLine, _val, &_1)])
                    | karma::lit("POLYGON")         << -type(_r1) << (empty [_pass = phx::bind(&GetPolygon, _val, nullptr)]       | ('(' << rings(_r1) << ')')            [_pass = phx::bind(&GetPolygon, _val, &_1)])
                    | karma::lit("MULTIPOINT")      << -type(_r1) << (empty [_pass = phx::bind(&GetMultiPoint, _val, nullptr)]    | ('(' << (pos(_r1) % ',') << ')')      [_pass = phx::bind(&GetMultiPoint, _val, &_1)])
                    | karma::lit("MULTILINESTRING") << -type(_r1) << (empty [_pass = phx::bind(&GetMultiLine, _val, nullptr)]     | ('(' << (ring(_r1) % ',') << ')')     [_pass = phx::bind(&GetMultiLine, _val, &_1)])
                    | karma::lit("MULTIPOLYGON")    << -type(_r1) << (empty [_pass = phx::bind(&GetMultiPolygon, _val, nullptr)]  | ('(' << (rings(_r1) % ',') << ')')    [_pass = phx::bind(&GetMultiPolygon, _val, &_1)])
                    | karma::lit("GEOMETRYCOLLECTION")            << (empty [_pass = phx::bind(&GetMultiGeometry, _val, nullptr)] | ('(' << (geometry(_r1) % ',') << ')') [_pass = phx::bind(&GetMultiGeometry, _val, &_1)])
                    ;
            }

            karma::rule<Iterator, MapPos(bool)> pos;
            karma::rule<Iterator, std::vector<MapPos>(bool)> ring;
            karma::rule<Iterator, std::vector<std::vector<MapPos> >(bool)> rings;
            karma::rule<Iterator, void(bool)> type;
            karma::rule<Iterator, void()> empty;
            karma::rule<Iterator, std::shared_ptr<Geometry>(bool)> geometry;

        private:
            static bool GetMapPos(const MapPos& mapPos, bool useZ, double& x, double& y, boost::optional<double>& z) {
                x = mapPos.getX();
                y = mapPos.getY();
                if (useZ) {
                    z = mapPos.getZ();
                }
                return true;
            }

            static bool GetPoint(const std::shared_ptr<Geometry>& geometry, MapPos* pos) {
                if (auto point = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
                    if (!pos) {
                        return false;
                    }
                    *pos = point->getPos();
                    return true;
                }
                return false;
            }

            static bool GetLine(const std::shared_ptr<Geometry>& geometry, std::vector<MapPos>* ring) {
                if (auto line = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
                    if (!ring) {
                        return line->getPoses().empty();
                    }
                    *ring = line->getPoses();
                    return true;
                }
                return false;
            }

            static bool GetPolygon(const std::shared_ptr<Geometry>& geometry, std::vector<std::vector<MapPos> >* rings) {
                if (auto polygon = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
                    if (!rings) {
                        return polygon->getRings().empty();
                    }
                    *rings = polygon->getRings();
                    return true;
                }
                return false;
            }

            static bool GetMultiPoint(const std::shared_ptr<Geometry>& geometry, std::vector<MapPos>* posList) {
                if (auto multiPoint = std::dynamic_pointer_cast<MultiPointGeometry>(geometry)) {
                    if (!posList) {
                        return multiPoint->getGeometryCount() == 0;
                    }
                    for (int i = 0; i < multiPoint->getGeometryCount(); i++) {
                        posList->push_back(multiPoint->getGeometry(i)->getPos());
                    }
                    return true;
                }
                return false;
            }

            static bool GetMultiLine(const std::shared_ptr<Geometry>& geometry, std::vector<std::vector<MapPos> >* ringList) {
                if (auto multiLine = std::dynamic_pointer_cast<MultiLineGeometry>(geometry)) {
                    if (!ringList) {
                        return multiLine->getGeometryCount() == 0;
                    }
                    for (int i = 0; i < multiLine->getGeometryCount(); i++) {
                        ringList->push_back(multiLine->getGeometry(i)->getPoses());
                    }
                    return true;
                }
                return false;
            }

            static bool GetMultiPolygon(const std::shared_ptr<Geometry>& geometry, std::vector<std::vector<std::vector<MapPos> > >* ringsList) {
                if (auto multiPolygon = std::dynamic_pointer_cast<MultiPolygonGeometry>(geometry)) {
                    if (!ringsList) {
                        return multiPolygon->getGeometryCount() == 0;
                    }
                    for (int i = 0; i < multiPolygon->getGeometryCount(); i++) {
                        ringsList->push_back(multiPolygon->getGeometry(i)->getRings());
                    }
                    return true;
                }
                return false;
            }

            static bool GetMultiGeometry(const std::shared_ptr<Geometry>& geometry, std::vector<std::shared_ptr<Geometry> >* geometries) {
                if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
                    if (!geometries) {
                        return multiGeometry->getGeometryCount() == 0;
                    }
                    for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                        geometries->push_back(multiGeometry->getGeometry(i));
                    }
                    return true;
                }
                return false;
            }
        };
    }

    template <typename Iterator> using WKTGeometryGenerator = WKTGeometryGeneratorImpl::Grammar<Iterator>;
}

#endif
