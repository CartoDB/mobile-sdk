/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPFILTERTABLE_H_
#define _CARTO_BITMAPFILTERTABLE_H_

#include "graphics/Bitmap.h"
#include "utils/Log.h"

#include <vector>

#include <cglib/bbox.h>

namespace carto {

    /**
     * Filter table using Elliptic Weighted Average filtering.
     */
    class BitmapFilterTable {
    public:
        struct Sample {
            short u;
            short v;
            float weight;
        };

        BitmapFilterTable(int minU, int minV, int maxU, int maxV) : _minU(minU), _minV(minV), _maxU(maxU), _maxV(maxV), _sampleCounts(), _samples() { }

        const std::vector<int>& getSampleCounts() const { return _sampleCounts; }
        const std::vector<Sample>& getSamples() const { return _samples; }
        
        template <typename Transform>
        void calculateFilterTable(const Transform& transform, int sizeX, int sizeY, float filterScale, int maxFilterWidth);

        template <typename Transform>
        static bool calculateFilterBounds(const Transform& transform, int sizeX, int sizeY, int sizeU, int sizeV, int& minU, int& minV, int& maxU, int& maxV, int maxFilterWidth);

    private:
        void calculatePixelSamples(int ui, int vi, float uf, float vf, int du, int dv, float a, float b, float c) {
            std::size_t sampleIndex = _samples.size();
            float samplesWeight = 0;

            // Filter using EWA
            float ddq = 2 * a;
            for (int v0 = -dv; v0 <= dv; v0++) {
                float uu = -du - uf + ui;
                float vv = v0 - vf + vi;
                float dq = a * (2 * uu + 1) + b*vv;
                float q = c*vv*vv + b*uu*vv + a*uu*uu;
                for (int u0 = -du; u0 <= du; u0++) {
                    if (q < _GaussTableSize) {
                        float qf = std::max(q, 0.0f);
                        int qi = static_cast<int>(qf);
                        float f0 = _GaussTable[qi + 0];
                        float f1 = _GaussTable[qi + 1];
                        float weight = f0 + (f1 - f0) * (qf - qi);
                        addSample(ui + u0, vi + v0, weight);
                        samplesWeight += weight;
                    }
                    q += dq;
                    dq += ddq;
                }
            }

            // Fallback to bilinear sampling, if needed
            if (samplesWeight == 0) {
                addSample(ui + 0, vi + 0, (1 - uf) * (1 - vf));
                addSample(ui + 1, vi + 0, uf * (1 - vf));
                addSample(ui + 0, vi + 1, (1 - uf) * vf);
                addSample(ui + 1, vi + 1, uf * vf);
            } else {
                float invSamplesWeight = 1.0f / samplesWeight;
                while (sampleIndex < _samples.size()) {
                    _samples[sampleIndex++].weight *= invSamplesWeight;
                }
            }
        }

        void addSample(int u, int v, float weight) {
            if (u >= _minU && v >= _minV && u < _maxU && v < _maxV) {
                _samples.emplace_back(Sample());
                Sample& sample = _samples.back();
                sample.u = static_cast<short>(u - _minU);
                sample.v = static_cast<short>(v - _minV);
                sample.weight = weight;
            }
        }

        int _minU, _minV;
        int _maxU, _maxV;
        std::vector<int> _sampleCounts;
        std::vector<Sample> _samples;

        static const int _GaussTableSize;
        static const float _GaussTable[];
    };

