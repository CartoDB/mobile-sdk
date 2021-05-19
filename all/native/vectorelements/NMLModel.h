/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODEL_H_
#define _CARTO_NMLMODEL_H_

#include "Billboard.h"
#include "core/MapPos.h"
#include "core/MapVec.h"

#include <memory>
#include <vector>

#include <cglib/mat.h>

namespace carto {
    class NMLModelDrawData;
    class NMLModelStyle;

    namespace nml {
        class Model;
    }

    /**
     * A 3D model that can be displayed on the map.
     *
     * NML models can be created from Collada files directly and placed anywhere on the map or converted from KMZ files.
     * NML models are optimized for fast loading and rendering.
     */
    class NMLModel : public Billboard {
    public:
        /**
         * Constructs a NMLModel object with the specified style and attaches it to a billboard element.
         * @param baseBillboard The billboard this model will be attached to.
         * @param style The style for this model.
         */
        NMLModel(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<NMLModelStyle>& style);
        /**
         * Constructs a NMLModel object from a geometry object and a source model.
         * @param geometry The geometry object that defines the location of this model.
         * @param style The style for this model.
         */
        NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<NMLModelStyle>& style);
        /**
         * Constructs a NMLModel object from a map position and a source model.
         * @param pos The map position that defines the location of this model.
         * @param style The style for this model.
         */
        NMLModel(const MapPos& pos, const std::shared_ptr<NMLModelStyle>& style);
        virtual ~NMLModel();

        /**
         * Returns the rotation angle of this model. This is deprecated. Use getRotation instead.
         * @return The rotation angle of this model in degrees.
         * @deprecated
         */
        float getRotationAngle() const;
        /**
         * Sets the rotation angle of this model. This is deprecated. Use setRotation instead.
         * @param angle The new rotation angle in degrees.
         * @deprecated
         */
        void setRotationAngle(float angle);
        /**
         * Returns the rotation axis of this model. If rotation angle is 0, then the axis is irrelevant.
         * @return The rotation axis vector.
         */
        MapVec getRotationAxis() const;
        /**
         * Sets the rotation axis of this model.
         * @param axis The new axis of rotation.
         */
        void setRotationAxis(const MapVec& axis);
        /**
         * Sets the rotation of this model using an axis and an angle.
         * @param axis The axis of rotation.
         * @param angle The rotation angle in degrees.
         */
        void setRotation(const MapVec& axis, float angle);

        /**
         * Returns the scale of this model.
         * @return model The relative scale.
         */
        float getScale() const;
        /**
         * Sets the scale of this model. The default is 1.
         * @param scale The relative scale of this model.
         */
        void setScale(float scale);
        
        /**
         * Returns the style of this object.
         * @return The style that defines what this object looks like.
         */
        std::shared_ptr<NMLModelStyle> getStyle() const;
        /**
         * Sets a style for this object.
         * @param style The new style that defines what this object looks like.
         */
        void setStyle(const std::shared_ptr<NMLModelStyle>& style);
        
    protected:
        friend class BillboardRenderer;
        friend class VectorLayer;
        
    private:
        MapVec _rotationAxis;
        
        float _scale;
        
        std::shared_ptr<NMLModelStyle> _style;
    };

}

#endif
