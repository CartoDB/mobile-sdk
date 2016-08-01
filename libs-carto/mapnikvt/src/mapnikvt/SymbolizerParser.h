/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_SYMBOLIZERPARSER_H_
#define _CARTO_MAPNIKVT_SYMBOLIZERPARSER_H_

#include <memory>
#include <string>

#include <pugixml.hpp>

namespace carto { namespace mvt {
    class Map;
    class Logger;
    class Symbolizer;
    
    class SymbolizerParser {
    public:
        explicit SymbolizerParser(std::shared_ptr<Logger> logger) : _logger(std::move(logger)) { }
        virtual ~SymbolizerParser() = default;

        virtual std::shared_ptr<Symbolizer> parseSymbolizer(const pugi::xml_node& node, const std::shared_ptr<Map>& map) const;

    protected:
        std::shared_ptr<Symbolizer> createSymbolizer(const pugi::xml_node& node, const std::shared_ptr<Map>& map) const;
            
        const std::shared_ptr<Logger> _logger;
    };
} }

#endif
