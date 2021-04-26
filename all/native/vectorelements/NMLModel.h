/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODEL_H_
#define _CARTO_NMLMODEL_H_

#include "VectorElement.h"
#include "core/MapPos.h"
#include "core/MapVec.h"

#include <memory>
#include <vector>

#include <cglib/mat.h>

namespace carto {
    class BinaryData;
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
    class NMLModel : public VectorElement {
    public:
        /**
         * Constructs a NMLModel object from a geometry object and a source model.
         * @param geometry The geometry object that defines the location of this model.
         * @param style The style to used for this model.
         */
        NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<NMLModelStyle>& style);
        /**
         * Constructs a NMLModel object from a map position and a source model.
         * @param pos The map position that defines the location of this model.
         * @param style The style to used for this model.
         */
        NMLModel(const MapPos& pos, const std::shared_ptr<NMLModelStyle>& style);
        /**
         * Constructs a NMLModel object from a geometry object and serialized model data.
         * @param geometry The geometry object that defines the location of this model.
         * @param sourceModelData Serialized data for 3D model.
         * @deprecated
         */
        NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<BinaryData>& sourceModelData);
        /**
         * Constructs a NMLModel object from a map position and serialized model data.
         * @param pos The map position that defines the location of this model.
         * @param sourceModelData Serialized data for 3D model.
         * @deprecated
         */
        NMLModel(const MapPos& pos, const std::shared_ptr<BinaryData>& sourceModelData);
        virtual ~NMLModel();

        /**
         * Sets the location for this model.
         * @param geometry The new geometry object that defines the location of this model.
         */
        void setGeometry(const std::shared_ptr<Geometry>& geometry);

        /**
         * Sets the location for this model.
         * @param pos The new map position that defines the location of this model.
         */
        void setPos(const MapPos& pos);
        
        /**
         * Returns the rotation axis of this model. If rotation angle is 0, then the axis is irrelevant.
         * @return The rotation axis vector.
         */
        MapVec getRotationAxis() const;
        /**
         * Returns the rotation angle of this model.
         * @return The rotation angle of this model in degrees.
         */
        float getRotationAngle() const;
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
        
        std::shared_ptr<NMLModelDrawData> getDrawData() const;
        void setDrawData(const std::shared_ptr<NMLModelDrawData>& drawData);

    protected:
        friend class NMLModelRenderer;
        friend class VectorLayer;
        
    private:
        std::shared_ptr<NMLModelDrawData> _drawData;
        
        MapVec _rotationAxis;
        float _rotationAngle;
        
        float _scale;
        
        std::shared_ptr<NMLModelStyle> _style;
    };

}

#endif
