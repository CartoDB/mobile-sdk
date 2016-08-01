/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_TORQUECARTOCSSMAPNIKTRANSLATOR_H_
#define _CARTO_CARTOCSS_TORQUECARTOCSSMAPNIKTRANSLATOR_H_

#include "CartoCSSMapnikTranslator.h"

namespace carto { namespace css {
    class TorqueCartoCSSMapnikTranslator : public CartoCSSMapnikTranslator {
    public:
        explicit TorqueCartoCSSMapnikTranslator(std::shared_ptr<mvt::Logger> logger) : CartoCSSMapnikTranslator(std::move(logger)) { }

        virtual std::shared_ptr<mvt::Symbolizer> buildSymbolizer(const std::string& symbolizerType, const std::list<CartoCSSCompiler::Property>& properties, const std::shared_ptr<mvt::Map>& map, int zoom) const override;

    protected:
        virtual std::string getPropertySymbolizerId(const std::string& propertyName) const override;

    private:
        static const std::map<std::string, std::string> _symbolizerPropertyMap;
    };
} }

#endif
