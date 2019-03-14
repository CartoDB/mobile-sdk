/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELSTYLEBUILDER_H_
#define _CARTO_NMLMODELSTYLEBUILDER_H_

#include "styles/StyleBuilder.h"

#include <memory>
#include <mutex>

namespace carto {
    class BinaryData;
    class NMLModelStyle;
    
    /**
     * A builder class for NMLModelStyle.
     */
    class NMLModelStyleBuilder : public StyleBuilder {
    public:
        /**
         * Constructs a NMLModelStyleBuilder object with all parameters set to defaults.
         */
        NMLModelStyleBuilder();
        virtual ~NMLModelStyleBuilder();
    
        /**
         * Returns the model asset of the object.
         * @return The model asset of the object.
         */
        std::shared_ptr<BinaryData> getModelAsset() const;
        /**
         * Sets the model asset that will be used for drawing the object. By default a sphere asset is used.
         * @param modelAsset The new model assets for the object.
         */
        void setModelAsset(const std::shared_ptr<BinaryData>& modelAsset);
        
        /**
         * Builds a new instance of the NMLModelStyle object using previously set parameters.
         * @return A new NMLModelStyle object.
         */
        std::shared_ptr<NMLModelStyle> buildStyle() const;
    
    protected:
        static std::shared_ptr<BinaryData> GetDefaultModelAsset();

        static std::shared_ptr<BinaryData> _DefaultNMLModel;
        static std::mutex _DefaultNMLModelMutex;
    
        std::shared_ptr<BinaryData> _modelAsset;
    };
    
}

#endif
