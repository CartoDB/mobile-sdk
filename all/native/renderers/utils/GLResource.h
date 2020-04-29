/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GLRESOURCE_H_
#define _CARTO_GLRESOURCE_H_

#include "renderers/utils/GLContext.h"

#include <memory>
#include <vector>

namespace carto {
    class GLResourceManager;
    
    class GLResource {
    public:
        virtual ~GLResource();
        
        bool isValid() const;
        
    protected:
        friend class GLResourceManager;

        GLResource(const std::weak_ptr<GLResourceManager>& manager);

        virtual void create() const = 0;
        virtual void destroy() const = 0;

        const std::weak_ptr<GLResourceManager> _manager;
    };
    
}

#endif
