/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELSTYLE_H_
#define _CARTO_NMLMODELSTYLE_H_

#include "styles/Style.h"

#include <memory>

namespace carto {
    class BinaryData;
    namespace nml {
        class Model;
    }
    
    /**
     * A style for NML models. Contains attributes for configuring how the model is drawn on the screen.
     */
    class NMLModelStyle : public Style {
    public:
        /**
         * Constructs a NMLModelStyle object from various parameters. Instantiating the object directly is
         * not recommended, NMLModelStyleBuilder should be used instead.
         * @param color The color for the object.
         * @param modelAsset The modelAsset for the onbject.
         */
        NMLModelStyle(const Color& color, const std::shared_ptr<BinaryData>& modelAsset);
        virtual ~NMLModelStyle();

        /**
         * Returns the source model used for this style.
         * @return The source model used for this style.
         */
        const std::shared_ptr<nml::Model>& getSourceModel() const;
    
    protected:
        std::shared_ptr<nml::Model> _sourceModel;
    };
    
}

#endif
