/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILELABELCULLER_H_
#define _CARTO_VT_TILELABELCULLER_H_

#include "ViewState.h"
#include "TileLabel.h"

#include <array>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/bbox.h>

namespace carto { namespace vt {
    class TileLabelCuller {
    public:
        explicit TileLabelCuller(std::shared_ptr<std::mutex> mutex, float scale);

        void setViewState(const cglib::mat4x4<double>& projectionMatrix, const cglib::mat4x4<double>& cameraMatrix, float zoom, float aspectRatio, float resolution);
        void process(const std::vector<std::shared_ptr<TileLabel>>& labelList);

    private:
        constexpr static int GRID_RESOLUTION = 16;

        struct Record {
            cglib::bbox2<float> bounds;
            std::array<cglib::vec2<float>, 4> envelope;
            std::shared_ptr<TileLabel> label;

            Record() = default;
            explicit Record(const cglib::bbox2<float>& bounds, const std::array<cglib::vec2<float>, 4>& envelope, std::shared_ptr<TileLabel> label) : bounds(bounds), envelope(envelope), label(std::move(label)) { }
        };

        void clearGrid();
        bool testOverlap(const std::shared_ptr<TileLabel>& label);

        static int getGridIndex(float x);
        static cglib::mat4x4<double> calculateLocalViewMatrix(const cglib::mat4x4<double>& cameraMatrix);

        cglib::mat4x4<float> _mvpMatrix;
        ViewState _viewState;
        float _resolution = 0;
        std::vector<Record> _recordGrid[GRID_RESOLUTION][GRID_RESOLUTION];

        const float _scale;
        const std::shared_ptr<std::mutex> _mutex;
    };
} }

#endif
