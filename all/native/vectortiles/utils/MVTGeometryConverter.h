/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MVTGEOMETRYCONVERTER_H_
#define _CARTO_MVTGEOMETRYCONVERTER_H_

#include "core/MapPos.h"
#include "core/MapBounds.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiPolygonGeometry.h"

#include <mapnikvt/Geometry.h>

#include <cglib/vec.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

namespace carto {

    struct MVTGeometryConverter {
        typedef std::function<MapPos(const cglib::vec2<float>& pos)> PointConversionFunc;

        explicit MVTGeometryConverter(const PointConversionFunc& convertFn) : _convertFunc(convertFn) { }

        explicit MVTGeometryConverter(const MapBounds& tileBounds) : _convertFunc([tileBounds](const cglib::vec2<float>& pos) {
            return MapPos(tileBounds.getMin().getX() + pos(0) * tileBounds.getDelta().getX(), tileBounds.getMax().getY() - pos(1) * tileBounds.getDelta().getY(), 0);
        }) { }

        std::shared_ptr<Geometry> operator() (const mvt::PointGeometry& mvtPoint) const {
            std::vector<MapPos> poses = convertPoints(mvtPoint.getVertices());
            std::vector<std::shared_ptr<PointGeometry> > points;
            points.reserve(poses.size());
            std::transform(poses.begin(), poses.end(), std::back_inserter(points), [](const MapPos& pos) {
                return std::make_shared<PointGeometry>(pos);
            });
            if (points.size() == 1) {
                return points.front();
            } else {
                return std::make_shared<MultiPointGeometry>(points);
            }
        }

        std::shared_ptr<Geometry> operator() (const mvt::LineGeometry& mvtLine) const {
            std::vector<std::vector<MapPos>> posesList = convertPointsList(mvtLine.getVerticesList());
            std::vector<std::shared_ptr<LineGeometry> > lines;
            lines.reserve(posesList.size());
            std::transform(posesList.begin(), posesList.end(), std::back_inserter(lines), [](const std::vector<MapPos>& poses) {
                return std::make_shared<LineGeometry>(poses);
            });
            if (lines.size() == 1) {
                return lines.front();
            } else {
                return std::make_shared<MultiLineGeometry>(lines);
            }
        }

        std::shared_ptr<Geometry> operator() (const mvt::PolygonGeometry& mvtPolygon) const {
            std::vector<std::vector<std::vector<MapPos> > > posesLists = convertPointsLists(mvtPolygon.getPolygonList());
            std::vector<std::shared_ptr<PolygonGeometry> > polygons;
            polygons.reserve(posesLists.size());
            std::transform(posesLists.begin(), posesLists.end(), std::back_inserter(polygons), [](const std::vector<std::vector<MapPos> >& posesList) {
                return std::make_shared<PolygonGeometry>(posesList);
            });
            if (polygons.size() == 1) {
                return polygons.front();
            } else {
                return std::make_shared<MultiPolygonGeometry>(polygons);
            }
        }

    private:
        PointConversionFunc _convertFunc;

        std::vector<MapPos> convertPoints(const std::vector<cglib::vec2<float> >& poses) const {
            std::vector<MapPos> points;
            points.reserve(poses.size());
            std::transform(poses.begin(), poses.end(), std::back_inserter(points), _convertFunc);
            return points;
        }

        std::vector<std::vector<MapPos> > convertPointsList(const std::vector<std::vector<cglib::vec2<float> > >& posesList) const {
            std::vector<std::vector<MapPos> > pointsList;
            pointsList.reserve(posesList.size());
            std::transform(posesList.begin(), posesList.end(), std::back_inserter(pointsList), [this](const std::vector<cglib::vec2<float> >& poses) {
                return convertPoints(poses);
            });
            return pointsList;
        }

        std::vector<std::vector<std::vector<MapPos> > > convertPointsLists(const std::vector<std::vector<std::vector<cglib::vec2<float> > > >& posesLists) const {
            std::vector<std::vector<std::vector<MapPos> > > pointsLists;
            pointsLists.reserve(posesLists.size());
            std::transform(posesLists.begin(), posesLists.end(), std::back_inserter(pointsLists), [this](const std::vector<std::vector<cglib::vec2<float> > >& posesList) {
                return convertPointsList(posesList);
            });
            return pointsLists;
        }
    };

}

#endif
