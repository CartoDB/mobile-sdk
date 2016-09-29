/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_RESULT_H_
#define _CARTO_ROUTING_RESULT_H_

#include "Base.h"
#include "Instruction.h"

#include <vector>
#include <numeric>

namespace carto { namespace routing {
    class Result final {
    public:
        enum class Status {
            FAILED,
            SUCCESS
        };

        Result() = default;
        explicit Result(std::vector<Instruction> instructions, std::vector<WGSPos> geometry) : _status(Status::SUCCESS), _instructions(std::move(instructions)), _geometry(std::move(geometry)) { }

        Status getStatus() const {
            return _status;
        }

        const std::vector<Instruction>& getInstructions() const {
            return _instructions;
        }

        const std::vector<WGSPos>& getGeometry() const {
            return _geometry;
        }

        double getTotalDistance() const {
            return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double dist, const Instruction& instruction) {
                return dist + instruction.getDistance();
            });
        }

        double getTotalTime() const {
            return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double time, const Instruction& instruction) {
                return time + instruction.getTime();
            });
        }

    private:
        Status _status = Status::FAILED;
        std::vector<Instruction> _instructions;
        std::vector<WGSPos> _geometry;
    };
} }

#endif
