/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORELEMENTEVENTLISTENER_H_
#define _CARTO_VECTORELEMENTEVENTLISTENER_H_

#include <memory>

namespace carto {
    class VectorElementClickInfo;
    
    /**
     * Listener for vector element events like clicks etc.
     */
    class VectorElementEventListener {
    public:
        virtual ~VectorElementEventListener() { }
    
        /**
         * Listener method that gets called when a click is performed on a vector element.
         * If there are multiple vector elements that are located at the click position, then the
         * results will be sorted by their distance to the camera. The closest element will be called first.
         * If the method returns true, all subsequent elements are ignored. Otherwise the method is called
         * on the next element and so on.
         * This method will NOT be called from the main thread.
         * @param clickInfo A container that provides information about the click.
         * @return True if the click is handled and subsequent elements should not be handled. False if the next element should be called.
         */
        virtual bool onVectorElementClicked(const std::shared_ptr<VectorElementClickInfo>& clickInfo) { return true; }
    };
    
}

#endif
