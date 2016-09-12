/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTOREDITEVENTLISTENER_H_
#define _CARTO_VECTOREDITEVENTLISTENER_H_

#ifdef _CARTO_EDITABLE_SUPPORT

#include "core/ScreenPos.h"
#include "ui/VectorElementDragInfo.h"

#include <memory>

namespace carto {
    class Geometry;
    class PointStyle;
    class VectorElement;
    
    namespace VectorElementDragResult {
        /**
         * The intended result of the drag operation.
         */
        enum VectorElementDragResult {
            /**
             * Dragging should be ignored. The input event will be passed on to other handlers.
             */
            VECTOR_ELEMENT_DRAG_RESULT_IGNORE,
            /**
             * Dragging should be ignored. The input event is not passed on to other handlers.
             */
            VECTOR_ELEMENT_DRAG_RESULT_STOP,
            /**
             * The underlying vector element (or vertex) should be modified.
             */
            VECTOR_ELEMENT_DRAG_RESULT_MODIFY,
            /**
             * The underlying vector element or vertex should be deleted.
             */
            VECTOR_ELEMENT_DRAG_RESULT_DELETE
        };
    }
    
    namespace VectorElementDragPointStyle {
        /**
         * Different types of styles for control points created for selected vector elements.
         */
        enum VectorElementDragPointStyle {
            /**
             * Normal control point. Corresponds to vertex.
             */
            VECTOR_ELEMENT_DRAG_POINT_STYLE_NORMAL,
            /**
             * Virtual control point (midpoint between actual vertices). Used for lines and polygons.
             */
            VECTOR_ELEMENT_DRAG_POINT_STYLE_VIRTUAL,
            /**
             * Selected control point.
             */
            VECTOR_ELEMENT_DRAG_POINT_STYLE_SELECTED
        };
    }

    /**
     * Listener interface for catching various events
     * during map editing interactions. 
     */
    class VectorEditEventListener {
    public:
        virtual ~VectorEditEventListener() { }

        /**
         * Called when element is being selected. Can be used to disallow element selection.
         * @param element The element being selected
         * @return true if the element is selectable, false otherwise
         */
        virtual bool onElementSelect(const std::shared_ptr<VectorElement>& element) { return true; }
        /**
         * Called when element is unselected.
         * @param element The element which was unselected
         */
        virtual void onElementDeselected(const std::shared_ptr<VectorElement>& element) { }

        /**
         * Called when element is being modified.
         * It the responsibility of the implementation of this method to update actual vector element geometry.
         * The geometry may be modified before updating the vector element, but topology of the element MUST not change!
         * @param element The element that is being edited
         * @param geometry The modified geometry for the element
         */
        virtual void onElementModify(const std::shared_ptr<VectorElement>& element, const std::shared_ptr<Geometry>& geometry) = 0;
        
        /**
         * Called when element should be deleted.
         * @param element The element that needs to be deleted
         */
        virtual void onElementDelete(const std::shared_ptr<VectorElement>& element) = 0;

        /**
         * Called before element or vertex is dragged.
         * @param dragInfo The information about the element or vertex.
         * @result The intended result of dragging.
         */
        virtual VectorElementDragResult::VectorElementDragResult onDragStart(const std::shared_ptr<VectorElementDragInfo>& dragInfo) { return VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY; }

        /**
         * Called when element or vertex is being dragged to specific location.
         * @param dragInfo The information about the element or vertex.
         * @result The intended result of dragging.
         */
        virtual VectorElementDragResult::VectorElementDragResult onDragMove(const std::shared_ptr<VectorElementDragInfo>& dragInfo) { return VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY; }

        /**
         * Called when element or vertex dragging is finished.
         * @param dragInfo The information about the element or vertex.
         * @result The intended result of dragging.
         */
        virtual VectorElementDragResult::VectorElementDragResult onDragEnd(const std::shared_ptr<VectorElementDragInfo>& dragInfo) { return VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY; }

        /**
         * Called when drag point style is needed.
         * @param element The vector element being dragged
         * @param dragPointStyle Drag point style.
         * @return The point style to use for the drag point.
         */
        virtual std::shared_ptr<PointStyle> onSelectDragPointStyle(const std::shared_ptr<VectorElement>& element, VectorElementDragPointStyle::VectorElementDragPointStyle dragPointStyle) = 0;
    };
        
}

#endif

#endif
