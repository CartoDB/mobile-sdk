/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_RAYINTERSECTEDELEMENT_H_
#define _CARTO_RAYINTERSECTEDELEMENT_H_

#include "core/MapPos.h"

#include <memory>

#include <boost/any.hpp>

namespace carto {
    class Layer;

    class RayIntersectedElement {
    public:
        template <typename T>
        RayIntersectedElement(const std::shared_ptr<T>& element, const std::shared_ptr<Layer>& layer, const MapPos& hitPos, const MapPos& elementPos, int order, bool is3D = false) :
            _element(element),
            _layer(layer),
            _hitPos(hitPos),
            _elementPos(elementPos),
            _order(order),
            _is3D(is3D)
        {
        }

        virtual ~RayIntersectedElement();

        template <typename T>
        std::shared_ptr<T> getElement() const {
            if (auto element = boost::any_cast<std::shared_ptr<T> >(&_element)) {
                return *element;
            }
            return std::shared_ptr<T>();
        }
    
        const std::shared_ptr<Layer>& getLayer() const;

        const MapPos& getHitPos() const;

        const MapPos& getElementPos() const;

        int getOrder() const;

        bool is3D() const;

        double getDistance(const MapPos& origin) const;
    
    private:
        boost::any _element;
        std::shared_ptr<Layer> _layer;
        MapPos _hitPos;
        MapPos _elementPos;
        int _order;
        bool _is3D;
    };
    
}

#endif
