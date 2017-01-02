#include "ProjUtils.h"

#include <cmath>

namespace carto { namespace geocoding {
	cglib::vec2<double> webMercatorToWgs84(const cglib::vec2<double>& mercatorPoint) {
		double x = mercatorPoint(0);
		double y = mercatorPoint(1);
		double num3 = x / consts::EARTH_RADIUS;
		double num4 = num3 * consts::RAD_TO_DEG;
		double num5 = std::floor((num4 + 180.0) / 360.0);
		double num6 = num4 - (num5 * 360.0);
		double num7 = 0.5 * consts::PI - (2.0 * std::atan(std::exp(-y / consts::EARTH_RADIUS)));
		return cglib::vec2<double>(num6, num7 * consts::RAD_TO_DEG);
	}

	cglib::vec2<double> wgs84ToWebMercator(const cglib::vec2<double>& wgsPoint) {
		double num = wgsPoint(0) * consts::DEG_TO_RAD;
		double x = consts::EARTH_RADIUS * num;
		double a = wgsPoint(1) * consts::DEG_TO_RAD;
		double y = 0.5 * consts::EARTH_RADIUS * std::log((1.0 + std::sin(a)) / (1.0 - std::sin(a)));
		return cglib::vec2<double>(x, y);
	}

	cglib::vec2<double> wgs84Meters(const cglib::vec2<double>& wgsPoint) {
		double lngMeters = consts::METERS_PER_LAT_DEGREE;
		double latMeters = consts::METERS_PER_LAT_DEGREE * std::cos(wgsPoint(1) * consts::DEG_TO_RAD);
		return cglib::vec2<double>(latMeters, lngMeters);
	}

	cglib::vec2<double> webMercatorMeters(const cglib::vec2<double>& wgsPoint) {
		double meters = std::cos(wgsPoint(1) * consts::DEG_TO_RAD);
		return cglib::vec2<double>(meters, meters);
	}
} }
