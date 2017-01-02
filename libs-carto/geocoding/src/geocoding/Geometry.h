/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_GEOMETRY_H_
#define _CARTO_GEOCODING_GEOMETRY_H_

#include <utility>
#include <algorithm>
#include <vector>
#include <memory>

#include <cglib/vec.h>

namespace carto { namespace geocoding {
	class Geometry {
	public:
		using Point = cglib::vec2<double>;
		
		virtual ~Geometry() = default;

		virtual Point calculateNearestPoint(const Point& p) const = 0;
	};

	class PointGeometry : public Geometry {
	public:
		explicit PointGeometry(const Point& point) : _point(point) { }

		const Point& getPoint() const {
			return _point;
		}

		virtual Point calculateNearestPoint(const Point& p) const override {
			return _point;
		}

	private:
		const Point _point;
	};

	class LineGeometry : public Geometry {
	public:
		explicit LineGeometry(std::vector<Point> points) : _points(std::move(points)) { }

		const std::vector<Point>& getPoints() const {
			return _points;
		}

		virtual Point calculateNearestPoint(const Point& p) const override {
			double minDist = std::numeric_limits<double>::infinity();
			Point nearestPoint = p;
			for (std::size_t i = 1; i < _points.size(); i++) {
				const Point& a = _points[i - 1];
				const Point& b = _points[i];
				Point point = a;
				if (a != b) {
					cglib::vec2<double> dir = b - a;
					double u = cglib::dot_product(p - a, dir) / cglib::dot_product(dir, dir);
					point = a + dir * std::max(0.0, std::min(1.0, u));
				}
				double dist = cglib::length(point - p);
				if (dist < minDist) {
					minDist = dist;
					nearestPoint = point;
				}
			}
			return nearestPoint;
		}

	private:
		const std::vector<Point> _points;
	};

	class PolygonGeometry : public Geometry {
	public:
		explicit PolygonGeometry(std::vector<Point> points, std::vector<std::vector<Point>> holes) : _points(std::move(points)), _holes(std::move(holes)) { }

		const std::vector<Point>& getPoints() const {
			return _points;
		}

		const std::vector<std::vector<Point>>& getHoles() const {
			return _holes;
		}

		virtual Point calculateNearestPoint(const Point& p) const override {
			if (isPointInsideRing(p, _points)) {
				for (const std::vector<Point>& hole : _holes) {
					if (isPointInsideRing(p, hole)) {
						return calculateNearestRingPoint(p, hole);
					}
				}
				return p;
			}
			return calculateNearestRingPoint(p, _points);
		}

	private:
		static Point calculateNearestRingPoint(const Point& p, const std::vector<Point>& points) {
			double minDist = std::numeric_limits<double>::infinity();
			Point nearestPoint = p;
			for (std::size_t i = 0; i < points.size(); i++) {
				const Point& a = points[i];
				const Point& b = points[(i + 1) % points.size()];
				Point point = a;
				if (a != b) {
					cglib::vec2<double> dir = b - a;
					double u = cglib::dot_product(p - a, dir) / cglib::dot_product(dir, dir);
					point = a + dir * std::max(0.0, std::min(1.0, u));
				}
				double dist = cglib::length(point - p);
				if (dist < minDist) {
					minDist = dist;
					nearestPoint = point;
				}
			}
			return nearestPoint;
		}

		static bool isPointInsideRing(const Point& p, const std::vector<Point>& points) {
			bool inside = false;
			for (std::size_t i = 0; i < points.size(); i++) {
				std::size_t j = (i + 1) % points.size();
				if ((points[i](1) >= p(1)) != (points[j](1) >= p(1))) {
					if (p(0) <= (points[j](0) - points[i](0)) * (p(1) - points[i](1)) / (points[j](1) - points[i](1)) + points[i](0)) {
						inside = !inside;
					}
				}
			}
			return inside;
		}

		const std::vector<Point> _points;
		const std::vector<std::vector<Point>> _holes;
	};

	class MultiGeometry : public Geometry {
	public:
		explicit MultiGeometry(std::vector<std::shared_ptr<Geometry>> geoms) : _geometries(std::move(geoms)) { }

		const std::vector<std::shared_ptr<Geometry>>& getGeometries() const {
			return _geometries;
		}

		virtual Point calculateNearestPoint(const Point& p) const override {
			double minDist = std::numeric_limits<double>::infinity();
			Point nearestPoint = p;
			for (const std::shared_ptr<Geometry>& geom : _geometries) {
				Point point = geom->calculateNearestPoint(p);
				double dist = cglib::length(point - p);
				if (dist < minDist) {
					minDist = dist;
					nearestPoint = point;
				}
			}
			return nearestPoint;
		}

	private:
		const std::vector<std::shared_ptr<Geometry>> _geometries;
	};
} }

#endif
