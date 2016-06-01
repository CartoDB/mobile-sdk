/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_GEOMETRYSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_GEOMETRYSYMBOLIZER_H_

#include "Symbolizer.h"

#include <boost/optional.hpp>

namespace carto {
    namespace mvt {
        class GeometrySymbolizer : public Symbolizer {
        protected:
            explicit GeometrySymbolizer(std::shared_ptr<Logger> logger) : Symbolizer(std::move(logger)) { }
            
            virtual void bindParameter(const std::string& name, const std::string& value) override;
            
            boost::optional<cglib::mat3x3<float>> _geometryTransform;
            std::string _compOp = "src-over";
        };
    }
}

#endif
