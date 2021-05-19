/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELSTYLEBUILDER_H_
#define _CARTO_NMLMODELSTYLEBUILDER_H_

#include "styles/BillboardStyleBuilder.h"

#include <memory>
#include <mutex>

namespace carto {
    class BinaryData;
    class NMLModelStyle;
    
    /**
     * A builder class for NMLModelStyle.
     */
    class NMLModelStyleBuilder : public BillboardStyleBuilder {
    public:
        /**
         * Constructs a NMLModelStyleBuilder object with all parameters set to defaults.
         */
        NMLModelStyleBuilder();
        virtual ~NMLModelStyleBuilder();
    
        /**
         * Returns the orientation mode of the model.
         * @return The orientation mode of the model.
         */
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
        /**
         * Sets the orientation mode for the model. The default is BillboardOrientation::BILLBOARD_ORIENTATION_GROUND.
         * @param orientationMode The new orientation mode for the model.
         */
        void setOrientationMode(BillboardOrientation::BillboardOrientation orientationMode);
        
        /**
         * Returns the scaling mode of the model.
         * @return The scaling mode of the model.
         */
        BillboardScaling::BillboardScaling getScalingMode() const;
        /**
         * Sets the scaling mode for the model. The default is BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE.
         * @param scalingMode The new scaling mode for the model.
         */
        void setScalingMode(BillboardScaling::BillboardScaling scalingMode);

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
    
        BillboardOrientation::BillboardOrientation _orientationMode;
        
        BillboardScaling::BillboardScaling _scalingMode;

        std::shared_ptr<BinaryData> _modelAsset;
    };
    
}

#endif
