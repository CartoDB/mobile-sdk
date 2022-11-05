/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_RAYINTERSECTEDELEMENTCOMPARATOR_H_
#define _CARTO_RAYINTERSECTEDELEMENTCOMPARATOR_H_

#include "graphics/ViewState.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "vectorelements/Billboard.h"

namespace carto {

    class RayIntersectedElementComparator {
    public:
        explicit RayIntersectedElementComparator(const ViewState& viewState) :
            _viewState(viewState)
        {
        }

        bool operator() (const RayIntersectedElement& element1, const RayIntersectedElement& element2) const {
            // If either element is billboard, use special billboard ordering rules
            std::shared_ptr<Billboard> billboard1 = std::dynamic_pointer_cast<Billboard>(element1.getElement<VectorElement>());
            std::shared_ptr<Billboard> billboard2 = std::dynamic_pointer_cast<Billboard>(element2.getElement<VectorElement>());
            if ((bool)billboard1 != (bool)billboard2) {
                return (bool)billboard1 < (bool)billboard2;
            }
            if (billboard1) {
                std::shared_ptr<BillboardDrawData> drawData1 = billboard1->getDrawData();
                std::shared_ptr<BillboardDrawData> drawData2 = billboard2->getDrawData();
                if (drawData1 && drawData2) {
                    return drawData1->isBefore(*drawData2);
                }
            }

            // If either element is 3D, order 3D on top of 2D and use distance from camera
            if (element1.is3DElement() != element2.is3DElement()) {
                return element1.is3DElement() < element2.is3DElement();
            }
            if (element1.is3DElement()) {
                double deltaDistance = element1.getDistance(_viewState.getCameraPos()) - element2.getDistance(_viewState.getCameraPos());
                if (deltaDistance != 0) {
                    return deltaDistance > 0;
                }
            }

            // Do not reorder
            return false;
        }
    
    private:
        const ViewState& _viewState;
    };
    
}

#endif
