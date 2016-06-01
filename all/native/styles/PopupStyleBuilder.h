/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POPUPSTYLEBUILDER_H_
#define _CARTO_POPUPSTYLEBUILDER_H_

#include "styles/BillboardStyleBuilder.h"

#include <memory>

namespace carto {
    class PopupStyle;
    
    /**
     * A builder class for PopupStyle.
     */
    class PopupStyleBuilder : public BillboardStyleBuilder {
    public:
        /**
         * Constructs a PopupStyleBuilder object with all parameters set to defaults.
         */
        PopupStyleBuilder();
        virtual ~PopupStyleBuilder();
    
        /**
         * Builds a new instance of the PopupStyle object using previously set parameters.
         * @return A new PopupStyle object.
         */
        std::shared_ptr<PopupStyle> buildStyle() const;
    };
    
}

#endif
