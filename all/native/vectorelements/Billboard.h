/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARD_H_
#define _CARTO_BILLBOARD_H_

#include "vectorelements/VectorElement.h"

namespace carto {
    class BillboardDrawData;
    class BillboardStyle;
    class MapPos;
    class VectorLayer;
    
    /**
     * A base class for billboard elements that can be displayed on the map.
     * Billboards can either be given a concrete position on the map or be attached to
     * other billboards.
     */
    class Billboard : public VectorElement {
    public:
        virtual ~Billboard();
    
        /**
         * Returns the base billboard this billboard is attached to.
         * @return The base billboard this billboard is attached to. Null if not attached to a billboard.
         */
        std::shared_ptr<Billboard> getBaseBillboard() const;
        /**
         * Attaches this billboard to another billboard, so it will always be drawn relative to the base billboard.
         * If this billboard has a geometry object assigned to it, it will first be set to null.
         * @param baseBillboard The billboard this billboard will be attached to.
         */
        void setBaseBillboard(const std::shared_ptr<Billboard>& baseBillboard);
        
        /**
         * Returns the bounds of this billboard or the base billboard, if there is one.
         * @return The bounds of this billboard.
         */
        MapBounds getBounds() const;
    
        /**
         * Returns the location of the root billboard. If this billboard has a location,
         * this method is equavalent to the Billboard::getGeometry method. If this billboard is attached 
         * to another billboard, the hierarchy is traveled recursively and the location of the root 
         * billboard is returned. 
         * @return The geometry object that defines the location of the root billboard. Null if there's no root billboard.
         */
        std::shared_ptr<Geometry> getRootGeometry() const;
        /**
         * Returns the geometry object that defines the location of this billboard.
         * @return The geometry object of this billboard.
         */
        std::shared_ptr<Geometry> getGeometry() const;
        /**
         * Sets the location for this billboard. If this billboard is attached
         * to another billboard, it will first be detached.
         * @param geometry The new geometry object that defines the location of this billboard.
         */
        void setGeometry(const std::shared_ptr<Geometry>& geometry);
        /**
         * Sets the location for this billboard. If this billboard is attached
         * to another billboard, it will first be detached.
         * @param pos The new map position that defines the location of this billboard.
         */
        void setPos(const MapPos& pos);
    
        /**
         * Returns the rotation angle of this billboard.
         * @return The rotation angle of this billboard in degrees.
         */
        float getRotation() const;
        /**
         * Sets the rotation angle of this billboard. The rotation angle is ignored if orientation mode is
         * set to BillboardOrientation::FACE_CAMERA_BILLBOARD. If the orientation mode is set to 
         * BillboardOrientation::FACE_CAMERA_GROUND then the rotation angle is added to the calculated billboard 
         * angle. If the orientation mode is set to BillboardOrientation::GROUND then the rotation means absolute
         * rotation, where 0 is points to the north.
         * @param rotation The new rotation angle of this billboard in degrees.
         */
        void setRotation(float rotation);
    
        std::shared_ptr<BillboardDrawData> getDrawData() const;
        void setDrawData(const std::shared_ptr<BillboardDrawData>& drawData);

    protected:
        friend class BillboardPlacementWorker;
        friend class BillboardRenderer;
        friend class BillboardSorter;
        friend class MapRenderer;
        friend class VectorLayer;
        
        Billboard(const std::shared_ptr<Billboard>& baseBillboard);
        Billboard(const std::shared_ptr<Geometry>& geometry);
        Billboard(const MapPos& pos);
        
    private:
        std::shared_ptr<Billboard> _baseBillboard;
        
        std::shared_ptr<BillboardDrawData> _drawData;
    
        float _rotation;
    };
    
}

#endif
