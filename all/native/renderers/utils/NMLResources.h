/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLRESOURCES_H_
#define _CARTO_NMLRESOURCES_H_

#include "renderers/utils/GLResource.h"

#include <memory>

#include <nml/GLResourceManager.h>

namespace carto {
    
    class NMLResources : public GLResource {
    public:
        virtual ~NMLResources();

        std::shared_ptr<nml::GLResourceManager> getResourceManager() const;

    protected:
        friend GLResourceManager;

        NMLResources(const std::weak_ptr<GLResourceManager>& manager);

        virtual void create();
        virtual void destroy();

    private:
        std::shared_ptr<nml::GLResourceManager> _resourceManager;
    };

}

#endif
