/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELSTYLE_H_
#define _CARTO_NMLMODELSTYLE_H_

#include "styles/BillboardStyle.h"

#include <memory>

namespace carto {
    class BinaryData;
    namespace nml {
        class Model;
    }
    
    /**
     * A style for NML models. Contains attributes for configuring how the model is drawn on the screen.
     */
    class NMLModelStyle : public BillboardStyle {
    public:
        /**
         * Constructs a NMLModelStyle object from various parameters. Instantiating the object directly is
         * not recommended, NMLModelStyleBuilder should be used instead.
         * @param color The color for the popup.
         * @param attachAnchorPointX The horizontal attaching anchor point for the popup.
         * @param attachAnchorPointY The vertical attaching anchor point for the popup.
         * @param causesOverlap The causes overlap flag for the popup.
         * @param hideIfOverlapped The hide if overlapped flag for the popup.
         * @param horizontalOffset The horizontal offset for the popup.
         * @param verticalOffset The vertical offset for the popup.
         * @param placementPriority The placement priority for the popup.
         * @param scaleWithDPI The scale with DPI flag for the popup.
         * @param animStyle The animation style for the popup.
         * @param modelAsset The modelAsset for the onbject.
         */
        NMLModelStyle(const Color& color,
                      float attachAnchorPointX,
                      float attachAnchorPointY,
                      bool causesOverlap,
                      bool hideIfOverlapped,
                      float horizontalOffset,
                      float verticalOffset,
                      int placementPriority,
                      bool scaleWithDPI,
                      const std::shared_ptr<AnimationStyle>& animStyle,
                      BillboardOrientation::BillboardOrientation orientationMode,
                      BillboardScaling::BillboardScaling scalingMode,
                      const std::shared_ptr<BinaryData>& modelAsset);
        virtual ~NMLModelStyle();

        /**
         * Returns the orientation mode of the model.
         * @return The orientation mode of the model.
         */
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
        
        /**
         * Returns the scaling mode of the model.
         * @return The scaling mode of the model.
         */
        BillboardScaling::BillboardScaling getScalingMode() const;

        /**
         * Returns the model asset of the object.
         * @return The model asset of the object.
         */
        std::shared_ptr<BinaryData> getModelAsset() const;

        /**
         * Returns the source model used for this style.
         * @return The source model used for this style.
         */
        std::shared_ptr<nml::Model> getSourceModel() const;
    
    protected:
        BillboardOrientation::BillboardOrientation _orientationMode;
        
        BillboardScaling::BillboardScaling _scalingMode;

        std::shared_ptr<BinaryData> _modelAsset;

        std::shared_ptr<nml::Model> _sourceModel;
    };
    
}

#endif
