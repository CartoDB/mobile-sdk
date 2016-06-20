/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_UTFGRIDEVENTLISTENER_H_
#define _CARTO_UTFGRIDEVENTLISTENER_H_

#include <memory>

namespace carto {
    class UTFGridClickInfo;
    
    /**
     * Listener for vector element events like clicks etc.
     */
    class UTFGridEventListener {
    public:
        virtual ~UTFGridEventListener() { }
    
        /**
         * Listener method that gets called when a click is performed on an UTF grid element.
         * If the method returns true, all subsequent layers are ignored and click event is not passed down.
         * This method will NOT be called from the main thread.
         * @param clickInfo A container that provides information about the click.
         * @return True if the click is handled and subsequent layers do not need to be notified.
         */
        virtual bool onUTFGridClicked(const std::shared_ptr<UTFGridClickInfo>& clickInfo) { return true; }
    };
    
}

#endif
