/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREEEVENTLISTENER_H_
#define _CARTO_NMLMODELLODTREEEVENTLISTENER_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include <memory>

namespace carto {
    class NMLModelLODTreeClickInfo;
    
    /**
     * Listener for NMLModelLODTree events like clicks etc.
     */
    class NMLModelLODTreeEventListener {
    public:
        virtual ~NMLModelLODTreeEventListener() { }
    
        /**
         * Listener method that gets called when a click is performed on a NML model element.
         * This method will NOT be called from the main thread.
         * @param clickInfo A container that provides information about the click.
         * @return True if the click is handled and subsequent elements should not be handled. False if the next element should be called.
         */
        virtual bool onNMLModelLODTreeClicked(const std::shared_ptr<NMLModelLODTreeClickInfo>& clickInfo) { return true; }
    };
    
}

#endif

#endif
