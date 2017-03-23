/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRYCONVERTER_H_
#define _CARTO_GEOMETRYCONVERTER_H_

#include "core/MapPos.h"

#include <mapnikvt/Geometry.h>

#include <cglib/vec.h>

#include <functional>

namespace carto {

    typedef std::function<MapPos(const cglib::vec2<float>& pos)> PointConversionFunction;

    inline std::vector<MapPos> convertPoints(const PointConversionFunction& convertFn, const std::vector<cglib::vec2<float> >& poses) {
        std::vector<MapPos> points;
        points.reserve(poses.size());
        std::transform(poses.begin(), poses.end(), std::back_inserter(points), convertFn);
        return points;
    }

    inline std::vector<std::vector<MapPos> > convertPointsList(const PointConversionFunction& convertFn, const std::vector<std::vector<cglib::vec2<float> > >& posesList) {
        std::vector<std::vector<MapPos> > pointsList;
        pointsList.reserve(posesList.size());
        std::transform(posesList.begin(), posesList.end(), std::back_inserter(pointsList), [&](const std::vector<cglib::vec2<float> >& poses) {
            return convertPoints(convertFn, poses);
        });
        return pointsList;
    }

    inline std::vector<std::vector<std::vector<MapPos> > > convertPointsLists(const PointConversionFunction& convertFn, const std::vector<std::vector<std::vector<cglib::vec2<float> > > >& posesLists) {
        std::vector<std::vector<std::vector<MapPos> > > pointsLists;
        pointsLists.reserve(posesLists.size());
        std::transform(posesLists.begin(), posesLists.end(), std::back_inserter(pointsLists), [&](const std::vector<std::vector<cglib::vec2<float> > >& posesList) {
            return convertPointsList(convertFn, posesList);
        });
        return pointsLists;
    }

    inline std::shared_ptr<Geometry> convertGeometry(const PointConversionFunction& convertFn, const std::shared_ptr<const mvt::Geometry>& mvtGeometry) {
        if (auto mvtPoint = std::dynamic_pointer_cast<const mvt::PointGeometry>(mvtGeometry)) {
            std::vector<MapPos> poses = convertPoints(convertFn, mvtPoint->getVertices());
            std::vector<std::shared_ptr<PointGeometry> > points;
            points.reserve(poses.size());
            std::transform(poses.begin(), poses.end(), std::back_inserter(points), [](const MapPos& pos) { return std::make_shared<PointGeometry>(pos); });
            if (points.size() == 1) {
                return points.front();
            }
            else {
                return std::make_shared<MultiPointGeometry>(points);
            }
        }
        else if (auto mvtLine = std::dynamic_pointer_cast<const mvt::LineGeometry>(mvtGeometry)) {
            std::vector<std::vector<MapPos>> posesList = convertPointsList(convertFn, mvtLine->getVerticesList());
            std::vector<std::shared_ptr<LineGeometry> > lines;
            lines.reserve(posesList.size());
            std::transform(posesList.begin(), posesList.end(), std::back_inserter(lines), [](const std::vector<MapPos>& poses) { return std::make_shared<LineGeometry>(poses); });
            if (lines.size() == 1) {
                return lines.front();
            }
            else {
                return std::make_shared<MultiLineGeometry>(lines);
            }
        }
        else if (auto mvtPolygon = std::dynamic_pointer_cast<const mvt::PolygonGeometry>(mvtGeometry)) {
            std::vector<std::vector<std::vector<MapPos> > > posesLists = convertPointsLists(convertFn, mvtPolygon->getPolygonList());
            std::vector<std::shared_ptr<PolygonGeometry> > polygons;
            polygons.reserve(posesLists.size());
            std::transform(posesLists.begin(), posesLists.end(), std::back_inserter(polygons), [](const std::vector<std::vector<MapPos> >& posesList) { return std::make_shared<PolygonGeometry>(posesList); });
            if (polygons.size() == 1) {
                return polygons.front();
            }
            else {
                return std::make_shared<MultiPolygonGeometry>(polygons);
            }
        }
        return std::shared_ptr<Geometry>();
    }

}

#endif
