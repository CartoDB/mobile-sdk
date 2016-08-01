/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_MAPGENERATOR_H_
#define _CARTO_MAPNIKVT_MAPGENERATOR_H_

#include "Value.h"

#include <memory>
#include <string>

#include <pugixml.hpp>

namespace carto { namespace mvt {
    class Map;
    class Logger;
    class SymbolizerGenerator;

    class MapGenerator {
    public:
        explicit MapGenerator(std::shared_ptr<SymbolizerGenerator> generator, std::shared_ptr<Logger> logger) : _symbolizerGenerator(std::move(generator)), _logger(std::move(logger)) { }
        virtual ~MapGenerator() = default;

        virtual std::shared_ptr<pugi::xml_document> generateMap(const Map& map) const;

    protected:
        std::string generateTypeString(const Value& value) const;

        const std::shared_ptr<SymbolizerGenerator> _symbolizerGenerator;
        const std::shared_ptr<Logger> _logger;

    private:
        struct TypeExtractor;
    };
} }

#endif