    template <typename Transform>
    void BitmapFilterTable::calculateFilterTable(const Transform& transform, int sizeX, int sizeY, float filterScale, int maxFilterWidth) {
        static const float epsilon = 1.0e-6f;

        // Calculate "local affine approximation" gradients at the center of destination
        // This is a good fit is the transform is affine (or close to affine), otherwise such calculations need to be performed for each pixel
        cglib::vec2<double> uv0 = transform(sizeX / 2 + 0, sizeY / 2 + 0);
        cglib::vec2<double> uvx = transform(sizeX / 2 + 1, sizeY / 2 + 0) - uv0;
        cglib::vec2<double> uvy = transform(sizeX / 2 + 0, sizeY / 2 + 1) - uv0;
        float ux = static_cast<float>(uvx(0));
        float vx = static_cast<float>(uvx(1));
        float uy = static_cast<float>(uvy(0));
        float vy = static_cast<float>(uvy(1));

        // Calculate bounding ellipse implicit form parameters
        float a = vx*vx + vy*vy + 1;
        float b = -2 * (ux*vx + uy*vy);
        float c = ux*ux + uy*uy + 1;
        float f = a*c - b*b*0.25f + epsilon;

        // Prescale so that f = _filter_table_size
        float s = _GaussTableSize * filterScale / f;
        a *= s; b *= s; c *= s; f *= s;

        // Find bounding box dimensions
        int du = std::min(maxFilterWidth, static_cast<int>(std::abs(ux) + std::abs(uy) + 1));
        int dv = std::min(maxFilterWidth, static_cast<int>(std::abs(vx) + std::abs(vy) + 1));

        // Reserve memory for samples
        _sampleCounts.reserve(sizeX * sizeY);
        _samples.reserve(sizeX * sizeY * std::min(du * dv * 4, 16));

        // Calculate all samples
        for (int y = 0; y < sizeY; y++) {
            for (int x = 0; x < sizeX; x++) {
                std::size_t sampleIndex = _samples.size();

                // Calculate samples
                cglib::vec2<double> uv = transform(x, y);
                float uf = static_cast<float>(uv(0));
                float vf = static_cast<float>(uv(1));
                int ui = static_cast<int>(std::floor(uf));
                int vi = static_cast<int>(std::floor(vf));
                if (ui + du >= _minU && vi + dv >= _minV && ui - du < _maxU && vi - dv < _maxV) {
                    calculatePixelSamples(ui, vi, uf, vf, du, dv, a, b, c);
                }

                // Add sample count
                _sampleCounts.push_back(static_cast<int>(_samples.size() - sampleIndex));
            }
        }
    }

    template <typename Transform>
    bool BitmapFilterTable::calculateFilterBounds(const Transform& transform, int sizeX, int sizeY, int sizeU, int sizeV, int& minU, int& minV, int& maxU, int& maxV, int maxFilterWidth) {
        // Calculate "local affine approximation" gradients at the center of destination
        cglib::vec2<double> uv0 = transform(sizeX / 2 + 0, sizeY / 2 + 0);
        cglib::vec2<double> uvx = transform(sizeX / 2 + 1, sizeY / 2 + 0) - uv0;
        cglib::vec2<double> uvy = transform(sizeX / 2 + 0, sizeY / 2 + 1) - uv0;
        float ux = static_cast<float>(uvx(0));
        float vx = static_cast<float>(uvx(1));
        float uy = static_cast<float>(uvy(0));
        float vy = static_cast<float>(uvy(1));

        // Find bounding box dimensions
        int du = std::min(maxFilterWidth, static_cast<int>(std::abs(ux) + std::abs(uy) + 1));
        int dv = std::min(maxFilterWidth, static_cast<int>(std::abs(vx) + std::abs(vy) + 1));

        // Find tile area in raster space. Use NxN grid, as the transform can be non-affine.
        cglib::bbox2<double> boundsUV = cglib::bbox2<double>::smallest();
        for (int y = 0; y <= 4; y++) {
            for (int x = 0; x <= 4; x++) {
                cglib::vec2<double> uv = transform(x * sizeX / 4, y * sizeY / 4);
                boundsUV.add(uv);
            }
        }

        // Calculate bounds with filter padding
        double minUf = std::floor(boundsUV.min(0)) - du;
        double minVf = std::floor(boundsUV.min(1)) - dv;
        double maxUf = std::ceil(boundsUV.max(0))  + du;
        double maxVf = std::ceil(boundsUV.max(1))  + dv;

        // Return true if bounds is inside source
        if (minUf < sizeU && minVf < sizeV && maxUf >= 0 && maxVf >= 0) {
            minU = static_cast<int>(minUf);
            minV = static_cast<int>(minVf);
            maxU = static_cast<int>(maxUf);
            maxV = static_cast<int>(maxVf);
            return true;
        }
        return false;
    }

}

#endif
