/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORELEMENT_H_
#define _CARTO_VECTORELEMENT_H_

#include "core/MapBounds.h"
#include "core/Variant.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace carto {
    class Geometry;
    class Layer;
    class VectorDataSource;
    
    /**
     * A base class for all vector elements (points, lines, texts, models, etc).
     */
    class VectorElement : public std::enable_shared_from_this<VectorElement> {
    
    public:
        virtual ~VectorElement();
        
        /**
         * Returns the bounds of this vector element.
         * @return The bounds of this vector element.
         */
        virtual MapBounds getBounds() const;
    
        /**
         * Returns the geometry object that defines the location of this vector element.
         * @return The geometry object of this vector element.
         */
        std::shared_ptr<Geometry> getGeometry() const;
    
        /** 
         * Returns the internal id of this vector element.
         * @return The internal id of this vector element.
         */
        long long getId() const;
        /**
         * Sets the internal id for this vector element. The id is used for internal caching, and
         * should not be changed by the user. User id can added to meta data as a key-value pair.
         * @param id The new internal id for this vector element.
         */
        void setId(long long id);
    
        /**
         * Returns a modifiable meta data map. Users may add their data as key-value pairs.
         * @return The modifiable meta data map of this vector element.
         */
        std::map<std::string, Variant> getMetaData() const;
        /**
         * Sets a new meta data map for the vector element. Old meta data values will be lost.
         * @param metaData The new meta data map for this vector element.
         */
        void setMetaData(const std::map<std::string, Variant>& metaData);
        
        /** 
         * Returns a meta data element corresponding to the key. If no value is found null variant is returned.
         * @param key The key to use.
         * @return The value corresponding to the key from the meta data map. May be null.
         */
        Variant getMetaDataElement(const std::string& key) const;
        /**
         * Adds a new key-value pair to the meta data map. If the key already exists in the map,
         * it's value will be replaced by the new value.
         * @param key The new key.
         * @param element The new value.
         */
        void setMetaDataElement(const std::string& key, const Variant& element);
    
        /**
         * Returns the state of the visibility flag of this vector element.
         * @return True if this vector element is visible.
         */
        bool isVisible() const;
        /**
         * Sets the state of the visibility flag for this vector element. If set to false the element will not be
         * drawn.
         * @param visible The new state of the visibility flag for the vector element.
         */
        void setVisible(bool visible);
    
        /**
         * Notifies this vector element's datasource about the change in this element.
         * The data source may then notify the layer to update the view.
         */
        void notifyElementChanged();
    
    protected:
        friend class VectorDataSource;
        
        explicit VectorElement(const std::shared_ptr<Geometry>& geometry);
        
        void attachToDataSource(const std::weak_ptr<VectorDataSource>& dataSource);
        void detachFromDataSource();
    
        std::weak_ptr<VectorDataSource> _dataSource;
    
        std::shared_ptr<Geometry> _geometry;
    
        mutable std::recursive_mutex _mutex;

    private:
        long long _id;
    
        std::map<std::string, Variant> _metaData;
    
        bool _visible;
    };
    
}

#endif
